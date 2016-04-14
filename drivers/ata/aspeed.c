#ifdef ASUSTOR_PATCH
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/jiffies.h>

#define MAX_PCI_DEVICE_COUNT            16
#define ASP_PCI_VENDOR_ID               0x1B4B
#define ICH_PCI_VENDOR_ID               0x8086
#define ICH_PCI_DEVICE_ID               0x3A16
#define ICH_PCI_DEVICE_MASK             0xFFF0
#define ICH_GPIO_BASE_REG               0x48
#define ICH_IO_BASE_MASK                0xFFFFFFF0
#define ICH_GPIO_USE_SEL                0x0
#define ICH_GPIO_IO_SEL                 0x4
#define ICH_GPIO_LVL                    0xC
#define ICH_GPIO_USE_SEL2               0x30
#define ICH_GPIO_IO_SEL2                0x34
#define ICH_GPIO_LVL2                   0x38
#define MODEL_MASK1                     0x40
#define MODEL_MASK2                     0x10
#define BAR5_OFFSET                     0x24

static unsigned int as_ich_gpio_base = 0;

///
/// @brief The driver initial routine.
///
static struct pci_dev * __init aspeed_check_platform (void)
{
  struct pci_dev *pdev = NULL;
  struct pci_dev *pdev1 = NULL;
  int i;
  int devcount = 0;
  unsigned data;

  for (i = 0; i < MAX_PCI_DEVICE_COUNT; i++)
  {
    pdev = pci_get_device (ASP_PCI_VENDOR_ID, PCI_ANY_ID, pdev);
    if (pdev != NULL)
    {
      devcount++;
//printk (KERN_INFO "[AS] marvell bus=0x%X found\n", (pdev->bus) ? pdev->bus->number : 0);
      if (!pdev1 && (pdev->bus) && (pdev->bus->number == 1))
        pdev1 = pdev;
      //break;
    } else
	    break;
  }
  if (devcount != 2)
    return (NULL);

  pdev = NULL;
    pdev = pci_get_device (ICH_PCI_VENDOR_ID, ICH_PCI_DEVICE_ID, pdev);
  if (pdev == NULL)
    return (NULL);

  pci_read_config_dword (pdev, ICH_GPIO_BASE_REG, &as_ich_gpio_base);
  as_ich_gpio_base &= ICH_IO_BASE_MASK;
  if (as_ich_gpio_base == ICH_IO_BASE_MASK)
    as_ich_gpio_base = 0;
//printk (KERN_INFO "[AS] ICH GPIO BASE=0x%X check_region=%d\n",
                      //as_ich_gpio_base, check_region (as_ich_gpio_base, 16));

  if (as_ich_gpio_base)
  {
    data = inl (as_ich_gpio_base + ICH_GPIO_USE_SEL);
    outl (data | MODEL_MASK1, as_ich_gpio_base + ICH_GPIO_USE_SEL);
    data = inl (as_ich_gpio_base + ICH_GPIO_IO_SEL);
    outl (data | MODEL_MASK1, as_ich_gpio_base + ICH_GPIO_IO_SEL);
    data = inl (as_ich_gpio_base + ICH_GPIO_LVL);

    if ((data & MODEL_MASK1) == 0)
      return (NULL);

    data = inl (as_ich_gpio_base + ICH_GPIO_USE_SEL2);
    outl (data | MODEL_MASK2, as_ich_gpio_base + ICH_GPIO_USE_SEL2);
    data = inl (as_ich_gpio_base + ICH_GPIO_IO_SEL2);
    outl (data | MODEL_MASK2, as_ich_gpio_base + ICH_GPIO_IO_SEL2);
    data = inl (as_ich_gpio_base + ICH_GPIO_LVL2);

    if ((data & MODEL_MASK2) != 0)
      return (NULL);
  }
  //printk (KERN_INFO "[AS] Found specific device for aspeed!\n");
  return (pdev1);
}

static int aspeed_modify_inited = 0;
///
/// @brief The driver initial routine.
///
static void __init aspeed_modify (void)
{
  struct pci_dev *pdev;
  u32 data;
  void __iomem *bar5_base = 0;


  if (aspeed_modify_inited)
    return;
  aspeed_modify_inited = 1;

  pdev = aspeed_check_platform ();
  if (pdev)
  {
    bar5_base = pci_iomap(pdev, 5, 0);

    if (bar5_base)
    {
      pci_read_config_dword (pdev, 0x4, &data);
      data |= 7;
      pci_write_config_dword (pdev, 0x4, data);

      writel (0x14, (void *) bar5_base + 0xA0);
      data = readl ((void *) bar5_base + 0xA4);
      data &= 0xfffff8f8;
      data |= 0x101;
      writel (data, (void *) bar5_base + 0xA4);
      pci_iounmap(pdev, bar5_base);
      printk (KERN_INFO "[AS] aspeed setting completed!\n");
      return;
    }
  }
  printk (KERN_INFO "[AS] no aspeed setting!\n");
}
#else
static void aspeed_modify (void)
{
}
#endif

