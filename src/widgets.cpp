#include "widgets.h"
#include "imgui/imgui.h"

namespace ImGui {

bool Switch(const char* sid, const char* label, const char* false_label, const char* true_label,
			bool* value, ImRect* out_rect_only) {
	ImGuiWindow* window = nullptr;
	ImGuiID		 id		= 0;

	if (out_rect_only == nullptr) {
		window = GetCurrentWindow();
		if (window->SkipItems) return false;
		id = window->GetID(sid);
	}

	ImVec2		 padding{GetFontSize(), GetFontSize() * 0.2f};
	float		 spacing		  = GetFontSize() * 1.5;
	const ImVec2 false_label_size = CalcTextSize(false_label);
	const ImVec2 true_label_size  = CalcTextSize(true_label);
	ImVec2		 label_size{0, 0};
	float		 full_label_width = 0.0f;
	float		 status_width	  = ImMax(false_label_size.x, true_label_size.x);
	ImVec2		 raw_size{
		  status_width * 2 + spacing + padding.x * 2,
		  true_label_size.y + padding.y * 2,
	  };
	if (label != nullptr) {
		label_size		 = CalcTextSize(label);
		full_label_width = label_size.x + spacing / 2;
		raw_size.x += full_label_width;
	}
	ImVec2 pos	= out_rect_only ? ImVec2(0, 0) : window->DC.CursorPos;
	ImVec2 size = CalcItemSize(ImVec2(0, 0), raw_size.x, raw_size.y);
	ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));

	if (out_rect_only != nullptr) {
		*out_rect_only = bb;
		return true;
	}

	ItemSize(size);
	if (!ItemAdd(bb, id)) return false;

	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held);
	if (pressed) *value = !*value;

	float switch_width = raw_size.x;
	if (label != nullptr) {
		RenderText(ImVec2(pos.x, pos.y + padding.y), label);
		bb.Min.x += full_label_width;
		pos.x += full_label_width;
		switch_width -= full_label_width;
	}

	float mid_x = (bb.Min.x + bb.Max.x) * 0.5f;

	auto active_col{GetColorU32(ImVec4(0.00, 0.48, 0.80, 1.00))};
	auto white_col{GetColorU32(ImVec4(1.00, 1.00, 1.00, 1.00))};
	auto black_col{GetColorU32(ImVec4(0.00, 0.00, 0.00, 1.00))};

	window->DrawList->PathRect(bb.Min, ImVec2(mid_x, bb.Max.y), 8.0, ImDrawFlags_RoundCornersLeft);
	window->DrawList->PathFillConvex(*value ? white_col : active_col);
	window->DrawList->PathRect(ImVec2(mid_x, bb.Min.y), bb.Max, 8.0, ImDrawFlags_RoundCornersRight);
	window->DrawList->PathFillConvex(*value ? active_col : white_col);

	ImVec2 text_pos{pos.x, pos.y + padding.y};
	PushStyleColor(ImGuiCol_Text, *value ? black_col : white_col);
	RenderText(ImVec2(text_pos.x + (switch_width / 2 - false_label_size.x) / 2, text_pos.y),
			   false_label);
	text_pos.x += switch_width / 2;
	PushStyleColor(ImGuiCol_Text, *value ? white_col : black_col);
	RenderText(ImVec2(text_pos.x + (switch_width / 2 - true_label_size.x) / 2, text_pos.y),
			   true_label);
	PopStyleColor(2);

	return pressed;
}

};	 // namespace ImGui