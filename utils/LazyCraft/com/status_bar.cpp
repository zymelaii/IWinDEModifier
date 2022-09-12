#include "status_bar.h"

#include <imgui/imgui_internal.h>
#include <windows.h>

namespace com::impl {

bool StatusBar::prepare() {
	auto w = ImGui::GetCurrentWindow();
	if (w->SkipItems) return false;

	auto	   canvas = w->DrawList;
	const auto id	  = w->GetID("IStatusBar");

	const auto rc = w->InnerRect;

	rc_bb		= w->InnerRect;
	rc_bb.Max.y = rc.Min.y + 20;

	ImGui::ItemSize(rc_bb.GetSize());
	if (!ImGui::ItemAdd(rc_bb, id)) return false;

	const auto spacing = ImGui::GetStyle().ItemSpacing.x;
	float	   offset  = rc_bb.Max.x - spacing;

	SYSTEM_POWER_STATUS power_status{};
	SYSTEMTIME			time{};
	GetSystemPowerStatus(&power_status);
	GetLocalTime(&time);

	show_battery = power_status.BatteryLifePercent != 0xff;
	if (show_battery) {
		const float	 unit = rc_bb.GetHeight() * 0.54 / 3;
		const ImVec2 body(unit * 5, unit * 3);
		const ImVec2 head(unit, body.y * 0.6);
		float		 posx = rc_bb.Max.x - spacing - (body.x + head.x);
		float		 posy = rc_bb.Min.y + (rc_bb.GetHeight() - body.y) / 2;

		rc_battery_body = ImRect(posx, posy, posx + body.x, posy + body.y);

		posx += body.x;
		posy += (body.y - head.y) / 2;

		rc_battery_head = ImRect(posx - 1, posy, posx + head.x, posy + head.y);

		sprintf_s(s_battery_capacity, "%d", power_status.BatteryLifePercent);

		ImGui::PushFont(font_battery_capacity);
		auto size = ImGui::CalcTextSize(s_battery_capacity);
		ImGui::PopFont();

		pos_battery_capacity = ImVec2(rc_bb.Max.x - spacing - head.x - (body.x + size.x) / 2,
									  rc_bb.Min.y + (rc_bb.GetHeight() - size.y) / 2);
		offset -= spacing + body.x + head.x;
	}

	sprintf_s(s_date,
			  "%4d-%02d-%02d %02d:%02d:%02d",
			  time.wYear,
			  time.wMonth,
			  time.wDay,
			  time.wHour,
			  time.wMinute,
			  time.wSecond);

	ImGui::PushFont(font_text);
	auto size = ImGui::CalcTextSize(s_date);
	ImGui::PopFont();

	pos_date = ImVec2(offset - size.x, rc_bb.Min.y + (rc_bb.GetHeight() - size.y) / 2);

	return true;
}

void StatusBar::render() const {
	auto canvas = ImGui::GetCurrentWindow()->DrawList;

	canvas->AddRectFilled(rc_bb.Min, rc_bb.Max, col_bg, 4, ImDrawFlags_RoundCornersBottom);

	if (show_battery) {
		canvas->AddRectFilled(
			rc_battery_body.Min, rc_battery_body.Max, col_battery, 2, ImDrawFlags_RoundCornersAll);

		canvas->AddRectFilled(rc_battery_head.Min,
							  rc_battery_head.Max,
							  col_battery,
							  2,
							  ImDrawFlags_RoundCornersRight);

		ImGui::PushFont(font_battery_capacity);
		canvas->AddText(ImGui::GetFont(),
						ImGui::GetFontSize(),
						pos_battery_capacity,
						col_battery_capacity,
						s_battery_capacity);
		ImGui::PopFont();
	}

	ImGui::PushFont(font_text);
	canvas->AddText(ImGui::GetFont(), ImGui::GetFontSize(), pos_date, col_date, s_date);
	ImGui::PopFont();
}

void StatusBar::setBatteryCapacityFont(ImFont* font) const {
	assert(font != nullptr && "invalid params for StatusBar::setBatteryCapacityFont");
	font_battery_capacity = font;
}

void StatusBar::setTextFont(ImFont* font) const {
	assert(font != nullptr && "invalid params for StatusBar::setTextFont");
	font_text = font;
}

}	// namespace com::impl
