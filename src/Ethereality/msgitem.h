#pragma once

#include <string>
#include <share/ui/backend.h>

class ChatHub;

class MsgItem {
public:
	virtual ImRect resolve(void* content, const ImRect& bb, ChatHub* hub) = 0;	 //!< 处理消息对象
};

class MsgItemFactory {
public:
	virtual MsgItem* build(const std::string& type) = 0;   //!< 构建消息对象
};

class ChatHub {
public:
	virtual bool   bindFactory(MsgItemFactory* factory) const = 0;	 //!< 绑定消息对象工厂
	virtual bool   shouldRender() const						  = 0;	 //!< 是否执行渲染
	virtual ImVec2 getMinSizeConstraint() const				  = 0;	 //!< 获取最小尺寸

	virtual uint64_t queryNext(uint64_t id)		= 0;   //!< 获取下一条消息
	virtual uint64_t queryPrevious(uint64_t id) = 0;   //!< 获取上一条消息
	virtual void*	 request(uint64_t id)		= 0;   //!< 从消息编号获取消息内容
	virtual bool	 jumpToItem(uint64_t id)	= 0;   //!< 跳转至消息
};