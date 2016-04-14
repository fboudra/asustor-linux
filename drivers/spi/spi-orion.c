/*
 * Marvell Orion SPI controller driver
 *
 * Author: Shadi Ammouri <shadi@marvell.com>
 * Copyright (C) 2007-2008 Marvell Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/spi/spi.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/clk.h>
#include <asm/unaligned.h>

#define DRIVER_NAME			"orion_spi"

#define ORION_NUM_CHIPSELECTS		4
#define ORION_CHIPSELECTS_OFFS		2
#define ORION_CHIPSELECTS_MASK		(0x3 << ORION_CHIPSELECTS_OFFS)
#define ORION_SPI_WAIT_RDY_MAX_LOOP	2000 /* in usec */

#define ORION_SPI_IF_CTRL_REG		0x00
#define ORION_SPI_IF_CONFIG_REG		0x04
#define ORION_SPI_DATA_OUT_REG		0x08
#define ORION_SPI_DATA_IN_REG		0x0c
#define ORION_SPI_INT_CAUSE_REG		0x10
#define ORION_SPI_TIMING_PARAMS_REG	0x18

#define ORION_SPI_TMISO_SAMPLE_MASK	(0x3 << 6)
#define ORION_SPI_TMISO_SAMPLE_1	(1 << 6)
#define ORION_SPI_TMISO_SAMPLE_2	(2 << 6)

#define ORION_SPI_MODE_CPOL			(1 << 11)
#define ORION_SPI_MODE_CPHA			(1 << 12)
#define ORION_SPI_IF_8_16_BIT_MODE	(1 << 5)
#define ORION_SPI_CLK_PRESCALE_MASK	0x1F
#define ARMADA_SPI_CLK_PRESCALE_MASK	0xDF
#define ORION_SPI_MODE_MASK			(ORION_SPI_MODE_CPOL | \
					 ORION_SPI_MODE_CPHA)

enum orion_spi_type {
	ORION_SPI,
	ARMADA_SPI,
	ARMADA_380_SPI,
};

struct orion_spi_dev {
	enum orion_spi_type	typ;
	unsigned int		min_divisor;
	unsigned int		max_divisor;
	unsigned int		max_frequency;
	u32			prescale_mask;
};

struct orion_spi {
	struct spi_master	*master;
	void __iomem		*base;
	struct clk			*clk;
	struct spi_device	*cur_spi;
	const struct orion_spi_dev *devdata;
};

static inline void __iomem *spi_reg(struct orion_spi *orion_spi, u32 reg)
{
	return orion_spi->base + reg;
}

static inline void
orion_spi_setbits(struct orion_spi *orion_spi, u32 reg, u32 mask)
{
	void __iomem *reg_addr = spi_reg(orion_spi, reg);
	u32 val;

	val = readl(reg_addr);
	val |= mask;
	writel(val, reg_addr);
}

static inline void
orion_spi_clrbits(struct orion_spi *orion_spi, u32 reg, u32 mask)
{
	void __iomem *reg_addr = spi_reg(orion_spi, reg);
	u32 val;

	val = readl(reg_addr);
	val &= ~mask;
	writel(val, reg_addr);
}

static int orion_spi_set_transfer_size(struct orion_spi *orion_spi, int size)
{
	if (size == 16) {
		orion_spi_setbits(orion_spi, ORION_SPI_IF_CONFIG_REG,
				  ORION_SPI_IF_8_16_BIT_MODE);
	} else if (size == 8) {
		orion_spi_clrbits(orion_spi, ORION_SPI_IF_CONFIG_REG,
				  ORION_SPI_IF_8_16_BIT_MODE);
	} else {
		pr_debug("Bad bits per word value %d (only 8 or 16 are "
			 "allowed).\n", size);
		return -EINVAL;
	}

	return 0;
}

static int orion_spi_baudrate_set(struct spi_device *spi, unsigned int speed)
{
	u32 tclk_hz;
	u32 rate;
	u32 prescale;
	u32 reg;
	struct orion_spi *orion_spi;
	const struct orion_spi_dev *devdata;

	orion_spi = spi_master_get_devdata(spi->master);
	devdata = orion_spi->devdata;

	tclk_hz = clk_get_rate(orion_spi->clk);

	if (devdata->typ == ARMADA_SPI || devdata->typ == ARMADA_380_SPI) {
		unsigned int clk, spr, sppr, sppr2, err;
		unsigned int best_spr, best_sppr, best_err;

		best_err = speed;
		best_spr = 0;
		best_sppr = 0;

		/* Iterate over the valid range looking for best fit */
		for (sppr = 0; sppr < 8; sppr++) {
			sppr2 = 0x1 << sppr;

			spr = tclk_hz / sppr2;
			spr = DIV_ROUND_UP(spr, speed);
			if ((spr == 0) || (spr > 15))
				continue;

			clk = tclk_hz / (spr * sppr2);
			err = speed - clk;

			if (err < best_err) {
				best_spr = spr;
				best_sppr = sppr;
				best_err = err;
			}
		}

		if ((best_sppr == 0) && (best_spr == 0))
			return -EINVAL;

		prescale = ((best_sppr & 0x6) << 5) |
			((best_sppr & 0x1) << 4) | best_spr;
	} else {
		/*
		 * the supported rates are: 4,6,8...30
		 * round up as we look for equal or less speed
		 */
		rate = DIV_ROUND_UP(tclk_hz, speed);
		rate = roundup(rate, 2);

		/* check if requested speed is too small */
		if (rate > 30)
			return -EINVAL;

		if (rate < 4)
			rate = 4;

		/* Convert the rate to SPI clock divisor value.	*/
		prescale = 0x10 + rate/2;
	}

	reg = readl(spi_reg(orion_spi, ORION_SPI_IF_CONFIG_REG));
	reg = ((reg & ~devdata->prescale_mask) | prescale);
	writel(reg, spi_reg(orion_spi, ORION_SPI_IF_CONFIG_REG));

	return 0;
}

static void
orion_spi_mode_set(struct spi_device *spi)
{
	u32 reg;
	struct orion_spi *orion_spi;

	orion_spi = spi_master_get_devdata(spi->master);

	reg = readl(spi_reg(orion_spi, ORION_SPI_IF_CONFIG_REG));
	reg &= ~ORION_SPI_MODE_MASK;
	if (spi->mode & SPI_CPOL)
		reg |= ORION_SPI_MODE_CPOL;
	if (spi->mode & SPI_CPHA)
		reg |= ORION_SPI_MODE_CPHA;
	writel(reg, spi_reg(orion_spi, ORION_SPI_IF_CONFIG_REG));
}

static void
orion_spi_50mhz_ac_timing_erratum(struct spi_device *spi, unsigned int speed)
{
	u32 reg;
	struct orion_spi *orion_spi;

	orion_spi = spi_master_get_devdata(spi->master);

	/* Erratum description: (Erratum NO. FE-9144572)
	 * The device SPI interface supports frequencies of up to 50 MHz.
	 * However, due to this erratum, when the device core clock is 250 MHz
	 * and the SPI interfaces is configured for 50MHz SPI clock and CPOL=CPHA=1
	 * there might occur data corruption on reads from the SPI device.
	 * Erratum Workaround:
	 * Work in one of the following configurations:
	 * 1. Set CPOL=CPHA=0 in "SPI Interface Configuration Register".
	 * 2. Set TMISO_SAMPLE value to 0x2 in "SPI Timing Parameters 1 Register"
	 *    before setting the interface.
	 */
	reg = readl(spi_reg(orion_spi, ORION_SPI_TIMING_PARAMS_REG));
	reg &= ~ORION_SPI_TMISO_SAMPLE_MASK;

	if (clk_get_rate(orion_spi->clk) == 250000000 &&
			speed == 50000000 && spi->mode & SPI_CPOL &&
			spi->mode & SPI_CPHA)
		reg |= ORION_SPI_TMISO_SAMPLE_2;
	else
		reg |= ORION_SPI_TMISO_SAMPLE_1; /* This is the default value */

	writel(reg, spi_reg(orion_spi, ORION_SPI_TIMING_PARAMS_REG));
}

/*
 * called only when no transfer is active on the bus
 */
static int
orion_spi_setup_transfer(struct spi_device *spi, struct spi_transfer *t)
{
	struct orion_spi *orion_spi;
	unsigned int speed = spi->max_speed_hz;
	unsigned int bits_per_word = spi->bits_per_word;
	int	rc;

	orion_spi = spi_master_get_devdata(spi->master);

	if ((t != NULL) && t->speed_hz)
		speed = t->speed_hz;

	if ((t != NULL) && t->bits_per_word)
		bits_per_word = t->bits_per_word;

	orion_spi_mode_set(spi);

	if (orion_spi->devdata->typ == ARMADA_380_SPI)
		orion_spi_50mhz_ac_timing_erratum(spi, speed);

	rc = orion_spi_baudrate_set(spi, speed);
	if (rc)
		return rc;

	return orion_spi_set_transfer_size(orion_spi, bits_per_word);
}

static void orion_spi_set_cs(struct orion_spi *orion_spi, int enable)
{
	orion_spi_clrbits(orion_spi, ORION_SPI_IF_CTRL_REG,
					  0x1 | ORION_CHIPSELECTS_MASK);
	if (enable)
		orion_spi_setbits(orion_spi, ORION_SPI_IF_CTRL_REG,
			0x1 | (orion_spi->cur_spi->chip_select << ORION_CHIPSELECTS_OFFS));
}

static inline int orion_spi_wait_till_ready(struct orion_spi *orion_spi)
{
	int i;

	for (i = 0; i < ORION_SPI_WAIT_RDY_MAX_LOOP; i++) {
		if (readl(spi_reg(orion_spi, ORION_SPI_INT_CAUSE_REG)))
			return 1;
		else
			udelay(1);
	}

	return -1;
}

static inline int
orion_spi_write_read_8bit(struct spi_device *spi,
			  const u8 **tx_buf, u8 **rx_buf)
{
	void __iomem *tx_reg, *rx_reg, *int_reg;
	struct orion_spi *orion_spi;
	bool cs_single_byte;

	cs_single_byte = spi->mode & SPI_1BYTE_CS;

	orion_spi = spi_master_get_devdata(spi->master);

	if (cs_single_byte)
		orion_spi_set_cs(orion_spi, 1);

	tx_reg = spi_reg(orion_spi, ORION_SPI_DATA_OUT_REG);
	rx_reg = spi_reg(orion_spi, ORION_SPI_DATA_IN_REG);
	int_reg = spi_reg(orion_spi, ORION_SPI_INT_CAUSE_REG);

	/* clear the interrupt cause register */
	writel(0x0, int_reg);

	if (tx_buf && *tx_buf)
		writel(*(*tx_buf)++, tx_reg);
	else
		writel(0, tx_reg);

	if (orion_spi_wait_till_ready(orion_spi) < 0) {
		if (cs_single_byte) {
			orion_spi_set_cs(orion_spi, 0);
			/* Satisfy some SLIC devices requirements */
			udelay(4);
		}
		dev_err(&spi->dev, "TXS timed out\n");
		return -1;
	}

	if (rx_buf && *rx_buf)
		*(*rx_buf)++ = readl(rx_reg);

	if (cs_single_byte) {
		orion_spi_set_cs(orion_spi, 0);
		/* Satisfy some SLIC devices requirements */
		udelay(4);
	}

	return 1;
}

static inline int
orion_spi_write_read_16bit(struct spi_device *spi,
			   const u16 **tx_buf, u16 **rx_buf)
{
	void __iomem *tx_reg, *rx_reg, *int_reg;
	struct orion_spi *orion_spi;

	orion_spi = spi_master_get_devdata(spi->master);
	tx_reg = spi_reg(orion_spi, ORION_SPI_DATA_OUT_REG);
	rx_reg = spi_reg(orion_spi, ORION_SPI_DATA_IN_REG);
	int_reg = spi_reg(orion_spi, ORION_SPI_INT_CAUSE_REG);

	/* clear the interrupt cause register */
	writel(0x0, int_reg);

	if (tx_buf && *tx_buf)
		writel(__cpu_to_le16(get_unaligned((*tx_buf)++)), tx_reg);
	else
		writel(0, tx_reg);

	if (orion_spi_wait_till_ready(orion_spi) < 0) {
		dev_err(&spi->dev, "TXS timed out\n");
		return -1;
	}

	if (rx_buf && *rx_buf)
		put_unaligned(__le16_to_cpu(readl(rx_reg)), (*rx_buf)++);

	return 1;
}

static unsigned int
orion_spi_write_read(struct spi_device *spi, struct spi_transfer *xfer)
{
	struct orion_spi *orion_spi;
	unsigned int count;
	int word_len;

	orion_spi = spi_master_get_devdata(spi->master);
	word_len = spi->bits_per_word;
	count = xfer->len;

	if (word_len == 8) {
		const u8 *tx = xfer->tx_buf;
		u8 *rx = xfer->rx_buf;

		do {
			if (orion_spi_write_read_8bit(spi, &tx, &rx) < 0)
				goto out;
			count--;
		} while (count);
	} else if (word_len == 16) {
		const u16 *tx = xfer->tx_buf;
		u16 *rx = xfer->rx_buf;

		do {
			if (orion_spi_write_read_16bit(spi, &tx, &rx) < 0)
				goto out;
			count -= 2;
		} while (count);
	}

out:
	return xfer->len - count;
}


static int orion_spi_transfer_one_message(struct spi_master *master,
					   struct spi_message *m)
{
	struct orion_spi *orion_spi = spi_master_get_devdata(master);
	struct spi_device *spi = m->spi;
	struct spi_transfer *t = NULL;
	bool cs_single_byte;
	int par_override = 0;
	int status = 0;
	int cs_active = 0;

	/* Load defaults */
	status = orion_spi_setup_transfer(spi, NULL);

	if (status < 0)
		goto msg_done;

	orion_spi->cur_spi = spi;

	cs_single_byte = spi->mode & SPI_1BYTE_CS;

	list_for_each_entry(t, &m->transfers, transfer_list) {
		/* make sure buffer length is even when working in 16
		 * bit mode*/
		if ((t->bits_per_word == 16) && (t->len & 1)) {
			dev_err(&spi->dev,
				"message rejected : "
				"odd data length %d while in 16 bit mode\n",
				t->len);
			status = -EIO;
			goto msg_done;
		}

		if (par_override || t->speed_hz || t->bits_per_word) {
			par_override = 1;
			status = orion_spi_setup_transfer(spi, t);
			if (status < 0)
				break;
			if (!t->speed_hz && !t->bits_per_word)
				par_override = 0;
		}

		if (!cs_active && !cs_single_byte) {
			orion_spi_set_cs(orion_spi, 1);
			cs_active = 1;
		}

		if (t->len)
			m->actual_length += orion_spi_write_read(spi, t);

		if (t->delay_usecs)
			udelay(t->delay_usecs);

		if (t->cs_change && !cs_single_byte) {
			orion_spi_set_cs(orion_spi, 0);
			cs_active = 0;
		}
	}

msg_done:
	if (cs_active && !cs_single_byte)
		orion_spi_set_cs(orion_spi, 0);

	m->status = status;
	spi_finalize_current_message(master);

	return 0;
}

static int orion_spi_reset(struct orion_spi *orion_spi)
{
	/* Verify that the CS is deasserted */
	orion_spi_set_cs(orion_spi, 0);

	return 0;
}

static const struct orion_spi_dev orion_spi_dev_data = {
	.typ = ORION_SPI,
	.min_divisor = 4,
	.max_divisor = 30,
	.max_frequency = 0,
	.prescale_mask = ORION_SPI_CLK_PRESCALE_MASK,
};

static const struct orion_spi_dev armada_spi_dev_data = {
	.typ = ARMADA_SPI,
	.min_divisor = 4,
	.max_divisor = 1920,
	.max_frequency = 50000000,
	.prescale_mask = ARMADA_SPI_CLK_PRESCALE_MASK,
};

static const struct orion_spi_dev armada_380_spi_dev_data = {
	.typ = ARMADA_380_SPI,
	.min_divisor = 4,
	.max_divisor = 1920,
	.max_frequency = 50000000,
	.prescale_mask = ARMADA_SPI_CLK_PRESCALE_MASK,
};

static const struct of_device_id orion_spi_of_match_table[] = {
	{ .compatible = "marvell,orion-spi", .data = &orion_spi_dev_data, },
	{ .compatible = "marvell,armada-370-spi", .data = &armada_spi_dev_data, },
	{ .compatible = "marvell,armada-380-spi", .data = &armada_380_spi_dev_data, },
	{}
};
MODULE_DEVICE_TABLE(of, orion_spi_of_match_table);

static int orion_spi_probe(struct platform_device *pdev)
{
	const struct of_device_id *of_id;
	const struct orion_spi_dev *devdata;
	struct spi_master *master;
	struct orion_spi *spi;
	struct resource *r;
	unsigned long tclk_hz;
	int status = 0;
	u32 ret;
	unsigned int num_cs;

	master = spi_alloc_master(&pdev->dev, sizeof *spi);
	if (master == NULL) {
		dev_dbg(&pdev->dev, "master allocation failed\n");
		return -ENOMEM;
	}

	if (pdev->id != -1)
		master->bus_num = pdev->id;
	if (pdev->dev.of_node) {
		u32 cell_index;
		if (!of_property_read_u32(pdev->dev.of_node, "cell-index",
					  &cell_index))
			master->bus_num = cell_index;

		ret = of_property_read_u32(pdev->dev.of_node, "num-cs", &num_cs);
		if (ret < 0)
			num_cs = ORION_NUM_CHIPSELECTS;
	}

	master->mode_bits = SPI_CPHA | SPI_CPOL | SPI_1BYTE_CS;
	master->transfer_one_message = orion_spi_transfer_one_message;
	master->num_chipselect = num_cs;

	dev_set_drvdata(&pdev->dev, master);

	spi = spi_master_get_devdata(master);
	spi->master = master;

	of_id = of_match_device(orion_spi_of_match_table, &pdev->dev);
	devdata = of_id->data;
	spi->devdata = devdata;

	spi->clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(spi->clk)) {
		status = PTR_ERR(spi->clk);
		goto out;
	}

	clk_prepare(spi->clk);
	clk_enable(spi->clk);
	tclk_hz = clk_get_rate(spi->clk);
	master->max_speed_hz = DIV_ROUND_UP(tclk_hz, devdata->min_divisor);
	master->min_speed_hz = DIV_ROUND_UP(tclk_hz, devdata->max_divisor);
	if ((devdata->max_frequency != 0) && (master->max_speed_hz > devdata->max_frequency))
		master->max_speed_hz = devdata->max_frequency;

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	spi->base = devm_ioremap_resource(&pdev->dev, r);
	if (IS_ERR(spi->base)) {
		status = PTR_ERR(spi->base);
		goto out_rel_clk;
	}

	if (orion_spi_reset(spi) < 0)
		goto out_rel_clk;

	master->dev.of_node = pdev->dev.of_node;
	status = spi_register_master(master);
	if (status < 0)
		goto out_rel_clk;

	return status;

out_rel_clk:
	clk_disable_unprepare(spi->clk);
out:
	spi_master_put(master);
	return status;
}


static int orion_spi_remove(struct platform_device *pdev)
{
	struct spi_master *master;
	struct orion_spi *spi;

	master = dev_get_drvdata(&pdev->dev);
	spi = spi_master_get_devdata(master);

	clk_disable_unprepare(spi->clk);

	spi_unregister_master(master);

	return 0;
}

MODULE_ALIAS("platform:" DRIVER_NAME);

static struct platform_driver orion_spi_driver = {
	.driver = {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(orion_spi_of_match_table),
	},
	.probe		= orion_spi_probe,
	.remove		= orion_spi_remove,
};

module_platform_driver(orion_spi_driver);

MODULE_DESCRIPTION("Orion SPI driver");
MODULE_AUTHOR("Shadi Ammouri <shadi@marvell.com>");
MODULE_LICENSE("GPL");
