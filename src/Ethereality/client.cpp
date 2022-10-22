#include <stddef.h>
#include <string.h>
#include <malloc.h>
#include <algorithm>
#include <iostream>
#include <compare>
#include <memory>
#include <string>
#include <regex>
#include <list>
#include <any>
#include <map>
#include <set>

#include "chathub.h"

class PrivateTextMsgItem : public ChatHubItem {
protected:
	ImVec2 Padding_{24.00, 12.00};	 //!< 消息框内边距
	float  RectRadius_{12.00};		 //!< 消息框圆角半径

public:
	using ContentType = struct {
		bool		self;
		const char* text;
	};

	virtual float invoke(void* content, const ImRect& bb, ChatHub* hub) override {
		using namespace ImGui;

		auto&  msg	  = *reinterpret_cast<ContentType*>(content);
		ImVec2 cursor = bb.Min;
		ImVec2 minmax = hub->GetMinMaxMsgItemWidth();

		const auto prev = reinterpret_cast<ContentType*>(hub->GetPrevItem());
		const auto next = reinterpret_cast<ContentType*>(hub->GetNextItem());

		ImDrawFlags test_RectRoundFlag;
		if (msg.self) {
			test_RectRoundFlag = ImDrawFlags_RoundCornersLeft;
			if (!prev || prev->self != msg.self)
				test_RectRoundFlag |= ImDrawFlags_RoundCornersTopRight;
			if (!next || next->self != msg.self)
				test_RectRoundFlag |= ImDrawFlags_RoundCornersBottomRight;
		} else {
			test_RectRoundFlag = ImDrawFlags_RoundCornersRight;
			if (!prev || prev->self != msg.self)
				test_RectRoundFlag |= ImDrawFlags_RoundCornersTopLeft;
			if (!next || next->self != msg.self)
				test_RectRoundFlag |= ImDrawFlags_RoundCornersBottomLeft;
		}

		float  test_TextWrapWidth = minmax.y - Padding_.x;
		ImVec2 test_MsgSize{CalcTextSize(msg.text, nullptr, false, test_TextWrapWidth)};

		float test_InnerRectWidth  = ImMax(test_MsgSize.x, minmax.x - Padding_.x);
		float test_InnerRectHeight = ImMax(test_MsgSize.y, GetFontSize());

		ImVec2 test_Cursor;
		if (msg.self) {
			test_Cursor.x = bb.Max.x - test_InnerRectWidth - Padding_.x;
			test_Cursor.y = bb.Min.y;
		} else {
			test_Cursor.x = bb.Min.x;
			test_Cursor.y = bb.Min.y;
		}

		ImVec2 test_MsgOrigin, test_RectRBPoint;
		test_MsgOrigin.x   = test_Cursor.x + Padding_.x * 0.50;
		test_RectRBPoint.x = test_MsgOrigin.x + test_InnerRectWidth + Padding_.x * 0.50;
		test_MsgOrigin.y   = test_Cursor.y + Padding_.y * 0.50;
		test_RectRBPoint.y = test_MsgOrigin.y + test_InnerRectHeight + Padding_.y * 0.50;

		ImU32 test_Color;
		if (msg.self) {
			test_Color = ImColor(204, 242, 207);
		} else {
			test_Color = ImColor(255, 255, 255);
		}

		ImU32 test_ShadowColor = ImColor(236, 237, 238, 200);

		auto drawlist = GetWindowDrawList();

		drawlist->AddRect(
			test_Cursor, test_RectRBPoint, test_ShadowColor, RectRadius_, test_RectRoundFlag, 4.00);
		drawlist->AddRectFilled(
			test_Cursor, test_RectRBPoint, test_Color, RectRadius_, test_RectRoundFlag);

		drawlist->AddText(ImGui::GetFont(),
						  ImGui::GetFontSize(),
						  test_MsgOrigin,
						  ImColor(0, 0, 0),
						  msg.text,
						  nullptr,
						  test_TextWrapWidth);

		return test_RectRBPoint.y - test_Cursor.y;
	}
};

class TextChatHub : public ChatHub {
private:
	PrivateTextMsgItem PrivateItem_;

	std::list<std::pair<int, const char*>>			 items_;
	std::list<std::pair<int, const char*>>::iterator cursor_;

	ImVec2	pos_;
	ImVec2	size_;
	ImFont* font_;

public:
	virtual void* GetPrevItem() override {
		auto it = cursor_;
		return it == items_.begin() ? nullptr : &*--it;
	}
	virtual void* GetNextItem() override {
		auto it = cursor_;
		return ++it == items_.end() ? nullptr : &*it;
	}
	virtual ChatHubItem* build(void* data) override { return &PrivateItem_; }

	TextChatHub() = default;
	~TextChatHub() {
		for (auto& e : items_) {
			free((void*)e.second);
		}
	}

	TextChatHub* push(bool self, const char* text) {
		items_.push_back({self, _strdup(text)});
		return this;
	}

	ImVec2 GetMinSize() const {
		ImRect padding		= GetChatPadding();
		float  vert_spacing = GetChatItemVertSpacing();
		float  minwid = padding.GetWidth() + GetMinMaxMsgItemWidth().y + GetMsgItemOverlapSpacing();
		float  minhei = padding.GetHeight() + GetInputPadding().GetHeight() + vert_spacing * 2.00;
		return ImVec2(minwid, minhei);
	}

	void config(ImVec2 pos, ImVec2 size, ImFont* font) {
		pos_  = pos;
		font_ = font;
		size_ = ImMax(GetMinSize(), size);
	}

	void render() {
		using namespace ImGui;

		ImGuiContext& g		   = *GImGui;
		ImGuiWindow*  window   = g.CurrentWindow;
		ImDrawList*	  drawlist = window->DrawList;
		if (window->SkipItems) return;

		int	   id = window->GetID("#TextChatHub");
		ImRect bb{{0, 0}, size_};
		bb.Translate(window->InnerRect.Min);
		bb.Translate(pos_);

		ItemSize(bb.GetSize());
		if (!ItemAdd(bb, id)) return;

		PushClipRect(bb.Min, bb.Max, true);
		drawlist->AddRectFilled(bb.Min, bb.Max, ImColor(248, 249, 250));   //!< background

		ImRect padding = GetChatPadding();

		PushFont(font_);
		ImRect item_bb;
		item_bb.Min.x = bb.Min.x + padding.Min.x;
		item_bb.Max.x = bb.Max.x - padding.Max.x;
		item_bb.Min.y = item_bb.Max.y = bb.Min.y + padding.Min.y + GetChatItemVertSpacing();

		cursor_ = items_.begin();
		while (cursor_ != items_.end()) {
			void*		 content = &*cursor_;
			ChatHubItem* item	 = build(content);

			float height = item->invoke(content, item_bb, this);
			item_bb.TranslateY(height + GetChatItemVertSpacing());

			if (item_bb.Max.y > bb.Max.y - padding.Max.y) break;

			++cursor_;
		}
		PopFont();

		PopClipRect();
	}
};

class Ethereality : public ImGuiApplication {
private:
	std::unique_ptr<Proxy::FontProxy> font_ascii = Proxy::FontProxy::require();
	std::unique_ptr<TextChatHub>	  chathub	 = std::make_unique<TextChatHub>();

public:
	Ethereality(const char* title, int width, int height) { build(title, width, height, 100, 100); }

	void configure() override {
		auto CN_glyph = ImGui::GetIO().Fonts->GetGlyphRangesChineseFull();
		font_ascii->add(R"(assets\DroidSans.ttf)", 20.00)
			->add(R"(assets\YaHei Consolas Hybrid.ttf)", 20.00, CN_glyph)
			->build(pd3dDevice_);
		chathub->config({0, 0}, {800, 800}, font_ascii->get());
	}

	void render() override {
		using namespace ImGui;

		ImVec2 MinSize = chathub->GetMinSize();
		SetNextWindowSizeConstraints({MinSize.x + GetStyle().WindowPadding.x, MinSize.y},
									 GetIO().DisplaySize);
		if (Begin("Ethereality ChatHub")) {
			chathub->config({0, 0},
							ImVec2(GetCurrentWindow()->InnerRect.GetWidth(),
								   GetCurrentWindow()->InnerRect.GetHeight() - 64.00),
							font_ascii->get());
			chathub->render();

			static char inbuf[1024];
			PushFont(font_ascii->get());

			InputTextWithHint("ChatHub#Input", "Type message", inbuf, sizeof(inbuf));
			auto window = GetCurrentWindow();
			if (GetFocusID() == window->GetID("ChatHub#Input") &&
				GetIO().KeysDown[ImGuiKey_Enter]) {
				auto id = window->GetID("ChatHub#Input");
				SetActiveID(id, window);

				if (inbuf[0]) chathub->push(rand() % 2, inbuf);

				auto state = GetInputTextState(id);
				state->ClearText();
				state->CursorFollow = true;
			}

			PopFont();

			End();
		}
	}
};

int main(int argc, char* argv[]) {
	auto app = std::make_unique<Ethereality>("Client Demo", 600, 400);
	return app->exec();
}