#include <iwindeapi/userchoice.h>
#include <ShlObj.h>
#include <memory>
#include <iostream>

extern "C" int cli_entry_flush(int argc, wchar_t* argv[]) {
	if (auto opt = GetCurrentUserName(); opt.has_value()) {
		const auto fmtpath	= LR"(C:\Users\%S\Desktop)";
		auto	   UserName = std::move(opt.value());
		size_t	   len		= _scwprintf(fmtpath, UserName.get());
		auto	   path		= std::make_unique<wchar_t[]>(len + 1);
		_swprintf(path.get(), fmtpath, UserName.get());
		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_FLUSH | SHCNF_PATH, path.get(), nullptr);
		return 0;
	} else {
		puts("failed to get desktop path");
		return 1;
	}
}