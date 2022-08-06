#include <windows.h>
#include <winerror.h>
#include <errhandlingapi.h>
#include <accctrl.h>
#include <aclapi.h>
#include <winnt.h>
#include <winreg.h>

#include <stdio.h>
int main(int argc, char* argv[]) {
	if (argc == 1) {
		return 0;
	}

	char subkey[256]{};
	sprintf(subkey,
			R"(SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.%s\UserChoice)",
			argv[1]);

	LRESULT lResult;
	HKEY	hkey;
	lResult = RegOpenKeyEx(HKEY_CURRENT_USER, subkey, 0, KEY_READ | KEY_WOW64_64KEY, &hkey);
	if (lResult == ERROR_SUCCESS) {
		puts("[INFO] RegOpenKeyEx pass");
	} else {
		printf("ErrorCode: %lld", lResult);
		exit(lResult);
	}

	char  buffer[256]{0};
	DWORD cbData = 256;
	DWORD dwType = 0;
	lResult		 = RegGetValue(hkey, NULL, "ProgId", RRF_RT_REG_SZ, &dwType, buffer, &cbData);
	if (lResult == ERROR_SUCCESS) {
		puts("[INFO] RegGetValue pass");
	} else {
		printf("ErrorCode: %lld", lResult);
		// ERROR_MORE_DATA
		exit(lResult);
	}
	printf("UserChoice.ProgId: %s\n", buffer);

	PSID				 pSidOwner				 = NULL;
	PACL				 pDACL					 = NULL;
	PACL				 pSACL					 = NULL;
	PSECURITY_DESCRIPTOR pSD					 = NULL;
	DWORD				 dwRtnCode				 = 0;
	BOOL				 bRtnBool				 = TRUE;
	LPTSTR				 AcctName				 = NULL;
	LPTSTR				 DomainName				 = NULL;
	DWORD				 dwAcctName				 = 1;
	DWORD				 dwDomainName			 = 1;
	SID_NAME_USE		 eUse					 = SidTypeUnknown;
	ULONG				 cCountOfExplicitEntries = 0;
	PEXPLICIT_ACCESS	 ListOfExplicitEntries	 = NULL;

	lResult = GetSecurityInfo(hkey,
							  SE_REGISTRY_KEY,
							  DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION,
							  &pSidOwner,
							  NULL,
							  &pDACL,
							  &pSACL,
							  &pSD);

	if (lResult == ERROR_SUCCESS) {
		puts("[INFO] GetSecurityInfo pass");
	} else {
		printf("ErrorCode: %lld", lResult);
		exit(lResult);
	}

	printf("SACL: 0x%p\n", pSACL);

	lResult = LookupAccountSid(
		NULL, pSidOwner, AcctName, (LPDWORD)&dwAcctName, DomainName, (LPDWORD)&dwDomainName, &eUse);
	AcctName   = (LPTSTR)GlobalAlloc(GMEM_FIXED, dwAcctName);
	DomainName = (LPTSTR)GlobalAlloc(GMEM_FIXED, dwDomainName);
	lResult	   = LookupAccountSid(
		   NULL, pSidOwner, AcctName, (LPDWORD)&dwAcctName, DomainName, (LPDWORD)&dwDomainName, &eUse);

	lResult = GetExplicitEntriesFromAcl(pDACL, &cCountOfExplicitEntries, &ListOfExplicitEntries);
	if (lResult == ERROR_SUCCESS) {
		puts("[INFO] GetExplicitEntriesFromAcl pass");
	} else {
		printf("ErrorCode: %lld", lResult);
		exit(lResult);
	}

	printf(R"(Owner: %s\\%s
Security Descriptor: 0x%04x
Access Entry Count: %lu
ACL Address: 0x%p
)",
		   DomainName,
		   AcctName,
		   static_cast<SECURITY_DESCRIPTOR*>(pSD)->Control,
		   cCountOfExplicitEntries,
		   ListOfExplicitEntries);

	if (cCountOfExplicitEntries > 0) {
		printf("Access List:\n");
		for (int i = 0; i < cCountOfExplicitEntries; ++i) {
			EXPLICIT_ACCESS ace = ListOfExplicitEntries[i];
			printf("    -> [%d] Access Mask: 0x%08lx, Access Mode: %s, Inheritance: 0x%02lx, "
				   "Trustee Type: %s\n",
				   i,
				   ace.grfAccessPermissions,
				   (char const*[]){
					   "NOT_USED_ACCESS",
					   "GRANT_ACCESS",
					   "SET_ACCESS",
					   "DENY_ACCESS",
					   "REVOKE_ACCESS",
					   "SET_AUDIT_SUCCESS",
					   "SET_AUDIT_FAILURE",
				   }[ace.grfAccessMode],
				   ace.grfInheritance,
				   (char const*[]){
					   "UNKNOWN",
					   "USER",
					   "GROUP",
					   "DOMAIN",
					   "ALIAS",
					   "WELL_KNOWN_GROUP",
					   "DELETED",
					   "INVALID",
					   "COMPUTER",
				   }[ace.Trustee.TrusteeType]);
		}
	}

	//! remove all directive access
	SetSecurityInfo(hkey, SE_REGISTRY_KEY, DACL_SECURITY_INFORMATION, pSD, nullptr, nullptr, nullptr);
	if (lResult == ERROR_SUCCESS) {
		puts("[INFO] SetSecurityInfo pass");
	} else {
		printf("ErrorCode: %lld", lResult);
		exit(lResult);
	}

	RegCloseKey(hkey);

	return 0;
}