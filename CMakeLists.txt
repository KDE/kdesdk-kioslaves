macro_optional_find_package(SVN)

if(SVN_FOUND)
    add_subdirectory(svn)
else(SVN_FOUND)
    message(STATUS "SVN was not found. Please install svn lib. kioslave/svn will not be compiled otherwise.")
endif(SVN_FOUND)

