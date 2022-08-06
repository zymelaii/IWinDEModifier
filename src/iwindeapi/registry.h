#include "internal.h"
#include <any>
#include <optional>
#include <winreg.h>

enum class RegInfo {
	ItemClass,				  //!< info type: wchar_t[]
	ItemClassLen,			  //!< info type: size_t (in characters)
	SubItemNum,				  //!< info type: size_t
	KeyNum,					  //!< info type: size_t
	MaxSubItemLen,			  //!< info type: size_t (in characters)
	MaxKeyLen,				  //!< info type: size_t (in characters)
	MaxValueSize,			  //!< info type: size_t (in bytes)
	SecurityDescriptorSize,	  //!< info type: size_t (in bytes)
	TimeStamp,				  //!< info type: FILETIME
};

/*!
 * \brief create new registry item
 *
 * \param parent parent item handle, not null
 * \param item target item path, not null (expected)
 * \param access access assigned to the item
 */
std::optional<HKEY> CreateRegistryItem(HKEY parent, const wchar_t* item,
									   REGSAM access = KEY_ALL_ACCESS | KEY_WOW64_64KEY);

/*!
 * \brief open registry item
 *
 * \param parent parent item handle, or set null to use item only
 * \param item target item path, or set null to use parent only
 * \param access access to open the item
 *
 * \note inappropriate accessibility or existence of item may cause failure
 */
std::optional<HKEY> OpenRegistryItem(HKEY parent, const wchar_t* item,
									 REGSAM access = KEY_ALL_ACCESS | KEY_WOW64_64KEY);

/*!
 * \brief close registry item
 *
 * \param item target item handle
 * \param flush flush registry cache to the hard disk, ensure the item has KEY_QUERY_VALUE access if
 * flush flag is manually specified
 */
bool CloseRegistryItem(HKEY item, bool flush = false);

/*!
 * \brief query registry item info
 *
 * \param item target item handle
 * \param info info type to query
 */
std::any QueryRegistryItemInfo(HKEY item, RegInfo info);

// std::optional<wchar_t*> EnumRegistrySubItem(HKEY item, int index);

// std::optional<std::tuple<wchar_t*, uint8_t*, size_t, DWORD>> EnumRegistryValue(HKEY	 item,
// 																			   DWORD index);