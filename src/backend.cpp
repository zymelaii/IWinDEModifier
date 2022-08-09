#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "backend.h"
#include <basetsd.h>
#include <errhandlingapi.h>
#include <winerror.h>
#include <winuser.h>

bool LoadTextureFromFile(ID3D11Device* device, const char* filename,
						 ID3D11ShaderResourceView** texture, ImVec2* size) {
	int			   image_width	= 0;
	int			   image_height = 0;
	unsigned char* image_data	= stbi_load(filename, &image_width, &image_height, nullptr, 4);
	if (image_data == nullptr) {
		return false;
	}

	// Create texture
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width			  = image_width;
	desc.Height			  = image_height;
	desc.MipLevels		  = 1;
	desc.ArraySize		  = 1;
	desc.Format			  = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage			  = D3D11_USAGE_DEFAULT;
	desc.BindFlags		  = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags	  = 0;

	ID3D11Texture2D*	   pTexture = NULL;
	D3D11_SUBRESOURCE_DATA subResource{};
	subResource.pSysMem			 = image_data;
	subResource.SysMemPitch		 = desc.Width * 4;
	subResource.SysMemSlicePitch = 0;
	device->CreateTexture2D(&desc, &subResource, &pTexture);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format					  = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension			  = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels		  = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(pTexture, &srvDesc, texture);
	pTexture->Release();

	size->x = image_width;
	size->y = image_height;
	stbi_image_free(image_data);

	return true;
}

bool ImGuiApplication::CreateDeviceD3D() {
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount						  = 2;
	sd.BufferDesc.Width					  = 0;
	sd.BufferDesc.Height				  = 0;
	sd.BufferDesc.Format				  = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator	  = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags							  = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage						  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow						  = hwnd_;
	sd.SampleDesc.Count					  = 1;
	sd.SampleDesc.Quality				  = 0;
	sd.Windowed							  = TRUE;
	sd.SwapEffect						  = DXGI_SWAP_EFFECT_DISCARD;

	UINT					createDeviceFlags = 0;
	D3D_FEATURE_LEVEL		featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_0,
	};
	if (D3D11CreateDeviceAndSwapChain(nullptr,
									  D3D_DRIVER_TYPE_HARDWARE,
									  nullptr,
									  createDeviceFlags,
									  featureLevelArray,
									  2,
									  D3D11_SDK_VERSION,
									  &sd,
									  &pSwapChain_,
									  &pd3dDevice_,
									  &featureLevel,
									  &pd3dDeviceContext_) != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void ImGuiApplication::CleanupDeviceD3D() {
	CleanupRenderTarget();
	if (pSwapChain_) {
		pSwapChain_->Release();
		pSwapChain_ = nullptr;
	}
	if (pd3dDeviceContext_) {
		pd3dDeviceContext_->Release();
		pd3dDeviceContext_ = nullptr;
	}
	if (pd3dDevice_) {
		pd3dDevice_->Release();
		pd3dDevice_ = nullptr;
	}
}

void ImGuiApplication::CreateRenderTarget() {
	ID3D11Texture2D* pBackBuffer = nullptr;
	pSwapChain_->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	pd3dDevice_->CreateRenderTargetView(pBackBuffer, nullptr, &mainRenderTargetView_);
	pBackBuffer->Release();
}

void ImGuiApplication::CleanupRenderTarget() {
	if (mainRenderTargetView_) {
		mainRenderTargetView_->Release();
		mainRenderTargetView_ = nullptr;
	}
}

void ImGuiApplication::ResizeBuffer(int width, int height) {
	if (pd3dDevice_ != nullptr) {
		CleanupRenderTarget();
		pSwapChain_->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
		CreateRenderTarget();
	}
}

LRESULT WINAPI ImGuiApplication::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	auto app = reinterpret_cast<ImGuiApplication*>(GetWindowLongPtr(hWnd, 0));
	if (app != nullptr) {
		if (auto opt = app->notify(msg, wParam, lParam); opt.has_value()) {
			return opt.value();
		}
	}
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
		return true;
	}
	switch (msg) {
		case WM_CREATE: {
			auto info = reinterpret_cast<CREATESTRUCT*>(lParam);
			auto errcode =
				SetWindowLongPtr(hWnd, 0, reinterpret_cast<LONG_PTR>(info->lpCreateParams));
			break;
		}
		case WM_SIZE: {
			if (wParam != SIZE_MINIMIZED) {
				app->ResizeBuffer((UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
			}
			app->prepare();
			app->render();
			app->present();
			return 0;
		}
		case WM_SYSCOMMAND: {
			if ((wParam & 0xfff0) == SC_KEYMENU) {
				return 0;
			}
			break;
		}
		case WM_DESTROY: {
			PostQuitMessage(0);
			return 0;
		}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

ImGuiApplication::ImGuiApplication(const char* title, int width, int height)
	: pd3dDevice_(nullptr)
	, pd3dDeviceContext_(nullptr)
	, pSwapChain_(nullptr)
	, mainRenderTargetView_(nullptr)
	, hwnd_(nullptr)
	, instance_(nullptr)
	, clsname_()
	, errno_(0)
	, backgroundColor(1.00f, 1.00f, 1.00f, 1.00f) {
	sprintf(clsname_, "ImGui.%s.Application", title);

	ImGui_ImplWin32_EnableDpiAwareness();

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
					 clsname_,
					 nullptr};
	RegisterClassEx(&wc);

	hwnd_ = CreateWindow(wc.lpszClassName,
						 title,
						 WS_OVERLAPPEDWINDOW,
						 100,
						 100,
						 width,
						 height,
						 nullptr,
						 nullptr,
						 wc.hInstance,
						 this);

	if (!CreateDeviceD3D()) {
		CleanupDeviceD3D();
		UnregisterClass(wc.lpszClassName, wc.hInstance);
		errno_ = 1;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui_ImplWin32_Init(hwnd_);
	ImGui_ImplDX11_Init(pd3dDevice_, pd3dDeviceContext_);
}

ImGuiApplication::~ImGuiApplication() {
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	DestroyWindow(hwnd_);
	UnregisterClass(clsname_, instance_);
}

std::optional<LRESULT> ImGuiApplication::notify(UINT msg, WPARAM wParam, LPARAM lParam) {
	return std::nullopt;
}

void ImGuiApplication::configure() {
	ImGui::StyleColorsLight();
}

void ImGuiApplication::prepare() {
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiApplication::present() {
	ImGui::Render();
	const float alphaClearColor[4] = {backgroundColor.x * backgroundColor.w,
									  backgroundColor.y * backgroundColor.w,
									  backgroundColor.z * backgroundColor.w,
									  backgroundColor.w};
	pd3dDeviceContext_->OMSetRenderTargets(1, &mainRenderTargetView_, nullptr);
	pd3dDeviceContext_->ClearRenderTargetView(mainRenderTargetView_, alphaClearColor);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	pSwapChain_->Present(1, 0);
}

int ImGuiApplication::exec() {
	if (errno_ != 0) {
		return errno_;
	}

	configure();

	ShowWindow(hwnd_, SW_SHOWDEFAULT);
	UpdateWindow(hwnd_);

	bool done = false;
	MSG	 msg{};
	while (!done) {
		while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) done = true;
		}
		if (done) break;

		prepare();
		render();
		present();
	}

	return msg.lParam;
}