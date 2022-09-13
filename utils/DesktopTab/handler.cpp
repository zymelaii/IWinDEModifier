#include <backend.h>
#include <locale.h>
#include <ShlObj.h>
#include <memory>
#include <vector>
#include <string_view>

auto cvts2w(const char* str) -> std::unique_ptr<wchar_t[]> {
	const char* loc = setlocale(LC_ALL, NULL);
	setlocale(LC_ALL, "chs");

	size_t len = strlen(str);

	auto wstr = std::make_unique<wchar_t[]>(len + 1);
	mbstowcs_s(nullptr, wstr.get(), len + 1, str, _TRUNCATE);

	setlocale(LC_ALL, loc);

	return std::move(wstr);
}

void switch_to(const char* target) {
	auto dest = std::move(cvts2w(target));
	if (auto hr = SHSetKnownFolderPath(FOLDERID_Desktop, 0, nullptr, dest.get()); SUCCEEDED(hr)) {
		LPITEMIDLIST list = nullptr;
		SHGetKnownFolderIDList(FOLDERID_Desktop, 0, nullptr, &list);
		SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_FLUSH | SHCNF_IDLIST, list, nullptr);
	}
}

struct GlobalHotKey {
	GlobalHotKey(int mod, int key)
		: id_(g_Id++) {
		RegisterHotKey(nullptr, id_, mod, key);
	}
	~GlobalHotKey() { UnregisterHotKey(nullptr, id_); }

private:
	static int g_Id;
	int		   id_;
};

int GlobalHotKey::g_Id = 0;

int main(int argc, char* argv[]) {
	const std::vector<std::string_view> desktops{
		R"(C:\Users\Nian\Desktop)",
		R"(E:\DesktopMap)",
	};

	GlobalHotKey tab(MOD_WIN | MOD_ALT, 'N');

	MSG msg{};
	int id = 0;

	while (GetMessage(&msg, nullptr, 0, 0) != 0) {
		TranslateMessage(&msg);
		if (msg.message == WM_HOTKEY) {
			const auto mod = (UINT)LOWORD(msg.lParam);
			const auto key = (UINT)HIWORD(msg.lParam);
			if (mod & (MOD_WIN | MOD_ALT) && key == 'N') {
				id = (id + 1) % desktops.size();
				switch_to(desktops[id].data());
			}
		}
	}

	return msg.wParam;
}