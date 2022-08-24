#include <backend.h>
#include <memory>
#include <locale.h>
#include <ShlObj.h>

class DesktopTabHandler : public ImGuiApplication {
private:
	bool visible_ = false;
	int	 nav_id_  = -1;

public:
	DesktopTabHandler(const char* title, int width, int height)
		: ImGuiApplication(title, width, height) {}

	~DesktopTabHandler() { UnregisterHotKey(hwnd_, 0); }

public:
	void render() override {}

public:
	void configure() override { RegisterHotKey(hwnd_, 0, MOD_WIN | MOD_ALT, 'N'); }

	std::optional<LRESULT> notify(UINT msg, WPARAM wParam, LPARAM lParam) override {
		if (msg == WM_HOTKEY) {
			if ((UINT)LOWORD(lParam) & (MOD_WIN | MOD_ALT) && (UINT)HIWORD(lParam) == 'N') {
				visible_ = !visible_;
				if (visible_) {
					// ShowWindow(hwnd_, SW_SHOW);
					switch_to(R"(C:\Users\Nian\Desktop)");
				} else {
					// ShowWindow(hwnd_, SW_HIDE);
					switch_to(R"(E:\DesktopMap)");
				}
			}
		}
		return std::nullopt;
	}

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
};

int main(int argc, char* argv[]) {
	auto app = std::make_unique<DesktopTabHandler>("DesktopTabHandler", 0, 0);
	return app->exec(false);
}