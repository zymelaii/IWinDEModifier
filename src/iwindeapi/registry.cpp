#include "registry.h"

#include <bcrypt.h>
#include <memory>
#include <sddl.h>
#include <wincrypt.h>
#include <winreg.h>
#include <winternl.h>
#include <accctrl.h>
#include <aclapi.h>
#include <winnt.h>

auto CreateRegistryItem(HKEY parent, const wchar_t* item, REGSAM access) -> std::optional<HKEY> {
	HKEY  key		  = nullptr;
	DWORD disposition = 0;	 //!< indicates whether target item already exists
	IWDEM_CheckOrReturn(ERROR_SUCCESS == RegCreateKeyExW(parent,
														 item,
														 0,
														 nullptr,
														 REG_OPTION_NON_VOLATILE,
														 access,
														 nullptr,
														 &key,
														 &disposition),
						std::nullopt);
	return {key};
}

auto OpenRegistryItem(HKEY parent, const wchar_t* item, REGSAM access) -> std::optional<HKEY> {
	HKEY key = nullptr;
	IWDEM_CheckOrReturn(ERROR_SUCCESS == RegOpenKeyExW(parent, item, 0, access, &key),
						std::nullopt);
	return {key};
}

bool CloseRegistryItem(HKEY item, bool flush) {
	IWDEM_CheckOrReturn(ERROR_SUCCESS == RegCloseKey(item), false);
	if (flush) {
		IWDEM_CheckOrReturn(ERROR_SUCCESS == RegFlushKey(item), false);
	}
	return true;
}

std::any QueryRegistryItemInfo(HKEY item, RegInfo info) {
	std::unique_ptr<wchar_t[]> itemClass = nullptr;
	DWORD					   values[8]{};
	FILETIME				   lastWriteTime{};

	RegQueryInfoKeyW(item,			  //!< item handle
					 nullptr,		  //!< RegInfo::ItemClass
					 &values[0],	  //!< RegInfo::ItemClassLen
					 nullptr,		  //<! reserved
					 &values[1],	  //!< RegInfo::SubItemNum
					 &values[2],	  //!< RegInfo::MaxSubItemLen
					 &values[3],	  //!< ignore
					 &values[4],	  //!< RegInfo::KeyNum
					 &values[5],	  //!< RegInfo::MaxKeyLen
					 &values[6],	  //!< RegInfo::MaxValueSize
					 &values[7],	  //!< RegInfo::SecurityDescriptorSize
					 &lastWriteTime	  //!< RegInfo::TimeStamp
	);

	switch (info) {
		case RegInfo::ItemClass: break;
		case RegInfo::ItemClassLen: return {static_cast<size_t>(values[0])};
		case RegInfo::SubItemNum: return {static_cast<size_t>(values[1])};
		case RegInfo::KeyNum: return {static_cast<size_t>(values[4])};
		case RegInfo::MaxSubItemLen: return {static_cast<size_t>(values[2])};
		case RegInfo::MaxKeyLen: return {static_cast<size_t>(values[5])};
		case RegInfo::MaxValueSize: return {static_cast<size_t>(values[6])};
		case RegInfo::SecurityDescriptorSize: return {static_cast<size_t>(values[7])};
		case RegInfo::TimeStamp: return {lastWriteTime};
	}

	DWORD szClass = values[0] + 1;
	itemClass	  = std::make_unique<wchar_t[]>(szClass);
	RegQueryInfoKeyW(item,
					 itemClass.get(),
					 &szClass,
					 nullptr,
					 nullptr,
					 nullptr,
					 nullptr,
					 nullptr,
					 nullptr,
					 nullptr,
					 nullptr,
					 nullptr);
	return {itemClass.release()};
}

////////////////

// auto EnumRegistrySubItem(HKEY hkey, DWORD index) -> std::optional<wchar_t*> {
// 	DWORD count	 = 0;
// 	DWORD maxlen = 0;
// 	RegQueryInfoKeyW(hkey,
// 					 nullptr,
// 					 nullptr,
// 					 nullptr,
// 					 &count,
// 					 &maxlen,
// 					 nullptr,
// 					 nullptr,
// 					 nullptr,
// 					 nullptr,
// 					 nullptr,
// 					 nullptr);
// 	if (index >= count) {
// 		return std::nullopt;
// 	}
// 	maxlen += 1;
// 	auto keyname = std::make_unique<wchar_t[]>(maxlen);
// 	auto result =
// 		RegEnumKeyExW(hkey, index, keyname.get(), &maxlen, nullptr, nullptr, nullptr, nullptr);
// 	if (result == ERROR_SUCCESS) {
// 		return keyname.release();
// 	} else {
// 		return std::nullopt;
// 	}
// }

// auto EnumRegistryValue(HKEY hkey, DWORD index)
// 	-> std::optional<std::tuple<wchar_t*, uint8_t*, size_t, DWORD>> {
// 	DWORD count		  = 0;
// 	DWORD maxlen_name = 0;
// 	DWORD maxlen_data = 0;
// 	DWORD value_type  = REG_NONE;
// 	RegQueryInfoKeyW(hkey,
// 					 nullptr,
// 					 nullptr,
// 					 nullptr,
// 					 nullptr,
// 					 nullptr,
// 					 nullptr,
// 					 &count,
// 					 &maxlen_name,
// 					 &maxlen_data,
// 					 nullptr,
// 					 nullptr);
// 	if (index >= count) {
// 		return std::nullopt;
// 	}
// 	maxlen_name += 1;
// 	auto name	= std::make_unique<wchar_t[]>(maxlen_name);
// 	auto data	= std::make_unique<uint8_t[]>(maxlen_data);
// 	auto result = RegEnumValueW(
// 		hkey, index, name.get(), &maxlen_name, nullptr, &value_type, data.get(), &maxlen_data);
// 	if (result == ERROR_SUCCESS) {
// 		return {{name.release(), data.release(), maxlen_data, value_type}};
// 	} else {
// 		return std::nullopt;
// 	}
// }