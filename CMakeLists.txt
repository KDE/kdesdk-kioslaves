option(WITH_subversion "enable support for subversion" ON)

if(WITH_subversion)
	message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(SVN_SUBDIR)")
	#add_subdirectory(svn)
endif(WITH_subversion)



########### install files ###############


#original Makefile.am contents follow:

#if include_kioslave_svn
#SVN_SUBDIR = svn
#endif
#
#SUBDIRS= $(SVN_SUBDIR)
#
