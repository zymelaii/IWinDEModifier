#pragma once

#include <string>

#include "../msgitem.h"

class PrivateMsgFactory : public MsgItemFactory {
public:
	virtual MsgItem* build(const std::string& type) override;
};