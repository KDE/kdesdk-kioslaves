kde4_header()

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(SVN_SUBDIR)")



########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#if include_kioslave_svn
#SVN_SUBDIR = svn
#endif
#
#SUBDIRS= $(SVN_SUBDIR)
#
