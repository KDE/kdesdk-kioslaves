macro_optional_find_package(SVN)
macro_log_feature(SVN_FOUND "SVN" "Subversion Development Libraries" "http://subversion.tigris.org/project_packages.html" FALSE "" "Required in order to build the Subversion kioslave")

if(SVN_FOUND)
  add_subdirectory(svn)
endif(SVN_FOUND)

