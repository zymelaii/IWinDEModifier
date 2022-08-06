#include <iwindeapi/userchoice.h>
#include <memory>
#include <iostream>

extern "C" int cli_entry_UserChoice(int argc, wchar_t* argv[]) {
	SYSTEMTIME				   timestamp{};
	std::unique_ptr<wchar_t[]> UserName{}, UserSID{}, UserChoice{}, Hash{};
	const wchar_t*			   FileExt = argc >= 2 ? argv[0] : L".txt";
	const wchar_t*			   ProgId  = argc >= 2 ? argv[1] : L"IWinDEModifier.plain";

	GetSystemTime(&timestamp);

	if (auto opt = GetCurrentUserName(); opt.has_value()) {
		UserName = std::move(opt.value());
	} else {
		puts("GetCurrentUserName() FAILED!");
		return 1;
	}

	if (auto opt = GetCurrentUserSid(); opt.has_value()) {
		UserSID = std::move(opt.value());
	} else {
		puts("GetCurrentUserSid() FAILED!");
		return 1;
	}

	if (auto opt = FormatUserChoice(L".txt", UserSID.get(), L"IWinDEModifier.plain", timestamp);
		opt.has_value()) {
		UserChoice = std::move(opt.value());
	} else {
		puts("FormatUserChoice() FAILED!");
		return 2;
	}

	if (auto opt = GetUserChoiceHash(UserChoice.get()); opt.has_value()) {
		Hash = std::move(opt.value());
	} else {
		puts("GetUserChoiceHash() FAILED!");
		return 3;
	}

	wprintf(LR"(---
Query Parameters : {FileExt: "%S", ProgId: "%S"}
Current User Name: %S
Current User SID : %S
UserChoice String: %S
UserChoice Hash  : %S
---)",
			FileExt,
			ProgId,
			UserName.get(),
			UserSID.get(),
			UserChoice.get(),
			Hash.get());

	return 0;
}