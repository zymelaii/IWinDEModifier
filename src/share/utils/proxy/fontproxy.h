#pragma once

#include <share/ui/imgui/imgui.h>

#include <d3d11.h>
#include <memory>

namespace Proxy {

struct FontProxy {
	static auto require() -> std::unique_ptr<FontProxy>;

	FontProxy* add(const char* fontpath, float size, const ImWchar* glyph_ranges);
	FontProxy* add(const char* fontpath, float size,
				   const std::initializer_list<ImWchar>&& ranges = {});
	FontProxy* build(ID3D11Device* device);
	ImFont*	   get();

	~FontProxy();

protected:
	FontProxy() = default;

private:
	ImFontAtlas				  atlas_{};
	ID3D11ShaderResourceView* texture_{nullptr};
};

}	// namespace Proxy