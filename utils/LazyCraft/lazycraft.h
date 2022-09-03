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

class LazyCraft : public ImGuiApplication {
private:
	using QuickLaunchItem = std::pair<std::wstring, ID3D11ShaderResourceView*>;
	ID3D11ShaderResourceView*			   texture_Background = nullptr;
	std::vector<QuickLaunchItem>		   QuickLaunchs{};

public:	  //!< lc_main.cpp
	LazyCraft(const char* title, int width, int height);

	std::optional<LRESULT> notify(UINT msg, WPARAM wParam, LPARAM lParam) override;
	void				   configure() override;
	void				   render() override;

public:	  //!< lc_com.cpp
	bool IQuickLaunch();

protected:	 //!< lc_utils.cpp
	ID3D11ShaderResourceView* LoadIconFromModule(const wchar_t* resPath, int szFavored = -1);
};