#include "lazycraft.h"

LazyCraft::LazyCraft(const char* title, int width, int height)
	: ImGuiApplication(title, width, height, 0, 0, WS_OVERLAPPED) {}

std::optional<LRESULT> LazyCraft::notify(UINT msg, WPARAM wParam, LPARAM lParam) {
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
			auto pos			 = reinterpret_cast<WINDOWPOS*>(lParam);
			pos->hwndInsertAfter = HWND_BOTTOM;
			break;
		}
	}
	return std::nullopt;
}

void LazyCraft::configure() {
	ImVec2 size{};
	LoadTextureFromFile(pd3dDevice_, R"(assets\image.png)", &texture_Background, &size);
	for (const auto& app : {
			 LR"(E:\PortableApp\VSCode\Code.exe)",
			 LR"(E:\PortableApp\Sublime Text 4\sublime_text.exe)",
			 LR"(E:\PortableApp\qimgv\qimgv.exe)",
			 LR"(E:\PortableApp\Chrome\App\chrome.exe)",
			 LR"(E:\PortableApp\PotPlayer\PotPlayer64\PotPlayerMini64.exe)",
			 LR"(E:\PortableApp\SumatraPDF\SumatraPDF.exe)",
			 LR"(E:\PortableApp\ImageGlass Kobe\ImageGlass.exe)",
			 LR"(E:\PortableApp\Bandicam\bdcam.exe)",
			 LR"(E:\PortableApp\DB Browser for SQLite\DB Browser for SQLite.exe)",
			 LR"(E:\PortableApp\ResourceHacker\ResourceHacker.exe)",
			 LR"(E:\PortableApp\FastCopy\FastCopy.exe)",
			 LR"(E:\PortableApp\MicroKMS\MicroKMS v20.09.12.exe)"
		 }) {
		auto texture = LoadIconFromModule(app);
		if (texture != nullptr) {
			// texture_QuickLaunchs.push_back(texture);
			QuickLaunchs.push_back({app, texture});
		}
	}

	ImGui::GetIO().Fonts->AddFontFromFileTTF(R"(assets\DroidSans.ttf)", 16.0f);
	ImGui::GetIO().Fonts->Build();

	RECT	   rc{};
	const auto desktop = GetDesktopWindow();
	GetWindowRect(desktop, &rc);
	SetWindowPos(hwnd_, HWND_BOTTOM, 0, 0, rc.right, rc.bottom, SWP_NOACTIVATE | SWP_SHOWWINDOW);
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

	IQuickLaunch();

	// ImGui::BeginChild("Parameter", ImVec2(400, 0));
	// ImGui::SliderInt("Slider#nItem", &nItem, 1, 16);
	// ImGui::SliderFloat("Slider#szItem", &szItem, 10, 256);
	// ImGui::SliderFloat("Slider#spacing", &spacing, 1, 128);
	// ImGui::SliderFloat("Slider#nVerticalPart", &nVerticalPart, 4, 8);
	// ImGui::ColorPicker4("ColorPicker#col_inner", reinterpret_cast<float*>(&col_inner.Value));
	// ImGui::ColorPicker4("ColorPicker#col_border",
	// reinterpret_cast<float*>(&col_border.Value)); ImGui::EndChild();

	w->BeginOrderWithinContext = -1;
	ImGui::End();
	ImGui::PopStyleVar(2);
}