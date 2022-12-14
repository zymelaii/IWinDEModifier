#include <filesystem>

#include "lazycraft.h"

namespace fs = std::filesystem;

using Proxy::LinkProxy;
using Proxy::VolumeProxy;

LazyCraft::LazyCraft() {
	// hdlhook	  = LoadLibrary("trap/lctrap.dll");
	// auto hook = reinterpret_cast<void (*)()>(GetProcAddress(hdlhook, "LazyCraftCBTHook"));
	// hook();

	build("LazyCraft", 0, 0, 0, 0);
}

LazyCraft::~LazyCraft() {
	DeregisterShellHookWindow(hwnd_);
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

void LazyCraft::cbshellhook(WPARAM wParam, LPARAM lParam) {
	fprintf(stderr, "WM_SHELLHOOKMESSAGE: %#llx %#llx\n", wParam, lParam);
	switch (wParam) {
		case HSHELL_APPCOMMAND: {
			auto proxy = VolumeProxy::require();
			switch (GET_APPCOMMAND_LPARAM(lParam)) {
				case APPCOMMAND_VOLUME_MUTE: {
					static bool is_mute = false;
					is_mute				= !is_mute;
					proxy->mute(is_mute);
					break;
				}
				case APPCOMMAND_VOLUME_DOWN: {
					proxy->set(proxy->get() - 1);
					break;
				}
				case APPCOMMAND_VOLUME_UP: {
					proxy->set(proxy->get() + 1);
					break;
				}
			}
			break;
		}
	}
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
	if (msg == WM_SHELLHOOKMESSAGE) {
		cbshellhook(wParam, lParam);
		return true;
	}

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

	auto	 proxy = LinkProxy::require();
	fs::path dir(LR"(C:\Users\Public\Desktop)");
	for (auto& item : fs::directory_iterator(dir)) {
		if (!item.is_regular_file()) continue;
		const auto& path = item.path();
		if (path.extension() != L".lnk") continue;
		char buffer[MAX_PATH]{};
		auto lnkpath = path.generic_wstring();

		if (proxy->query(lnkpath.data(), buffer, MAX_PATH, LinkProxy::Attribute::Source)) {
			wchar_t execpath[MAX_PATH] = {0}, workdir[MAX_PATH] = {0};
			MultiByteToWideChar(CP_ACP, 0, buffer, -1, execpath, MAX_PATH);
			if (proxy->query(lnkpath.data(), buffer, MAX_PATH, LinkProxy::Attribute::WorkDir)) {
				MultiByteToWideChar(CP_ACP, 0, buffer, -1, workdir, MAX_PATH);
			}
			auto texture = LoadIconFromModule(pd3dDevice_, execpath);
			if (texture != nullptr) {
				QuickLaunchs.push_back({path.stem().string(), execpath, workdir, texture});
			}
		}
	}

	const auto range_full = ImGui::GetIO().Fonts->GetGlyphRangesChineseFull();

	font_charge->add(R"(assets\DroidSans.ttf)", 12.0f, {'0', '9', 0})->build(pd3dDevice_);
	font_ascii->add(R"(assets\DroidSans.ttf)", 16.0f)
		->add(R"(assets\YaHei Consolas Hybrid.ttf)", 16.0f, range_full)
		->build(pd3dDevice_);
	font_full->add(R"(assets\SmileySans-Oblique.ttf)", 24.0f, range_full)->build(pd3dDevice_);

	toggle(false);

	if (!RegisterHotKey(hwnd_, 0, MOD_ALT, VK_SPACE)) {
		fprintf(stderr,
				"%s#%d RegisterHotKey(hwnd_, 0, MOD_ALT, VK_SPACE): failure for %lu\n",
				__FILE__,
				__LINE__,
				GetLastError());
	}

	if (!SetTimer(hwnd_, 0, 1000, nullptr)) {
		fprintf(stderr,
				"%s#%d SetTimer(hwnd_, 0, 1000, nullptr): failure for %lu\n",
				__FILE__,
				__LINE__,
				GetLastError());
	}

	if (!RegisterShellHookWindow(hwnd_)) {
		fprintf(stderr,
				"%s#%d RegisterShellHookWindow(hwnd_): failure for %lu\n",
				__FILE__,
				__LINE__,
				GetLastError());
	}

	{
		auto item = std::make_unique<BatteryItem>();
		item->setCapacityFont(font_charge->get())->setRelHeight(20);
		statusbar.add_util(item.release(), StatusBar::Alignment::Right);
	}

	{
		auto item = std::make_unique<DateItem>();
		item->setFont(font_ascii->get());
		statusbar.add_util(item.release(), StatusBar::Alignment::Right);
	}
}

void LazyCraft::render() {
	RECT client{};
	GetClientRect(hwnd_, &client);
	const ImRect rc(client.left, client.top, client.right, client.bottom);
	ImGui::GetBackgroundDrawList()->AddImage(texture_Background, rc.Min, rc.Max);

	const auto flag = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoFocusOnAppearing |
					  ImGuiWindowFlags_NoBringToFrontOnFocus;

	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowBgAlpha(0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	ImGui::Begin("LazyCraft.Main", nullptr, flag);

	if (statusbar.prepare()) {
		statusbar.render();
	}

	IQuickLaunch();

	if (toggle_se) {
		ISearchEngine();
	}

	ImGui::End();
	ImGui::PopStyleVar(2);
}
