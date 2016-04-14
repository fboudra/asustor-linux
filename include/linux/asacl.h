/* Copyright (c) 2011-2014  Asustor Systems, Inc.  All Rights Reserved. */
/** \file asacl.h
 * \brief The header file for the functions of ASACL management.
 *
 * - History
 *  - 2014/04/23		clko	File created.
 *  - 2014/08/28		clko	Reduce the ACE size from 12 byte to 8 byte.
 */
#ifndef _ASUSTOR_ACL_INC_
#define _ASUSTOR_ACL_INC_
#include <linux/slab.h>

#define FLAG_ASACL_MODE_COMPATIBLE	0x00000001		///< Flag to indicate if this ACL is file-mode compatible.
#define FLAG_ASACL_INHERIT_PARENT	0x00000002		///< Flag to indicate if including the inheritable permissions from parent. (Correspond to the Samba 'SEC_DESC_DACL_PROTECTED' flag.)
#define FLAG_ASACL_OWNER_IS_GROUP	0x00000004		///< Flag to indicate if the owner of this file is a group object.
#define FLAG_ASACL_MS_READ_ONLY		0x00000008		///< Flag to indicate if the Windows file attribute "Read only" is enabled or not.
/// Mask for valid ASACL flags.
#define MASK_VALID_ASACL_FLAGS		(FLAG_ASACL_MODE_COMPATIBLE | FLAG_ASACL_INHERIT_PARENT | \
										FLAG_ASACL_OWNER_IS_GROUP | FLAG_ASACL_MS_READ_ONLY)

#define FLAG_ASACE_FILE_INHERIT		0x00000001		///< Flag to indicate if this ACE will inherit to the files under this folder.
#define FLAG_ASACE_FOLDER_INHERIT	0x00000002		///< Flag to indicate if this ACE will inherit to the subfolders.
#define FLAG_ASACE_INHERIT_ONLY		0x00000004		///< Flag to indicate if this ACE if for inherit only (and won't apply to itself).
#define FLAG_ASACE_DENY_ACE			0x00000008		///< Flag to indicate if this ACE is a deny ACE.
#define FLAG_ASACE_NO_PROPAGATE		0x00000010		///< Flag to indicate if this ACE will only be inherited by it child item, and will not be propagate to grant child.
//#define FLAG_ASACE_INHERITED		0x00000020		///< Flag to indicate if this ACE is inherited from ancestors.

// Mask for valid ASACE flags.
#define MASK_VALID_ASACE_FLAGS			(FLAG_ASACE_FILE_INHERIT | FLAG_ASACE_FOLDER_INHERIT | \
											FLAG_ASACE_INHERIT_ONLY | FLAG_ASACE_DENY_ACE | FLAG_ASACE_NO_PROPAGATE)
// Mask for valid ASACE file flags.
#define MASK_VALID_ASACE_FILE_FLAGS		FLAG_ASACE_DENY_ACE

#define ASACL_PERM_READ_DATA_OR_LIST_DIR	0x00000001	///< Permission to read data / list directory.
#define ASACL_PERM_WRITE_DATA_OR_ADD_FILE	0x00000002	///< Permission to write data / add file.
#define ASACL_PERM_APPEND_DATA_OR_ADD_DIR	0x00000004	///< Permission to append data / add dir.
#define ASACL_PERM_READ_EX_ATTRS			0x00000008	///< Permission to read extended attributes.
#define ASACL_PERM_WRITE_EX_ATTRS			0x00000010	///< Permission to write extended attributes.
#define ASACL_PERM_EXECUTE_OR_TRAVERSE		0x00000020	///< Permission to execute / traverse.
#define ASACL_PERM_DELETE_CHILD				0x00000040	///< Permission to delete child file.
#define ASACL_PERM_READ_ATTRS				0x00000080	///< Permission to read attribute.
#define ASACL_PERM_WRITE_ATTRS				0x00000100	///< Permission to write attribute.
#define ASACL_PERM_DELETE					0x00000200	///< Permission to delete.
#define ASACL_PERM_READ_PERM				0x00000400	///< Permission to read file permission.
#define ASACL_PERM_WRITE_PERM				0x00000800	///< Permission to write file permission.
#define ASACL_PERM_TAKE_OWNERSHIP			0x00001000	///< Permission to take ownership.

/// The full control permission for file.
#define ASACL_PERM_FILE_FULL_CTRL		(ASACL_PERM_READ_DATA_OR_LIST_DIR | ASACL_PERM_WRITE_DATA_OR_ADD_FILE | \
											ASACL_PERM_APPEND_DATA_OR_ADD_DIR | ASACL_PERM_READ_EX_ATTRS | \
											ASACL_PERM_WRITE_EX_ATTRS | ASACL_PERM_EXECUTE_OR_TRAVERSE | \
											ASACL_PERM_READ_ATTRS | ASACL_PERM_WRITE_ATTRS | ASACL_PERM_DELETE | \
											ASACL_PERM_READ_PERM | ASACL_PERM_WRITE_PERM | ASACL_PERM_TAKE_OWNERSHIP)
/// The full control permission for dir.
#define ASACL_PERM_DIR_FULL_CTRL		(ASACL_PERM_FILE_FULL_CTRL | ASACL_PERM_DELETE_CHILD)

//#define MASK_VALID_ASACL_FILE_PERM		ASACL_PERM_FILE_FULL_CTRL
#define MASK_VALID_ASACL_DIR_PERM		ASACL_PERM_DIR_FULL_CTRL


#define ID_ASACE_TYPE_OWNER			0		///< The owner entry. (We only use this entry type to map the 'CREATOR OWNER' entry of Windows.)
#define ID_ASACE_TYPE_USER			1		///< The user entry.
#define ID_ASACE_TYPE_GROUP			2		///< The group entry.
#define ID_ASACE_TYPE_EVERYONE		3		///< The everyone entry.

#define CNMAX_ASACL_ENTRY_NUMBER		250					///< The maximum allowable entry number. (The explicit limitation for user.)
#define CNMAX_ASACL_ENTRY_NUM_SYSTEM	300					///< The maximum allowable entry number. (The implicit limitation for system.)
#define ASACL_REVISION_1				1					///< The ASACL revision 1.
#define ASACL_CURRENT_REVISION			ASACL_REVISION_1	///< The current ASACL revision.
#define ASACL_XATTR_TAG					"system.ASACL"		///< The extended attribute identifier for ASACL.
#define ASMAX_UINT8						((1U << 8) - 1)

// The types of the admin-privilege mechanism.
#define IDTYPE_ADMIN_PRIV_INIT		(-1)	///< The initial admin-privilege stat.
#define IDTYPE_ADMIN_PRIV_NONE		0		///< No admin-privilege is applied.
#define IDTYPE_ADMIN_PRIV_USER		1		///< Only the 'admin' user has the admin-privilege.
#define IDTYPE_ADMIN_PRIV_GROUP		2		///< The members of 'administrators' group have the admin-privilege.
#define DEFAULT_ADMIN_PRIV_TYPE		IDTYPE_ADMIN_PRIV_GROUP		///< The default admin-privilege type.
#define ADMIN_PRIV_IDENTIFIER		999		///< The uid/gid of the admin user or administrators group.
#define MAX_UID_GENERAL_ACCOUNT		65534	///< The maximum uid for general user.

#if 0 // This is the old version of the ACL/ACE structure, deprecated.
/** \brief The ACE struct of ASACL. */
typedef struct _t_asace_
{
	uint8_t		idType;			///< The type of this entry. Check 'ID_ASACE_TYPE_XXX' for more detail.
	uint8_t		mFlags;			///< ASACE flags. Check 'FLAG_ASACE_XXX' for more detail.
	uint16_t	cnInherit;		///< The generation number of inheritance. 0 means no inherit.
	uint32_t	mPerm;			///< The permission mask.
	uint32_t	iIdentifier;	///< The identifier of this entry. It may be a uid or gid.
} T_ASACE, *PT_ASACE, **PPT_ASACE;

/** \brief The struct for a ASACL object. */
typedef struct _t_asacl_
{
	union
	{
		atomic_t			tRefCount;		///< The reference counter.
		struct rcu_head		tRcuHead;
	};
	int8_t		iRevision;		///< Revision number.
	uint8_t		mFlags;			///< ASACL flags. Check 'FLAG_ASACL_XXX' for more detail.
	uint16_t	cnEntry;		///< The number of entries.
	T_ASACE		ryEntry[0];		///< The entry array.
} T_ASACL, *PT_ASACL, **PPT_ASACL;
#endif

/** \brief The type set. */
typedef struct _t_ace_type_
{
	uint8_t		idType:4;			///< The type of this entry. Check 'ID_ASACE_TYPE_XXX' for more detail.
	uint8_t		mPreserve:4;		///< The preserved bits. It is meaningless in the present.
} T_ACE_TYPE;

/** \brief The number set. */
typedef struct _t_ace_num_
{
	uint32_t	cnInherit:8;		///< The generation number of inheritance. 0 means no inherit.
	uint32_t	iIdentifier:24;		///< The identifier of this entry. It may be a uid or gid.
} T_ACE_NUM;

/** \brief The ACE struct of ASACL. */
typedef struct _t_asace_
{
	union
	{
		uint8_t		iTypeAlias;	///< Just a alias for 'tatType', we use it to make the casting more easier.
		T_ACE_TYPE	tatType;
	};
	uint8_t		mFlags;			///< ASACE flags. Check 'FLAG_ASACE_XXX' for more detail.
	uint16_t	mPerm;			///< The permission mask.
	union
	{
		uint32_t	iNumAlias;	///< Just a alias for 'tanNum', we use it to make the casting more easier.
		T_ACE_NUM	tanNum;
	};
} T_ASACE, *PT_ASACE, **PPT_ASACE;

/** \brief The struct for a ASACL object. */
typedef struct _t_asacl_
{
	union
	{
		atomic_t			tRefCount;		///< The reference counter.
		struct rcu_head		tRcuHead;
	};
	int8_t		iRevision;		///< Revision number.
	uint8_t		mPreserve1;		///< The preserved bits. It is meaningless in the present.
	uint16_t	cnEntry;		///< The number of entries.
	uint16_t	mFlags;			///< ASACL flags. Check 'FLAG_ASACL_XXX' for more detail.
	uint16_t	mPreserve2;		///< The preserved bits. It is meaningless in the present.
	T_ASACE		ryEntry[0];		///< The entry array.
} T_ASACL, *PT_ASACL, **PPT_ASACL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// For debug messsage.

#include <linux/printk.h>

#define DEBUG_PRINTF(fmt, ...)  printk(KERN_DEBUG "## ASACL ## - %s(%d):" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ERROR_PRINTF(fmt, ...)  printk(KERN_ERR "## ASACL ## - %s(%d):" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)


////////////////////////////////////////////////////////////////////////////////////
// New staff in Kernel version.

static inline PT_ASACL Asacl_Dup(PT_ASACL ptasAcl)
{
	if (ptasAcl)
	{
		atomic_inc(&(ptasAcl->tRefCount));
	}
	return ptasAcl;
}

static inline void Asacl_Release(PT_ASACL ptasAcl)
{
	// Decrease the reference counter. If the counter is down to 0, we could release its memory.
	if (ptasAcl && atomic_dec_and_test(&(ptasAcl->tRefCount)))
	{
		kfree_rcu(ptasAcl, tRcuHead);
	}
}

#ifdef CONFIG_FS_ASACL

/** \brief To get the cached ASACL from inode.
 *
 */
static inline PT_ASACL Get_Cached_Asacl(struct inode *ptInode)
{
	PT_ASACL  *pptaclTmp, ptasAcl;

	pptaclTmp = &ptInode->i_asacl;
	ptasAcl = ACCESS_ONCE(*pptaclTmp);
	if (ptasAcl)
	{
		spin_lock(&ptInode->i_lock);
		ptasAcl = *pptaclTmp;
		if (ptasAcl != ACL_NOT_CACHED)
			ptasAcl = Asacl_Dup(ptasAcl);
		spin_unlock(&ptInode->i_lock);
	}
	return ptasAcl;
}

static inline PT_ASACL Get_Cached_Asacl_RCU(struct inode *ptInode)
{
	return rcu_dereference(ptInode->i_asacl);
}

/** \brief To set the cached ASACL of a inode.
 *
 */
static inline void Set_Cached_Asacl(struct inode *ptInode, PT_ASACL ptasAcl)
{
	PT_ASACL  ptaclOld = NULL;
	spin_lock(&ptInode->i_lock);
	ptaclOld = ptInode->i_asacl;
//	ptInode->i_asacl = Asacl_Dup(ptasAcl);
	rcu_assign_pointer(ptInode->i_asacl, Asacl_Dup(ptasAcl));
	spin_unlock(&ptInode->i_lock);
	if (ptaclOld != ACL_NOT_CACHED)
		Asacl_Release(ptaclOld);
}

/** \brief To clear the cached ASACL of a inode.
 *
 */
static inline void Forget_Cached_Asacl(struct inode *ptInode)
{
	PT_ASACL ptaclOld = NULL;
	spin_lock(&ptInode->i_lock);
	ptaclOld = ptInode->i_asacl;
	ptInode->i_asacl = ACL_NOT_CACHED;
	spin_unlock(&ptInode->i_lock);
	if (ptaclOld != ACL_NOT_CACHED)
		Asacl_Release(ptaclOld);
}
#endif /* CONFIG_FS_ASACL */


/** \brief Allocate a empty ASACL obj with the specified entry number.
 *
 * \param		cnEntry 	The number of entries you would like this ASACL obj have.
 * \param[out]	pptaclBuf	Pointer to the buffer to get the allocated ASACL obj.
 * \return	It returns 0 if successful, or returns a negative value if encountering an error.
 */
extern int Alloc_Asacl_Obj(int cnEntry, PPT_ASACL pptaclBuf);

#if 0
/** \brief To convert the inode permission mask bits (MAY_XXX) to the ASACL permission mask bits (ASACL_PERM_XXX).
 *
 * \param	mInodeMask	The inode permission mask bits.
 * \return	It returns the converted ASACL permission mask bits.
 */
extern uint16_t Convert_Inode_Mask_To_Asacl_Mask(unsigned int mInodeMask);
#endif

/** \brief To check if the current process grants the requested permission to a file.
 *
 * \param	ptInode	The inode of the specified file.
 * \param	ptasAcl	The ASACL of the specified inode.
 * \param	mMask	The requested permission bits.
 * \return	It returns 0 if the requested permission is granted, returns -EADV if the requested permission has "delete-child"(or "delete-self") and it is explicitly denied,
 * 			returns -EACCES if not granted, or returns a negative value if encountering an error.
 * 			-EINVAL				Invalid input parameter.
 */
extern int Check_Asacl_Permission(struct inode *ptInode, const PT_ASACL ptasAcl, uint16_t mMask);

/** \brief To set the file mode of a inode after we have updated its ASACL.
 *
 * \param	ptasAcl	The updated ASACL of the inode.
 * \param	ptInode	The specified inode.
 * \return		It returns 0 if successful, or returns a negative value if encountering an error.
 */
extern int Set_File_Mode_After_Updating_Asacl(const PT_ASACL ptasAcl, struct inode *ptInode);

/** \brief To create a ASACL obj for a new file.
 *
 * \param		ptaclParent	The ASACL of the parent dir of the new file.
 * \param		ptInode		The inode of the new file.
 * \param[out]	pptasAcl	Pointer to the buffer to get the returned ASACL obj.
 * \return		It returns 0 if successful, or returns a negative value if encountering an error.
 * 				-EINVAL			Invalid input parameter.
 * 				-ENOMEM			Out of memory.
 */
extern int Create_Asacl_For_New_File(const PT_ASACL ptaclParent, struct inode *ptInode, PPT_ASACL pptasAcl);

/** \brief	To check ASACL permission.
 * 			We use this function to replace the original check_acl() function (fs/namei.c).
 * \param	ptInode	The inode of thie file.
 * \param	mWant	The requested permission ('MAY_XXX' defined in include/linux/fs.h).
 * \return	It returns 0 if the requested permission is granted, or returns a negative value if encountering an error.
 *			-EAGAIN			The ASACL is not supported.
 */
extern int Check_Asacl(struct inode *ptInode, int mWant);

/** \brief To check ASACL permission. (Without considering the RCU.)
 *
 * \param	ptInode		The inode of thie file.
 * \param	mAsaclMask	The requested permission. ('ASACL_PERM_XXX' defined in include/linux/asacl.h.)
 * \return	It returns 0 if the requested permission is granted, or returns a negative value if encountering an error.
 */
extern int Check_Asacl_Without_RCU(struct inode *ptInode, int mAsaclMask);

/** \brief To change the owner-is-group flag of a ASACL.
 *
 * \param		ptasAcl			The specified ASACL obj to be changed.
 * \param		bOwnerIsGroup	The new owner-is-group flag.
 * \param[out]	pptaclClone		Pointer to the buffer to get the cloned ASACL obj that has new owner-is-group flag.
 * \return	It returns 0 if successful, returns EALREADY if the flag is not changed, or returns a negative value if encountering an error.
 */
extern int Change_Asacl_Owner_Flag(PT_ASACL ptasAcl, int bOwnerIsGroup, PPT_ASACL pptaclClone);

/** \brief To change the mode-compatible flag of a ASACL.
 *
 * \param		ptasAcl	The ASACL of the file to be changed.
 * \param		bEnable	Enable or disable the mode-compatible flag.
 * \param[out]	pptaclClone	Pointer to the buffer to get the returned new ASACL obj.
 * \return		It returns 0 if successful, or returns a negative value if encountering an error.
 */
extern int Change_Asacl_Mode_Compatible_Flag(PT_ASACL ptasAcl, int bEnable, PPT_ASACL pptaclClone);

#if 0
/** \brief To create a mode compatible ASACL obj. This mode-compatible ASACL is required by the "chmod" command.
 *
 * \param		ptasAcl	The ASACL of the file to be change-mode.
 * \param		ptInode	The inode of the file to be change-mode.
 * \param[out]	pptaclNew	Pointer to the buffer to get the returned new ASACL obj.
 * \return		It returns 0 if successful, or returns a negative value if encountering an error.
 */
extern int Get_Mode_Compatible_Asacl(PT_ASACL ptasAcl, struct inode *ptInode, PPT_ASACL pptaclNew);
#endif

/** \brief To initialize the admin-priv proc entry file.
 *
 * \return None.
 */
extern void Init_Admin_Priv_Procfs(void);

/** \brief To check if we having the admin-privilege.
 *
 * \return	It returns 'AS_TRUE' (1) if the current process has the admin-privilege, returns 'AS_FALSE' (0) if it doesn't.
 */
extern int If_Have_Admin_Privilege(void);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Xattr related APIs.

#if 0 // This is the old version of the ACL/ACE structure, deprecated.
typedef struct _t_xattr_asace_
{
	unsigned char	idType;			///< The type of this entry. Check 'ID_ASACE_TYPE_XXX' for more detail.
	unsigned char	mFlags;			///< ASACE flags. Check 'FLAG_ASACE_XXX' for more detail.
	__le16			cnInherit;		///< The generation number of inheritance. 0 means no inherit.
	__le32			mPerm;			///< The permission mask.
	__le32			iIdentifier;	///< The identifier of this entry. It may be a uid or gid.
} T_XATTR_ASACE, *PT_XATTR_ASACE;

typedef struct _t_xattr_asacl_
{
	char			iRevision;		///< Revision number.
	unsigned char	mFlags;			///< ASACL flags. Check 'FLAG_ASACL_XXX' for more detail.
	__le16			cnEntry;		///< The number of entries.
} T_XATTR_ASACL, *PT_XATTR_ASACL;
#endif

typedef struct _t_xattr_asace_
{
	unsigned char	iTypeAlias;		///< The alias for the member of "T_ACE_TYPE" type.
	unsigned char	mFlags;			///< ASACE flags. Check 'FLAG_ASACE_XXX' for more detail.
	__le16			mPerm;			///< The permission mask.
	__le32			iNumAlias;		///< The alias for the member of "T_ACE_NUM" type.
} T_XATTR_ASACE, *PT_XATTR_ASACE;

typedef struct _t_xattr_asacl_
{
	char			iRevision;		///< Revision number.
	unsigned char	mPreserve1;		///< The preserved bits. It is meaningless in the present.
	__le16			cnEntry;		///< The number of entries.
	__le16			mFlags;			///< ASACL flags. Check 'FLAG_ASACL_XXX' for more detail.
	__le16			mPreserve2;		///< The preserved bits. It is meaningless in the present.
} T_XATTR_ASACL, *PT_XATTR_ASACL;

extern size_t Get_Asacl_Xattr_Size(int cnEntry);

/** \brief Get the ASACL obj from a xattr data.
 *
 * \param		pstXattr	The xattr data.
 * \param		cbSizeXattr	The size in byte of the xattr data.
 * \param[out]	pptaclBuf	Pointer to the buffer to get the returned ASACL obj.
 * \return	It returns 0 if successful, or returns a negative value if encountering an error.
 * 			-EBADMSG			The reversion of the xattr data is not the current ASACL reversion number.
 */
extern int Xattr_To_Asacl(const void *pstXattr, size_t cbSizeXattr, PPT_ASACL pptaclBuf);

/** \brief Transform the ASACL obj into a xattr data.
 *
 * \param			ptasAcl		The given ASACL obj to be transformed.
 * \param[in,out]	pcbXattrBuf	Pointer to the buffer that record the size of the given xattr buffer.
 * 								When function succeed, the size of the returned xattr buffer will be wrote back to this buffer.
 * \param[out]		pstXattr	The provided buffer to get the returned xattr data.
 * \return			It returns 0 if successful, or returns a negative value if encountering an error.
 */
extern int Asacl_To_Xattr(const PT_ASACL ptasAcl, size_t *pcbXattrBuf, void *pstXattr);

#endif /* _ASUSTOR_ACL_INC_ */
