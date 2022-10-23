#pragma once

#include <string>

#include "msgitem.h"
#include "PrivateMsg.hpp"

class PrivateMsgFactory : public MsgItemFactory {
public:
	virtual MsgItem* build(const std::string& type) override { return new PrivateMsg; }
};