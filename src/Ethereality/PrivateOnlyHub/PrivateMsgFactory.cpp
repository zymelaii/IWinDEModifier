#include "PrivateMsgFactory.h"
#include "PrivateMsg.h"

MsgItem* PrivateMsgFactory::build(const std::string& type) {
	return new PrivateMsg;
}