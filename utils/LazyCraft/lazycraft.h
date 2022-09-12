#include "utils/proxy/linkproxy.h"
#include "utils/proxy/fontproxy.h"
#include "utils/texture.h"

#include <iostream>
#include <vector>
#include <array>
#include <tuple>
#include <memory>
#include <codecvt>
#include <locale>

#include <backend.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

using Proxy::LinkProxy;
using Proxy::FontProxy;

class LazyCraft : public ImGuiApplication {
private:
	using QuickLaunchItem = std::tuple<std::string, std::wstring, ID3D11ShaderResourceView*>;
	using FontResource	  = std::unique_ptr<FontProxy>;
	ID3D11ShaderResourceView*	 texture_Background = nullptr;
	FontResource				 font_charge		= FontProxy::require();
	FontResource				 font_ascii			= FontProxy::require();
	std::vector<QuickLaunchItem> QuickLaunchs{};
	bool						 toggle_se{false};
	UINT_PTR					 timer{};
	HMODULE						 hdlhook = nullptr;

public:	  //!< lc_main.cpp
	LazyCraft();
	~LazyCraft();

	void toggle(bool active);

	LazyCraft*			   build(const char* title, int width, int height, int x, int y) override;
	std::optional<LRESULT> notify(UINT msg, WPARAM wParam, LPARAM lParam) override;
	void				   configure() override;
	void				   render() override;

public:	  //!< lc_com.cpp
	bool IStatusBar();
	bool IQuickLaunch();
	bool ISearchEngine();
	bool ISearchEngineClassic();
};
