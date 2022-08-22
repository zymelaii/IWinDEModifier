#include <iwindeapi/registry.h>
#include <iostream>
#include <accctrl.h>
#include <aclapi.h>
#include <securitybaseapi.h>
#include <winbase.h>
#include <winerror.h>
#include <winnt.h>

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

int ListItemRecursive(HKEY item, bool show_keys = true, bool value_only = false, int depth = 0,
					  int indent = 0) {
	auto nSubItem = QueryRegistryItemInfo<size_t>(item, RegInfo::SubItemNum);

	if (show_keys | value_only) {
		auto nKey = QueryRegistryItemInfo<size_t>(item, RegInfo::KeyNum);
		for (int j = 0; j < nKey; ++j) {
			auto KeyName = std::move(EnumRegistryKey(item, j).value_or(nullptr));
			IWDEM_CheckOrReturn(KeyName.get() != nullptr, 1);

			std::unique_ptr<uint8_t[]> ValueBytes = nullptr;
			auto [success, KeyType, ValueSize] = GetRegistryValue(item, KeyName.get(), ValueBytes);
			IWDEM_CheckOrReturn(success, 2);

			wprintf(L"%*S- %S%S%S\n",
					indent * 2,
					L"",
					KeyName.get()[0] == 0 ? L"(Default)" : KeyName.get(),
					KeyType == REG_SZ || KeyType == REG_EXPAND_SZ ? L": " : L"",
					KeyType == REG_SZ || KeyType == REG_EXPAND_SZ
						? reinterpret_cast<wchar_t*>(ValueBytes.get())
						: L"");
		}
	}

	IWDEM_CheckOrReturn(depth >= 0, 0);

	if (!value_only) {
		for (int i = 0; i < nSubItem; ++i) {
			auto SubItemName = std::move(EnumRegistrySubItem(item, i).value_or(nullptr));
			IWDEM_CheckOrReturn(SubItemName.get() != nullptr, 3);
			auto SubItem = OpenRegistryItem(item, SubItemName.get(), KEY_READ).value_or(nullptr);
			IWDEM_CheckOrReturn(SubItem != nullptr, 4);
			wprintf(L"%*S+ %S\n", indent * 2, L"", SubItemName.get());
			ListItemRecursive(SubItem, show_keys, value_only, depth - 1, indent + 1);
			CloseRegistryItem(SubItem);
		}
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
				wprintf(L"invalid registry path \"HKEY_CURRENT_USER\\%S\"\n", path);
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
	bool value_only = QueryCommandFlags(argc, argv, L"-V", L"--value-only");
	bool show_keys	= QueryCommandFlags(argc, argv, L"-k", L"--show-keys");
	bool global		= QueryCommandFlags(argc, argv, L"-g", L"--global");
	HKEY item		= nullptr;
	HKEY root		= global ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
	auto sroot		= global ? L"HKEY_LOCAL_MACHINE" : L"HKEY_CURRENT_USER";

	for (int i = 0; i < argc; ++i) {
		if (argv[i][0] == 0) continue;
		if (auto opt = OpenRegistryItem(root, argv[i], KEY_READ); opt.has_value()) {
			item = opt.value();
			wprintf(L"[%S\\%S]\n", sroot, argv[i]);
			ListItemRecursive(item, show_keys, value_only);
			CloseRegistryItem(item);
		}
	}

	return 0;
}

extern "C" int cli_entry_regkey_acl(int argc, wchar_t* argv[]) {
	const bool			 global = QueryCommandFlags(argc, argv, L"-g", L"--global");
	const bool			 remove = QueryCommandFlags(argc, argv, L"-r", L"--remove");
	const HKEY			 root	= global ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
	constexpr auto		 access = STANDARD_RIGHTS_READ /* KEY_READ */;
	const wchar_t* const aceMode[]{L"<not-used>",
								   L"<grant>",
								   L"<set>",
								   L"<deny>",
								   L"<revoke>",
								   L"<set-audit-success>",
								   L"<set-audit-failure>"};

	for (int i = 0; i < argc; ++i) {
		if (argv[i][0] == 0) continue;
		if (auto opt = OpenRegistryItem(root, argv[i], access); opt.has_value()) {
			auto				 item				   = opt.value();
			PACL				 pDACL				   = nullptr;
			PSECURITY_DESCRIPTOR pSD				   = nullptr;
			PSID				 pSidOwner			   = nullptr;
			ULONG				 nExplicitEntries	   = 0;
			PEXPLICIT_ACCESS_W	 ListOfExplicitEntries = nullptr;
			LRESULT				 result				   = ERROR_SUCCESS;

			result = GetSecurityInfo(item,
									 SE_REGISTRY_KEY,
									 DACL_SECURITY_INFORMATION,
									 nullptr,
									 nullptr,
									 &pDACL,
									 nullptr,
									 &pSD);
			IWDEM_CheckOrReturn(ERROR_SUCCESS == result, result, CloseRegistryItem(item));

			result = GetExplicitEntriesFromAclW(pDACL, &nExplicitEntries, &ListOfExplicitEntries);
			IWDEM_CheckOrReturn(ERROR_SUCCESS == result, result, CloseRegistryItem(item));

			wprintf(LR"([%S\%S]
Security Descriptor Control: 0x%04x
Number of Explicit Entries: %d
)",
					global ? L"HKEY_LOCAL_MACHINE" : L"HKEY_CURRNET_USER",
					argv[i],
					static_cast<SECURITY_DESCRIPTOR*>(pSD)->Control,
					nExplicitEntries);

			if (nExplicitEntries > 0) {
				wprintf(L"Explicit Entries List:\n");
				for (int j = 0; j < nExplicitEntries; ++j) {
					const auto& ace = ListOfExplicitEntries[j];
					wprintf(L"    [%02d] -> {"
							L"\"permissions\": 0x%08lx, "
							L"\"access_mode\": \"%S\", "
							L"\"inheritance\": 0x%02lx}\n",
							j + 1,
							ace.grfAccessPermissions,
							aceMode[ace.grfAccessMode],
							ace.grfInheritance);
				}
			}
			wprintf(L"---\n");

			if (remove) {
				TOKEN_PRIVILEGES tp{};
				LUID			 luid{};
				LookupPrivilegeValue(nullptr, SE_SECURITY_NAME, &luid);
				tp.PrivilegeCount			= 1;
				tp.Privileges[0].Luid		= luid;
				tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
				IWDEM_CheckOrReturn(
					ERROR_SUCCESS ==
						AdjustTokenPrivileges(
							item, false, &tp, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr),
					1,
					puts("[ERROR] SE_SECURITY_NAME assignment failure."));

				SetSecurityDescriptorControl(pSD, SE_DACL_PROTECTED | SE_DACL_AUTO_INHERITED, 0);
				SetSecurityDescriptorDacl(pSD, true, pDACL, true);

				{
					result = SetSecurityInfo(item,
											 SE_REGISTRY_KEY,
											 DACL_SECURITY_INFORMATION,
											 nullptr,
											 nullptr,
											 pDACL,
											 nullptr);
				}

				tp.Privileges[0].Attributes = SE_PRIVILEGE_REMOVED;
				IWDEM_CheckOrReturn(
					ERROR_SUCCESS ==
						AdjustTokenPrivileges(
							item, false, &tp, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr),
					1,
					puts("[ERROR] SE_SECURITY_NAME erasure failure."));

				if (result != ERROR_SUCCESS) {
					printf("GetLastError(): %lu\n", GetLastError());
				}
			}

			CloseRegistryItem(item, true);
		}
	}
	return 0;
}

extern "C" int cli_entry_regkey(int argc, wchar_t* argv[]) {
	IWDEM_CheckOrReturn(argc > 0, 1, puts("Usage: regkey <Command> [Options]"));
	if (wcscmp(argv[0], L"query") == 0) {
		return cli_entry_regkey_query(argc - 1, argv + 1);
	} else if (wcscmp(argv[0], L"list") == 0) {
		return cli_entry_regkey_list(argc - 1, argv + 1);
	} else if (wcscmp(argv[0], L"acl") == 0) {
		return cli_entry_regkey_acl(argc - 1, argv + 1);
	} else {
		wprintf(LR"(unknown command "%S")", argv[0]);
		return 2;
	}
}