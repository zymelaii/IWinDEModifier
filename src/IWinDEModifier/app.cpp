#include <stddef.h>
#include <algorithm>
#include <regex>
#include <compare>
#include <any>
#include <memory>
#include <map>
#include <set>
#include <string>
#include <iostream>

#include <share/ui/backend.h>
#include <share/utils/texture.h>
#include <iwindeapi/registry.h>

#include "widgets.h"

class IWinDEModifierApp : public ImGuiApplication {
private:   //!< Configuration
	//! flags
	std::map<std::string, bool> flags{};
	bool						open_MainMenuBar = true;
	bool						open_About		 = false;
	bool						open_Explorer	 = true;
	bool						open_Settings	 = false;
	//! status
	enum class AssocType { None, Irregular, User, System, App };
	bool										 HandlePublicRegistry = false;
	bool										 RegistryScopeChanged = true;
	int											 CurrentSettingsOn	  = 0;
	AssocType									 AssocAppType		  = AssocType::None;
	std::unique_ptr<std::string>				 AssocApp			  = nullptr;
	size_t										 nEmptyKey			  = 0;
	ImVec2										 size_MainMenuBar{0, 0};
	ImVec2										 size_Explorer{100, 0};
	ImVec2										 size_ExplorerMinTrack{0, 0};
	ImFont*										 FontClear = nullptr;
	ImFont*										 FontSmall = nullptr;
	std::set<std::string>						 AssocApps{};
	std::map<std::string, std::set<std::string>> AppSupportTypes{};
	//! config
	const ImVec2   StatusBarPadding{12, 4};
	const wchar_t* ExtRegItemPath = LR"(SOFTWARE\Classes)";
	const SIZE	   WindowsMinTrack{640, 400};
	const float	   InitialExplorerWidth = 200;
	//! resource
	ID3D11ShaderResourceView* texture_Background = nullptr;
	ImVec2					  size_Background{};

public:	  //!< Components
	void ShowBackground() {
		auto flag = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoFocusOnAppearing |
					ImGuiWindowFlags_NoBringToFrontOnFocus;
		ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowBgAlpha(0);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

		ImGui::Begin("Background", nullptr, flag);
		ImGui::GetCurrentWindow()->BeginOrderWithinContext = -1;
		ImGui::Image(texture_Background, ImGui::GetContentRegionAvail());
		ImGui::End();

		ImGui::PopStyleVar(2);
	}

	void ShowMainMenuBar() {
		if (!ImGui::BeginMainMenuBar()) {
			open_MainMenuBar = false;
			return;
		}

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
			if (ImGui::MenuItem("Settings")) {
				open_Settings = true;
			}
			ImGui::Separator();
			if (ImGui::MenuItem("About")) {
				open_About = true;
			}
			ImGui::EndMenu();
		}

		if (flags["DisplayFPS"]) {
			auto menubar = ImGui::GetCurrentWindow();
			ImGui::AlignTextToFramePadding();
			char FPS[64]{};
			sprintf(FPS, "FPS: %.2f", ImGui::GetIO().Framerate);
			ImGui::ItemSize(ImVec2(menubar->Size.x - menubar->DC.CursorPos.x -
									   ImGui::CalcTextSize(FPS).x - ImGui::GetFontSize(),
								   menubar->Size.y));
			size_MainMenuBar = menubar->Size;
			ImGui::TextColored(ImVec4(1.00, 0.00, 0.00, 1.00), "%s", FPS);
		}

		ImGui::EndMainMenuBar();
	}

	void ShowSettings() {
		auto flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
		ImGui::SetNextWindowFocus();
		ImGui::SetNextWindowSize(ImVec2(480, 400));
		ImGui::Begin("IWinDEModifier Settings", &open_Settings, flags);
		ImGui::BeginChild("Options", ImVec2(100, 0), true);
		if (ImGui::Selectable(
				"General", CurrentSettingsOn == 0, ImGuiSelectableFlags_AllowDoubleClick)) {
			if (ImGui::GetIO().MouseClickedCount[0] == 2) {	  //! left button double click
				CurrentSettingsOn = 0;
				// TODO
			} else {
				CurrentSettingsOn = CurrentSettingsOn == 0 ? -1 : 0;
			}
		}
		ImGui::EndChild();
		ImGui::SameLine();
		ImGui::BeginChild("SettingsView", ImVec2(0, 0));
		if (ImGui::BeginTabBar("Anonymous.SettingsTabs", ImGuiTabBarFlags_None)) {
			switch (CurrentSettingsOn) {
				case 0: {
					if (ImGui::BeginTabItem("General")) {
						ImGui::Checkbox("Display FPS", &this->flags.find("DisplayFPS")->second);
						ImGui::Checkbox("Show empty registry items",
										&this->flags.find("ShowEmptyRegistryItems")->second);
						ImGui::Checkbox("Expand key values on default ",
										&this->flags.find("ExpandKeyValuesOnDefault")->second);
						ImGui::EndTabItem();
					}
					break;
				}
			}
			ImGui::EndTabBar();
		}
		ImGui::EndChild();
		ImGui::End();
	}

	void ShowAbout() {
		ImGui::SetNextWindowFocus();
		auto flag = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar |
					ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;

		ImGui::Begin("About", &open_About, flag);
		ImGui::TextColored(ImVec4(0.00f, 0.47f, 0.84f, 1.00f), "IWinDEModifier");
		ImGui::Separator();
		ImGui::BulletText("Version: %d.%d.%d", 0, 2, 5);
		ImGui::BulletText("Author: %s", "zymelaii");
		auto nemptyval = 0;
		ImGui::End();
	}

	void ShowExplorer() {
		constexpr auto flag = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
							  ImGuiWindowFlags_NoBringToFrontOnFocus;
		const auto client = ImGui::GetIO().DisplaySize;
		auto	   offset = open_MainMenuBar ? size_MainMenuBar.y : 0;
		if (flags["OnExplorerInit"]) {
			size_Explorer.x			= InitialExplorerWidth;
			flags["OnExplorerInit"] = false;
		}
		size_Explorer.y = client.y - offset + ImGui::GetStyle().WindowBorderSize;
		ImGui::SetNextWindowSize(size_Explorer);
		ImGui::SetNextWindowBgAlpha(0.72);
		ImGui::SetNextWindowSizeConstraints(
			ImVec2(size_ExplorerMinTrack.x + ImGui::GetStyle().WindowPadding.x * 2,
				   size_Explorer.y),
			ImVec2(client.x / 2, size_Explorer.y));
		ImGui::SetNextWindowPos(ImVec2(0, offset - ImGui::GetStyle().WindowBorderSize));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, ImGui::GetFontSize() * 0.5);
		ImGui::Begin("Explorer", &open_Explorer, flag);
		if (ImGui::Switch(
				"#RegsitryScopeSwitch", "Explorer", "Private", "Public", &HandlePublicRegistry)) {
			RegistryScopeChanged = true;
		}
		ImGui::Separator();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(0, 0, 0, 0));
		ImGui::BeginChild("#Explorer", ImVec2(0, 0), false, flag);
		if (ImGui::CollapsingHeader("AssocApplication", ImGuiTreeNodeFlags_DefaultOpen)) {
			constexpr auto leaf_flag  = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
			constexpr auto regex_flag = std::regex::ECMAScript | std::regex::icase;
			bool		   pressed = false, is_open = false;
			for (auto& app : AssocApps) {
				is_open = ImGui::TreeNode(app.c_str(), &pressed);
				if (!is_open) continue;
				if (pressed) {
					flags["ViewAssocApp"]		  = true;
					flags["ViewAssocAppDetailed"] = false;
					if (app == "<System>") {
						AssocAppType = AssocType::System;
					} else if (app == "<User>") {
						AssocAppType = AssocType::User;
					} else if (app == "<Irregular>") {
						AssocAppType = AssocType::Irregular;
					} else {
						AssocAppType = AssocType::App;
						AssocApp	 = std::make_unique<std::string>(app);
					}
				}
				for (auto& assoc : AppSupportTypes[app]) {
					is_open = ImGui::TreeNode(assoc.c_str(), &pressed, leaf_flag);
					if (!is_open) continue;
					if (pressed) {
						flags["ViewAssocApp"]		  = false;
						flags["ViewAssocAppDetailed"] = true;
					}
					ImGui::TreePop();
				}
				ImGui::TreePop();
			}
		}
		ImGui::EndChild();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
		size_Explorer = ImGui::GetWindowSize();
		ImGui::End();

		ImGui::PopStyleVar();
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
			size.y -= size_MainMenuBar.y;
			pos.y += size_MainMenuBar.y;
		}

		ImVec2 size_StatusBar{size.x, ImGui::GetFontSize() + StatusBarPadding.y * 2};
		size.y -= size_StatusBar.y;

		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(size);
		ImGui::SetNextWindowBgAlpha(0.88);

		// ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(0, 0, 0, 0));
		// ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
		// 					ImVec2(0, ImGui::GetStyle().WindowPadding.y));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
		ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0);
		ImGui::Begin("MainView", nullptr, flag | ImGuiWindowFlags_NoScrollbar);
		ImGui::BeginChild("MainViewPanel");

		bool flag_List = flags["ViewAssocApp"] | flags["ViewAssocAppDetailed"];
		if (flags["ViewAssocApp"]) {
			switch (AssocAppType) {
				case AssocType::System: {
					for (auto& FileExt : AppSupportTypes["<System>"]) {
						ImGui::AssocViewerItem((FileExt + "File").c_str());
					}
					break;
				}
				case AssocType::User: {
					for (auto& FileExt : AppSupportTypes["<User>"]) {
						ImGui::AssocViewerItem((FileExt + "_auto_file").c_str());
					}
					break;
				}
				case AssocType::Irregular: {
					for (auto& App : AppSupportTypes["<Irregular>"]) {
						ImGui::AssocViewerItem(App.c_str());
					}
					break;
				}
				case AssocType::App: {
					const auto& App = *AssocApp.get();
					for (auto& Item : AppSupportTypes[App]) {
						ImGui::AssocViewerItem((App + "." + Item).c_str());
					}
					break;
				}
				default: break;
			}
		} else if (flags["ViewAssocAppDetailed"]) {
			// TODO
		}

		ImGui::EndChild();
		ImGui::End();
		ImGui::PopStyleVar(2);
		// ImGui::PopStyleColor();

		ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y + size.y));
		ImGui::SetNextWindowSize(size_StatusBar);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.00, 0.48, 0.80, 1.00));
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00, 1.00, 1.00, 1.00));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, StatusBarPadding);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
		ImGui::Begin("RegistryStatusBar", nullptr, flag);
		// TODO
		ImGui::End();
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(2);
	}

public:	  //!< Main Program
	IWinDEModifierApp(const char* title, int width, int height) {
		build(title, width, height, -1, -1);
	}

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
		flags["FirstToggleExplorer"]	  = false;
		flags["OnExplorerInit"]			  = true;
		flags["DisplayFPS"]				  = true;
		flags["ShowEmptyRegistryItems"]	  = false;
		flags["ExpandKeyValuesOnDefault"] = true;
		flags["ViewAssocApp"]			  = false;
		flags["ViewAssocAppDetailed"]	  = false;

		auto FontSmall =
			ImGui::GetIO().Fonts->AddFontFromFileTTF(R"(assets\DroidSans.ttf)", 16.0f);
		ImGui::GetIO().Fonts->Build();

		LoadTextureFromFile(
			pd3dDevice_, R"(assets\image.png)", &texture_Background, &size_Background);
	}

	void prepare() override {
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

		//! bring background to the bottom
		auto& windows = ImGui::GetCurrentContext()->Windows;
		std::partial_sort(windows.begin(),
						  windows.begin() + 1,
						  windows.end(),
						  [](const ImGuiWindow* lhs, const ImGuiWindow* rhs) {
							  return lhs->BeginOrderWithinContext < rhs->BeginOrderWithinContext;
						  });

		TransactionHandler();

		ImGuiApplication::prepare();

		ImRect rect_Switch{};
		ImGui::Switch(
			"#RegsitryScopeSwitch", "Explorer", "Private", "Public", nullptr, &rect_Switch);
		size_ExplorerMinTrack = rect_Switch.GetSize();
	}

	void render() override {
		ShowBackground();
		if (open_MainMenuBar) {
			ShowMainMenuBar();
		}
		if (open_About) {
			ShowAbout();
		}
		if (open_Settings) {
			ShowSettings();
		}
		if (open_Explorer) {
			ShowExplorer();
		}
		ShowMainView();
	}

	void TransactionHandler() {
		if (RegistryScopeChanged) {
			HKEY RootItem = HandlePublicRegistry ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
			auto Access	  = KEY_READ | KEY_WOW64_64KEY;
			HKEY Item	  = OpenRegistryItem(RootItem, ExtRegItemPath, Access).value_or(nullptr);
			IM_ASSERT_USER_ERROR(Item != nullptr, "registry access failure");

			size_t nSubItem =
				std::any_cast<size_t>(QueryRegistryItemInfo(Item, RegInfo::SubItemNum));

			AssocApps.clear();
			AppSupportTypes.clear();

			AssocApps.insert("<System>");
			AssocApps.insert("<User>");
			AssocApps.insert("<Irregular>");

			nEmptyKey = 0;
			for (int i = 0; i < nSubItem; ++i) {
				if (auto opt = EnumRegistrySubItem(Item, i); opt.has_value()) {
					char buffer[64]{};
					auto wsSubItem = std::move(opt.value());
					if (static_cast<char>(wsSubItem[0]) != '.') {
						continue;
					}
					for (int j = 0, len = wcslen(wsSubItem.get()); j < len; ++j) {
						buffer[j] = static_cast<char>(wsSubItem[j]);
					}
					HKEY SubItem = OpenRegistryItem(Item, wsSubItem.get(), Access).value();
					IM_ASSERT_USER_ERROR(SubItem != nullptr, "registry access failure");
					auto nkey =
						std::any_cast<size_t>(QueryRegistryItemInfo(SubItem, RegInfo::KeyNum));
					if (nkey != 0) {
						std::unique_ptr<uint8_t[]> bytes = nullptr;
						GetRegistryValue(SubItem, L"", bytes);
						constexpr auto flag	 = std::regex::ECMAScript | std::regex::icase;
						auto		   value = reinterpret_cast<const wchar_t*>(bytes.get());
						if (value != nullptr && wcscmp(value, L"") != 0) {
							std::regex re_system(R"(^([^_\.]+)file$)", flag),
								re_user(R"(^([^\.]+)_auto_file$)", flag),
								re_app(R"(^([^\.]+)\.(.+)$)", flag);
							int j = 0;
							do {
								buffer[j] = static_cast<char>(value[j]);
							} while (value[j++] != 0);
							if (std::cmatch m{}; std::regex_search(buffer, m, re_app)) {
								auto app{std::move(m[1].str())};
								AppSupportTypes[app].insert(std::move(m[2].str()));
								AssocApps.insert(std::move(app));
							} else if (std::cmatch m{}; std::regex_search(buffer, m, re_user)) {
								AppSupportTypes["<User>"].insert(std::move(m[1].str()));
							} else if (std::cmatch m{}; std::regex_search(buffer, m, re_system)) {
								AppSupportTypes["<System>"].insert(std::move(m[1].str()));
							} else {
								AppSupportTypes["<Irregular>"].insert(buffer);
							}
						}
						CloseRegistryItem(SubItem);
					} else {
						nEmptyKey += nkey == 0;
					}
				}
			}

			for (auto app : {"<System>", "<User>", "<Irregular>"}) {
				if (AppSupportTypes[app].size() == 0) {
					AssocApps.erase(app);
				}
			}

			CloseRegistryItem(Item);
			RegistryScopeChanged = false;
		}
	}
};

int main(int argc, char* argv[]) {
	auto app = std::make_unique<IWinDEModifierApp>("IWinDEModifier", 800, 600);
	return app->exec();
}