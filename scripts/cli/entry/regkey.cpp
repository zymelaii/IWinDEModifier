#include <iwindeapi/registry.h>
#include <iostream>
#include <memory>
#include <stdint.h>
#include <winnt.h>
#include <winreg.h>

template<typename T> T QueryRegistryItemInfo(HKEY item, RegInfo info) {
	return {std::any_cast<T>(QueryRegistryItemInfo(item, info))};
}

template<bool erase = true, typename... Flags>
bool QueryCommandFlags(int& argc, wchar_t* argv[], Flags... flags) {
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

int ListItemRecursive(HKEY item, bool show_keys = true, int depth = 0, int indent = 0) {
	IWDEM_CheckOrReturn(depth >= 0, 0);
	auto nSubItem = QueryRegistryItemInfo<size_t>(item, RegInfo::SubItemNum);
	for (int i = 0; i < nSubItem; ++i) {
		auto SubItemName = std::move(EnumRegistrySubItem(item, i).value_or(nullptr));
		IWDEM_CheckOrReturn(SubItemName.get() != nullptr, 1);
		auto SubItem = OpenRegistryItem(item, SubItemName.get(), KEY_READ).value_or(nullptr);
		IWDEM_CheckOrReturn(SubItem != nullptr, 2);
		auto nKeyOfSubItem = QueryRegistryItemInfo<size_t>(SubItem, RegInfo::KeyNum);
		wprintf(L"%*S%S\n", indent * 2, L"", SubItemName.get());
		if (show_keys) {
			for (int j = 0; j < nKeyOfSubItem; ++j) {
				auto KeyName = std::move(EnumRegistryKey(SubItem, j).value_or(nullptr));
				IWDEM_CheckOrReturn(KeyName.get() != nullptr, 3);

				std::unique_ptr<uint8_t[]> ValueBytes = nullptr;
				auto [success, KeyType, ValueSize] =
					GetRegistryValue(SubItem, KeyName.get(), ValueBytes);
				IWDEM_CheckOrReturn(success, 4);

				wprintf(L"%*S- %S%S%S\n",
						(indent + 1) * 2,
						L"",
						KeyName.get()[0] == 0 ? L"(Default)" : KeyName.get(),
						KeyType == REG_SZ || KeyType == REG_EXPAND_SZ ? L": " : L"",
						KeyType == REG_SZ || KeyType == REG_EXPAND_SZ
							? reinterpret_cast<wchar_t*>(ValueBytes.get())
							: L"");
			}
		}
		ListItemRecursive(SubItem, show_keys, depth - 1, indent + 1);
		CloseRegistryItem(SubItem);
	}
	return 0;
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

extern "C" int cli_entry_regkey_list(int argc, wchar_t* argv[]) {
	bool show_keys = QueryCommandFlags(argc, argv, L"-k", L"--show-keys");

	HKEY item = OpenRegistryItem(HKEY_CURRENT_USER, nullptr).value_or(nullptr);
	ListItemRecursive(item, show_keys);
	CloseRegistryItem(item);

	return 0;
}

extern "C" int cli_entry_regkey(int argc, wchar_t* argv[]) {
	IWDEM_CheckOrReturn(argc > 0, 1, puts("Usage: regkey <Command> [Options]"));
	if (wcscmp(argv[0], L"query") == 0) {
		return cli_entry_regkey_query(argc - 1, argv + 1);
	} else if (wcscmp(argv[0], L"list") == 0) {
		return cli_entry_regkey_list(argc - 1, argv + 1);
	} else {
		wprintf(LR"(unknown command "%S")", argv[0]);
		return 2;
	}
}