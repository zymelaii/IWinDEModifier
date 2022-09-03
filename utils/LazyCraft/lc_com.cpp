#include "lazycraft.h"

bool LazyCraft::IQuickLaunch() {
	auto w = ImGui::GetCurrentWindow();
	if (w->SkipItems) return false;

	auto	   canvas = w->DrawList;
	const auto id	  = w->GetID("LazyCraft#QuickLaunch");
	const auto rc	  = w->Rect();

	constexpr std::tuple params{50.00f, 24.00f, 8.00f};

	static bool	  hovered = false;
	const int	  nItem	  = QuickLaunchs.size();
	const ImColor col_inner(0x41, 0x75, 0x62, 0x80), col_border(0x0c, 0x2a, 0x5e, 0x80);

	if (nItem <= 0) return false;

	int	   selection{-255};
	float  width{}, height{}, middle{}, rounding{}, partion{}, posy{};
	ImRect view{};

	auto calcParams = [&] {
		const auto [szItem, spacing, nVerticalPart] = params;

		width  = nItem * (szItem + spacing);
		height = szItem + spacing;
		middle = (rc.Min.x + rc.Max.x) / 2, rounding = height / 3;
		partion = (rc.Max.y - rc.Min.y) / nVerticalPart;
		posy	= partion * (nVerticalPart - 1) + (partion - height) / 2;
		view	= ImRect(middle - width / 2, posy, middle + width / 2, posy + height);

		if (hovered) {
			const auto& io = ImGui::GetIO();
			selection	   = floor((io.MousePos.x - view.Min.x) / (szItem + spacing));
		}
	};

	calcParams();
	if (!ImGui::ItemAdd(view, id)) return false;

	bool pressed = ImGui::ButtonBehavior(view, id, &hovered, nullptr);
	calcParams();

	//! 画个盘盘先
	const ImRect viewex{view.Min.x - 16, view.Min.y, view.Max.x + 16, view.Max.y};
	canvas->PathRect(viewex.Min, viewex.Max, rounding);
	canvas->AddConvexPolyFilled(canvas->_Path.Data, canvas->_Path.Size, col_inner);
	canvas->PathStroke(col_border, ImDrawFlags_Closed, 2.00f);

	//! 渲染应用图标
	for (int i = 0; i < nItem; ++i) {
		const auto [szItem, spacing, nVerticalPart] = params;
		const auto x = view.Min.x + spacing / 2 + i * (szItem + spacing), y = posy + spacing / 2;

		const auto k   = 1.00 / sqrt(2 * 3.1415926535);
		float	   rel = k * exp(-pow(i - selection, 2) * 0.5);

		ImRect box{x, y, x + szItem, y + szItem};
		box.Min.x -= spacing * 0.8 * rel;
		box.Max.x += spacing * 0.8 * rel;
		box.Min.y -= spacing * 2.5 * rel;
		box.Max.y -= spacing * 0.5 * rel;

		if (ImGui::GetIO().MouseDown[0] && selection == i) {
			box.Min.y += spacing * 0.5 * rel;
			box.Max.y += spacing * 0.5 * rel;
		}

		canvas->AddImage(QuickLaunchs[i].second, box.Min, box.Max);
	}

	//! 绘制选中应用的标签
	if (selection >= 0 && selection < nItem) {
		const auto [szItem, spacing, nVerticalPart] = params;
		const auto x = view.Min.x + spacing / 2 + selection * (szItem + spacing),
				   y = posy + spacing / 2;
		const ImRect box{x, y, x + szItem, y + szItem};

		const auto& wtext{QuickLaunchs[selection % QuickLaunchs.max_size()].first};
		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv{};

		std::string text{conv.to_bytes(wtext)};

		const auto	 size	  = ImGui::CalcTextSize(text.c_str());
		const auto	 rounding = 8.00f;
		const ImVec2 tip{x + szItem / 2, y - spacing * 1.32f};
		const ImVec2 oritext{tip.x - size.x / 2, tip.y - size.y - rounding * 1.6f};
		const ImRect descbox{oritext.x - rounding,
							 oritext.y - rounding,
							 oritext.x + size.x + rounding,
							 oritext.y + size.y + rounding};

		canvas->AddRectFilled(descbox.Min, descbox.Max, 0x80ffffff, rounding);
		canvas->AddCircleFilled(tip, rounding * 0.36f, 0x80ffffff);

		canvas->AddText(ImGui::GetFont(), ImGui::GetFontSize(), oritext, 0xff000000, text.c_str());

		if (pressed) {
			STARTUPINFOW		startinfo{};
			PROCESS_INFORMATION procinfo{};
			// startinfo.wShowWindow = SW_MAXIMIZE;
			startinfo.cb		  = sizeof(STARTUPINFOW);
			CreateProcessW(wtext.c_str(),
						   nullptr,
						   nullptr,
						   nullptr,
						   false,
						   CREATE_NEW_PROCESS_GROUP | HIGH_PRIORITY_CLASS,
						   nullptr,
						   nullptr,
						   &startinfo,
						   &procinfo);
		}
	}

	if (hovered) {
		SetCursor(LoadCursor(nullptr, IDC_HAND));
	} else {
		SetCursor(LoadCursor(nullptr, IDC_ARROW));
	}

	return true;
}