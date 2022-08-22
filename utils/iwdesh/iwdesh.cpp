#include <cstring>
#include <iostream>
#include <string_view>
#include <map>

using cliapi = int (*)(int, wchar_t*[]);

// extern "C" {
// int cli_entry_UserChoice(int, wchar_t*[]);
// int cli_entry_regkey(int, wchar_t*[]);
// int cli_entry_flush(int, wchar_t*[]);
// int cli_entry_shell(int, wchar_t*[]);
// };

int main(int argc, char* argv[]) {
	std::map<std::string_view, cliapi> entries{
		// {"UserChoice", cli_entry_UserChoice},
		// {"regkey", cli_entry_regkey},
		// {"flush", cli_entry_flush},
		// {"shell", cli_entry_shell},
	};

	if (argc < 2) {
		printf("Usage: %s <Command> [Options]", argv[0]);
		return 0;
	}

	wchar_t** wargv = nullptr;
	argc -= 2;

	if (argc > 0) {
		wargv = new wchar_t* [argc] {};
		for (int i = 0; i < argc; ++i) {
			auto size = strlen(argv[i + 2]) + 1;
			wargv[i]  = new wchar_t[size]{};
			_swprintf(wargv[i], L"%S", argv[i + 2]);
		}
	}

	int result = 0;
	if (auto it = entries.find(argv[1]); it != entries.end()) {
		auto entry = it->second;
		result	   = entry(argc, wargv);
	} else {
		printf(R"(unknown command "%s")", argv[1]);
		return 0;
	}

	if (argc > 0) {
		for (int i = 0; i < argc; ++i) {
			delete[] wargv[i];
		}
		delete[] wargv;
	}

	return result;
}