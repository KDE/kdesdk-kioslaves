bin_PROGRAMS = kio_svn_helper

INCLUDES = $(all_includes)
AM_LDFLAGS = $(all_libraries)

kio_svn_helper_SOURCES = kio_svn_helper.cpp subversioncheckout.ui subversionswitch.ui subversionlog.ui subversiondiff.ui

kio_svn_helper_LDFLAGS = $(KDE_RPATH) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KSYCOCA) $(LIB_KIO) $(all_libraries)

servicemenudir = \
    $(kde_datadir)/konqueror/servicemenus

servicemenu_DATA = \
    subversion.desktop subversion_toplevel.desktop


METASOURCES = AUTO

