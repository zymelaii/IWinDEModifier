#include "statusbar.h"

#include <windows.h>

namespace com::impl {

const ImRect StatusBar::rect() const {
	const auto w  = ImGui::GetCurrentWindow();
	auto	   rc = w->InnerRect;

	rc.Max.y = rc.Min.y + height_;

	return rc;
}

const ImVec2 StatusBar::cursor() const {
	switch (align_) {
		case Alignment::Left: return cursor_pos_[0];
		case Alignment::Right: return cursor_pos_[1];
	}

	return {};
}

bool StatusBar::prepare(const IUtility* parent) {
	auto w = ImGui::GetCurrentWindow();
	if (w->SkipItems) return false;

	const auto bb	   = rect();
	const auto spacing = ImGui::GetStyle().ItemSpacing.x;

	ImGui::ItemSize(bb);

	cursor_pos_[0] = bb.Min;
	cursor_pos_[1] = ImVec2(bb.Max.x, bb.Min.y);

	for (auto& e : utils_) {
		auto& [util, align, ok] = e;

		const auto ubb = util->rect();
		align_		   = align;

		auto prev_cursor = cursor();

		switch (align_) {
			case Alignment::Left: {
				cursor_pos_[0].x += spacing;
				cursor_pos_[0].y = bb.Min.y + (bb.GetHeight() - ubb.GetHeight()) * 0.5;
				ok = util->prepare(this);
				cursor_pos_[0].x += ubb.GetWidth();
				if (!ok) cursor_pos_[0] = prev_cursor;
				break;
			}
			case Alignment::Right: {
				cursor_pos_[1].x -= spacing + ubb.GetWidth();
				cursor_pos_[1].y = bb.Min.y + (bb.GetHeight() - ubb.GetHeight()) * 0.5;
				ok = util->prepare(this);
				if (!ok) cursor_pos_[1] = prev_cursor;
				break;
			}
		}
	}

	return true;
}

void StatusBar::render() const {
	auto canvas = ImGui::GetCurrentWindow()->DrawList;
	auto flag	= ImDrawFlags_RoundCornersBottom;

	const auto rc_bb = rect();

	canvas->AddRectFilled(rc_bb.Min, rc_bb.Max, ImColor(128, 128, 128), 4, flag);

	for (auto& e : utils_) {
		auto& [util, align, ok] = e;
		if (ok) {
			util->render();
		}
	}
}

StatusBar* StatusBar::add_util(IUtility* util, Alignment align) {
	utils_.push_back({Utility(util), align, true});
	return this;
}

}	// namespace com::impl
