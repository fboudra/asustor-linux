/* Copyright (c) 2011-2014  Asustor Systems, Inc.  All Rights Reserved. */
/** \file asacl_ext4.c
 * \brief The implementation of the functions of ASACL management.
 *
 * - History
 *  - 2014/05/08		clko	File created.
 */
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/asacl.h>

#include "ext4.h"
#include "ext4_jbd2.h"
#include "xattr.h"
#include "acl.h"
#include "asacl_ext4.h"

PT_ASACL Ext4_Get_Asacl(struct inode *ptInode)
{
	PT_ASACL ptasAcl;

	ptasAcl = Get_Cached_Asacl(ptInode);

	return ptasAcl;
}


static int Ext4_Set_Asacl(handle_t *ptHandle, struct inode *ptInode, PT_ASACL ptasAcl)
{
	return 0;
}


int Ext4_Init_Asacl(handle_t *ptHandle, struct inode *ptInode, struct inode *ptParentInode)
{
	return 0;
}

int Ext4_Asacl_Chmod(struct inode *ptInode)
{
	return 0;
}

int Ext4_Asacl_Chown(struct inode *ptInode, int bOwnerIsGroup)
{
	return 0;
}

void Ext4_Init_Admin_Priv_Procfs(void)
{
	Init_Admin_Priv_Procfs();
}


static size_t Ext4_Xattr_List_Asacl(struct dentry *dentry, char *list, size_t list_len, const char *name, size_t name_len, int type)
{
	return 0;
}

static int Ext4_Xattr_Get_Asacl(struct dentry *dentry, const char *name, void *buffer, size_t buffer_size, int type)
{
	return 0;
}

static int Ext4_Xattr_Set_Asacl(struct dentry *dentry, const char *name, const void *value, size_t size, int flags, int type)
{
	return 0;
}

/// The handler for process ASACL extended attribute (system.asacl).
const struct xattr_handler  Ext4_Asacl_Xattr_Handler =
{
	.prefix	= ASACL_XATTR_TAG,
	.list	= Ext4_Xattr_List_Asacl,
	.get	= Ext4_Xattr_Get_Asacl,
	.set	= Ext4_Xattr_Set_Asacl,
};
