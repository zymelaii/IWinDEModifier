#include "volumeproxy.h"

#include <stdexcept>

namespace Proxy {

auto VolumeProxy::require() -> std::unique_ptr<VolumeProxy> {
	try {
		auto instance = new VolumeProxy;
		return std::unique_ptr<VolumeProxy>{instance};
	} catch (const std::exception& e) {
		return {};
	}
}

VolumeProxy::VolumeProxy() {
	CoInitialize(nullptr);

	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID	IID_IMMDeviceEnumerator	 = __uuidof(IMMDeviceEnumerator);
	const IID	IID_IAudioEndpointVolume = __uuidof(IAudioEndpointVolume);
	const IID	IID_IAudioClient		 = __uuidof(IAudioClient);

	HRESULT hr = S_OK;
	hr		   = CoCreateInstance(CLSID_MMDeviceEnumerator,
						  nullptr,
						  CLSCTX_ALL,
						  IID_IMMDeviceEnumerator,
						  (void**)&dev_enumerator_);

	if (FAILED(hr)) {
		throw std::runtime_error("VolumeProxy unexpected constructor failure ("
								 "IMMDeviceEnumerator)");
	}

	hr = dev_enumerator_->GetDefaultAudioEndpoint(eRender, eMultimedia, &device_);
	if (FAILED(hr)) {
		throw std::runtime_error("VolumeProxy unexpected constructor failure ("
								 "GetDefaultAudioEndpoint)");
	}

	hr = device_->Activate(IID_IAudioEndpointVolume, CLSCTX_ALL, nullptr, (void**)&audio_epvolume_);
	if (FAILED(hr)) {
		throw std::runtime_error("VolumeProxy unexpected constructor failure ("
								 "IAudioEndpointVolume)");
	}

	hr = device_->Activate(IID_IAudioClient, CLSCTX_ALL, nullptr, (void**)&client_);
	if (FAILED(hr)) {
		throw std::runtime_error("VolumeProxy unexpected constructor failure ("
								 "IAudioClient)");
	}
}

void VolumeProxy::mute(bool on) {
	audio_epvolume_->SetMute(on, nullptr);
}

int VolumeProxy::get() const {
	float volume = 0.00f;
	audio_epvolume_->GetMasterVolumeLevelScalar(&volume);

	return static_cast<int>(volume * 100 + 0.5);
}

void VolumeProxy::set(int volume) {
	if (volume > 100) volume = 100;
	if (volume < 0) volume = 0;

	audio_epvolume_->SetMasterVolumeLevelScalar(volume / 100.0, &GUID_NULL);
}

VolumeProxy::~VolumeProxy() {
	if (client_ != nullptr) client_->Release();
	if (audio_epvolume_ != nullptr) audio_epvolume_->Release();
	if (device_ != nullptr) device_->Release();
	if (dev_enumerator_ != nullptr) dev_enumerator_->Release();

	CoUninitialize();
}

}	// namespace Proxy