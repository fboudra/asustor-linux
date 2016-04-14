/* Copyright (c) 2013  Asustor Systems, Inc.  All Rights Reserved. */
/**
 * \file 	sysstat.h
 * \brief	Define sytem status defined file 
 * - History
 *  - 2013/01/16	willyhuang	File created.
 */

#define ASUSTOR_SYS_RECORD_TIMES	63
#define ASUSTOR_CPU_COUNT_MAX		16
#define ASUSTOR_NETIF_COUNT_MAX		8
#define ASUSTOR_MDEV_COUNT_MAX		32
#define ASUSTOR_SATA_COUNT_MAX		32
#define ASUSTOR_ATA_ERROR_LOG_MAX	32
#define ASUSTOR_MD_ERROR_LOG_MAX	8
#define ASUSTOR_DEV_NAME_LENGTH		7
#define ASUSTOR_MD_SECTOR_SIZE		512
#define ASUSTOR_STAT_SLEEP_TIME		2000

#define PAGETOMB(x) ((x) >> (20 - PAGE_SHIFT))
#define PAGETOKB(x) ((x) << (PAGE_SHIFT - 10))
#define BYTETOKB(x) ((x) >> 10)
 
typedef struct _T_ASUSTOR_CPU_USAGE_ {
	uint8_t iPusr;
	uint8_t iPsys;
	uint8_t iPidle;
	uint8_t iPiowait;
} T_ASUSTOR_CPU_USAGE;

typedef struct _T_ASUSTOR_CPU_USAGE_TOTAL_ {
	T_ASUSTOR_CPU_USAGE tcuTotal;
	T_ASUSTOR_CPU_USAGE rytcuThread[ASUSTOR_CPU_COUNT_MAX];
} T_ASUSTOR_CPU_USAGE_TOTAL;

typedef struct _T_ASUSTOR_MEM_USAGE_ {
	uint32_t kbFree;
	uint32_t kbCached;
	uint32_t kbBuffered;
	uint32_t kbSwaptotal;
	uint32_t kbSwapfree;
} T_ASUSTOR_MEM_USAGE;

typedef struct _T_ASUSTOR_NETIF_LOAD_ {
	struct 		_T_ASUSTOR_NETIF_LOAD_ *ptNext;
	char 		szIfname[ASUSTOR_DEV_NAME_LENGTH];
	uint8_t		iFillFlag;
	uint64_t 	kbTxTotal;
	uint64_t 	kbRxTotal;
	uint64_t 	rykbTx[ASUSTOR_SYS_RECORD_TIMES+1];	
	uint64_t 	rykbRx[ASUSTOR_SYS_RECORD_TIMES+1];
} T_ASUSTOR_NETIF_LOAD, *PT_ASUSTOR_NETIF_LOAD, **PPT_ASUSTOR_NETIF_LOAD;

typedef struct _T_ASUSTOR_MD_ERROR_ {
	uint64_t	iRecordTime;
	char 		szDiskname[ASUSTOR_DEV_NAME_LENGTH];
} T_ASUSTOR_MD_ERROR;

typedef struct _T_ASUSTOR_MDEV_LOAD_ {
	struct 		_T_ASUSTOR_MDEV_LOAD_ *ptNext;
	char 		szDevname[ASUSTOR_DEV_NAME_LENGTH];
	uint8_t		iFillFlag;
	uint64_t 	kbReadTotal;
	uint64_t 	kbWriteTotal;
	uint64_t 	rykbRead[ASUSTOR_SYS_RECORD_TIMES+1];	
	uint64_t 	rykbWrite[ASUSTOR_SYS_RECORD_TIMES+1];
	uint8_t		cnMdErrCount;
	T_ASUSTOR_MD_ERROR 	rttMdError[ASUSTOR_MD_ERROR_LOG_MAX];
} T_ASUSTOR_MDEV_LOAD, *PT_ASUSTOR_MDEV_LOAD, **PPT_ASUSTOR_MDEV_LOAD;

typedef struct _T_ASUSTOR_ATA_ERROR_ {
	uint64_t	iRecordTime;
	uint32_t	iErrMask;
	uint8_t		iAtaCommand;
	uint8_t		iAtaStatus;
	uint8_t		iAtaErrFlag;
	uint8_t		iSataErrFlag;
} T_ASUSTOR_ATA_ERROR;

typedef struct _ASUSTOR_SATA_STAT_ {
	struct 					_ASUSTOR_SATA_STAT_	*ptNext;	
	bool 					bPortOnline;
	bool 					bDiskOnline;
	char					szDeviceName[ASUSTOR_DEV_NAME_LENGTH];
	uint8_t					cnAtaNumber;
	uint8_t					cnResetCount;
	uint8_t					cnResetFailCount;
	uint8_t					cnErrorCount;
	uint8_t					cnScsiErrCount;
	T_ASUSTOR_ATA_ERROR 	rttAtaError[ASUSTOR_ATA_ERROR_LOG_MAX];
} T_ASUSTOR_SATA_STAT, *PT_ASUSTOR_SATA_STAT, **PPT_ASUSTOR_SATA_STAT;

typedef struct _T_ASUSTOR_SYSTEM_STAT_FOR_COUNT_{
	bool						bInitial;
	uint8_t						iCurrentFlag;
	uint8_t						iFillFlag;
	uint8_t 					iCpuCount;
	uint8_t						iNetCount;
	uint8_t						iSataCount;
	uint8_t						iMdevCount;
	uint32_t					iMemTotal;
	T_ASUSTOR_CPU_USAGE_TOTAL 	rytCpuUsage[ASUSTOR_SYS_RECORD_TIMES+1];
	T_ASUSTOR_MEM_USAGE 		rytMemUsage[ASUSTOR_SYS_RECORD_TIMES+1];
} T_ASUSTOR_SYSTEM_STAT_FOR_COUNT;

typedef struct _T_ASUSTOR_SYSTEM_STAT_ {
	bool						bInitial;
	uint8_t						iCurrentFlag;
	uint8_t						iFillFlag;
	uint8_t 					iCpuCount;
	uint8_t						iNetCount;
	uint8_t						iSataCount;
	uint8_t						iMdevCount;
	uint32_t					iMemTotal;
	T_ASUSTOR_CPU_USAGE_TOTAL 	rytCpuUsage[ASUSTOR_SYS_RECORD_TIMES+1];
	T_ASUSTOR_MEM_USAGE 		rytMemUsage[ASUSTOR_SYS_RECORD_TIMES+1];
	PT_ASUSTOR_NETIF_LOAD		ptNetworkLoadList;
	PT_ASUSTOR_MDEV_LOAD		ptMdevLoadList;
	PT_ASUSTOR_SATA_STAT		ptSataPortList;
} T_ASUSTOR_SYSTEM_STAT;

typedef struct _T_ASUSTOR_SYSTEM_STAT_FOR_USER_{
	bool						bInitial;
	uint8_t						iCurrentFlag;
	uint8_t						iFillFlag;
	uint8_t 					iCpuCount;
	uint8_t						iNetCount;
	uint8_t						iSataCount;
	uint8_t						iMdevCount;
	uint32_t					iMemTotal;
	T_ASUSTOR_CPU_USAGE_TOTAL 	rytCpuUsage[ASUSTOR_SYS_RECORD_TIMES+1];
	T_ASUSTOR_MEM_USAGE 		rytMemUsage[ASUSTOR_SYS_RECORD_TIMES+1];
	T_ASUSTOR_NETIF_LOAD		rytNetworkLoadList[ASUSTOR_NETIF_COUNT_MAX];
	T_ASUSTOR_MDEV_LOAD			rytMdevLoadList[ASUSTOR_MDEV_COUNT_MAX];
	T_ASUSTOR_SATA_STAT			rytSataPortList[ASUSTOR_SATA_COUNT_MAX];
} T_ASUSTOR_SYSTEM_STAT_FOR_USER, *PT_ASUSTOR_SYSTEM_STAT_FOR_USER;

extern T_ASUSTOR_SYSTEM_STAT Asustor_Sys_Stat;

extern spinlock_t Pers_lock;

extern int Add_Asustor_Sata_Retry_Count(int cnAtaNumber);

extern int Add_Asustor_Sata_Retry_Failed(int cnAtaNumber);

extern int Add_Asustor_Sata_Scsi_Error(int cnAtaNumber);

extern int Add_Asustor_Sata_Error_Log(int cnAtaNumber, uint8_t ata_command, uint32_t err_mask, uint8_t ata_status, uint8_t ata_error_flag, uint8_t sata_error_flag, uint64_t ata_lba);

extern int Add_Asustor_Sata_Port(int cnAtaNumber);

extern int Remove_Asustor_Sata_Port(int cnAtaNumber);

extern int Link_Asustor_Sata_Port(int cnAtaNumber);

extern int Unlink_Asustor_Sata_Port(int cnAtaNumber);

extern int Add_Asustor_Sata_Disk(int cnAtaNumber, const char *pszdiskname);

extern int Add_Asustor_Netif_Stat(const char *szIfname);

extern int Remove_Asustor_Netif_Stat(const char *szIfname);

extern int Add_Asustor_Mddev_Stat(const char *szMdName);

extern int Remove_Asustor_Mddev_Stat(const char *szMdName);

extern void Init_Sys_Stat(void);
