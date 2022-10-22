#include "widgets.h"

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

void AssocViewerItem(const char* assoc) {
	ImGuiWindow* window = GetCurrentWindow();
	char		 sid[256]{"AssocViewerItem#"};
	strcat_s(sid, assoc);
	ImGuiID		  id = window->GetID(sid);
	ImGuiContext& g	 = *GImGui;

	float height = GetFrameHeight() * 2.8;

	PushStyleColor(ImGuiCol_ChildBg, ImVec4(1, 0.9805, 0.9375, 0.72));
	PushStyleVar(ImGuiStyleVar_ChildRounding, height * 0.25f);
	PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(GetFontSize(), GetFontSize()));

	BeginChild(sid, ImVec2(0, height), false, ImGuiWindowFlags_NoScrollbar);
	Text("%s", assoc);
	EndChild();

	PopStyleVar(2);
	PopStyleColor();
}

bool TreeNode(const char* label, bool* ppressed, ImGuiTreeNodeFlags flags) {
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems) return false;

	ImGuiID			  id			= window->GetID(label);
	ImGuiContext&	  g				= *GImGui;
	const char*		  label_end		= nullptr;
	const ImGuiStyle& style			= g.Style;
	const bool		  display_frame = (flags & ImGuiTreeNodeFlags_Framed) != 0;
	const ImVec2	  padding =
		 (display_frame || (flags & ImGuiTreeNodeFlags_FramePadding))
				 ? style.FramePadding
				 : ImVec2(style.FramePadding.x,
					  ImMin(window->DC.CurrLineTextBaseOffset, style.FramePadding.y));

	if (!label_end) label_end = FindRenderedTextEnd(label);
	const ImVec2 label_size = CalcTextSize(label, label_end, false);


	const float frame_height =
		ImMax(ImMin(window->DC.CurrLineSize.y, g.FontSize + style.FramePadding.y * 2),
			  label_size.y + padding.y * 2);
	ImRect frame_bb;
	frame_bb.Min.x = (flags & ImGuiTreeNodeFlags_SpanFullWidth) ? window->WorkRect.Min.x
																: window->DC.CursorPos.x;
	frame_bb.Min.y = window->DC.CursorPos.y;
	frame_bb.Max.x = window->WorkRect.Max.x;
	frame_bb.Max.y = window->DC.CursorPos.y + frame_height;
	if (display_frame) {



		frame_bb.Min.x -= IM_FLOOR(window->WindowPadding.x * 0.5f - 1.0f);
		frame_bb.Max.x += IM_FLOOR(window->WindowPadding.x * 0.5f);
	}

	const float text_offset_x = g.FontSize + (display_frame ? padding.x * 3 : padding.x * 2);
	const float text_offset_y = ImMax(padding.y, window->DC.CurrLineTextBaseOffset);
	const float text_width =
		g.FontSize + (label_size.x > 0.0f ? label_size.x + padding.x * 2 : 0.0f);
	ImVec2 text_pos(window->DC.CursorPos.x + text_offset_x, window->DC.CursorPos.y + text_offset_y);
	ItemSize(ImVec2(text_width, frame_height), padding.y);

	ImRect interact_bb = frame_bb;
	if (!display_frame &&
		(flags & (ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth)) == 0)
		interact_bb.Max.x = frame_bb.Min.x + text_width + style.ItemSpacing.x * 2.0f;

	const bool is_leaf = (flags & ImGuiTreeNodeFlags_Leaf) != 0;
	bool	   is_open = TreeNodeUpdateNextOpen(id, flags);
	if (is_open && !g.NavIdIsAlive && (flags & ImGuiTreeNodeFlags_NavLeftJumpsBackHere) &&
		!(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
		window->DC.TreeJumpToParentOnPopMask |= (1 << window->DC.TreeDepth);

	bool item_add = ItemAdd(interact_bb, id);
	g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HasDisplayRect;
	g.LastItemData.DisplayRect = frame_bb;

	if (!item_add) {
		if (is_open && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen)) TreePushOverrideID(id);
		IMGUI_TEST_ENGINE_ITEM_INFO(g.LastItemData.ID,
									label,
									g.LastItemData.StatusFlags |
										(is_leaf ? 0 : ImGuiItemStatusFlags_Openable) |
										(is_open ? ImGuiItemStatusFlags_Opened : 0));
		return is_open;
	}

	ImGuiButtonFlags button_flags = ImGuiTreeNodeFlags_None;
	if (flags & ImGuiTreeNodeFlags_AllowItemOverlap)
		button_flags |= ImGuiButtonFlags_AllowItemOverlap;
	if (!is_leaf) button_flags |= ImGuiButtonFlags_PressedOnDragDropHold;

	const float arrow_hit_x1 = (text_pos.x - text_offset_x) - style.TouchExtraPadding.x;
	const float arrow_hit_x2 =
		(text_pos.x - text_offset_x) + (g.FontSize + padding.x * 2.0f) + style.TouchExtraPadding.x;
	const bool is_mouse_x_over_arrow =
		(g.IO.MousePos.x >= arrow_hit_x1 && g.IO.MousePos.x < arrow_hit_x2);
	if (window != g.HoveredWindow || !is_mouse_x_over_arrow)
		button_flags |= ImGuiButtonFlags_NoKeyModifiers;

	if (is_mouse_x_over_arrow)
		button_flags |= ImGuiButtonFlags_PressedOnClick;
	else if (flags & ImGuiTreeNodeFlags_OpenOnDoubleClick)
		button_flags |=
			ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick;
	else
		button_flags |= ImGuiButtonFlags_PressedOnClickRelease;

	bool	   selected		= (flags & ImGuiTreeNodeFlags_Selected) != 0;
	const bool was_selected = selected;

	bool hovered, held;
	bool pressed = ButtonBehavior(interact_bb, id, &hovered, &held, button_flags);
	bool toggled = false;

	if (ppressed != nullptr) {
		*ppressed = pressed;
	}

	if (!is_leaf) {
		if (pressed && g.DragDropHoldJustPressedId != id) {
			if ((flags & (ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick)) ==
					0 ||
				(g.NavActivateId == id))
				toggled = true;
			if (flags & ImGuiTreeNodeFlags_OpenOnArrow)
				toggled |= is_mouse_x_over_arrow && !g.NavDisableMouseHover;
			if ((flags & ImGuiTreeNodeFlags_OpenOnDoubleClick) && g.IO.MouseClickedCount[0] == 2)
				toggled = true;
		} else if (pressed && g.DragDropHoldJustPressedId == id) {
			IM_ASSERT(button_flags & ImGuiButtonFlags_PressedOnDragDropHold);
			if (!is_open) toggled = true;
		}

		if (g.NavId == id && g.NavMoveDir == ImGuiDir_Left && is_open) {
			toggled = true;
			NavMoveRequestCancel();
		}
		if (g.NavId == id && g.NavMoveDir == ImGuiDir_Right && !is_open)

		{
			toggled = true;
			NavMoveRequestCancel();
		}

		if (toggled) {
			is_open = !is_open;
			window->DC.StateStorage->SetInt(id, is_open);
			g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_ToggledOpen;
		}
	}
	if (flags & ImGuiTreeNodeFlags_AllowItemOverlap) SetItemAllowOverlap();

	if (selected != was_selected)
		g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

	const ImU32			   text_col			   = GetColorU32(ImGuiCol_Text);
	ImGuiNavHighlightFlags nav_highlight_flags = ImGuiNavHighlightFlags_TypeThin;
	if (display_frame) {
		const ImU32 bg_col = GetColorU32((held && hovered) ? ImGuiCol_HeaderActive
										 : hovered		   ? ImGuiCol_HeaderHovered
														   : ImGuiCol_Header);
		RenderFrame(frame_bb.Min, frame_bb.Max, bg_col, true, style.FrameRounding);
		RenderNavHighlight(frame_bb, id, nav_highlight_flags);
		if (flags & ImGuiTreeNodeFlags_Bullet) {
			RenderBullet(window->DrawList,
						 ImVec2(text_pos.x - text_offset_x * 0.60f, text_pos.y + g.FontSize * 0.5f),
						 text_col);
		} else if (!is_leaf) {
			RenderArrow(window->DrawList,
						ImVec2(text_pos.x - text_offset_x + padding.x, text_pos.y),
						text_col,
						is_open ? ImGuiDir_Down : ImGuiDir_Right,
						1.0f);
		} else {
			text_pos.x -= text_offset_x;
		}

		if (flags & ImGuiTreeNodeFlags_ClipLabelForTrailingButton)
			frame_bb.Max.x -= g.FontSize + style.FramePadding.x;

		if (g.LogEnabled) LogSetNextTextDecoration("###", "###");

		RenderTextClipped(text_pos, frame_bb.Max, label, label_end, &label_size);
	} else {
		if (hovered || selected) {
			const ImU32 bg_col = GetColorU32((held && hovered) ? ImGuiCol_HeaderActive
											 : hovered		   ? ImGuiCol_HeaderHovered
															   : ImGuiCol_Header);
			RenderFrame(frame_bb.Min, frame_bb.Max, bg_col, false);
		}
		RenderNavHighlight(frame_bb, id, nav_highlight_flags);
		if (flags & ImGuiTreeNodeFlags_Bullet)
			RenderBullet(window->DrawList,
						 ImVec2(text_pos.x - text_offset_x * 0.5f, text_pos.y + g.FontSize * 0.5f),
						 text_col);
		else if (!is_leaf)
			RenderArrow(
				window->DrawList,
				ImVec2(text_pos.x - text_offset_x + padding.x, text_pos.y + g.FontSize * 0.15f),
				text_col,
				is_open ? ImGuiDir_Down : ImGuiDir_Right,
				0.70f);
		if (g.LogEnabled) LogSetNextTextDecoration(">", NULL);
		RenderText(text_pos, label, label_end, false);
	}

	if (is_open && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen)) TreePushOverrideID(id);
	IMGUI_TEST_ENGINE_ITEM_INFO(id,
								label,
								g.LastItemData.StatusFlags |
									(is_leaf ? 0 : ImGuiItemStatusFlags_Openable) |
									(is_open ? ImGuiItemStatusFlags_Opened : 0));
	return is_open;
}

};	 // namespace ImGui