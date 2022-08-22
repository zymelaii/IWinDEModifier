#include "internal.h"
#include <accctrl.h>
#include <aclapi.h>
#include <securitybaseapi.h>

PACL RequireDACLFullAccess(HANDLE handle, SE_OBJECT_TYPE type);

void RestoreDACL(HANDLE handle, SE_OBJECT_TYPE type, PACL pDACL);