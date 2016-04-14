/* Copyright (c) 2011-2014  Asustor Systems, Inc.  All Rights Reserved. */
/** \file asacl_ext4.h
 * \brief The header file for the functions of ASACL management.
 *
 * - History
 *  - 2014/05/08		clko	File created.
 */
#ifndef _ASACL_EXT4_INC_
#define _ASACL_EXT4_INC_

#include <linux/asacl.h>

#ifdef CONFIG_EXT4_FS_ASACL

#define EXT4_IS_ASACL(inode) IS_ASACL(inode)

/** \brief	The inode operation for "get_asacl".
 * 			Reference function: Original "ext4_get_acl()" function at fs/ext4/acl.c.
 *
 * \param	ptInode	The specified inode to get ASACL.
 * \return	It returns the ASACL obj of the inode if successful, returns NULL if the inode doesn't have ASACL,
 * 			or returns a negative address (use IS_ERR() to check) if encountering an error.
 */
extern PT_ASACL Ext4_Get_Asacl(struct inode *ptInode);

/** \brief	Initialize the ACLs of a new inode. Called from ext4_new_inode.
 * 			Reference function: Original "ext4_init_acl()" function at fs/ext4/acl.c.
 *
 * \param	ptInode			The inode of the new file.
 * \param	ptParentInode	The inode of the parent dir of the new file.
 * \return	It returns 0 if successful, or returns a negative value if encountering an error.
 */
extern int Ext4_Init_Asacl(handle_t *ptHandle, struct inode *ptInode, struct inode *ptParentInode);

/** \brief	Does chmod for an inode that may have an Access Control List.
 * 			Reference function: Original "ext4_acl_chmod()" function at fs/ext4/acl.c.
 *
 * The inode->i_mode field must be updated to the desired value by the caller
 * before calling this function.
 *
 * \param	ptInode	The specified inode to be chmod.
 * \return	It returns 0 if successful, or returns a negative value if encountering an error.
 */
extern int Ext4_Asacl_Chmod(struct inode *ptInode);

/** \brief The ASACL chown handling function for ext4.
 *
 * \param	ptInode			The specified inode to be chown.
 * \param	bOwnerIsGroup	The new owner-is-group flag.
 * \return	It returns 0 if successful, or returns a negative value if encountering an error.
 */
extern int Ext4_Asacl_Chown(struct inode *ptInode, int bOwnerIsGroup);

/** \brief To initialize the admin-priv proc entry file.
 *
 * \return None.
 */
extern void Ext4_Init_Admin_Priv_Procfs(void);

#else /* CONFIG_EXT4_FS_ASACL */

#define EXT4_IS_ASACL(inode) (0)
#define Ext4_Get_Asacl   NULL

static inline int Ext4_Init_Asacl(handle_t *ptHandle, struct inode *ptInode, struct inode *ptParentInode)
{
	return 0;
}

static inline int Ext4_Asacl_Chmod(struct inode *ptInode)
{
	return 0;
}

static inline int Ext4_Asacl_Chown(struct inode *ptInode, int bOwnerIsGroup)
{
	return 0;
}

static inline void Ext4_Init_Admin_Priv_Procfs(void)
{
	return;
}

#endif /* CONFIG_EXT4_FS_ASACL */

#endif /* _ASACL_EXT4_INC_ */
