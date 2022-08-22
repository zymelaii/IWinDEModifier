#include "acl.h"
#include <securitybaseapi.h>

PACL RequireDACLFullAccess(HANDLE handle, SE_OBJECT_TYPE type) {
	PACL				 pDACL				   = nullptr;
	PSECURITY_DESCRIPTOR pSD				   = nullptr;
	ULONG				 nExplicitEntries	   = 0;
	PEXPLICIT_ACCESS_W	 ListOfExplicitEntries = nullptr;
	LRESULT				 result				   = ERROR_SUCCESS;

	result = GetSecurityInfo(
		handle, type, DACL_SECURITY_INFORMATION, nullptr, nullptr, &pDACL, nullptr, &pSD);
	IWDEM_CheckOrReturn(ERROR_SUCCESS == result, nullptr);

#if 0
	result = GetExplicitEntriesFromAclW(pDACL, &nExplicitEntries, &ListOfExplicitEntries);
	IWDEM_CheckOrReturn(ERROR_SUCCESS == result, nullptr);
#endif

	SetSecurityDescriptorControl(pSD, SE_DACL_PROTECTED | SE_DACL_AUTO_INHERITED, 0);

	result = SetSecurityInfo(
		handle, type, DACL_SECURITY_INFORMATION, nullptr, nullptr, nullptr, nullptr);
	IWDEM_CheckOrReturn(ERROR_SUCCESS == result, nullptr);

	return pDACL;
}

void RestoreDACL(HANDLE handle, SE_OBJECT_TYPE type, PACL pDACL) {
	SetSecurityInfo(handle, type, DACL_SECURITY_INFORMATION, nullptr, nullptr, pDACL, nullptr);
}