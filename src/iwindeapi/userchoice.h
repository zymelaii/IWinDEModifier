#include "internal.h"

/*!
 * \brief get SID of current user
 */
std::optional<std::unique_ptr<wchar_t[]>> GetCurrentUserSid();

/*!
 * \brief get name of current user
 */
std::optional<std::unique_ptr<wchar_t[]>> GetCurrentUserName();

/*!
 * \brief format params to UserChoice string
 * 
 * \param fileExt extension of file type, of .<ext> format
 * \param userSid SID of target user, defaultly it indicates the current user which can be got from
 * GetCurrentUserSid()
 * \param progId ProgId that already registerd in registry (expected)
 * \param timestamp a SYSTEMTIME timestamp which indicates when the UserChoice was applied, where
 * second and millisecond section will be ignored by UserChoice
 *
 * \note UserChoice is a registry key under
 * *\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.<file-ext>,
 * while UserChoice string indicates a string consists of required params.
 */
std::optional<std::unique_ptr<wchar_t[]>> FormatUserChoice(const wchar_t* fileExt,
														   const wchar_t* userSid,
														   const wchar_t* progId,
														   SYSTEMTIME	  timestamp);

/*!
 * \brief calculate hash code of UserChoice

 * \param userChoice UserChoice string made by FormatUserChoice
 */
std::optional<std::unique_ptr<wchar_t[]>> GetUserChoiceHash(const wchar_t* userChoice);