/* Copyright (c) 2011-2014  Asustor Systems, Inc.  All Rights Reserved. */
/** \file asacl.c
 * \brief The implementation of the functions of ASACL management.
 *
 * - History
 *  - 2014/04/23		clko	File created.
 */
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/asacl.h>

#define INIT_MODE_EQUIV_PERM  (ASACL_PERM_READ_DATA_OR_LIST_DIR | ASACL_PERM_WRITE_DATA_OR_ADD_FILE | \
								ASACL_PERM_APPEND_DATA_OR_ADD_DIR | ASACL_PERM_EXECUTE_OR_TRAVERSE)

#define DEFAULT_PERM_EVERYONE	(ASACL_PERM_READ_DATA_OR_LIST_DIR | ASACL_PERM_WRITE_DATA_OR_ADD_FILE | \
									ASACL_PERM_APPEND_DATA_OR_ADD_DIR | ASACL_PERM_READ_EX_ATTRS | \
									ASACL_PERM_WRITE_EX_ATTRS | ASACL_PERM_EXECUTE_OR_TRAVERSE | \
									ASACL_PERM_READ_ATTRS | ASACL_PERM_WRITE_ATTRS | ASACL_PERM_DELETE | \
									ASACL_PERM_READ_PERM | ASACL_PERM_DELETE_CHILD)

#define SZ_ADMIN_PRIV_PROCFS		"as_admpriv"
#define CN_DEFAULT_ACL_ENTRY		2		///< The number of entries of the default ACL.
static int g_iAdmPrivType = IDTYPE_ADMIN_PRIV_INIT;
static int g_ryidAdmPriv[3] = { ADMIN_PRIV_IDENTIFIER, -1, -1 };


int Alloc_Asacl_Obj(int cnEntry, PPT_ASACL pptaclBuf)
{
	return 0;
}


static int Clone_Asacl_Obj(PT_ASACL ptaclSrc, PPT_ASACL pptaclDst)
{
	return 0;
}


static uint16_t Convert_Inode_Mask_To_Asacl_Mask(unsigned int mInodeMask)
{
	return 0;
}


static int Is_Identifier_Matched(PT_ASACE ptAce, int bIsOwner)
{
	return AS_TRUE;
}


int Check_Asacl_Permission(struct inode *ptInode, const PT_ASACL ptasAcl, uint16_t mMask)
{
	return 0;
}


static inline int Is_Inheritable_Ace_For_File(unsigned int mFlags)
{
	return 0;
}


static inline int Is_Inheritable_Ace_For_Dir(unsigned int mFlags)
{
	return 1;
}


static int Get_Inherited_Asacl_From_Parent(const PT_ASACL ptaclParent, int bIsDir, kuid_t iOwnerUid, PPT_ASACL pptasAcl)
{
	return 0;
}


static int Create_Default_Asacl(struct inode *ptInode, PPT_ASACL pptasAcl)
{
	return 0;
}


static inline mode_t Asacl_Perm_Bit_To_Mode(uint16_t mPerm)
{
	return 0;
}


static mode_t Get_Asacl_Equivalet_Mode(const PT_ASACL ptasAcl, const struct inode *ptInode)
{
	mode_t  mMode = 0;

	return mMode;
}


int Set_File_Mode_After_Updating_Asacl(const PT_ASACL ptasAcl, struct inode *ptInode)
{
	return 0;
}


int Create_Asacl_For_New_File(const PT_ASACL ptaclParent, struct inode *ptInode, PPT_ASACL pptasAcl)
{
	return 0;
}


int Check_Asacl(struct inode *ptInode, int mWant)
{
	return 0;
}


int Check_Asacl_Without_RCU(struct inode *ptInode, int mAsaclMask)
{
	return 0;
}


int Change_Asacl_Owner_Flag(PT_ASACL ptasAcl, int bOwnerIsGroup, PPT_ASACL pptaclClone)
{
	return 0;
}


int Change_Asacl_Mode_Compatible_Flag(PT_ASACL ptasAcl, int bEnable, PPT_ASACL pptaclClone)
{
	return 0;
}

/** \brief The callback function when the admin-priv proc entry file is read. */
static int Admin_Priv_Show_Proc(struct seq_file *m, void *v)
{
	return 0;
}

static int Admin_Priv_Open_Proc(struct inode *inode, struct file *file)
{
	return single_open(file, Admin_Priv_Show_Proc, inode->i_private);
}

/** \brief The callback function when the admin-priv proc entry file is writen. */
static ssize_t Admin_Priv_Write_Proc(struct file *file, const char __user *buf,
                       size_t count, loff_t *pos)
{
	return count;
}

static const struct file_operations admpriv_fops = 
{
	.open  = Admin_Priv_Open_Proc,
	.read = seq_read,
	.write = Admin_Priv_Write_Proc,
	.llseek  = seq_lseek,
	.release = single_release,
};


void Init_Admin_Priv_Procfs(void)
{
	return;
}


int If_Have_Admin_Privilege(void)
{
	return AS_TRUE;
}



size_t Get_Asacl_Xattr_Size(int cnEntry)
{
	return (sizeof(T_XATTR_ASACL) + cnEntry*sizeof(T_XATTR_ASACE));
}


int Xattr_To_Asacl(const void *pstXattr, size_t cbSizeXattr, PPT_ASACL pptaclBuf)
{
	return 0;
}


int Asacl_To_Xattr(const PT_ASACL ptasAcl, size_t *pcbXattrBuf, void *pstXattr)
{
	return 0;
}
