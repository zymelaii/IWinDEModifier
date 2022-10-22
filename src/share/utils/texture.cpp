#include <assert.h>

#include <share/ui/stb_image.h>
#include <share/utils/tinyico/tinyico.h>
extern "C" int ti__extract_as_argb(const uint8_t* const data, uint8_t** ppixels);

#include "texture.h"

bool LoadTextureFromMemory(ID3D11Device* device, const uint8_t* data, size_t width, size_t height,
						   ID3D11ShaderResourceView** texture) {
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width			  = width;
	desc.Height			  = height;
	desc.MipLevels		  = 1;
	desc.ArraySize		  = 1;
	desc.Format			  = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage			  = D3D11_USAGE_DEFAULT;
	desc.BindFlags		  = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags	  = 0;

	ID3D11Texture2D*	   pTexture = NULL;
	D3D11_SUBRESOURCE_DATA subResource{};
	subResource.pSysMem			 = data;
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

	return true;
}

bool LoadTextureFromFile(ID3D11Device* device, const char* filename,
						 ID3D11ShaderResourceView** texture, ImVec2* size) {
	int			   image_width = 0, image_height = 0;
	unsigned char* image_data = stbi_load(filename, &image_width, &image_height, nullptr, 4);
	if (image_data == nullptr) {
		return false;
	}

	LoadTextureFromMemory(device, image_data, image_width, image_height, texture);

	size->x = image_width;
	size->y = image_height;
	stbi_image_free(image_data);

	return true;
}

ID3D11ShaderResourceView* LoadIconFromModule(ID3D11Device* device, const wchar_t* resPath,
											 int szFavored) {
	auto module = LoadLibraryExW(resPath, nullptr, LOAD_LIBRARY_AS_DATAFILE);
	if (!module) return nullptr;

	ID3D11ShaderResourceView* texture = nullptr;
	HRSRC					  res	  = nullptr;

	const auto RT_GROUP_ICON_W = MAKEINTRESOURCEW((ULONG_PTR)(RT_ICON) + DIFFERENCE);
	EnumResourceNamesW(
		module,
		RT_GROUP_ICON_W,
		[](HMODULE hModule, LPCWSTR lpType, LPWSTR lpName, LONG_PTR lParam) -> BOOL {
			HRSRC* const pres = reinterpret_cast<HRSRC*>(lParam);
			HRSRC		 res  = FindResourceW(hModule, lpName, lpType);
			const auto	 size = SizeofResource(hModule, res);
			if (!res) return true;
			*pres = res;
			return false;
		},
		reinterpret_cast<intptr_t>(&res));

	if (res != nullptr) {
		auto		   resload = LoadResource(module, res);
		auto		   data	   = reinterpret_cast<uint8_t*>(LockResource(resload));
		ti_resource_t* rc	   = nullptr;
		auto		   err	   = ti_dfopen(&rc, &data, SizeofResource(module, res));

		if (err == E_NULL) {
			const auto& entries = reinterpret_cast<ti_dfdir_t*>(&rc->directory)->entries;
			int			i = 0, index = -1, id = -1;
			int			width = -1, height = -1;

			while (i < rc->directory.count) {
				const auto& entry = entries[i];
				if (entry.width >= 32 && entry.bpp == 32 &&
					(index == -1 || entry.width > entries[index].width)) {
					index = i;
				}
				++i;
			}
			if (index == -1) {
				index = 0;
			}
			id	   = entries[index].id;
			width  = entries[index].width;
			height = entries[index].height;

			res = FindResource(module, MAKEINTRESOURCE(id), RT_ICON);
			assert(res != nullptr && "FindResource failure");
			resload = LoadResource(module, res);
			assert(resload != nullptr && "LoadResource failure");
			data = reinterpret_cast<uint8_t*>(LockResource(resload));
			assert(data != nullptr && "LockResource failure");

			int		 n = 0;
			uint8_t* bytes =
				stbi_load_from_memory(data, SizeofResource(module, res), &width, &height, &n, 4);
			if (bytes) {
				LoadTextureFromMemory(device, bytes, width, height, &texture);
				stbi_image_free(bytes);
			} else {
				err = ti__extract_as_argb(data, &bytes);
				if (bytes) {
					LoadTextureFromMemory(device, bytes, width, height, &texture);
					free(bytes);
				} else {
					printf("ti__extract_as_argb error: %d\n", err);
				}
			}

			ti_close(&rc);
		} else {
			printf("error: %d\n", err);
		}
	}

	FreeLibrary(module);
	return texture;
}