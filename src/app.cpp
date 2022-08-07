#include "backend.h"
#include "imgui/imgui.h"
#include <memory>
#include <stdio.h>
#include <time.h>

class IWinDEModifierApp : public ImGuiApplication {
public:
	IWinDEModifierApp(const char* title, int width, int height)
		: ImGuiApplication(title, width, height) {}

	void configure() override {
		ImGuiApplication::configure();
		ImGui::GetIO().Fonts->AddFontFromFileTTF(
			R"(E:\DesktopMap\dev\IWinDEModifier\assets\DroidSans.ttf)", 16.0f);
	}

	void render() override {
		ImGuiIO&	   io			 = ImGui::GetIO();
		static bool	   show_leftpane = false;
		static clock_t timestamp	 = -1;
		static bool	   show_about	 = false;
		static ImVec2  szmenu{0, 0};
		static ImVec2  szleftpane{0, 0};

		RECT client{};
		GetClientRect(hwnd_, &client);
		int width  = client.right;
		int height = client.bottom;

		if (io.KeyCtrl && io.KeyAlt && io.KeysDown['B']) {
			if (timestamp == -1) {
				timestamp = static_cast<clock_t>(clock() * 1000.0 / CLOCKS_PER_SEC);
			}
		}
		if (!(io.KeyCtrl && io.KeyAlt && io.KeysDown['B']) && timestamp != -1) {
			auto now = static_cast<clock_t>(clock() * 1000.0 / CLOCKS_PER_SEC);
			if (now - timestamp > 10) {
				show_leftpane = !show_leftpane;
			}
			timestamp = -1;
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::GetCurrentContext()->NavWindowingToggleLayer = true;

		//! Application Render Program
		if (ImGui::BeginMainMenuBar()) {
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
				ImGui::Checkbox("Toggle Sidebar", &show_leftpane);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Help(H)")) {
				if (ImGui::MenuItem("About")) {
					show_about = true;
				}
				ImGui::EndMenu();
			}
			szmenu = ImGui::GetWindowSize();
			ImGui::EndMainMenuBar();
		}

		if (show_about) {
			ImGui::SetNextWindowFocus();
			ImGui::Begin("About",
						 &show_about,
						 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar |
							 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);

			ImGui::TextColored(ImVec4(0.00f, 0.47f, 0.84f, 1.00f), "IWinDEModifier");
			ImGui::Separator();
			ImGui::BulletText("Version: %d.%d.%d", 0, 1, 3);
			ImGui::BulletText("Author: %s", "zymelaii");

			ImGui::End();
		}

		if (show_leftpane) {
			static int reg_root_id = 0;
			if (szleftpane.y == 0) {
				szleftpane.x = 160;
				ImGui::SetNextWindowSize(szleftpane);
			}
			szleftpane.y = height - szmenu.y;
			ImGui::SetNextWindowBgAlpha(0.6);
			ImGui::SetNextWindowPos(ImVec2(0, szmenu.y));
			ImGui::SetNextWindowSizeConstraints(ImVec2(100, szleftpane.y),
												ImVec2(240, szleftpane.y));
			ImGui::Begin("Side Pane", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
			szleftpane = ImGui::GetWindowSize();
			ImGui::Text("Explorer");
			ImGui::Separator();
			if (ImGui::CollapsingHeader("Registry", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Indent(12.00f);
				if (ImGui::RadioButton("HKEY_ROOT_CLASS", reg_root_id == 0)) {
					reg_root_id = 0;
				}
				if (ImGui::RadioButton("HKEY_CURRENT_USER", reg_root_id == 1)) {
					reg_root_id = 1;
				}
				if (ImGui::RadioButton("HKEY_LOCAL_MACHINE", reg_root_id == 2)) {
					reg_root_id = 2;
				}
				if (ImGui::RadioButton("HKEY_USERS", reg_root_id == 3)) {
					reg_root_id = 3;
				}
				if (ImGui::RadioButton("HKEY_CURRENT_CONFIG", reg_root_id == 4)) {
					reg_root_id = 4;
				}
				ImGui::Unindent();
			}
			ImGui::End();
		}

		ImGui::SetNextWindowSize(
			ImVec2(width - (show_leftpane ? szleftpane.x : 0), height - szmenu.y));
		ImGui::SetNextWindowPos(ImVec2(show_leftpane ? szleftpane.x : 0, szmenu.y));
		ImGui::SetNextWindowBgAlpha(0.6);
		ImGui::Begin(
			"Default Menu",
			NULL,
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

		if (ImGui::IsWindowHovered()) {
			ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
			ImGui::SameLine();
		}
		ImGui::Text("Hello World!");

		ImGui::ColorEdit3("Backgound Color", reinterpret_cast<float*>(&this->backgroundColor));

		ImGui::End();
	}
};

int main(int argc, char* argv[]) {
	auto app = std::make_unique<IWinDEModifierApp>("IWinDEModifier", 800, 600);
	return app->exec();
}