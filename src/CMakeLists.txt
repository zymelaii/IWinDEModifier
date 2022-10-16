add_subdirectory(share)

if (BUILD_DesktopTab)
	add_subdirectory(DesktopTab)
endif()

if (BUILD_LazyCraft)
	add_subdirectory(LazyCraft)
endif()

if (BUILD_iwdesh)
	add_subdirectory(iwdesh)
endif()
