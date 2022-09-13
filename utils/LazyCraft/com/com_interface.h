#pragma once

#include <imgui/imgui_internal.h>
#include <imgui/imgui.h>

namespace com {

struct IUtility {
	virtual const ImRect rect() const	= 0;
	virtual const ImVec2 cursor() const = 0;

	virtual bool prepare(const IUtility* parent) = 0;
	virtual void render() const							   = 0;
};

}	// namespace com