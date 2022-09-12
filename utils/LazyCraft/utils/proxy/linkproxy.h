#include <ShlObj.h>

namespace Proxy {

struct LinkProxy {
	LinkProxy();
	~LinkProxy();
	bool query(const wchar_t* lnkpath, char* destbuf, size_t szbuf);

private:
	IShellLink*	  shlink_;
	IPersistFile* lnkfile_;
};

};
