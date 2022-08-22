#include "shfolder.h"

HRESULT GetIdlFromParsingName(const wchar_t* szParsingName, LPITEMIDLIST* pidl) {
	if (szParsingName == nullptr || pidl == nullptr) {
		return E_FAIL;
	}

	IShellFolder* pDesktopFolder = nullptr;
	HRESULT		  hr			 = ERROR_SUCCESS;

	hr = SHGetDesktopFolder(&pDesktopFolder);

	if (SUCCEEDED(hr)) {
		wchar_t szParsingNameTemp[MAX_PATH];
		wcscpy_s(szParsingNameTemp, szParsingName);

		hr = pDesktopFolder->ParseDisplayName(nullptr, nullptr, szParsingNameTemp, nullptr, pidl, nullptr);

		pDesktopFolder->Release();
	}

	return hr;
}