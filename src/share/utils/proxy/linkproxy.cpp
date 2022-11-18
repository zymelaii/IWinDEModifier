#include "linkproxy.h"

#include <assert.h>
#include <stdexcept>

namespace Proxy {

auto LinkProxy::require() -> std::unique_ptr<LinkProxy> {
	try {
		auto instance = new LinkProxy;
		return std::unique_ptr<LinkProxy>{instance};
	} catch (const std::exception& e) {
		return {};
	}
}

LinkProxy::LinkProxy() {
	CoInitialize(nullptr);

	HRESULT hr = CoCreateInstance(
		CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&shlink_);

	if (!SUCCEEDED(hr))
		throw std::runtime_error("LinkProxy unexpected constructor failure (IShellLink)");

	hr = shlink_->QueryInterface(IID_IPersistFile, (void**)&lnkfile_);
	if (!SUCCEEDED(hr)) {
		shlink_->Release();
		shlink_ = nullptr;
	}

	if (!SUCCEEDED(hr))
		throw std::runtime_error("LinkProxy unexpected constructor failure (IPersistFile)");
}

bool LinkProxy::query(const wchar_t* lnkpath, char* destbuf, size_t szbuf, Attribute attr) {
	HRESULT hr = lnkfile_->Load(lnkpath, STGM_READ);

	if (SUCCEEDED(hr)) {
		switch (attr) {
			case Attribute::Source: {
				WIN32_FIND_DATA fd{};
				hr = shlink_->GetPath(destbuf, szbuf, &fd, 0);
				break;
			}
			case Attribute::WorkDir: {
				hr = shlink_->GetWorkingDirectory(destbuf, szbuf);
				break;
			}
		}
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