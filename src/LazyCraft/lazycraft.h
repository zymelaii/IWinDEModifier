#include <iostream>
#include <vector>
#include <array>
#include <tuple>
#include <memory>
#include <codecvt>
#include <locale>

#include <share/utils/proxy/linkproxy.h>
#include <share/utils/proxy/fontproxy.h>
#include <share/utils/proxy/volumeproxy.h>
#include <share/utils/texture.h>
#include <share/ui/backend.h>
#include <share/ui/imgui/imgui.h>
#include <share/ui/imgui/imgui_internal.h>

#include "com/statusbar.h"
#include "widget/batteryitem.h"
#include "widget/dateitem.h"

using Proxy::FontProxy;
using com::impl::StatusBar;

class LazyCraft : public ImGuiApplication {
private:
	using QuickLaunchItem = std::tuple<std::string, std::wstring, ID3D11ShaderResourceView*>;
	using FontResource	  = std::unique_ptr<FontProxy>;
	ID3D11ShaderResourceView*	 texture_Background = nullptr;
	FontResource				 font_charge		= FontProxy::require();
	FontResource				 font_ascii			= FontProxy::require();
	FontResource				 font_full			= FontProxy::require();
	std::vector<QuickLaunchItem> QuickLaunchs{};
	bool						 toggle_se{false};
	UINT_PTR					 timer{};
	HMODULE						 hdlhook			 = nullptr;
	const UINT					 WM_SHELLHOOKMESSAGE = RegisterWindowMessage("SHELLHOOK");
	StatusBar					 statusbar;

public:
	LazyCraft();
	~LazyCraft();

	void toggle(bool active);

	void cbshellhook(WPARAM wParam, LPARAM lParam);

	LazyCraft*			   build(const char* title, int width, int height, int x, int y) override;
	std::optional<LRESULT> notify(UINT msg, WPARAM wParam, LPARAM lParam) override;
	void				   configure() override;
	void				   render() override;

public:
	bool IQuickLaunch();
	bool ISearchEngine();
	bool ISearchEngineClassic();
};
