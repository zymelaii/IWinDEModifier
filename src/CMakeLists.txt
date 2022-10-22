add_subdirectory(share)

add_subdirectory(iwindeapi)
add_subdirectory(IWinDEModifier)

if (BUILD_DesktopTab)
	add_subdirectory(DesktopTab)
endif()

if (BUILD_iwdesh)
	add_subdirectory(iwdesh)
endif()

if (BUILD_LazyCraft)
	add_subdirectory(LazyCraft)
endif()

if (BUILD_Ethereality)
	add_subdirectory(Ethereality)
endif()