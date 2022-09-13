#include "dateitem.h"

#include <windows.h>

const ImRect DateItem::rect() const {
	if (font_) ImGui::PushFont(font_);
	auto sz_text = ImGui::CalcTextSize(s_date_);
	if (font_) ImGui::PopFont();

	ImRect bb({0.00f, 0.00f}, sz_text);
	bb.Translate(pos_);

	return bb;
}

const ImVec2 DateItem::cursor() const {
	return pos_;
}

bool DateItem::prepare(const com::IUtility* parent) {
	if (parent != nullptr) {
		const auto &pos = parent->cursor();
		setPosition(pos.x, pos.y);
	}

	SYSTEMTIME time{};
	GetLocalTime(&time);

	sprintf_s(s_date_,
			  "%4d-%02d-%02d %02d:%02d:%02d",
			  time.wYear,
			  time.wMonth,
			  time.wDay,
			  time.wHour,
			  time.wMinute,
			  time.wSecond);

	return true;
}

void DateItem::render() const {
	auto canvas = ImGui::GetCurrentWindow()->DrawList;

	if (font_) ImGui::PushFont(font_);
	canvas->AddText(ImGui::GetFont(), ImGui::GetFontSize(), pos_, ImColor(0, 0, 0), s_date_);
	if (font_) ImGui::PopFont();
}

DateItem* DateItem::setPosition(float x, float y) {
	pos_ = ImVec2(x, y);
	return this;
}

DateItem* DateItem::setFont(ImFont* font) {
	font_ = font;
	return this;
}