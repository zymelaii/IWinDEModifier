#include <string_view>
#include <functional>

#include "GroupOnlyHub.h"

GroupOnlyHub::GroupOnlyHub()
	: should_render_(true)
	, factory_(nullptr)
	, scroll_state_(0.00)
	, scroll_range_{0.00, 0.00}
	, scroll_anime_state_(0)
	, items_{}
	, size_(getMinSizeConstraint())
	, identity_("guest") {
	bindFactory(new GroupMsgFactory);
}

void GroupOnlyHub::login_as(const std::string& identity) {
	identity_ = identity;
}

const std::string& GroupOnlyHub::getIdentity() const {
	return identity_;
}

std::optional<std::string> GroupOnlyHub::getRequest() {
	std::string req;
	mutex_.lock();
	bool empty = req_queue_.empty();
	if (!empty) {
		req = req_queue_.front();
		req_queue_.pop();
	}
	mutex_.unlock();
	return empty ? std::nullopt : std::optional<std::string>{req};
}

void GroupOnlyHub::addRequest(std::string req) {
	mutex_.lock();
	req_queue_.push(req);
	mutex_.unlock();
}

void GroupOnlyHub::push(const std::string& sender, const std::string& msgid,
						const std::string& text) {
	json item{
		{"type", "private"},
		{"id", msgid},
		{"sender", sender},
		{"text", text},
	};
	items_.push_back({std::hash<std::string_view>{}(msgid), item});
}

nlohmann::json GroupOnlyHub::parse(uint64_t id) {
	return *reinterpret_cast<json*>(request(id));
}

float GroupOnlyHub::getScrollState() const {
	return scroll_state_;
}

void GroupOnlyHub::scrollTo(float cursor) {
	if (abs(cursor - scroll_state_) < 1e-3) return;
	scroll_state_		= ImClamp(cursor, scroll_range_.x, scroll_range_.y);
	scroll_anime_state_ = 144;
}

void GroupOnlyHub::resize(ImVec2 size) {
	//! 这段代码为什么能正常运行我还不明确，但是重要的是，它正常运行了
	//! 如果后续出现什么BUG的话，就考虑来修下这儿吧
	float delta		= size.y - size_.y;
	scroll_range_.y = ImMax(scroll_range_.x, scroll_range_.y - delta);
	scroll_state_	= ImClamp(scroll_state_ - delta, scroll_range_.x, scroll_range_.y);
	if (abs(size.x - size_.x) > 1e-3) scroll_range_.y = scroll_range_.x = 0;
	size_ = size;
}

void GroupOnlyHub::renderScrollBar(const ImRect& bb) {
	ImU32 color		   = ImColor(81, 85, 77, scroll_anime_state_ * 255.00 / 50);
	float spacing	   = 2.00;
	float total_height = bb.GetHeight() - spacing * 2;
	float range		   = scroll_range_.y - scroll_range_.x;
	if (range < 1e-3) return;

	float scale		 = (scroll_state_ - scroll_range_.x) / range;
	float bar_height = bb.GetHeight() / (range + bb.GetHeight()) * total_height / 2;
	float bar_width	 = spacing * 3;

	ImRect scroll_bb = bb;
	scroll_bb.Min.x	 = scroll_bb.Max.x - spacing * 2 - bar_width;
	scroll_bb.Max.y	 = scroll_bb.Min.y + spacing * 2 + total_height;

	const auto& io = ImGui::GetIO();
	if (!scroll_lock_ && scroll_bb.Contains(io.MousePos)) {
		scroll_anime_state_ = ImMax(scroll_anime_state_, 25);
	}

	if (scroll_anime_state_ == 0) return;

	ImRect bar_bb = scroll_bb;
	bar_bb.Expand({-spacing, -spacing});
	bar_bb.Min.y += (total_height - bar_height) * scale;
	bar_bb.Max.y = bar_bb.Min.y + bar_height;

	ImGui::GetWindowDrawList()->AddRectFilled(
		bar_bb.Min, bar_bb.Max, color, bar_width / 2, ImDrawFlags_RoundCornersAll);

	if (!scroll_lock_) --scroll_anime_state_;

	if (bar_bb.Contains(io.MousePos)) {
		scroll_anime_state_ = 50;
		if (!scroll_lock_) scroll_lock_ = true;
	}

	if (scroll_lock_ && io.MouseDown[ImGuiMouseButton_Left]) {
		scroll_anime_state_ = 144;
		float ratio			= range / (total_height - bar_height);
		scrollTo(getScrollState() + io.MouseDelta.y * ratio);
	} else {
		scroll_lock_ = false;
	}
}

void GroupOnlyHub::resolvePanel(const ImRect& bb) {
	ImRect	Padding{16.00, 4.00, 16.00, 4.00};			   //!< 聊天面板内边距
	float	VertSpacing{6.00};							   //!< 消息项垂直间距
	float	InputHeight{48.00};							   //!< 输入栏高度
	ImRect	InputPadding{16.00, 4.00, 16.00, 4.00};		   //!< 输入栏内边距
	ImColor SelectionColor = ImColor(124, 185, 218, 64);   //!< 选中背景色

	ImRect panel_bb;
	panel_bb.Min.y = bb.Min.y;
	panel_bb.Max.y = bb.Max.y - InputHeight - InputPadding.GetHeight();
	panel_bb.Min.x = bb.Min.x;
	panel_bb.Max.x = bb.Max.x;

	//! Register & Resolve Event
	auto window = ImGui::GetCurrentWindow();
	auto id		= window->GetID("##ChatHub.GroupOnlyHub::Panel");
	bool hovered, held;
	if (ImGui::ButtonBehavior(panel_bb, id, &hovered, &held)) {
		ImGui::SetFocusID(id, window);
	}

	ImRect clip_bb = panel_bb;
	clip_bb.Expand({0, -Padding.Min.y});

	//! Begin Render
	auto drawlist = ImGui::GetWindowDrawList();
	ImGui::PushClipRect(clip_bb.Min, clip_bb.Max, true);

	ImRect item_bb = clip_bb;
	item_bb.Expand({-Padding.Min.x, 0});
	item_bb.Min.y = item_bb.Max.y = clip_bb.Min.y + VertSpacing - scroll_state_;

	for (auto& item : items_) {
		std::unique_ptr<MsgItem> resolver{factory_->build("PrivateMsg")};

		should_render_ = false;
		ImRect full_bb = resolver->resolve(&item.second, item_bb, this);

		if (clip_bb.Overlaps(full_bb)) {
			should_render_ = true;
			resolver->resolve(&item.second, item_bb, this);

			if (full_bb.Contains(ImGui::GetIO().MousePos)) {
				ImRect select_bb;
				select_bb.Min.x = clip_bb.Min.x;
				select_bb.Max.x = clip_bb.Max.x;
				select_bb.Min.y = ImCeil(full_bb.Min.y - VertSpacing * 0.50 - 0.99);
				select_bb.Max.y = ImFloor(full_bb.Max.y + VertSpacing * 0.50 + 0.99);
				drawlist->AddRectFilled(select_bb.Min, select_bb.Max, SelectionColor);
			}
		}

		item_bb.TranslateY(full_bb.GetHeight() + VertSpacing);
		if (item_bb.Min.y > clip_bb.Max.y) break;
	}

	if (items_.empty()) {
		const char* hint = "No messages";
		ImVec2		size = ImGui::CalcTextSize(hint);
		ImVec2		pos{clip_bb.Min.x + (clip_bb.GetWidth() - size.x) / 2,
					clip_bb.Min.y + (clip_bb.GetHeight() - size.y) / 2};
		drawlist->AddText(
			ImGui::GetFont(), ImGui::GetFontSize(), pos, ImColor(159, 169, 178), hint);
	}

	ImGui::PopClipRect();

	//! Update Scroll Range
	const auto& io = ImGui::GetIO();

	float delta		= item_bb.Min.y - clip_bb.Min.y + scroll_state_;
	scroll_range_.y = ImMax(scroll_range_.y, delta - clip_bb.GetHeight());
	renderScrollBar(panel_bb);

	if (ImGui::GetFocusID() == id || panel_bb.Contains(io.MousePos)) {
		scrollTo(getScrollState() - io.MouseWheel * 60.00);
	}
}

void GroupOnlyHub::resolveInput(const ImRect& bb) {
	ImRect	Padding{16.00, 4.00, 16.00, 4.00};			   //!< 聊天面板内边距
	float	VertSpacing{6.00};							   //!< 消息项垂直间距
	float	InputHeight{48.00};							   //!< 输入栏高度
	ImRect	InputPadding{16.00, 4.00, 16.00, 4.00};		   //!< 输入栏内边距
	ImColor SelectionColor = ImColor(124, 185, 218, 64);   //!< 选中背景色

	ImRect input_bb = bb;
	input_bb.Min.y	= bb.Max.y - InputHeight - InputPadding.GetHeight();

	ImRect clip_bb = input_bb;
	clip_bb.Expand({-InputPadding.Min.x, -InputPadding.Min.y});

	//! Begin Render
	auto		window	 = ImGui::GetCurrentWindow();
	auto		drawlist = window->DrawList;
	const auto& io		 = ImGui::GetIO();
	const auto& style	 = ImGui::GetStyle();
	ImGui::PushClipRect(clip_bb.Min, clip_bb.Max, true);

	const char*		label		  = "##ChatHub.GroupOnlyHub::Input";
	const char*		hint		  = "Type message";
	static char		typebuf[1024] = {0};
	static uint64_t nextid		  = 10000;
	bool			should_post	  = false;

	window->DC.CursorPos = clip_bb.Min;
	ImVec2 input_size	 = clip_bb.GetSize();
	ImVec2 label_size	 = ImGui::CalcTextSize(label, nullptr, true);

	ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, {InputPadding.Min.x, 0});

	input_size.x -= label_size.x > 0 ? label_size.x + style.ItemInnerSpacing.x : 0;

	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.00);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, ImGui::GetFontSize() * 0.75);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
						{12.00, (input_size.y - ImGui::GetFontSize()) / 2});

	ImGui::PushStyleColor(ImGuiCol_Text, ImColor(10, 10, 10).Value);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImColor(255, 255, 255).Value);
	ImGui::PushStyleColor(ImGuiCol_Border, ImColor(225, 228, 232).Value);

	ImGui::InputTextEx(label, hint, typebuf, sizeof(typebuf), input_size, 0);

	ImGui::PopStyleColor(3);
	ImGui::PopStyleVar(4);

	ImGui::PopClipRect();

	//! Resolve Post
	auto id = window->GetID(label);
	if (ImGui::GetFocusID() == id && io.KeysDown[ImGuiKey_Enter]) {
		ImGui::SetActiveID(id, window);
		auto state = ImGui::GetInputTextState(id);
		state->ClearText();
		state->CursorFollow = true;
		should_post |= typebuf[0] != '\0';
	}

	if (should_post) {
		char sid[32] = {0};
		sprintf(sid, "GroupOnlyHub#%llu", nextid++);
		push(identity_, sid, typebuf);
	}
}

void GroupOnlyHub::render(ImVec2 pos, ImVec2 size) {
	ImGuiWindow* window	  = ImGui::GetCurrentWindow();
	ImDrawList*	 drawlist = window->DrawList;
	if (window->SkipItems) return;

	resize(size);

	auto   id = window->GetID("##ChatHub.GroupOnlyHub");
	ImRect bb{{0, 0}, size};
	bb.Translate(window->InnerRect.Min);
	bb.Translate(pos);

	if (!ImGui::ItemAdd(bb, id)) return;

	ImGui::PushClipRect(bb.Min, bb.Max, true);

	ImU32 BgColor = ImColor(248, 249, 250);	  //!< 背景色
	drawlist->AddRectFilled(bb.Min, bb.Max, BgColor);

	resolvePanel(bb);
	resolveInput(bb);

	ImGui::PopClipRect();
}

bool GroupOnlyHub::bindFactory(MsgItemFactory* factory) const {
	if (!factory) return false;
	factory_.reset(factory);
	return true;
}

bool GroupOnlyHub::shouldRender() const {
	return should_render_;
}

ImVec2 GroupOnlyHub::getMinSizeConstraint() const {
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

uint64_t GroupOnlyHub::queryNext(uint64_t id) {
	if (id == 0) return items_.size() == 0 ? 0 : items_[0].first;
	bool captured = false;
	for (const auto& [this_id, item] : items_) {
		if (captured) return this_id;
		if (this_id == id) captured = true;
	}
	return 0;
}

uint64_t GroupOnlyHub::queryPrevious(uint64_t id) {
	if (id == 0) return items_.size() == 0 ? 0 : items_[0].first;
	uint64_t prev_id = 0;
	for (const auto& [this_id, item] : items_) {
		if (this_id == id) return prev_id;
		prev_id = this_id;
	}
	return 0;
}

void* GroupOnlyHub::request(uint64_t id) {
	if (id == 0) return nullptr;
	for (auto& [this_id, item] : items_) {
		if (this_id == id) return reinterpret_cast<void*>(&item);
	}
	return nullptr;
}

bool GroupOnlyHub::jumpToItem(uint64_t id) {
	return false;
}