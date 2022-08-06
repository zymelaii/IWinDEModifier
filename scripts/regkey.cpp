#include <stdint.h>
#include <windows.h>
#include <bcrypt.h>
#include <sddl.h>
#include <wincrypt.h>
#include <winternl.h>
#include <winerror.h>
#include <errhandlingapi.h>
#include <accctrl.h>
#include <aclapi.h>
#include <winnt.h>
#include <winreg.h>

#include <cstdint>
#include <cstddef>
#include <memory>
#include <optional>
#include <string_view>
#include <tuple>
#include <string>

/*!
 * \brief Create new registry key
 * \param hkey root key
 * \param subkey target key path
 * \param access access assigned to new key
 * \return tuple consists of key and extra info
 * \retval {<null>, <error-code>} failed creating new key
 * \retval {<non-null>, <disposition>} succeeded creating new key
 */
auto IWDEM_CreateKey(HKEY hkey, const wchar_t* subkey,
					 REGSAM access = KEY_ALL_ACCESS | KEY_WOW64_64KEY) -> std::tuple<HKEY, DWORD> {
	HKEY  key		  = nullptr;
	DWORD disposition = 0;
	auto  result	  = RegCreateKeyExW(
		  hkey, subkey, 0, nullptr, REG_OPTION_NON_VOLATILE, access, nullptr, &key, &disposition);
	if (result == ERROR_SUCCESS) {
		return {key, disposition};
	} else {
		return {nullptr, result};
	}
}

/*!
 * \brief Open target registry key
 * \param hkey root key
 * \param subkey target key path
 * \param access access assigned to new key
 * \param create_if_not_exist enable creating new key if expected key doesn't exist
 * \return key handle
 * \retval std::nullopt key not exist or failed creating new key unser force mode
 */
auto IWDEM_OpenKey(HKEY hkey, const wchar_t* subkey, bool create_if_not_exist = false,
				   REGSAM access = KEY_ALL_ACCESS | KEY_WOW64_64KEY) -> std::optional<HKEY> {
	HKEY key	= nullptr;
	auto result = RegOpenKeyExW(hkey, subkey, 0, access, &key);
	if (result == ERROR_FILE_NOT_FOUND && create_if_not_exist) {
		auto [newkey, disposition] = IWDEM_CreateKey(hkey, subkey);
		if (newkey == nullptr) {
			result = disposition;
		} else {
			key = newkey;
		}
	}
	if (result == ERROR_SUCCESS) {
		return {key};
	} else {
		return std::nullopt;
	}
}

/*!
 * \brief Close target registry key
 * \param hkey target key
 */
void WIDEM_CloseKey(HKEY hkey) {
	RegCloseKey(hkey);
}

auto IWDEM_EnumKey(HKEY hkey, DWORD index) -> std::optional<wchar_t*> {
	DWORD count	 = 0;
	DWORD maxlen = 0;
	RegQueryInfoKeyW(hkey,
					 nullptr,
					 nullptr,
					 nullptr,
					 &count,
					 &maxlen,
					 nullptr,
					 nullptr,
					 nullptr,
					 nullptr,
					 nullptr,
					 nullptr);
	if (index >= count) {
		return std::nullopt;
	}
	maxlen += 1;
	auto keyname = std::make_unique<wchar_t[]>(maxlen);
	auto result =
		RegEnumKeyExW(hkey, index, keyname.get(), &maxlen, nullptr, nullptr, nullptr, nullptr);
	if (result == ERROR_SUCCESS) {
		return keyname.release();
	} else {
		return std::nullopt;
	}
}

auto IWDEM_EnumValue(HKEY hkey, DWORD index)
	-> std::optional<std::tuple<wchar_t*, uint8_t*, size_t, DWORD>> {
	DWORD count		  = 0;
	DWORD maxlen_name = 0;
	DWORD maxlen_data = 0;
	DWORD value_type  = REG_NONE;
	RegQueryInfoKeyW(hkey,
					 nullptr,
					 nullptr,
					 nullptr,
					 nullptr,
					 nullptr,
					 nullptr,
					 &count,
					 &maxlen_name,
					 &maxlen_data,
					 nullptr,
					 nullptr);
	if (index >= count) {
		return std::nullopt;
	}
	maxlen_name += 1;
	auto name	= std::make_unique<wchar_t[]>(maxlen_name);
	auto data	= std::make_unique<uint8_t[]>(maxlen_data);
	auto result = RegEnumValueW(
		hkey, index, name.get(), &maxlen_name, nullptr, &value_type, data.get(), &maxlen_data);
	if (result == ERROR_SUCCESS) {
		return {{name.release(), data.release(), maxlen_data, value_type}};
	} else {
		return std::nullopt;
	}
}

#include <iostream>
void ListKeys(HKEY hkey, const wchar_t* ext) {
	if (hkey == nullptr) {
		return;
	}
	wprintf(L"[%S]\n", ext);
	int index = 0;
	while (true) {
		if (auto opt = IWDEM_EnumKey(hkey, index++); opt.has_value()) {
			auto keyname = opt.value();
			wprintf(L"  -> %S\n", keyname);
			auto subkey = IWDEM_OpenKey(hkey, keyname).value();
			delete[] keyname;
			int j = 0;
			while (true) {
				if (auto opt = IWDEM_EnumValue(subkey, j++); opt.has_value()) {
					const auto& [name, data, size, type] = opt.value();
					switch (type) {
						case REG_SZ: wprintf(L"    => %S: %S\n", name, data); break;
						case REG_NONE: wprintf(L"    => %S\n", name); break;
					}
					delete[] name;
					delete[] data;
				} else {
					break;
				}
			}
		} else {
			break;
		}
	}
}
int main(int argc, char* argv[]) {
	auto [key, unused] = IWDEM_CreateKey(
		HKEY_CURRENT_USER, LR"(SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\FileExts)");
	wchar_t FileExt[256]{L".txt"};
	if (argc == 1) {
		ListKeys(IWDEM_OpenKey(key, FileExt).value_or(nullptr), FileExt);
	} else {
		for (int i = 1; i < argc; ++i) {
			_swprintf(FileExt, L".%S", argv[i]);
			ListKeys(IWDEM_OpenKey(key, FileExt).value_or(nullptr), FileExt);
		}
	}
	return 0;
}