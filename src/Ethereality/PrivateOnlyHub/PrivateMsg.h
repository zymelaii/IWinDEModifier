#pragma once

#include "../msgitem.h"

class PrivateMsg : public MsgItem {
public:
	virtual ImRect resolve(void* content, const ImRect& bb, ChatHub* hub) override;

private:
	ImDrawFlags resovleRoundCorner(uint64_t id, bool is_self, ChatHub* hub);
};
