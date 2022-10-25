#pragma once

#include <share/ui/imgui/imgui.h>

#include <stdint.h>
#include <d3d11.h>

bool LoadTextureFromMemory(ID3D11Device* device, const uint8_t* data, size_t width, size_t height,
						   ID3D11ShaderResourceView** texture);

bool LoadTextureFromFile(ID3D11Device* device, const char* filename,
						 ID3D11ShaderResourceView** texture, ImVec2* size);

ID3D11ShaderResourceView* LoadIconFromModule(ID3D11Device* device, const wchar_t* resPath,
											 int szFavored = -1);