#include "backend.h"
#include <memory>
#include <map>
#include <string_view>
#include <iostream>

class IWinDEModifierApp : public ImGuiApplication {
private:   //!< Configuration
	//! flags
	std::map<std::string_view, bool> flags{};
	bool							 open_MainMenuBar = false;
	bool							 open_About		  = false;
	bool							 open_Explorer	  = false;
	//! status
	int	   RegEntrySelectionId = -1;
	ImVec2 size_MainMenuBar{0, 0};
	ImVec2 size_Explorer{0, 0};
	//! config
	const SIZE WindowsMinTrack{640, 400};
	const struct {
		const char* entry;
		int			id;
	} RegistryEntries[5]{
		{"HKEY_ROOT_CLASS", 0},
		{"HKEY_CURRENT_USER", 1},
		{"HKEY_LOCAL_MACHINE", 2},
		{"HKEY_USERS", 3},
		{"HKEY_CURRENT_CONFIG", 4},
	};

public:	  //!< Components
	bool ShowMainMenuBar() {
		if (!ImGui::BeginMainMenuBar()) return false;

		if (ImGui::BeginMenu("File(F)")) {
			if (ImGui::MenuItem("New", "Ctrl+N")) {}
			if (ImGui::MenuItem("Open", "Ctrl+O")) {}
			ImGui::Separator();
			if (ImGui::MenuItem("Save", "Ctrl+S")) {}
			if (ImGui::MenuItem("SaveAs...", "Ctrl+Shift+S")) {}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit(E)")) {
			if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
			if (ImGui::MenuItem("Redo", "Ctrl+Y")) {}
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "Ctrl+X")) {}
			if (ImGui::MenuItem("Copy", "Ctrl+C")) {}
			if (ImGui::MenuItem("Paste", "Ctrl+V")) {}
			ImGui::Separator();
			if (ImGui::MenuItem("Search", "Ctrl+F")) {}
			if (ImGui::MenuItem("Replace", "Ctrl+H")) {}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View(V)")) {
			ImGui::Checkbox("Toggle Sidebar", &open_Explorer);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help(H)")) {
			if (ImGui::MenuItem("About")) {
				open_About = true;
			}
			ImGui::EndMenu();
		}

		size_MainMenuBar = ImGui::GetWindowSize();
		ImGui::EndMainMenuBar();

		return true;
	}

	void ShowAbout() {
		ImGui::SetNextWindowFocus();
		auto flag = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar |
					ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;
		ImGui::Begin("About", &open_About, flag);
		ImGui::TextColored(ImVec4(0.00f, 0.47f, 0.84f, 1.00f), "IWinDEModifier");
		ImGui::Separator();
		ImGui::BulletText("Version: %d.%d.%d", 0, 1, 3);
		ImGui::BulletText("Author: %s", "zymelaii");
		ImGui::End();
	}

	void ShowExplorer() {
		auto flag = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
					ImGuiWindowFlags_NoBringToFrontOnFocus;
		auto client = ImGui::GetIO().DisplaySize;
		auto offset = open_MainMenuBar ? size_MainMenuBar.y : 0;
		if (flags["OnExplorerInit"]) {
			size_Explorer.x			= 200;
			flags["OnExplorerInit"] = false;
		}
		size_Explorer.y = client.y - offset;
		ImGui::SetNextWindowSize(size_Explorer);
		ImGui::SetNextWindowBgAlpha(0.6);
		ImGui::SetNextWindowSizeConstraints(ImVec2(100, client.y), ImVec2(320, client.y));
		ImGui::SetNextWindowPos(ImVec2(0, offset));
		ImGui::Begin("Explorer", &open_Explorer, flag);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, ImGui::GetFontSize() * 0.5);
		ImGui::Text("Explorer");
		ImGui::Separator();
		if (ImGui::CollapsingHeader("Registry", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Indent(12.00f);
			for (auto [entry, id] : RegistryEntries) {
				if (auto active = RegEntrySelectionId == id; ImGui::RadioButton(entry, active)) {
					RegEntrySelectionId = active ? -1 : id;
				}
			}
			ImGui::Unindent();
		}
		size_Explorer = ImGui::GetWindowSize();
		ImGui::PopStyleVar();
		ImGui::End();
	}

	void ShowMainView() {
		auto flag = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
					ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus;
		ImVec2 size = ImGui::GetIO().DisplaySize;
		ImVec2 pos{0, 0};
		if (open_Explorer) {
			size.x = size.x - size_Explorer.x;
			pos.x  = size_Explorer.x;
		}
		if (open_MainMenuBar) {
			pos.y += size_MainMenuBar.y;
		}
		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(size);
		ImGui::SetNextWindowBgAlpha(0.6);

		ImGui::Begin("MainView", NULL, flag);
		if (ImGui::IsWindowHovered()) {
			ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
			ImGui::SameLine();
		}
		ImGui::Text("Hello World!");
		ImGui::ColorEdit3("Backgound Color", reinterpret_cast<float*>(&this->backgroundColor));
		float color = 0;
		ImGui::End();
	}

public:	  //!< Main Program
	IWinDEModifierApp(const char* title, int width, int height)
		: ImGuiApplication(title, width, height) {}

	std::optional<LRESULT> notify(UINT msg, WPARAM wParam, LPARAM lParam) override {
		if (msg == WM_GETMINMAXINFO) {
			MINMAXINFO* info = reinterpret_cast<MINMAXINFO*>(lParam);
			RECT		window, client;
			GetWindowRect(hwnd_, &window);
			GetClientRect(hwnd_, &client);
			SIZE nc{window.right - window.left - client.right,
					window.bottom - window.top - client.bottom};
			info->ptMinTrackSize.x = WindowsMinTrack.cx + nc.cx;
			info->ptMinTrackSize.y = WindowsMinTrack.cy + nc.cy;
		}
		return std::nullopt;
	}

	void configure() override {
		ImGuiApplication::configure();
		flags["FirstToggleExplorer"] = false;
		flags["OnExplorerInit"]		 = true;
		ImGui::GetIO().Fonts->AddFontFromFileTTF(R"(..\assets\DroidSans.ttf)", 16.0f);
	}

	void prepare() override {
		ImGuiApplication::prepare();
		ImGuiIO& io = ImGui::GetIO();
		if (auto key = ImGui::GetKeyData('B'); key->Down && io.KeyAlt && io.KeyCtrl) {
			if (!flags["FirstToggleExplorer"]) {
				open_Explorer				 = !open_Explorer;
				flags["FirstToggleExplorer"] = true;
			} else if (key->DownDuration > 0.6) {
				auto n = static_cast<int>((key->DownDuration - 0.6) / 0.1);
				if (fabs(key->DownDuration - 0.6 - n * 0.1) < 0.03) {
					open_Explorer = !open_Explorer;
				}
			}
		} else {
			flags["FirstToggleExplorer"] = false;
		}
	}

	void render() override {
		open_MainMenuBar = ShowMainMenuBar();

		if (open_About) {
			ShowAbout();
		}

		if (open_Explorer) {
			ShowExplorer();
		}

		ShowMainView();
	}
};

int main(int argc, char* argv[]) {
	auto app = std::make_unique<IWinDEModifierApp>("IWinDEModifier", 800, 600);
	return app->exec();
}