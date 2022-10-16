#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_internal.h"
#include <d3d11.h>
#include <optional>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam,
															 LPARAM lParam);

class ImGuiApplication {
public:	  //! backend support
	static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void				  CreateRenderTarget();
	void				  CleanupRenderTarget();
	void				  CleanupDeviceD3D();
	bool				  CreateDeviceD3D();
	void				  ResizeBuffer(int width, int height);

public:
	ImGuiApplication();
	virtual ~ImGuiApplication();
	virtual ImGuiApplication*	   build(const char* title, int width, int height, int x, int y);
	virtual std::optional<LRESULT> notify(UINT msg, WPARAM wParam, LPARAM lParam);
	virtual void				   configure();
	virtual void				   prepare();
	virtual void				   render() = 0;
	virtual void				   present();
	int							   exec(bool visible = true);
	int							   lazy_exec(bool visible = true);

public:
	ImVec4 backgroundColor;

protected:
	HWND hwnd_;
	char class_[256];
	int	 errno_;

protected:
	ID3D11Device*			pd3dDevice_;
	ID3D11DeviceContext*	pd3dDeviceContext_;
	IDXGISwapChain*			pSwapChain_;
	ID3D11RenderTargetView* mainRenderTargetView_;
};