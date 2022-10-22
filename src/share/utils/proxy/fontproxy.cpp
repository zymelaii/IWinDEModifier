#include <share/utils/texture.h>

#include "fontproxy.h"

#include <assert.h>

namespace Proxy {

auto FontProxy::require() -> std::unique_ptr<FontProxy> {
	auto instance = new FontProxy;
	return std::unique_ptr<FontProxy>{instance};
}

FontProxy* FontProxy::add(const char* fontpath, float size, const ImWchar* glyph_ranges) {
	ImFontConfig config{};
	config.MergeMode	  = true;
	ImFontConfig* cfg_ptr = atlas_.Fonts.size() > 0 ? &config : nullptr;
	atlas_.AddFontFromFileTTF(fontpath, size, cfg_ptr, glyph_ranges);
	return this;
}

FontProxy* FontProxy::add(const char* fontpath, float size,
						  const std::initializer_list<ImWchar>&& ranges) {
	const ImWchar* glyph_ranges =
		ranges.size() > 0 ? ranges.begin() : ImGui::GetIO().Fonts->GetGlyphRangesDefault();
	return add(fontpath, size, glyph_ranges);
}

FontProxy* FontProxy::build(ID3D11Device* device) {
	uint8_t* pixels = nullptr;
	int		 width{}, height{}, comp{};
	atlas_.GetTexDataAsRGBA32(&pixels, &width, &height, &comp);
	LoadTextureFromMemory(device, pixels, width, height, &texture_);
	atlas_.SetTexID(texture_);
	IM_FREE(pixels);
	return this;
}

ImFont* FontProxy::get() {
	assert(texture_ != nullptr && "FontProxy: bad construction");
	return atlas_.Fonts[0];
}

FontProxy::~FontProxy() {
	if (texture_ != nullptr) {
		texture_->Release();
		texture_ = nullptr;
	}
}

}	// namespace Proxy