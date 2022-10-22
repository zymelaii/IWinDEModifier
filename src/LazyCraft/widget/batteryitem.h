#pragma once

#include "../com/com_interface.h"

struct BatteryItem : public com::IUtility {
	virtual const ImRect rect() const override;
	virtual const ImVec2 cursor() const override;

	virtual bool prepare(const com::IUtility* parent) override;
	virtual void render() const override;

	BatteryItem();

	BatteryItem* setRelHeight(float height);
	BatteryItem* setPosition(float x, float y);
	BatteryItem* setCapacityFont(ImFont* font);

private:
	float	rel_height_{};
	ImVec2	pos_{0.00f, 0.00f};
	ImVec2	pos_text_{};
	ImVec2	sz_body_{};
	ImVec2	sz_head_{};
	char	s_capacity_[4];
	ImFont* font_{nullptr};
};