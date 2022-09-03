#include "lazycraft.h"
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define TINYICO_IMPLEMENT
#include "tinyico.h"

uint8_t* lazycraft__ParseIconFileFromMemory(uint8_t* data, size_t len) {
	auto p = reinterpret_cast<BITMAPINFOHEADER*>(data);

	if (p->biSizeImage == 0) return nullptr;

	const auto bitcount	 = p->biBitCount;
	const auto pixels	 = p->biWidth * p->biHeight / 2;
	const auto linebytes = ((p->biWidth * p->biBitCount + 31) & 0xffffffe0) / 8;
	// const auto ncolquad	 = (len - p->biSize - p->biHeight * linebytes / 2) / 4;

	auto bytes = std::make_unique<uint8_t[]>(p->biWidth * p->biHeight * 2);
	bool ok	   = true;

	auto bits__test = [](uint8_t* bits, int i, int width) -> bool {
		if (width & (width - 1)) {
			int skip = i / width;
			i += skip * (32 - width % 32);
		}
		int n = i / 8;
		int k = i % 8;
		return !!(bits[n] & (0x80 >> k));
	};

	std::vector<RGBQUAD> palette(p->biClrUsed);
	if (palette.size() != 0) {
		RGBQUAD* q = reinterpret_cast<RGBQUAD*>(p + 1);
		for (auto& col : palette) {
			col = *q++;
		}
		int	 i	 = pixels;
		auto col = bytes.get();
		if (palette.size() == 16) {
			auto index	  = reinterpret_cast<uint8_t*>(q);
			auto maskbits = index + pixels / 2;
			while (i) {
				auto rgbquad = palette[*index & 0x0f];
				col[2]		 = rgbquad.rgbBlue;
				col[1]		 = rgbquad.rgbGreen;
				col[0]		 = rgbquad.rgbRed;
				col[3]		 = bits__test(maskbits, pixels - i, p->biWidth) ? 0x00 : 0xff;
				col += 4;
				--i;

				rgbquad = palette[*index & 0xf0];
				col[2]	= rgbquad.rgbBlue;
				col[1]	= rgbquad.rgbGreen;
				col[0]	= rgbquad.rgbRed;
				col[3]	= bits__test(maskbits, pixels - i, p->biWidth) ? 0x00 : 0xff;
				--i;
				col += 4;

				++index;
			}
			printf("(4->16|%ldx%ld) left: %llu\n",
				   p->biWidth,
				   p->biHeight / 2,
				   len - static_cast<intptr_t>(maskbits - data));
		} else if (palette.size() == 256) {
			auto index	  = reinterpret_cast<uint8_t*>(q);
			auto maskbits = index + pixels;
			while (i--) {
				auto& rgbquad = palette[*index++];
				col[2]		  = rgbquad.rgbBlue;
				col[1]		  = rgbquad.rgbGreen;
				col[0]		  = rgbquad.rgbRed;
				col[3]		  = bits__test(maskbits, pixels - i, p->biWidth) ? 0x00 : 0xff;
				col += 4;
			}
			printf("(8->256|%ldx%ld) left: %llu\n",
				   p->biWidth,
				   p->biHeight / 2,
				   len - static_cast<intptr_t>(maskbits - data));
		}
	} else if (bitcount == 16) {
		ok = false;
	} else if (bitcount == 24) {
		ok = false;
	} else if (bitcount == 32) {
		RGBQUAD* q		  = reinterpret_cast<RGBQUAD*>(p + 1);
		auto	 maskbits = reinterpret_cast<uint8_t*>(q + pixels);
		auto	 col	  = bytes.get();
		for (int i = 0; i < pixels; ++i) {
			const auto& rgbquad = q[i];
			col[2]				= rgbquad.rgbBlue;
			col[1]				= rgbquad.rgbGreen;
			col[0]				= rgbquad.rgbRed;
			col[3]				= rgbquad.rgbReserved;
			// col[3]				= bits__test(maskbits, i, p->biWidth) ? 0x0f : 0xff;
			col += 4;
		}
	} else {
		ok = false;
	}

	// if (ok) {
	// 	static int order = 0;
	// 	char	   buffer[MAX_PATH]{};
	// 	sprintf_s(buffer, R"(C:\Users\zymelaii\Desktop\preview\%d.png)", ++order);
	// 	stbi_flip_vertically_on_write(true);
	// 	stbi_write_png(buffer, p->biWidth, p->biHeight / 2, 4, bytes.get(), p->biWidth * 4);
	// }

	// printf("bitcount: %hu\t, linebytes: %lu\t, clacsize: %lu\t, imagesize: %lu\t, size: %zu\t\n",
	// 	   bitcount,
	// 	   linebytes,
	// 	   linebytes * p->biHeight,
	// 	   p->biSizeImage,
	// 	   len);

	printf("indicate.size: %#llx\n"
		   "header.biSize: %lu\n"
		   "header.biWidth: %ld\n"
		   "header.biHeight: %ld\n"
		   "header.biPlanes: %d\n"
		   "header.biBitCount: %d\n"
		   "header.biCompression: %lu\n"
		   "header.biSizeImage: %lu\n"
		   "header.biXPelsPerMeter: %ld\n"
		   "header.biYPelsPerMeter: %ld\n"
		   "header.biClrUsed: %lu\n"
		   "header.biClrImportant: %lu\n\n",
		   p->biSize + p->biClrUsed * sizeof(DWORD) +
			   (p->biWidth * p->biHeight / 2) * (p->biBitCount >> 2),
		   // p->biSize + p->biClrUsed * sizeof(DWORD) + ((p->biWidth * p->biBitCount + 3) / 4) * 4
		   // * p->biHeight / 2,
		   p->biSize,
		   p->biWidth,
		   p->biHeight,
		   p->biPlanes,
		   p->biBitCount,
		   p->biCompression,
		   p->biSizeImage,
		   p->biXPelsPerMeter,
		   p->biYPelsPerMeter,
		   p->biClrUsed,
		   p->biClrImportant);

	// // 2e8 -> 268 ~ 128 ~ 32
	// // 128 -> e8 ~ 64 ~ 16
	// // a68 -> 868 ~ 512 ~ 64

	// printf("indicate.size: %llu -> real.size: %zu, diff: %lld\n",
	// 	   p->biSize + p->biClrUsed * sizeof(DWORD) + (p->biWidth * p->biHeight) * (p->biBitCount >>
	// 2) + p->biSizeImage, 	   len, 	   p->biSize + p->biClrUsed * sizeof(DWORD) +
	// (p->biWidth
	// * p->biHeight / 2) * (p->biBitCount >> 2) + p->biSizeImage - len);

	return data;
}

static CALLBACK BOOL lazycraft__EnumIconCallback(HMODULE module, LPCWSTR type, LPWSTR name,
												 LONG_PTR lParam) {
	auto	   params		= reinterpret_cast<void**>(lParam);
	auto	   pres			= reinterpret_cast<HRSRC*>(params[0]);
	const auto favored_size = *reinterpret_cast<int*>(params[1]);
	auto	   reserved		= reinterpret_cast<int*>(params[2]);

	auto res = FindResourceW(module, name, type);

	if (res == nullptr) {
		return true;
	}

	auto	   resload = LoadResource(module, res);
	const auto data	   = reinterpret_cast<uint8_t*>(LockResource(resload));
	const auto len	   = SizeofResource(module, res);

	int	 width{}, height{}, comp{};
	bool ok = stbi_info_from_memory(data, len, &width, &height, &comp);
	if (!ok) {
		lazycraft__ParseIconFileFromMemory(data, len);
		return true;
	}

	const auto diff = width - favored_size;

	if (*pres == nullptr) {
		*pres	  = res;
		*reserved = diff;
		return true;
	}

	if (abs(diff) < abs(*reserved) || abs(diff) == abs(*reserved) && diff > *reserved) {
		*pres	  = res;
		*reserved = diff;
	}

	return true;
}

void pentry(ti_resource_t* res, int index, bool isdf = false) {
	assert(res != nullptr && "reject null icon resource");
	assert(index >= 0 && index < res->directory.count &&
		   "index to entry of icon resource out of range");

	const auto& entry = isdf ? *(ti_entry_t*)(((ti_dfdir_t*)&res->directory)->entries + index)
							 : res->directory.entries[index];

	printf(R"(TIEntry[%d] {
    width:    %d,
    height:   %d,
    ncol:     %d,
    reserved: %d,
    planes:   %d,
    bpp:      %d,
    nbytes:   %d,
    %s:%*s%d
})"
		   "\n",
		   index,
		   entry.width,
		   entry.height,
		   entry.ncol,
		   entry.reserved,
		   entry.planes,
		   entry.bpp,
		   entry.nbytes,
		   isdf ? "id" : "offset",
		   isdf ? 7 : 3,
		   " ",
		   isdf ? ((ti_dfentry_t*)&entry)->id : entry.offset);
}

ID3D11ShaderResourceView* LazyCraft::LoadIconFromModule(const wchar_t* resPath, int szFavored) {
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
				LoadTextureFromMemory(pd3dDevice_, bytes, width, height, &texture);
				stbi_image_free(bytes);
			} else {
				err = ti__extract_as_argb(data, &bytes);
				if (bytes) {
					LoadTextureFromMemory(pd3dDevice_, bytes, width, height, &texture);
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