#include "backend.h"
#include "widgets.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include <any>
#include <errhandlingapi.h>
#include <regex>
#include <iwindeapi/registry.h>

#include <algorithm>
#include <compare>
#include <memory>
#include <map>
#include <stdint.h>
#include <string>
#include <iostream>
#include <winnt.h>
#include <winreg.h>

class IWinDEModifierApp : public ImGuiApplication {
private:   //!< Configuration
	//! flags
	std::map<std::string, bool> flags{};
	std::vector<bool>			AnonymousFlagPool{};
	bool						open_MainMenuBar = true;
	bool						open_About		 = false;
	bool						open_Explorer	 = true;
	bool						open_Settings	 = false;
	//! status
	std::vector<std::string> FileExts{};
	bool					 HandlePublicRegistry = false;
	bool					 RegistryScopeChanged = true;
	int						 CurrentSettingsOn	  = 0;
	char					 FilterRegexBuffer[256]{};
	size_t					 nEmptyKey = 0;
	ImVec2					 size_MainMenuBar{0, 0};
	ImVec2					 size_Explorer{0, 0};
	ImFont*					 FontClear = nullptr;
	ImFont*					 FontSmall = nullptr;
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

		auto menubar = ImGui::GetCurrentWindow();
		ImGui::AlignTextToFramePadding();
		char FPS[64]{};
		sprintf(FPS, "FPS: %.2f", ImGui::GetIO().Framerate);
		ImGui::ItemSize(ImVec2(menubar->Size.x - menubar->DC.CursorPos.x -
								   ImGui::CalcTextSize(FPS).x - ImGui::GetFontSize(),
							   menubar->Size.y));
		size_MainMenuBar = menubar->Size;
		ImGui::TextColored(ImVec4(1.00, 0.00, 0.00, 1.00), "%s", FPS);
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
															  // TODO
			}
			if (CurrentSettingsOn == 0) {
				CurrentSettingsOn = -1;
			}
		}
		ImGui::EndChild();
		ImGui::SameLine();
		ImGui::BeginChild("SettingsView", ImVec2(0, 0));
		if (ImGui::BeginTabBar("Anonymous.SettingsTabs", ImGuiTabBarFlags_None)) {
			switch (CurrentSettingsOn) {
				case 0: {
					if (ImGui::BeginTabItem("General")) {
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
		auto flag = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
					ImGuiWindowFlags_NoBringToFrontOnFocus;
		auto client = ImGui::GetIO().DisplaySize;
		auto offset = open_MainMenuBar ? size_MainMenuBar.y : 0;
		if (flags["OnExplorerInit"]) {
			size_Explorer.x			= InitialExplorerWidth;
			flags["OnExplorerInit"] = false;
		}
		size_Explorer.y = client.y - offset;
		ImGui::SetNextWindowSize(size_Explorer);
		ImGui::SetNextWindowBgAlpha(0.72);
		ImGui::SetNextWindowSizeConstraints(ImVec2(100, client.y), ImVec2(320, client.y));
		ImGui::SetNextWindowPos(ImVec2(0, offset));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, ImGui::GetFontSize() * 0.5);

		ImGui::Begin("Explorer", &open_Explorer, flag);
		if (ImGui::Switch("RegsitryScopeSwitch", "Private", "Public", &HandlePublicRegistry)) {
			RegistryScopeChanged = true;
		}
		size_Explorer = ImGui::GetWindowSize();
		ImGui::End();

		ImGui::PopStyleVar();
	}

	int ShowRegistryItemContent(HKEY item, const char* ext, bool disable = false) {
		wchar_t buffer[64]{};
		_swprintf(buffer, L"%S", ext);
		auto extitem = OpenRegistryItem(item, buffer, KEY_READ | KEY_WOW64_64KEY).value_or(nullptr);
		IM_ASSERT_USER_ERROR(extitem != nullptr, "registry key open failure");

		size_t nkey = std::any_cast<size_t>(QueryRegistryItemInfo(extitem, RegInfo::KeyNum));
		for (int i = 0; !disable && i < nkey; ++i) {
			auto		   key = std::move(EnumRegistryKey(extitem, i).value());
			char		   buffer[256]{};
			char*		   p = buffer;
			const wchar_t* q = key[0] == 0 ? L"(Default)" : key.get();
			do {
				*p++ = *q++;
			} while (*q != 0);
			auto [result, type, size] = GetRegistryValue(extitem, key.get());
			if (type == REG_SZ || type == REG_EXPAND_SZ) {
				std::unique_ptr<uint8_t[]> value = nullptr;
				GetRegistryValue(extitem, key.get(), value);
				*p++ = ':';
				*p++ = ' ';
				q	 = reinterpret_cast<wchar_t*>(value.get());
				do {
					*p++ = *q;
				} while (*q++ != 0);
			}
			*p = 0;
			ImGui::BulletText("%s", buffer);
		}
		CloseRegistryItem(extitem);
		return nkey;
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

		if (!FileExts.empty()) {
			size.y -= ImGui::GetFontSize() + StatusBarPadding.y * 2;   //!< for status bar
		}

		// if (!FileExts.empty()) {
		// 	auto height = ImGui::GetFontSize() * 1.6 + StatusBarPadding.y * 2;
		// 	ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y - height));
		// 	ImGui::SetNextWindowSize(ImVec2(size.x, height));
		// 	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		// 	ImGui::Begin("ResultFilter", nullptr, flag);
		// 	ImGui::InputTextWithHint(
		// 		"filter", "input text filter regex", FilterRegexBuffer, sizeof(FilterRegexBuffer));
		// 	ImGui::End();
		// 	ImGui::PopStyleVar();
		// }

		bool						EnableFilter = strlen(FilterRegexBuffer) > 0;
		std::unique_ptr<std::regex> filter		 = nullptr;
		try {
			filter = std::make_unique<std::regex>(FilterRegexBuffer);
		} catch (std::regex_error) {
			EnableFilter = false;
		}

		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(size);
		ImGui::SetNextWindowBgAlpha(0.88);
		ImGui::Begin("MainView", nullptr, flag);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::BeginChild("FilterInput", ImVec2(0, ImGui::GetFontSize() * 2));
		ImGui::InputTextWithHint(
			"filter", "input text filter regex", FilterRegexBuffer, sizeof(FilterRegexBuffer));
		ImGui::EndChild();
		ImGui::PopStyleVar();

		ImGui::PushFont(FontClear);

		bool PublicScope = HandlePublicRegistry && !RegistryScopeChanged ||
						   !HandlePublicRegistry && RegistryScopeChanged;
		HKEY RootItem = PublicScope ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
		auto Access	  = KEY_READ | KEY_WOW64_64KEY;
		HKEY Item	  = OpenRegistryItem(RootItem, ExtRegItemPath, Access).value_or(nullptr);
		IM_ASSERT_USER_ERROR(Item != nullptr, "registry access failure");

		int		   index				  = 0;
		const bool ShowEmptyRegistryItems = flags["ShowEmptyRegistryItems"];
		for (auto ext : FileExts) {
			auto nkey = ShowRegistryItemContent(Item, ext.data(), true);
			if (ShowEmptyRegistryItems || nkey != 0) {
				if (!EnableFilter || EnableFilter && std::regex_search(ext.data(), *filter.get())) {
					if (ImGui::RadioButton(ext.data(), AnonymousFlagPool[index])) {
						AnonymousFlagPool[index] = !AnonymousFlagPool[index];
					}
					ImGui::Indent();
					ShowRegistryItemContent(Item, ext.data(), !AnonymousFlagPool[index]);
					ImGui::Unindent();
				}
			}
			++index;
		}
		CloseRegistryItem(Item);
		ImGui::PopFont();

		ImGui::End();

		if (!FileExts.empty()) {
			ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y + size.y));
			ImGui::SetNextWindowSize(ImVec2(size.x, ImGui::GetFontSize() + StatusBarPadding.y * 2));
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.00, 0.48, 0.80, 1.00));
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00, 1.00, 1.00, 1.00));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, StatusBarPadding);
			ImGui::Begin("RegistryStatusBar", nullptr, flag);
			ImGui::Text("found %llu items in total (%zu empty keys)", FileExts.size(), nEmptyKey);
			ImGui::End();
			ImGui::PopStyleVar();
			ImGui::PopStyleColor(2);
		}
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
		flags["FirstToggleExplorer"]	  = false;
		flags["OnExplorerInit"]			  = true;
		flags["ShowEmptyRegistryItems"]	  = false;
		flags["ExpandKeyValuesOnDefault"] = true;
		auto FontSmall =
			ImGui::GetIO().Fonts->AddFontFromFileTTF(R"(..\assets\DroidSans.ttf)", 16.0f);
		ImGui::GetIO().Fonts->Build();
		LoadTextureFromFile(
			pd3dDevice_, R"(..\assets\image.png)", &texture_Background, &size_Background);
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

			std::vector<std::string> Extensions{};
			size_t					 nSubItem =
				std::any_cast<size_t>(QueryRegistryItemInfo(Item, RegInfo::SubItemNum));
			Extensions.reserve(nSubItem);

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
					nEmptyKey += ShowRegistryItemContent(Item, buffer, true) == 0;
					Extensions.emplace_back(buffer);
				}
			}

			CloseRegistryItem(Item);

			Extensions.shrink_to_fit();
			FileExts.swap(Extensions);
			AnonymousFlagPool.resize(FileExts.size());
			std::fill(AnonymousFlagPool.begin(),
					  AnonymousFlagPool.end(),
					  flags["ExpandKeyValuesOnDefault"]);

			RegistryScopeChanged = false;
		}
	}
};

int main(int argc, char* argv[]) {
	auto app = std::make_unique<IWinDEModifierApp>("IWinDEModifier", 800, 600);
	return app->exec();
}