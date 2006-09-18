
if(SVN_FOUND)
    add_subdirectory(svn)
else(SVN_FOUND)
    message(STATUS "SVN was not found. Please install svn lib.Disable kioslave/svn module for the moment")
endif(SVN_FOUND)

