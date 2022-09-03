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

// startup programs
/*!
 * + SOFTWARE\Microsoft\Windows\CurrentVersion
 *   + Run
 *     - <ProgId...>: <cmdline>
 * + Explorer\StartupApproved
 *   + Run
 *     - <ProgId...> -> BINARY: <Startup Flags>
 *   + StartupFolder
 *     - <SHFolder...> -> BINARY: <Startup Flags>
 */

#include <windows.h>

void SetDirToModulePath() {
	wchar_t path[MAX_PATH]{};
	DWORD	dwfile			   = GetModuleFileNameW(nullptr, path, MAX_PATH);
	wcsrchr(path, L"\\"[0])[0] = 0;
	SetCurrentDirectoryW(path);
}

void LaunchOnStartup() {
	const wchar_t* startup[]{
		LR"("C:\Program Files (x86)\Parblo\parbloDriver.exe")",
		LR"("E:\PortableApp\Everything\Everything.exe" -startup)",
		LR"("E:\PortableApp\Snipaste\Snipaste.exe")",
		LR"("E:\PortableApp\Console\Console.exe")",
	};
	for (auto& prog : startup) {
		STARTUPINFOW		startinfo{};
		PROCESS_INFORMATION procinfo{};
		startinfo.wShowWindow = SW_HIDE | SW_FORCEMINIMIZE;
		startinfo.dwFlags	  = STARTF_USESHOWWINDOW;
		startinfo.cb		  = sizeof(STARTUPINFOW);
		wchar_t cmdline[MAX_PATH]{};
		wcscpy_s(cmdline, prog);
		CreateProcessW(nullptr,
					   cmdline,
					   nullptr,
					   nullptr,
					   false,
					   CREATE_NEW_PROCESS_GROUP | IDLE_PRIORITY_CLASS,
					   nullptr,
					   nullptr,
					   &startinfo,
					   &procinfo);
		WaitForInputIdle(procinfo.hProcess, INFINITE);
	}
}

#include <thread>
int main(int argc, char const* argv[]) {
	if (FindWindow("IWinDEModifier::LazyCraft.MainWindow", nullptr) != nullptr) {
		return 0;
	}

	SetDirToModulePath();
	std::thread(LaunchOnStartup).detach();

	auto app = std::make_unique<LazyCraft>();
	return app->lazy_exec(false);
}