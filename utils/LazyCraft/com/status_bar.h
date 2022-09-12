#include "imgui/imgui_internal.h"
#include <imgui/imgui.h>

namespace com {

struct IStatusBar {
	virtual bool prepare()		= 0;
	virtual void render() const = 0;
};

}	// namespace com

namespace com::impl {

using com::IStatusBar;

struct StatusBar final : public IStatusBar {
	virtual bool prepare() override;
	virtual void render() const override;

	void setBatteryCapacityFont(ImFont* font) const;
	void setTextFont(ImFont* font) const;

private:
	const ImColor col_bg{128, 128, 128};
	const ImColor col_battery{255, 255, 255};
	const ImColor col_date{240, 240, 240};
	const ImColor col_battery_capacity{0, 0, 0};

	ImVec2 pos_battery_capacity{};
	ImVec2 pos_date{};

	ImRect rc_bb{};
	ImRect rc_battery_body{};
	ImRect rc_battery_head{};

	char s_date[20];
	char s_battery_capacity[4];

	bool show_battery{true};

	mutable ImFont* font_battery_capacity{nullptr};
	mutable ImFont* font_text{nullptr};
};

}	// namespace com::impl