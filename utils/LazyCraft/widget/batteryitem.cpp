#include "batteryitem.h"

#include <windows.h>

BatteryItem::BatteryItem() {
	setRelHeight(20.00f);
}

const ImRect BatteryItem::rect() const {
	const float unit = rel_height_ * 0.54 / 3;
	ImRect		bb(0.00f, 0.00f, unit * 6, unit * 3);
	bb.Translate(pos_);
	return bb;
}

const ImVec2 BatteryItem::cursor() const {
	return pos_;
}

bool BatteryItem::prepare(const com::IUtility* parent) {
	if (parent != nullptr) {
		const auto& bb	= parent->rect();
		const auto& pos = parent->cursor();
		setRelHeight(bb.GetHeight());
		setPosition(pos.x, pos.y);
	}

	SYSTEM_POWER_STATUS power_status{};
	GetSystemPowerStatus(&power_status);

	if (power_status.BatteryLifePercent > 100) return false;

	sprintf_s(s_capacity_, "%d", power_status.BatteryLifePercent);

	if (font_) ImGui::PushFont(font_);
	auto sz_text = ImGui::CalcTextSize(s_capacity_);
	if (font_) ImGui::PopFont();

	const auto	bb	 = rect();
	const float unit = rel_height_ * 0.54 / 3;
	sz_body_		 = ImVec2(unit * 5, unit * 3);
	sz_head_		 = ImVec2(unit, unit * 1.8);

	pos_text_.x = bb.Min.x + (sz_body_.x - sz_text.x) * 0.5;
	pos_text_.y = bb.Min.y + (bb.GetHeight() - sz_text.y) * 0.5;

	return true;
}

void BatteryItem::render() const {
	auto canvas = ImGui::GetCurrentWindow()->DrawList;
	int	 flag	= 0;

	ImRect rc_body({0, 0}, sz_body_);
	rc_body.Translate(pos_);

	ImRect rc_head({0, 0}, sz_head_);
	rc_head.Translate(ImVec2(rc_body.Max.x, pos_.y + (sz_body_.y - sz_head_.y) * 0.5));
	rc_head.Min.x -= 1.00f;

	flag = ImDrawFlags_RoundCornersAll;
	canvas->AddRectFilled(rc_body.Min, rc_body.Max, ImColor(255, 255, 255), 2, flag);

	flag = ImDrawFlags_RoundCornersRight;
	canvas->AddRectFilled(rc_head.Min, rc_head.Max, ImColor(255, 255, 255), 2, flag);

	if (font_) ImGui::PushFont(font_);
	canvas->AddText(
		ImGui::GetFont(), ImGui::GetFontSize(), pos_text_, ImColor(0, 0, 0), s_capacity_);
	if (font_) ImGui::PopFont();
}

BatteryItem* BatteryItem::setRelHeight(float height) {
	assert(height > 0);
	rel_height_ = height;
	return this;
}

BatteryItem* BatteryItem::setPosition(float x, float y) {
	pos_ = ImVec2(x, y);
	return this;
}

BatteryItem* BatteryItem::setCapacityFont(ImFont* font) {
	font_ = font;
	return this;
}