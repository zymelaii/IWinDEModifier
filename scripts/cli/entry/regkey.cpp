#include <iwindeapi/registry.h>
#include <iostream>

// void ListKeys(HKEY hkey, const wchar_t* ext) {
// 	if (hkey == nullptr) {
// 		return;
// 	}
// 	wprintf(L"[%S]\n", ext);
// 	int index = 0;
// 	while (true) {
// 		if (auto opt = EnumRegistrySubItem(hkey, index++); opt.has_value()) {
// 			auto keyname = opt.value();
// 			wprintf(L"  -> %S\n", keyname);
// 			auto subkey = IWDEM_OpenKey(hkey, keyname).value();
// 			delete[] keyname;
// 			int j = 0;
// 			while (true) {
// 				if (auto opt = IWDEM_EnumValue(subkey, j++); opt.has_value()) {
// 					const auto& [name, data, size, type] = opt.value();
// 					switch (type) {
// 						case REG_SZ: wprintf(L"    => %S: %S\n", name, data); break;
// 						case REG_NONE: wprintf(L"    => %S\n", name); break;
// 					}
// 					delete[] name;
// 					delete[] data;
// 				} else {
// 					break;
// 				}
// 			}
// 		} else {
// 			break;
// 		}
// 	}
// }
// int main(int argc, char* argv[]) {
// 	auto [key, unused] = CreateRegistryItem(
// 		HKEY_CURRENT_USER, LR"(SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\FileExts)");
// 	wchar_t FileExt[256]{L".txt"};
// 	if (argc == 1) {
// 		ListKeys(IWDEM_OpenKey(key, FileExt).value_or(nullptr), FileExt);
// 	} else {
// 		for (int i = 1; i < argc; ++i) {
// 			_swprintf(FileExt, L".%S", argv[i]);
// 			ListKeys(IWDEM_OpenKey(key, FileExt).value_or(nullptr), FileExt);
// 		}
// 	}
// 	return 0;
// }

template<typename T> T QueryRegistryItemInfo(HKEY item, RegInfo info) {
	return {std::any_cast<T>(QueryRegistryItemInfo(item, info))};
}

template<bool erase = true, typename... Flags>
bool QueryCommandFlags(int argc, wchar_t* argv[], Flags... flags) {
	bool found = false;
	for (int i = 0; !found && i < argc; ++i) {
		for (auto value : {(wcscmp(argv[i], flags) == 0)...}) {
			found |= value;
			if (value && erase) {
				argv[i][0] = 0;
			}
		}
	}
	return found;
}

extern "C" int cli_entry_regkey_query(int argc, wchar_t* argv[]) {
	bool halt_on_error = QueryCommandFlags(argc, argv, L"--halt-on-error");
	bool silent		   = QueryCommandFlags(argc, argv, L"-s", L"--silent");

	std::unique_ptr<wchar_t[]> ItemClass = nullptr;
	FILETIME				   LastTimeOfWrite{};
	const wchar_t*			   path = nullptr;
	HKEY					   item = nullptr;

	for (int i = 0; i < argc; ++i) {
		if (argv[i][0] == 0) continue;	 //!< skip argument erased by QueryCommandFlags
		path = argv[i];
		if (auto opt = OpenRegistryItem(HKEY_CURRENT_USER, path); opt.has_value()) {
			item = opt.value();
		} else {
			if (!silent) {
				wprintf(LR"(invalid registry path "HKEY_CURRENT_USER\%S")"
						L"\n",
						path);
			}
			if (halt_on_error) {
				return 1;
			}
			continue;
		}
		ItemClass.reset(QueryRegistryItemInfo<wchar_t*>(item, RegInfo::ItemClass));
		wprintf(LR"([HKEY_CURRENT_USER\%S]
Item Class              : %S
Key Number              : %lu
SubItem Number          : %lu
Max Key Length          : %luch
Max Value Size          : %lubytes
Max SubItem Length      : %luch
Security Descriptor Size: %lubytes
---
)",
				path,
				ItemClass.get(),
				QueryRegistryItemInfo<size_t>(item, RegInfo::KeyNum),
				QueryRegistryItemInfo<size_t>(item, RegInfo::SubItemNum),
				QueryRegistryItemInfo<size_t>(item, RegInfo::MaxSubItemLen),
				QueryRegistryItemInfo<size_t>(item, RegInfo::MaxKeyLen),
				QueryRegistryItemInfo<size_t>(item, RegInfo::MaxValueSize),
				QueryRegistryItemInfo<size_t>(item, RegInfo::SecurityDescriptorSize));

		CloseRegistryItem(item);
	}

	return 0;
}

extern "C" int cli_entry_regkey(int argc, wchar_t* argv[]) {
	IWDEM_CheckOrReturn(argc > 0, 1, puts("Usage: regkey <Command> [Options]"));
	if (wcscmp(argv[0], L"query") == 0) {
		return cli_entry_regkey_query(argc - 1, argv + 1);
	} else {
		wprintf(LR"(unknown command "%S")", argv[0]);
		return 2;
	}
}