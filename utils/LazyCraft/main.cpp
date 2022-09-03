#include <cstddef>
#include <cmath>
#include <algorithm>
#include <regex>
#include <compare>
#include <any>
#include <memory>
#include <map>
#include <set>
#include <array>
#include <string>
#include <iostream>

#include "lazycraft.h"

// register as winlogon
/*!
 * # Original
 * + SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon
 *   - Shell: explorer.exe
 * # Modified 
 * + SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon
 *   - Shell: lazycraft.exe
 */

#include <windows.h>

void SetDirToModulePath() {
	wchar_t path[MAX_PATH]{};
	DWORD dwfile = GetModuleFileNameW(nullptr, path, MAX_PATH);
	wcsrchr(path, L"\\"[0])[0] = 0;
	SetCurrentDirectoryW(path);
}

int main(int argc, char const* argv[]) {
	SetDirToModulePath();
	auto app = std::make_unique<LazyCraft>("LazyCraft", 0, 0);
	return app->lazy_exec(false);
}