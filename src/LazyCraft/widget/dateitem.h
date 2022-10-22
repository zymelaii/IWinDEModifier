#pragma once

#include "../com/com_interface.h"

struct DateItem : public com::IUtility {
	virtual const ImRect rect() const override;
	virtual const ImVec2 cursor() const override;

	virtual bool prepare(const com::IUtility* parent) override;
	virtual void render() const override;

	DateItem* setPosition(float x, float y);
	DateItem* setFont(ImFont* font);

private:
	ImVec2	pos_{0.00f, 0.00f};
	char	s_date_[24];
	ImFont* font_{nullptr};
};