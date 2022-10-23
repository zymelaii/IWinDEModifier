#pragma once

#include "msgitem.h"
#include "json.hpp"

class PrivateMsg : public MsgItem {
public:
	virtual ImRect resolve(void* content, const ImRect& bb, ChatHub* hub) override {
		using ContentType = nlohmann::json;
		const auto hash	  = std::hash<std::string_view>{};

		const auto& item	= *reinterpret_cast<ContentType*>(content);
		const bool	is_self = item["sender"] == "zymelaii";	  //!< 判断是否为本身
		const char* text = item["text"].get_ref<const std::string&>().c_str();	 //!< 获取消息文本
		const uint64_t id = hash(item["id"].get_ref<const std::string&>());	  //!< 获取消息编号

		ImVec2 padding{24.00, 12.00};		 //!< 消息框内边距
		float  radius{12.00};				 //!< 消息框圆角半径
		float  no_overlap_spacing = 64.00;	 //!< 消息框非重叠部分间距
		float  spare_width		  = bb.GetWidth() - no_overlap_spacing;	  //!< 可用宽度

		ImDrawFlags RectRoundFlag = resovleRoundCorner(id, is_self, hub);	//!< 消息框圆角标志

		float minw = ImMin(spare_width, 128.00f);	//!< 文本区最小宽度
		float maxw = ImMin(spare_width, 512.00f);	//!< 文本区最大宽度

		maxw = ImMax(minw, maxw);
		minw = ImMin(minw, maxw);

		ImVec2 TextSize = ImGui::CalcTextSize(text, nullptr, false, maxw);	 //!< 文本区估计大小
		ImVec2 InnerSize{ImMax(ImMin(TextSize.x, maxw), minw),
						 ImMax(TextSize.y, ImGui::GetFontSize())};	 //!< 文本区实际大小

		ImRect ValidRegion;	  //!< 有效区域矩形盒
		ValidRegion.Min.x = is_self ? bb.Max.x - InnerSize.x - padding.x : bb.Min.x;
		ValidRegion.Min.y = bb.Min.y;
		ValidRegion.Max.x = ValidRegion.Min.x + InnerSize.x + padding.x;
		ValidRegion.Max.y = ValidRegion.Min.y + InnerSize.y + padding.y;

		ImVec2 TextOrigin;	 //!< 文本区原点
		TextOrigin.x = ValidRegion.Min.x + padding.x * 0.50;
		TextOrigin.y = ValidRegion.Min.y + padding.y * 0.50;

		ImU32 TextBgColor =
			is_self ? ImColor(204, 242, 207) : ImColor(255, 255, 255);	 //!< 文本区背景色
		ImU32 TextFgColor	  = ImColor(0, 0, 0);						 //!< 文本区前景色
		ImU32 TextShadowColor = ImColor(236, 237, 238, 200);			 //!< 文本区阴影色

		if (hub->shouldRender()) {
			auto drawlist = ImGui::GetWindowDrawList();

			const auto vrmin = ValidRegion.Min, vrmax = ValidRegion.Max;
			const auto font = ImGui::GetFont();
			const auto size = ImGui::GetFontSize();

			drawlist->AddRect(vrmin, vrmax, TextShadowColor, radius, RectRoundFlag, 4.00);
			drawlist->AddRectFilled(vrmin, vrmax, TextBgColor, radius, RectRoundFlag);
			drawlist->AddText(font, size, TextOrigin, TextFgColor, text, nullptr, maxw);
		}

		ImRect ok_bb = bb;	 //!< 实际使用的矩形盒
		ok_bb.Max.y	 = ImMax(ok_bb.Min.y + ValidRegion.GetHeight(), ok_bb.Min.y);
		return ok_bb;
	}

private:
	ImDrawFlags resovleRoundCorner(uint64_t id, bool is_self, ChatHub* hub) {
		ImDrawFlags flag;

		using ContentType	 = nlohmann::json;
		const auto prev_item = reinterpret_cast<ContentType*>(hub->request(hub->queryPrevious(id)));
		const auto next_item = reinterpret_cast<ContentType*>(hub->request(hub->queryNext(id)));

		if (is_self) {
			flag = ImDrawFlags_RoundCornersLeft;
			if (!prev_item || prev_item->at("sender") != "zymelaii") {
				flag |= ImDrawFlags_RoundCornersTopRight;
			}
			if (!next_item || next_item->at("sender") != "zymelaii") {
				flag |= ImDrawFlags_RoundCornersBottomRight;
			}
		} else {
			flag = ImDrawFlags_RoundCornersRight;
			if (!prev_item || prev_item->at("sender") == "zymelaii") {
				flag |= ImDrawFlags_RoundCornersTopLeft;
			}
			if (!next_item || next_item->at("sender") == "zymelaii") {
				flag |= ImDrawFlags_RoundCornersBottomLeft;
			}
		}

		return flag;
	}
};