#include <memory>
#include <ShlObj.h>

namespace Proxy {

struct LinkProxy {
	static auto require() -> std::unique_ptr<LinkProxy>;

	enum class Attribute {
		Source,
		WorkDir,
	};

	bool query(const wchar_t* lnkpath, char* destbuf, size_t szbuf,
			   Attribute attr = Attribute::Source);

	~LinkProxy();

	LinkProxy(const LinkProxy&)			   = delete;
	LinkProxy(LinkProxy&&)				   = delete;
	LinkProxy& operator=(const LinkProxy&) = delete;

protected:
	LinkProxy();

private:
	IShellLink*	  shlink_;
	IPersistFile* lnkfile_;
};

};	 // namespace Proxy
