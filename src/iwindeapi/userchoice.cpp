#include "userchoice.h"

#include <errhandlingapi.h>
#include <memory>
#include <optional>
#include <windows.h>
#include <bcrypt.h>
#include <sddl.h>
#include <wincrypt.h>
#include <winerror.h>
#include <winnt.h>
#include <winternl.h>

/*!
 * \brief MD5 CNG hash encrypt algorithm
 * \note require bcrypt.lib
 */
auto CNG_MD5(const uint8_t* bytes, size_t size) -> std::optional<std::unique_ptr<DWORD[]>> {
	const size_t			 MD5_BYTES	= 16;
	const size_t			 MD5_DWORDS = MD5_BYTES / sizeof(DWORD);
	BCRYPT_ALG_HANDLE		 hAlg		= nullptr;
	BCRYPT_HASH_HANDLE		 hHash		= nullptr;
	std::unique_ptr<DWORD[]> hash		= nullptr;

	IWDEM_CheckOrReturn(
		NT_SUCCESS(BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_MD5_ALGORITHM, nullptr, 0)),
		std::nullopt);

	IWDEM_CheckOrReturn(NT_SUCCESS(BCryptCreateHash(hAlg, &hHash, nullptr, 0, nullptr, 0, 0)),
						std::nullopt,
						BCryptCloseAlgorithmProvider(hAlg, 0));

	IWDEM_CheckOrReturn(NT_SUCCESS(BCryptHashData(hHash, const_cast<uint8_t*>(bytes), size, 0)),
						std::nullopt,
						BCryptDestroyHash(hHash));

	hash = std::make_unique<DWORD[]>(MD5_DWORDS);
	IWDEM_CheckOrReturn(
		NT_SUCCESS(BCryptFinishHash(
			hHash, reinterpret_cast<uint8_t*>(hash.get()), MD5_DWORDS * sizeof(DWORD), 0)),
		std::nullopt);

	BCryptDestroyHash(hHash);
	BCryptCloseAlgorithmProvider(hAlg, 0);

	return {std::move(hash)};
}

/*!
 * \brief base64 encode algorithm
 * \note require Crypt32.lib
 */
auto Base64Encode(const uint8_t* bytes, size_t size) -> std::optional<std::unique_ptr<wchar_t[]>> {
	DWORD					   base64Len = 0;
	std::unique_ptr<wchar_t[]> base64	 = nullptr;

	//! calcultate required length
	IWDEM_CheckOrReturn(
		CryptBinaryToStringW(
			bytes, size, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, nullptr, &base64Len),
		std::nullopt);

	//! apply base64 encode
	base64 = std::make_unique<wchar_t[]>(base64Len);
	IWDEM_CheckOrReturn(
		CryptBinaryToStringW(
			bytes, size, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, base64.get(), &base64Len),
		std::nullopt);

	return {std::move(base64)};
}

auto GetCurrentUserSid() -> std::optional<std::unique_ptr<wchar_t[]>> {
	HANDLE					   processTokenHandle = nullptr;
	DWORD					   userSize			  = 0;
	wchar_t*				   rawSid			  = nullptr;
	std::unique_ptr<uint8_t[]> userBytes		  = nullptr;
	size_t					   sidLen			  = 0;
	std::unique_ptr<wchar_t[]> sid				  = nullptr;

	IWDEM_CheckOrReturn(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &processTokenHandle),
						std::nullopt);

	//! get size of user bytes
	GetTokenInformation(processTokenHandle, TokenUser, nullptr, 0, &userSize);

	userBytes = std::make_unique<uint8_t[]>(userSize);
	IWDEM_CheckOrReturn(
		GetTokenInformation(processTokenHandle, TokenUser, userBytes.get(), userSize, &userSize),
		std::nullopt);

	IWDEM_CheckOrReturn(
		ConvertSidToStringSidW(reinterpret_cast<PTOKEN_USER>(userBytes.get())->User.Sid, &rawSid),
		std::nullopt);

	sidLen = lstrlenW(rawSid) + 1;
	sid	   = std::make_unique<wchar_t[]>(sidLen);
	memcpy(sid.get(), rawSid, sidLen * sizeof(wchar_t));

	return {std::move(sid)};
}

auto GetCurrentUserName() -> std::optional<std::unique_ptr<wchar_t[]>> {
	HANDLE					   processTokenHandle = nullptr;
	DWORD					   userSize			  = 0;
	std::unique_ptr<uint8_t[]> userBytes		  = nullptr;
	PSID					   pSid				  = nullptr;
	SID_NAME_USE			   eUse				  = SidTypeUnknown;
	DWORD					   szName			  = 1;
	DWORD					   szDomain			  = 1;
	std::unique_ptr<wchar_t[]> name				  = nullptr;
	std::unique_ptr<wchar_t[]> domain			  = nullptr;

	IWDEM_CheckOrReturn(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &processTokenHandle),
						std::nullopt);

	//! get size of user bytes
	GetTokenInformation(processTokenHandle, TokenUser, nullptr, 0, &userSize);

	userBytes = std::make_unique<uint8_t[]>(userSize);
	IWDEM_CheckOrReturn(
		GetTokenInformation(processTokenHandle, TokenUser, userBytes.get(), userSize, &userSize),
		std::nullopt);

	pSid = reinterpret_cast<PTOKEN_USER>(userBytes.get())->User.Sid;

	//! get size of domain and user name
	LookupAccountSidW(nullptr, pSid, nullptr, &szName, nullptr, &szDomain, &eUse);

	name   = std::make_unique<wchar_t[]>(szName);
	domain = std::make_unique<wchar_t[]>(szDomain);
	IWDEM_CheckOrReturn(
		LookupAccountSidW(nullptr, pSid, name.get(), &szName, domain.get(), &szDomain, &eUse),
		std::nullopt);

	return {std::move(name)};
}

auto FormatUserChoice(const wchar_t* fileExt, const wchar_t* userSid, const wchar_t* progId,
					  SYSTEMTIME timestamp) -> std::optional<std::unique_ptr<wchar_t[]>> {
	FILETIME				   fileTime{};
	const wchar_t*			   fmtUserChoice = L"%s%s%s%08lx%08lx%s";
	size_t					   szUserChoice	 = 0;
	std::unique_ptr<wchar_t[]> userChoice	 = nullptr;

	//! magic value decided by version of shell32.dll
	const wchar_t* magicval_UserExperience[] = {
		L"User Choice set via Windows User Experience {D18B6DD5-6124-4341-9318-804003BAFA0B}",
		L"User Choice set via Windows User Experience {480368b3-f2e4-45ae-ba6d-852c4f639a40}",
	};

	timestamp.wSecond		= 0;
	timestamp.wMilliseconds = 0;
	IWDEM_CheckOrReturn(SystemTimeToFileTime(&timestamp, &fileTime), std::nullopt);

	szUserChoice = _scwprintf(fmtUserChoice,
							  fileExt,
							  userSid,
							  progId,
							  fileTime.dwHighDateTime,
							  fileTime.dwLowDateTime,
							  magicval_UserExperience[0]);
	szUserChoice += 1;	 //!< consider null terminate

	userChoice = std::make_unique<wchar_t[]>(szUserChoice);
	_snwprintf_s(userChoice.get(),
				 szUserChoice,
				 _TRUNCATE,
				 fmtUserChoice,
				 fileExt,
				 userSid,
				 progId,
				 fileTime.dwHighDateTime,
				 fileTime.dwLowDateTime,
				 magicval_UserExperience[0]);

	CharLowerW(userChoice.get());

	return {std::move(userChoice)};
}

auto GetUserChoiceHash(const wchar_t* userChoice) -> std::optional<std::unique_ptr<wchar_t[]>> {
	const size_t			 DWORDS_PER_BLOCK = 2;
	const size_t			 BLOCK_SIZE		  = sizeof(DWORD) * DWORDS_PER_BLOCK;
	auto					 inputBytes		  = reinterpret_cast<const uint8_t*>(userChoice);
	int						 inputByteCount	  = (lstrlenW(userChoice) + 1) * sizeof(wchar_t);
	int						 blockCount		  = inputByteCount / BLOCK_SIZE;
	std::unique_ptr<DWORD[]> md5			  = nullptr;
	DWORD					 h0 = 0, h1 = 0, h0Acc = 0, h1Acc = 0, input = 0;

	if (blockCount == 0) {
		return std::nullopt;
	}

	if (auto opt = CNG_MD5(inputBytes, inputByteCount); opt.has_value()) {
		md5 = std::move(opt.value());
	} else {
		return std::nullopt;
	}

	const DWORD C0s[DWORDS_PER_BLOCK][5] = {
		{md5[0] | 1, 0XCF98B111UL, 0X87085B9FUL, 0X12CEB96DUL, 0X257E1D83UL},
		{md5[1] | 1, 0XA27416F5UL, 0XD38396FFUL, 0X7C932B89UL, 0XBFA49F69UL}};

	const DWORD C1s[DWORDS_PER_BLOCK][5] = {
		{md5[0] | 1, 0XEF0569FBUL, 0X689B6B9FUL, 0X79F8A395UL, 0XC3EFEA97UL},
		{md5[1] | 1, 0XC31713DBUL, 0XDDCD1F0FUL, 0X59C3AF2DUL, 0X35BD1EC9UL}};

	for (int i = 0; i < blockCount; ++i) {
		for (int j = 0; j < DWORDS_PER_BLOCK; ++j) {
			const DWORD *C0 = C0s[j], *C1 = C1s[j];
			memcpy(&input, &inputBytes[(i * DWORDS_PER_BLOCK + j) * sizeof(DWORD)], sizeof(DWORD));

			h0 += input;
			h1 += input;

			h0 *= C0[0];
			h0 = ((h0 >> 16) | (h0 << 16)) * C0[1];
			h0 = ((h0 >> 16) | (h0 << 16)) * C0[2];
			h0 = ((h0 >> 16) | (h0 << 16)) * C0[3];
			h0 = ((h0 >> 16) | (h0 << 16)) * C0[4];
			h0Acc += h0;

			h1 = ((h1 >> 16) | (h1 << 16)) * C1[1] + h1 * C1[0];
			h1 = (h1 >> 16) * C1[2] + h1 * C1[3];
			h1 = ((h1 >> 16) | (h1 << 16)) * C1[4] + h1;
			h1Acc += h1;
		}
	}

	DWORD hash[2] = {h0 ^ h1, h0Acc ^ h1Acc};

	return Base64Encode(reinterpret_cast<const uint8_t*>(hash), sizeof(hash));
}