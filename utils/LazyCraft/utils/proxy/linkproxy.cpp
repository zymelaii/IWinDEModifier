#include "linkproxy.h"

#include <assert.h>

namespace Proxy {

LinkProxy::LinkProxy() {
	CoInitialize(nullptr);
	HRESULT hr = CoCreateInstance(
		CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&shlink_);
	assert(SUCCEEDED(hr) && "LinkProxy unexpected constructor failure (IShellLink)");
	hr = shlink_->QueryInterface(IID_IPersistFile, (void**)&lnkfile_);
	if (!SUCCEEDED(hr)) {
		shlink_->Release();
		shlink_ = nullptr;
	}
	assert(SUCCEEDED(hr) && "LinkProxy unexpected constructor failure (IPersistFile)");
}

bool LinkProxy::query(const wchar_t* lnkpath, char* destbuf, size_t szbuf) {
	HRESULT hr = lnkfile_->Load(lnkpath, STGM_READ);
	if (SUCCEEDED(hr)) {
		WIN32_FIND_DATA fd{};
		hr = shlink_->GetPath(destbuf, szbuf, &fd, 0);
	}
	return SUCCEEDED(hr);
}

LinkProxy::~LinkProxy() {
	if (shlink_) {
		lnkfile_->Release();
		shlink_->Release();
	}
	CoUninitialize();
}

}	// namespace Proxy