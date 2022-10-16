#include <memory>
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <audioclient.h>

namespace Proxy {

struct VolumeProxy {
	static auto require() -> std::unique_ptr<VolumeProxy>;

	void mute(bool on);
	int	 get() const;
	void set(int volume);

	~VolumeProxy();

protected:
	VolumeProxy();

private:
	IMMDeviceEnumerator*  dev_enumerator_;
	IMMDevice*			  device_;
	IAudioEndpointVolume* audio_epvolume_;
	IAudioClient*		  client_;
};

}	// namespace Proxy