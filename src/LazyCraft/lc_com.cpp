#include <string>
#include <thread>

#include "lazycraft.h"

bool LazyCraft::IQuickLaunch() {
	auto w = ImGui::GetCurrentWindow();
	if (w->SkipItems) return false;

	auto	   canvas = w->DrawList;
	const auto id	  = w->GetID("LazyCraft.QuickLaunch");
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

	if (hovered && ImGui::GetIO().MouseDown[0]) {
		toggle_se = false;
		ImGui::SetActiveID(id, w);
		ImGui::SetFocusID(id, w);
	}

	const ImRect viewex{view.Min.x - 16, view.Min.y, view.Max.x + 16, view.Max.y};
	canvas->PathRect(viewex.Min, viewex.Max, rounding);
	canvas->AddConvexPolyFilled(canvas->_Path.Data, canvas->_Path.Size, col_inner);
	canvas->PathStroke(col_border, ImDrawFlags_Closed, 2.00f);

	for (int i = 0; i < nItem; ++i) {
		const auto [szItem, spacing, nVerticalPart] = params;
		const auto x = view.Min.x + spacing / 2 + i * (szItem + spacing), y = posy + spacing / 2;

		const auto	k	= 1.00 / sqrt(2 * 3.1415926535);
		const float rel = k * exp(-pow(i - selection, 2) * 0.5);

		ImRect box{x, y, x + szItem, y + szItem};
		box.Min.x -= spacing * 0.8 * rel;
		box.Max.x += spacing * 0.8 * rel;
		box.Min.y -= spacing * 2.5 * rel;
		box.Max.y -= spacing * 0.5 * rel;

		if (ImGui::GetIO().MouseDown[0] && selection == i) {
			box.Min.y += spacing * 0.5 * rel;
			box.Max.y += spacing * 0.5 * rel;
		}

		canvas->AddImage(std::get<3>(QuickLaunchs[i]), box.Min, box.Max);
	}

	ImGui::SetMouseCursor(hovered ? ImGuiMouseCursor_Hand : ImGuiMouseCursor_Arrow);

	if (!(selection >= 0 && selection < nItem)) return true;

	const auto [szItem, spacing, nVerticalPart] = params;
	const auto x = view.Min.x + spacing / 2 + selection * (szItem + spacing),
			   y = posy + spacing / 2;
	const ImRect box{x, y, x + szItem, y + szItem};

	const auto& [title, execpath, workdir, unused] = QuickLaunchs[selection];

	ImGui::PushFont(font_ascii->get());
	const auto	 size	= ImGui::CalcTextSize(title.c_str());
	const auto	 roundw = 8.00f;
	const ImVec2 tip{x + szItem / 2, y - spacing * 1.32f};
	const ImVec2 oritext{tip.x - size.x / 2, tip.y - size.y - roundw * 1.6f};
	const ImRect descbox{oritext.x - roundw,
						 oritext.y - roundw,
						 oritext.x + size.x + roundw,
						 oritext.y + size.y + roundw};

	canvas->AddRectFilled(descbox.Min, descbox.Max, 0x80ffffff, roundw);
	canvas->AddCircleFilled(tip, roundw * 0.36f, 0x80ffffff);

	canvas->AddText(ImGui::GetFont(), ImGui::GetFontSize(), oritext, 0xff000000, title.c_str());
	ImGui::PopFont();

	if (pressed) {
		std::thread(
			[](const wchar_t* prog, const wchar_t* workdir) {
				STARTUPINFOW		startinfo{};
				PROCESS_INFORMATION procinfo{};
				startinfo.cb = sizeof(STARTUPINFOW);
				CreateProcessW(prog,
							   nullptr,
							   nullptr,
							   nullptr,
							   false,
							   CREATE_NEW_PROCESS_GROUP | HIGH_PRIORITY_CLASS,
							   nullptr,
							   workdir[0] ? workdir : nullptr,
							   &startinfo,
							   &procinfo);
				WaitForInputIdle(procinfo.hProcess, INFINITE);
			},
			execpath.c_str(),
			workdir.c_str())
			.detach();
	}

	return true;
}

bool LazyCraft::ISearchEngineClassic() {
	auto		window = ImGui::GetCurrentWindow();
	const auto& style  = ImGui::GetStyle();

	static char buffer[1024]{};
	const auto	strid = "##LazyCraft.SearchEngine";

	ImGui::SetKeyboardFocusHere();

	ImGui::PushFont(font_full->get());
	const char* hint	  = "LazyCraft Search Engine...";
	const float searchhei = ImGui::GetFontSize() * 2;
	const float searchwid = window->SizeFull.x / 3;
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.00);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, ImGui::GetFontSize() * 0.75);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
						{12.00, (searchhei - ImGui::GetFontSize()) / 2});
	ImGui::PushStyleColor(ImGuiCol_Text, ImColor(10, 10, 10).Value);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImColor(255, 255, 255).Value);
	ImGui::PushStyleColor(ImGuiCol_Border, ImColor(225, 228, 232).Value);

	ImVec2 prevCursorPos   = window->DC.CursorPos;
	window->DC.CursorPos.x = (window->SizeFull.x - searchwid) / 2;
	window->DC.CursorPos.y = window->SizeFull.y / 8;
	ImGui::InputTextEx(strid, hint, buffer, 1024, ImVec2(searchwid, searchhei), 0);
	window->DC.CursorPos = prevCursorPos;

	ImGui::PopStyleColor(3);
	ImGui::PopStyleVar(3);
	ImGui::PopFont();

	const auto this_id = ImGui::GetCurrentWindow()->GetID(strid);

	if (ImGui::GetFocusID() == this_id) {
		if (ImGui::GetIO().KeysDown[ImGuiKey_Enter]) {
			char  execfile[256] = {0};
			char* params		= strchr(buffer, ' ');
			memcpy(execfile, buffer, params ? params - buffer : strlen(buffer) + 1);
			HINSTANCE inst =
				ShellExecute(nullptr, nullptr, execfile, params, nullptr, SW_SHOWDEFAULT);
			if (reinterpret_cast<intptr_t>(inst) <= 32) {
				printf("caught failure on ShellExecute, with %lld\n",
					   reinterpret_cast<intptr_t>(inst));
			} else {
				toggle_se = false;
			}
		}
		if (ImGui::GetIO().KeysDown[ImGuiKey_Escape]) {
			toggle_se = false;
		}
	}

	if (!toggle_se) {
		RECT client{};
		GetClientRect(hwnd_, &client);
		InvalidateRect(hwnd_, &client, false);
		buffer[0] = '\0';
	}

	return true;
}

bool LazyCraft::ISearchEngine() {
	return ISearchEngineClassic();

	auto w = ImGui::GetCurrentWindow();
	if (w->SkipItems) return false;

	static char input[256]{};
	auto&		g	   = *GImGui;
	auto		canvas = w->DrawList;
	auto&		io	   = ImGui::GetIO();

	const auto	 id = w->GetID("LazyCraft.SearchEngine");
	const ImVec2 szinput{128, 16};

	const ImVec2 origin{300, 300};
	const ImRect rcinput{origin.x, origin.y, origin.x + szinput.x, origin.y + szinput.y};

	ImGui::ItemSize(rcinput);
	if (!ImGui::ItemAdd(rcinput, id, &rcinput, ImGuiItemFlags_Inputable)) return false;

	const auto hovered = ImGui::ItemHoverable(rcinput, id);
	if (hovered) {
		g.MouseCursor = ImGuiMouseCursor_TextInput;
	}

	auto	   state		   = ImGui::GetInputTextState(id);
	const auto should_activate = hovered && io.MouseClicked[0];
	if (g.ActiveId != id && should_activate) {
		state = &g.InputTextState;
		state->CursorAnimReset();

		const auto len = strlen(input);
		state->InitialTextA.resize(len + 1);
		memcpy_s(state->InitialTextA.Data, len + 1, input, len + 1);

		const char* endpoint = nullptr;
		state->ID			 = id;
		state->TextW.resize(len + 1);
		state->TextA.resize(0);
		state->TextAIsValid = false;
		state->CurLenW		= ImTextStrFromUtf8(state->TextW.Data, len, input, nullptr, &endpoint);
		state->CurLenA		= static_cast<int>(endpoint - input);

		// state->ScrollX = 0.0f;
		// stb_textedit_initialize_state(&state->Stb, true);

		ImGui::SetActiveID(id, w);
		ImGui::SetFocusID(id, w);
		ImGui::FocusWindow(w);

		g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
		ImGui::SetActiveIdUsingKey(ImGuiKey_Escape);
		ImGui::SetActiveIdUsingKey(ImGuiKey_Home);
		ImGui::SetActiveIdUsingKey(ImGuiKey_End);
	}

	if (g.ActiveId == id && !state) {
		ImGui::ClearActiveID();
	}

	canvas->AddText(g.Font, g.FontSize, origin, ImGui::GetColorU32(ImGuiCol_Text), input);

	return true;
}