#include <windows.h>
#include <stdio.h>

HHOOK	  hhook = NULL;
HINSTANCE hInst = NULL;

LRESULT CALLBACK LazyCraftCBTHookProc(int code, WPARAM wParam, LPARAM lParam) {
	printf("CBT {\"code\": %d, \"wParam\": %llu, \"lParam\": %lld}\n", code, wParam, lParam);

	if (code < 0) return CallNextHookEx(0, code, wParam, lParam);

	switch (code) {
		/* 0 */ case HCBT_MOVESIZE:
			break;
		/* 1 */ case HCBT_MINMAX:
			break;
		/* 2 */ case HCBT_QS:
			break;
		/* 3 */ case HCBT_CREATEWND:
			break;
		/* 4 */ case HCBT_DESTROYWND:
			break;
		/* 5 */ case HCBT_ACTIVATE:
			break;
		/* 6 */ case HCBT_CLICKSKIPPED:
			break;
		/* 7 */ case HCBT_KEYSKIPPED:
			break;
		/* 8 */ case HCBT_SYSCOMMAND:
			break;
		/* 9 */ case HCBT_SETFOCUS:
			break;
	}

	return CallNextHookEx(0, code, wParam, lParam);
}

void LazyCraftCBTHook() {
	hhook = SetWindowsHookEx(WH_CBT, LazyCraftCBTHookProc, hInst, 0);
}

void LazyCraftCBTUnhook() {
	UnhookWindowsHookEx(hhook);
	DWORD_PTR unused;
	UINT	  flags = SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG;
	SendMessageTimeout(HWND_BROADCAST, WM_NULL, 0, 0, flags, 1000, &unused);
}

BOOLEAN APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	hInst = hModule;

	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH: break;
		case DLL_THREAD_ATTACH: break;
		case DLL_THREAD_DETACH: break;
		case DLL_PROCESS_DETACH: break;
	}

	return TRUE;
}