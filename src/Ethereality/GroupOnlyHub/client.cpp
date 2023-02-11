#include <iostream>
#include <windowsx.h>

#include <share/utils/proxy/fontproxy.h>

#include "GroupOnlyHub.h"

class Ethereality : public ImGuiApplication {
private:
	std::unique_ptr<Proxy::FontProxy> font_ascii = Proxy::FontProxy::require();
	GroupOnlyHub*					  chathub	 = nullptr;

public:
	Ethereality(const char* title, ChatHub* hub)
		: chathub(dynamic_cast<GroupOnlyHub*>(hub)) {
		ImGui_ImplWin32_EnableDpiAwareness();
		float width	 = 480.00;
		float height = 600;
		int	  x		 = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
		int	  y		 = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
		build(title, width, height, x, y);
	}

	Ethereality* build(const char* title, int width, int height, int x, int y) override {
		sprintf(class_, "##Ethereality::Chathub[%s]", title);

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

		hwnd_ = CreateWindowEx(WS_EX_APPWINDOW,
							   wc.lpszClassName,
							   title,
							   WS_POPUP,
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

	std::optional<LRESULT> notify(UINT msg, WPARAM wParam, LPARAM lParam) override {
		if (msg == WM_NCHITTEST) {
			POINT pos{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
			ScreenToClient(hwnd_, &pos);
			RECT rect;
			GetClientRect(hwnd_, &rect);
			float TitleBarHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
			if (pos.y < TitleBarHeight) return HTCAPTION;
			if (pos.y > rect.bottom - 8) {
				if (pos.x < rect.left + 8) return HTBOTTOMLEFT;
				if (pos.x > rect.right - 8) return HTBOTTOMRIGHT;
				return HTBOTTOM;
			}
			if (pos.y > TitleBarHeight) {
				if (pos.x < rect.left + 8) return HTLEFT;
				if (pos.x > rect.right - 8) return HTRIGHT;
			}
			return HTCLIENT;
		}
		if (msg == WM_GETMINMAXINFO) {
			auto info = reinterpret_cast<MINMAXINFO*>(lParam);

			auto min_size		 = chathub->getMinSizeConstraint();
			info->ptMinTrackSize = {(LONG)min_size.x, (LONG)min_size.y};

			ImVec2 ChatHubMargin{4.00, 4.00};
			float  TitleBarHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
			info->ptMaxTrackSize  = {(LONG)(GetSystemMetrics(SM_CXSCREEN) / 2),
									 (LONG)(720.00f + ChatHubMargin.y * 2 + TitleBarHeight)};

			return 0;
		}
		if (msg == WM_KEYDOWN && wParam == VK_ESCAPE) {
			ShowWindow(hwnd_, SW_HIDE);
			PostQuitMessage(0);
			return 0;
		}
		return std::nullopt;
	}

	void configure() override {
		auto CN_glyph = ImGui::GetIO().Fonts->GetGlyphRangesChineseFull();
		font_ascii->add(R"(assets\DroidSans.ttf)", 20.00)
			->add(R"(assets\YaHei Consolas Hybrid.ttf)", 20.00, CN_glyph)
			->build(pd3dDevice_);
	}

	void render() override {
		const auto& style = ImGui::GetStyle();
		const auto& io	  = ImGui::GetIO();

		float  TitleBarHeight = ImGui::GetFontSize() + style.FramePadding.y * 2.0f;
		ImVec2 MinSize		  = chathub->getMinSizeConstraint();
		ImVec2 WindowPadding  = style.WindowPadding;
		ImVec2 ChatHubMargin{4.00, 4.00};
		ImVec2 WindowMinSize{MinSize.x + WindowPadding.x + ChatHubMargin.x * 2,
							 MinSize.y + ChatHubMargin.y * 2 + TitleBarHeight};
		ImVec2 ChatHubPanelSize{io.DisplaySize.x - ChatHubMargin.x * 2,
								io.DisplaySize.y - ChatHubMargin.y * 2 - TitleBarHeight};

		ImGui::SetNextWindowSizeConstraints(WindowMinSize, io.DisplaySize);
		ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);
		ImGui::SetNextWindowPos({0, 0}, ImGuiCond_Always);
		ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed,
							  ImGui::GetColorU32(ImGuiCol_TitleBgActive));
		ImGuiWindowFlags flag = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove |
								ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
								ImGuiWindowFlags_NoCollapse;
		if (ImGui::Begin("Ethereality ChatHub##ChatHub.GroupOnlyHub", nullptr, flag)) {
			ImGui::PushFont(font_ascii->get());
			chathub->render(ChatHubMargin, ChatHubPanelSize);
			ImGui::PopFont();
		}
		ImGui::End();
		ImGui::PopStyleColor();
	}
};

int main(int argc, char* argv[]) {
	GroupOnlyHub hub;
	if (argc > 1) hub.login_as(argv[1]);
	auto app = std::make_unique<Ethereality>("Ethereality Sample", &hub);

	return app->exec();
}

