#include "internal.h"
#include <functional>
#include <memory>
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
 * \param item handle of target item
 * \param flush flush registry cache to the hard disk, ensure the item has KEY_QUERY_VALUE access if
 * flush flag is manually specified
 */
bool CloseRegistryItem(HKEY item, bool flush = false);

/*!
 * \brief query registry item info
 *
 * \param item handle of target item
 * \param info info type to query
 */
std::any QueryRegistryItemInfo(HKEY item, RegInfo info);

/*!
 * \brief enum subitem of target item
 *
 * \param item handle of target item
 * \param index index of subitem
 *
 * \note get number of subitems via QueryRegistryItemInfo
 */
std::optional<std::unique_ptr<wchar_t[]>> EnumRegistrySubItem(HKEY item, int index);

/*!
 * \brief enum key of target item
 *
 * \param item handle of target item
 * \param index index of key
 *
 * \note get number of key via QueryRegistryItemInfo
 */
std::optional<std::unique_ptr<wchar_t[]>> EnumRegistryKey(HKEY item, int index);

/*!
 * \brief get registry key value
 *
 * \param item handle of target item
 * \param key key of target value
 * \param bytes buffer to receive key value, notice that nullopt indicates query-only-action.
 * if bytes contains no buffer then it will be assigned automaticly, otherwise data will be directly
 * wrote to bytes
 *
 * \return triple of exec result, type of key and value size (in bytes)
 */
std::tuple<bool, uint8_t, size_t> GetRegistryValue(
	HKEY item, const wchar_t* key,
	std::optional<std::reference_wrapper<std::unique_ptr<uint8_t[]>>> bytes = std::nullopt);