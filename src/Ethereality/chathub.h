#pragma once

#include <share/ui/backend.h>
#include <share/utils/texture.h>
#include <share/utils/proxy/fontproxy.h>

class ChatHubItem;

class ChatHub {
protected:
	ImRect ChatPadding_{16.00, 4.00, 16.00, 4.00};	  //!< 聊天面板内边距
	ImRect InputPadding_{16.00, 4.00, 16.00, 4.00};	  //!< 输入框内边距
	float  ChatItemVertSpacing_{6.00};				  //!< 消息项垂直间距
	float  MsgItemOverlapSpacing_{64.00};			  //!< 双方消息框重叠部分的间距
	ImVec2 MinMaxMsgItemWidth_{128.00, 256.00};		  //!< 消息框最小最大宽度

public:
	const ImRect GetChatPadding() const { return ChatPadding_; }
	const ImRect GetInputPadding() const { return InputPadding_; }
	const float	 GetChatItemVertSpacing() const { return ChatItemVertSpacing_; }
	const float	 GetMsgItemOverlapSpacing() const { return MsgItemOverlapSpacing_; }
	const ImVec2 GetMinMaxMsgItemWidth() const { return MinMaxMsgItemWidth_; }

	virtual void*		 GetPrevItem()	   = 0;
	virtual void*		 GetNextItem()	   = 0;
	virtual ChatHubItem* build(void* data) = 0;
};

class ChatHubItem {
public:
	virtual float invoke(void* content, const ImRect& bb, ChatHub* hub) = 0;
};