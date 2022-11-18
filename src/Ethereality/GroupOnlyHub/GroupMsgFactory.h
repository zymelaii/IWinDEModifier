#pragma once

#include <string>

#include "../msgitem.h"

class GroupMsgFactory : public MsgItemFactory {
public:
	virtual MsgItem* build(const std::string& type) override;
};