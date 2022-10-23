#pragma once

#include <vector>
#include <memory>
#include <string_view>
#include <functional>

#include "msgitem.h"
#include "json.hpp"

#include "PrivateMsg.hpp"
#include "PrivateMsgFactory.hpp"

class PrivateOnlyHub : public ChatHub {
private:
	using FactoryModel = std::unique_ptr<MsgItemFactory>;
	using json		   = nlohmann::json;
	using ItemType	   = std::pair<uint64_t, json>;

	mutable FactoryModel  factory_;
	bool				  should_render_;
	std::vector<ItemType> items_;
	ImVec2				  size_;
	float				  scroll_state_;
	ImVec2				  scroll_range_;

public:
	PrivateOnlyHub()
		: should_render_(true)
		, factory_(nullptr)
		, scroll_state_(0.00)
		, scroll_range_{0.00, 0.00}
		, items_{}
		, size_(getMinSizeConstraint()) {
		bindFactory(new PrivateMsgFactory);
	}

	void push(bool is_self, const std::string& msgid, const std::string& text) {
		json item{
			{"type", "private"},
			{"id", msgid},
			{"sender", is_self ? "zymelaii" : "robot"},
			{"receiver", is_self ? "robot" : "zymelaii"},
			{"text", text},
		};
		items_.push_back({std::hash<std::string_view>{}(msgid), item});
	}

	json parse(uint64_t id) { return *reinterpret_cast<json*>(request(id)); }

	float getScrollState() const { return scroll_state_; }

	void scroolTo(float cursor) {
		scroll_state_ = ImClamp(cursor, scroll_range_.x, scroll_range_.y);
	}

	void resize(ImVec2 size) {
		//! 这段代码为什么能正常运行我还不明确，但是重要的是，它正常运行了
		//! 如果后续出现什么BUG的话，就考虑来修下这儿吧
		float delta		= size.y - size_.y;
		scroll_range_.y = ImMax(scroll_range_.x, scroll_range_.y - delta);
		scroll_state_	= ImClamp(scroll_state_ - delta, scroll_range_.x, scroll_range_.y);
		if (abs(size.x - size_.x) > 1e-4) scroll_range_.y = scroll_range_.x;
		size_ = size;
	}

	void render(ImVec2 pos, ImVec2 size) {
		ImGuiWindow* window	  = ImGui::GetCurrentWindow();
		ImDrawList*	 drawlist = window->DrawList;
		if (window->SkipItems) return;

		resize(size);

		int	   id = window->GetID("#PrivateMsgFactory");
		ImRect bb{{0, 0}, size};
		bb.Translate(window->InnerRect.Min);
		bb.Translate(pos);

		if (!ImGui::ItemAdd(bb, id)) return;

		bool hovered, held;
		if (ImGui::ButtonBehavior(bb, id, &hovered, &held)) {
			ImGui::SetFocusID(id, window);
		}

		ImU32  BgColor = ImColor(248, 249, 250);	//!< 背景色
		ImRect Padding{16.00, 4.00, 16.00, 4.00};	//!< 聊天面板内边距
		float  VertSpacing{6.00};					//!< 消息项垂直间距

		ImGui::PushClipRect(bb.Min, bb.Max, true);
		drawlist->AddRectFilled(bb.Min, bb.Max, BgColor);

		ImRect item_bb;
		float  baseline = bb.Min.y + Padding.Min.y;
		item_bb.Min.x	= bb.Min.x + Padding.Min.x;
		item_bb.Max.x	= bb.Max.x - Padding.Max.x;
		item_bb.Min.y = item_bb.Max.y = baseline + VertSpacing - scroll_state_;

		auto it = items_.begin();
		while (it != items_.end()) {
			std::unique_ptr<MsgItem> item{factory_->build("PrivateMsg")};

			should_render_ = false;
			ImRect full_bb = item->resolve(&it->second, item_bb, this);

			if (bb.Overlaps(full_bb)) {
				should_render_ = true;
				item->resolve(&it->second, item_bb, this);

				if (it->second["sender"].get_ref<const std::string&>() != "zymelaii") {
					ImRect select_bb{
						{bb.Min.x, ImCeil(full_bb.Min.y - VertSpacing * 0.50 - 0.99)},
						{bb.Max.x, ImFloor(full_bb.Max.y + VertSpacing * 0.50 + 0.99)}};
					drawlist->AddRectFilled(
						select_bb.Min, select_bb.Max, ImColor(124, 185, 218, 64));
				}
			}

			item_bb.TranslateY(full_bb.GetHeight() + VertSpacing);
			if (item_bb.Min.y > bb.Max.y) break;
			++it;
		}

		float window_height = bb.GetHeight() - Padding.Min.y;
		float bottomline	= item_bb.Min.y + scroll_state_;
		float delta			= bottomline - baseline;
		scroll_range_.y		= ImMax(scroll_range_.y, delta - window_height);

		ImGui::PopClipRect();	//!< ChatHub
	}

public:
	virtual bool bindFactory(MsgItemFactory* factory) const override {
		if (!factory) return false;
		factory_.reset(factory);
		return true;
	}

	virtual bool shouldRender() const override { return should_render_; }

	virtual ImVec2 getMinSizeConstraint() const override {
		ImRect padding{16.00, 4.00, 16.00, 4.00};		  //!< 聊天面板内边距
		ImRect input_padding{16.00, 4.00, 16.00, 4.00};	  //!< 输入框内边距
		float  vert_spacing		  = 6.00;				  //!< 消息项垂直间距
		float  box_minwid		  = 256.00;				  //!< 消息框参考最小宽度
		float  no_overlap_spacing = 64.00;				  //!< 消息框非重叠部分间距

		float minwid = padding.GetWidth() + box_minwid + no_overlap_spacing;
		// float minhei = padding.GetHeight() + input_padding.GetHeight() + vert_spacing * 2.00;
		float minhei = minwid;

		return {minwid, minhei};
	};

	virtual uint64_t queryNext(uint64_t id) override {
		if (id == 0) return items_.size() == 0 ? 0 : items_[0].first;
		bool captured = false;
		for (const auto& [this_id, item] : items_) {
			if (captured) return this_id;
			if (this_id == id) captured = true;
		}
		return 0;
	}

	virtual uint64_t queryPrevious(uint64_t id) override {
		if (id == 0) return items_.size() == 0 ? 0 : items_[0].first;
		uint64_t prev_id = 0;
		for (const auto& [this_id, item] : items_) {
			if (this_id == id) return prev_id;
			prev_id = this_id;
		}
		return 0;
	}

	virtual void* request(uint64_t id) override {
		if (id == 0) return nullptr;
		for (auto& [this_id, item] : items_) {
			if (this_id == id) return reinterpret_cast<void*>(&item);
		}
		return nullptr;
	}

	virtual bool jumpToItem(uint64_t id) override { return false; }
};