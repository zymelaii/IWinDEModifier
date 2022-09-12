#include <filesystem>

#include "lazycraft.h"

namespace fs = std::filesystem;

LazyCraft::LazyCraft() {
	// hdlhook	  = LoadLibrary("trap/lctrap.dll");
	// auto hook = reinterpret_cast<void (*)()>(GetProcAddress(hdlhook, "LazyCraftCBTHook"));
	// hook();

	build("LazyCraft", 0, 0, 0, 0);
}

LazyCraft::~LazyCraft() {
	UnregisterHotKey(hwnd_, 0);
	KillTimer(hwnd_, 0);

	// auto unhook = reinterpret_cast<void (*)()>(GetProcAddress(hdlhook, "LazyCraftCBTUnhook"));
	// unhook();
	// FreeLibrary(hdlhook);
	// hdlhook = nullptr;
}

void LazyCraft::toggle(bool active) {
	RECT	   rc{};
	const auto desktop = GetDesktopWindow();
	GetWindowRect(desktop, &rc);

	HWND  hwnd = nullptr;
	DWORD from = -1, to = -1;

	SetWindowPos(hwnd_, HWND_BOTTOM, 0, 0, rc.right, rc.bottom, SWP_NOACTIVATE | SWP_SHOWWINDOW);

	if (active) {
		from = GetCurrentThreadId();
		to	 = GetWindowThreadProcessId(GetForegroundWindow(), nullptr);
		hwnd = hwnd_;
	} else {
		HWND itwnd = GetWindow(hwnd_, GW_HWNDFIRST);
		while (itwnd && !IsWindowVisible(itwnd)) {
			itwnd = GetWindow(itwnd, GW_HWNDNEXT);
		}
		from = GetWindowThreadProcessId(itwnd, nullptr);
		to	 = GetCurrentThreadId();
		hwnd = itwnd;
	}

	AttachThreadInput(from, to, true);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);
	AttachThreadInput(from, to, false);
}

LazyCraft* LazyCraft::build(const char* title, int width, int height, int x, int y) {
	ImGui_ImplWin32_EnableDpiAwareness();

	strcpy_s(class_, "IWinDEModifier::LazyCraft.MainWindow");

	WNDCLASSEX wc = {sizeof(WNDCLASSEX),
					 CS_CLASSDC,
					 ImGuiApplication::WndProc,
					 0,
					 sizeof(void*),
					 GetModuleHandle(nullptr),
					 nullptr,
					 nullptr,
					 nullptr,
					 nullptr,
					 class_,
					 nullptr};
	RegisterClassEx(&wc);

	hwnd_ = CreateWindowEx(WS_EX_NOACTIVATE,
						   wc.lpszClassName,
						   title,
						   WS_OVERLAPPED,
						   x,
						   y,
						   width,
						   height,
						   nullptr,
						   nullptr,
						   wc.hInstance,
						   this);

	if (!CreateDeviceD3D()) {
		CleanupDeviceD3D();
		UnregisterClass(class_, GetModuleHandle(nullptr));
		errno_ = 1;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui_ImplWin32_Init(hwnd_);
	ImGui_ImplDX11_Init(pd3dDevice_, pd3dDeviceContext_);

	return this;
}

std::optional<LRESULT> LazyCraft::notify(UINT msg, WPARAM wParam, LPARAM lParam) {
	static HWND prevForeground = nullptr;
	switch (msg) {
		case WM_MOUSEACTIVATE: {
			return MA_NOACTIVATE;
		}
		case WM_NCHITTEST: {
			return HTCLIENT;
		}
		case WM_NCCALCSIZE: {
			return wParam ? WVR_REDRAW : 0;
		}
		case WM_SYSCOMMAND: {
			return true;
		}
		case WM_WINDOWPOSCHANGED: {
			auto pos = reinterpret_cast<WINDOWPOS*>(lParam);
			// pos->hwndInsertAfter = HWND_BOTTOM;
			break;
		}
		case WM_KILLFOCUS: {
			if (toggle_se) {
				toggle_se = false;
				RECT client{};
				GetClientRect(hwnd_, &client);
				InvalidateRect(hwnd_, &client, false);
			}
			break;
		}
		case WM_HOTKEY: {
			toggle_se = !toggle_se;
			RECT client{};
			GetClientRect(hwnd_, &client);
			InvalidateRect(hwnd_, &client, false);
			toggle(toggle_se);
			break;
		}
		case WM_TIMER: {
			if (wParam == 0) {
				RECT client{};
				GetClientRect(hwnd_, &client);
				InvalidateRect(hwnd_, &client, false);
			}
			break;
		}
	}
	return std::nullopt;
}

void LazyCraft::configure() {
	ImVec2 size{};
	LoadTextureFromFile(pd3dDevice_, R"(assets\image.png)", &texture_Background, &size);

	LinkProxy proxy;
	fs::path  dir(LR"(C:\Users\Public\Desktop)");
	for (auto& item : fs::directory_iterator(dir)) {
		if (!item.is_regular_file()) continue;
		const auto& path = item.path();
		if (path.extension() != L".lnk") continue;
		char buffer[MAX_PATH]{};
		auto lnkpath = path.generic_wstring();
		if (proxy.query(lnkpath.data(), buffer, MAX_PATH)) {
			wchar_t execpath[MAX_PATH];
			MultiByteToWideChar(CP_ACP, 0, buffer, -1, execpath, MAX_PATH);
			auto texture = LoadIconFromModule(pd3dDevice_, execpath);
			if (texture != nullptr) {
				QuickLaunchs.push_back({path.stem().string(), execpath, texture});
			}
		}
	}

	font_charge->add(R"(assets\DroidSans.ttf)", 12.0f, {'0', '9', 0})->build(pd3dDevice_);
	font_ascii->add(R"(assets\DroidSans.ttf)", 16.0f)->build(pd3dDevice_);

	toggle(false);

	if (!RegisterHotKey(hwnd_, 0, MOD_ALT, VK_SPACE)) {
		// printf("failure as %lu\n", GetLastError());
	}
	if (!SetTimer(hwnd_, 0, 1000, nullptr)) {
		// printf("failure as %lu\n", GetLastError());
	}
}

void LazyCraft::render() {
	const auto flag = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoFocusOnAppearing |
					  ImGuiWindowFlags_NoBringToFrontOnFocus;
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowBgAlpha(0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	ImGui::Begin("Background", nullptr, flag);
	auto	   w	  = ImGui::GetCurrentWindow();
	auto	   canvas = w->DrawList;
	const auto rc	  = w->Rect();

	canvas->AddImage(texture_Background, rc.Min, rc.Max);

	IStatusBar();

	IQuickLaunch();

	if (toggle_se) {
		ISearchEngine();
	}

	w->BeginOrderWithinContext = -1;
	ImGui::End();
	ImGui::PopStyleVar(2);
}
