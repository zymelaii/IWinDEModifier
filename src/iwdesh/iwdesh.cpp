#include "iwdesh.h"
#include <windows.h>
#include <vector>
#include <string>
#include <iostream>
#include <string_view>
#include <fstream>
#include <filesystem>
#include <map>
#include <memory>

namespace fs = std::filesystem;

using scope_dict = std::map<std::string_view, std::string_view>;
using entry_dict = std::map<std::string_view, fn_iwdesh_entry>;	  //!< designed for stream mode

static bool		  refresh = false;
static scope_dict g_scope{};
static entry_dict g_entries{};

auto cvts2w(const char* str) -> std::unique_ptr<wchar_t[]> {
	const char* loc = setlocale(LC_ALL, NULL);
	setlocale(LC_ALL, "chs");

	size_t len = strlen(str);

	auto wstr = std::make_unique<wchar_t[]>(len + 1);
	mbstowcs_s(nullptr, wstr.get(), len + 1, str, _TRUNCATE);

	setlocale(LC_ALL, loc);

	return std::move(wstr);
}

auto GetHomePath() -> fs::path {
	char buffer[MAX_PATH]{};
	GetModuleFileName(NULL, buffer, MAX_PATH);
	return fs::path(buffer).remove_filename();
}

auto ReadEntryList() -> std::unique_ptr<std::vector<std::pair<std::string, std::string>>> {
	const auto&	 home = GetHomePath();
	std::fstream entrycnf(home.string() + "entries", std::ios::in);

	using value_type	   = std::pair<std::string, std::string>;
	auto		entry_list = std::make_unique<std::vector<value_type>>();
	std::string line{};

	while (std::getline(entrycnf, line)) {
		if (auto it = line.find("::"); it != line.npos) {
			auto item = std::make_pair(line.substr(0, it), line.substr(it + 2));
			if (item.first.empty() || item.second.empty()) continue;
			entry_list->emplace_back(std::move(item));
		}
	}

	entrycnf.close();

	return std::move(entry_list);
}

void ExportEntryList() {
	const auto&	 home = GetHomePath();
	std::fstream entrycnf(home.string() + "entries", std::ios::out | std::ios::trunc);

	for (const auto &item : g_scope) {
		entrycnf << item.second << "::" << item.first << "\n";
	}

	entrycnf.close();
}

auto GetEntry(const char* scope, const char* entry, HMODULE* pinst) -> fn_iwdesh_entry {
	if (!pinst || !scope || !entry) return nullptr;

	const auto plugins = fs::path(GetHomePath().string() + "plugins\\");
	const auto plugin  = fs::path(plugins.string() + scope + ".iwdesh-plugin");
	const auto prefix  = std::string("iwdesh_entry_");

	if (!fs::exists(plugin)) return nullptr;

	*pinst = LoadLibrary(plugin.string().c_str());
	if (!*pinst) return nullptr;

	auto ptr = GetProcAddress(*pinst, (prefix + entry).c_str());
	return reinterpret_cast<fn_iwdesh_entry>(ptr);
}

auto SearchEntry(const char* entry, HMODULE* pinst) -> fn_iwdesh_entry {
	if (!pinst || !entry) return nullptr;

	if (auto it = g_scope.find(entry); it != g_scope.end()) {
		auto fn_entry = GetEntry(it->second.data(), entry, pinst);
		if (fn_entry) return fn_entry;
	}

	for (const auto	 plugins = fs::path(GetHomePath().string() + "plugins");
		 const auto& item : fs::directory_iterator(plugins)) {
		if (item.is_directory()) continue;
		if (item.path().extension() != ".iwdesh-plugin") continue;
		const auto& scope	 = item.path().filename().replace_extension().string();
		auto		fn_entry = GetEntry(scope.c_str(), entry, pinst);
		if (fn_entry) {
			refresh		   = true;
			g_scope[entry] = scope;
			return fn_entry;
		}
	}

	return nullptr;
}

struct PluginModule {
	HMODULE instance = nullptr;
	~PluginModule() {
		if (instance) {
			FreeLibrary(instance);
		}
	}
};

int main(int argc, char* argv[]) {
	if (argc == 1) {
		printf("Usage: %s <Command> [Options]", argv[0]);
		return 0;
	}

	auto entry_list = ReadEntryList();

	for (const auto& item : *entry_list.get()) {
		g_scope[item.second]   = item.first;
		g_entries[item.second] = nullptr;
	}

	PluginModule module{};
	const auto	 entryid = argv[1];
	auto		 entry	 = SearchEntry(entryid, &module.instance);
	if (!entry) {
		printf("[ERROR] \"%s\" not installed\n", entryid);
		return -1;
	}

	std::unique_ptr<wchar_t*[]> wargv = nullptr;
	if (argc -= 2; argc > 0) {
		wargv = std::make_unique<wchar_t*[]>(argc);
		for (int i = 0; i < argc; ++i) {
			wargv[i] = cvts2w(argv[i + 2]).release();
		}
	}

	int result = entry(argc, wargv.get());

	for (int i = 0; i < argc; ++i) {
		delete[] wargv[i];
	}

	if (refresh) {
		ExportEntryList();
	}

	return result;
}