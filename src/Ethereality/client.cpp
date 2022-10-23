#include "msgitem.h"
#include "json.hpp"

#include "PrivateMsg.hpp"
#include "PrivateMsgFactory.hpp"
#include "PrivateOnlyHub.hpp"

#include <share/utils/proxy/fontproxy.h>
#include <iostream>

class Ethereality : public ImGuiApplication {
private:
	std::unique_ptr<Proxy::FontProxy> font_ascii	  = Proxy::FontProxy::require();
	PrivateOnlyHub*					  chathub		  = nullptr;
	ImVec2							  chathub_size	  = ImVec2{480.00, 640.00};
	float							  titlebar_height = 0.00;

protected:
	bool TightInput(const char* label, const char* hint, char* buffer, size_t bufsize, ImVec2 pos,
					ImVec2 size) {
		auto		window = ImGui::GetCurrentWindow();
		const auto& io	   = ImGui::GetIO();
		auto&		style  = ImGui::GetStyle();

		window->DC.CursorPos = window->Pos;
		window->DC.CursorPos.x += pos.x;
		window->DC.CursorPos.y += pos.y + window->TitleBarHeight() + style.ItemSpacing.y;

		ImVec2 labelsize = ImGui::CalcTextSize(label);
		size.x -= labelsize.x + style.ItemSpacing.x;

		ImGui::InputTextEx(label, hint, buffer, bufsize, size, 0);
		auto id = window->GetID(label);

		if (ImGui::GetFocusID() == id && io.KeysDown[ImGuiKey_Enter]) {
			ImGui::SetActiveID(id, window);
			auto state = ImGui::GetInputTextState(id);
			state->ClearText();
			state->CursorFollow = true;
			return buffer[0] != '\0';
		}

		return false;
	}

public:
	Ethereality(const char* title, int width, int height, ChatHub* hub)
		: chathub(dynamic_cast<PrivateOnlyHub*>(hub)) {
		build(title, width, height, 100, 100);
	}

	void configure() override {
		auto CN_glyph = ImGui::GetIO().Fonts->GetGlyphRangesChineseFull();
		font_ascii->add(R"(assets\DroidSans.ttf)", 20.00)
			->add(R"(assets\YaHei Consolas Hybrid.ttf)", 20.00, CN_glyph)
			->build(pd3dDevice_);
	}

	void render() override {
		ImVec2 MinSize		 = chathub->getMinSizeConstraint();
		ImVec2 WindowPadding = ImGui::GetStyle().WindowPadding;
		ImVec2 ChatHubMargin{32.00, 32.00};
		ImVec2 WindowMinSize{MinSize.x + WindowPadding.x + ChatHubMargin.x * 2,
							 MinSize.y + ChatHubMargin.y * 2};
		ImVec2 ChatHubPanelSize{
			chathub_size.x - ChatHubMargin.x * 2,
			ImMin(600.00f, chathub_size.y - ChatHubMargin.y * 2 - titlebar_height)};

		ImGui::SetNextWindowSizeConstraints(WindowMinSize, ImGui::GetIO().DisplaySize);
		ImGui::SetNextWindowSize(chathub_size, ImGuiCond_Always);

		if (ImGui::Begin("Ethereality ChatHub")) {
			ImGui::PushFont(font_ascii->get());

			//! `ChatHub' Render & Control
			chathub->render(ChatHubMargin, ChatHubPanelSize);
			chathub->scroolTo(chathub->getScrollState() - ImGui::GetIO().MouseWheel * 60.00);

			//! `ChatHub Input' Render & Control
			static char TextInput[1024] = {0};
			ImVec2		InputPos{ChatHubMargin.x, ChatHubMargin.y + ChatHubPanelSize.y};
			ImVec2		InputSize{ChatHubPanelSize.x, 0.00f};
			if (TightInput("ChatHub Input",
						   "Type message",
						   TextInput,
						   sizeof(TextInput),
						   InputPos,
						   InputSize)) {
				static uint64_t MsgDelegateID = 10000;
				char			MsgID[64];
				sprintf(MsgID, "PrivateOnlyHub#%llu", MsgDelegateID++);
				chathub->push(!!(rand() % 2), MsgID, TextInput);
			}

			ImGui::PopFont();
			chathub_size	= ImGui::GetWindowSize();
			titlebar_height = ImGui::GetCurrentWindow()->TitleBarHeight();
			ImGui::End();
		}
	}
};

int main(int argc, char* argv[]) {
	PrivateOnlyHub hub;

	hub.push(true,
			 "PrivateOnlyHub#25",
			 "骂别人不革命，便是革命者，则自己不做事，而骂别人的事做得不好，自然便是更做事者。若与"
			 "此辈理论，可以被牵连到白费唇舌，一事无成，也就是白活一世，于己于人，都无益处。我现在"
			 "得了妙法，是谣言不辩，诬蔑不洗，只管自己做事。\n——鲁迅");
	hub.push(true,
			 "PrivateOnlyHub#26",
			 "同一件事，费了苦功而达到的，也比并不费力而达到的可贵。\n——鲁迅的人生名言");
	hub.push(
		false, "PrivateOnlyHub#27", "有些人死了，但他还活着；有些人活着，但他已经死了。\n——鲁迅");
	hub.push(
		true,
		"PrivateOnlyHub#28",
		"她却是什么都记得：我的言辞，竟至于读熟了的一般，能够滔滔背诵；我的举动，就如有一张我"
		"所看不见的影片挂在眼下，叙述得如生，很细微，自然连那使我不愿再想的浅薄的电影的一闪。\n"
		"——关于鲁迅的名言大全");
	hub.push(
		false, "PrivateOnlyHub#29", "娜拉走后怎样，不是堕落，就是回来。\n——鲁迅先生的一句名言");
	hub.push(false,
			 "PrivateOnlyHub#30",
			 "在我生存时，曾经玩笑地设想：假使一个人的死亡，只是运动神经的废灭，而知觉还在，那就比"
			 "全死了更可怕。谁知道我的预想竟的中了，我自己就在证实这预想。\n——关于鲁迅的名言名句");
	hub.push(false, "PrivateOnlyHub#31", "轻敌，最容易失败。\n——鲁迅");
	hub.push(true,
			 "PrivateOnlyHub#32",
			 "父母对于子女，应该健全的产生，尽力的教育，完全的解放。\n——鲁迅");
	hub.push(true,
			 "PrivateOnlyHub#33",
			 "一见短袖子，立刻想到白臂膊，立刻想到全裸体，立刻想到生殖器，立刻想到性交，立刻想到杂"
			 "交，立刻想到私生子。中国人的想象惟在这一层能够如此跃进。\n——关于鲁迅先生的名言");
	hub.push(true,
			 "PrivateOnlyHub#34",
			 "倘有陌生的声音叫你的名字，你万不可答应它。\n——关于鲁迅先生的名言");
	hub.push(
		false, "PrivateOnlyHub#35", "世间本无路，走的人多了，也就成了路。\n——关于鲁迅的名言大全");
	hub.push(true,
			 "PrivateOnlyHub#36",
			 "所以我们且不要高谈什么连自己也并不了然的社会组织或意志强弱的滥调，先来设身处地的想一"
			 "想罢。\n——有关鲁迅的名言");
	hub.push(true, "PrivateOnlyHub#37", "我向来不惮以最坏的恶意揣测中国人。\n——有关鲁迅的名言");
	hub.push(true,
			 "PrivateOnlyHub#38",
			 "凡对于以真话为笑话的，以笑话为真话的，以笑话为笑话的，只有一个方法：就是不说话。于是"
			 "我从此尽量少说话。\n——关于鲁迅的名言名句");
	hub.push(false,
			 "PrivateOnlyHub#39",
			 "美国人说，时间就是金钱。但我想：时间就是性命。无端的空耗别人的时间，其实是无异于谋财"
			 "害命的。\n——有关鲁迅先生的名言");

	auto app = std::make_unique<Ethereality>("Ethereality Sample", 1200, 800, &hub);
	return app->exec();
}