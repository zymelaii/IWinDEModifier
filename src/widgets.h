#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

namespace ImGui {

bool Switch(const char* sid, const char* label, const char* false_label, const char* true_label,
			bool* value, ImRect* out_rect_only = nullptr);

};