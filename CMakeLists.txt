
if(SVN_FOUND)
	message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(SVN_SUBDIR)")
    add_subdirectory(svn)
endif(SVN_FOUND)


