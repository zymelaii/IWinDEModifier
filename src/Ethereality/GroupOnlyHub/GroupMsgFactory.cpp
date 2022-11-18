#include "GroupMsgFactory.h"
#include "GroupMsg.h"

MsgItem* GroupMsgFactory::build(const std::string& type) {
	return new GroupMsg;
}