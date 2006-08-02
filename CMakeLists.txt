
if(SVN_FOUND)
    add_subdirectory(svn)
else(SVN_FOUND)
    message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(SVN_SUBDIR)")
endif(SVN_FOUND)

