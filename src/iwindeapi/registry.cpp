#include "registry.h"
#include <memory>

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
						std::nullopt);	 //!< [likely] access denied
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
	wchar_t					   buffer[256]{};
	std::unique_ptr<wchar_t[]> itemClass = nullptr;
	DWORD					   values[8]{256};
	FILETIME				   lastWriteTime{};

	RegQueryInfoKeyW(item,			  //!< item handle
					 buffer,		  //!< RegInfo::ItemClass
					 &values[0],	  //!< RegInfo::ItemClassLen
					 nullptr,		  //!< reserved
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
	wcscpy_s(itemClass.get(), szClass, buffer);

	return {itemClass.release()};
}

auto EnumRegistrySubItem(HKEY item, int index) -> std::optional<std::unique_ptr<wchar_t[]>> {
	wchar_t					   buffer[256]{};
	std::unique_ptr<wchar_t[]> name = nullptr;
	DWORD					   size = 256;

	IWDEM_CheckOrReturn(
		ERROR_SUCCESS ==
			RegEnumKeyExW(item, index, buffer, &size, nullptr, nullptr, nullptr, nullptr),
		std::nullopt);

	size += 1;
	name = std::make_unique<wchar_t[]>(size);
	wcscpy_s(name.get(), size, buffer);

	return {std::move(name)};
}

auto EnumRegistryKey(HKEY item, int index) -> std::optional<std::unique_ptr<wchar_t[]>> {
	wchar_t					   buffer[256]{};
	std::unique_ptr<wchar_t[]> name = nullptr;
	DWORD					   size = 256;

	IWDEM_CheckOrReturn(
		ERROR_SUCCESS ==
			RegEnumValueW(item, index, buffer, &size, nullptr, nullptr, nullptr, nullptr),
		std::nullopt);

	size += 1;
	name = std::make_unique<wchar_t[]>(size);
	wcscpy_s(name.get(), size, buffer);

	return {std::move(name)};
}

bool RemoveRegistryTree(HKEY parent, const wchar_t *item) {
	IWDEM_CheckOrReturn(ERROR_SUCCESS == RegDeleteTreeW(parent, item), false);
	return true;
}

auto GetRegistryValue(HKEY item, const wchar_t* key,
					  std::optional<std::reference_wrapper<std::unique_ptr<uint8_t[]>>> bytes)
	-> std::tuple<bool, uint8_t, size_t> {
	DWORD type = REG_NONE;
	DWORD size = 0;

	IWDEM_CheckOrReturn(
		ERROR_SUCCESS == RegGetValueW(item, nullptr, key, RRF_RT_ANY, &type, nullptr, &size),
		std::make_tuple(false, 0, 0));

	if (bytes.has_value()) {
		auto& data = bytes.value().get();
		if (data.get() == nullptr) {
			data = std::make_unique<uint8_t[]>(size);
		} else {
			//! buffer size must be strictly greater than size if it is manually specified
		}

		//! a value acquisition failure doesn't abandon type and size info
		IWDEM_CheckOrReturn(
			ERROR_SUCCESS == RegGetValueW(item, nullptr, key, RRF_RT_ANY, &type, data.get(), &size),
			std::make_tuple(false, type, size));
	}
	return {true, type, size};
}