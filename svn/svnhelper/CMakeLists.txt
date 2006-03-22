kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kio_svn_helper_SRCS kio_svn_helper.cpp )

kde4_automoc(${kio_svn_helper_SRCS})

kde4_add_ui_files(kio_svn_helper_SRCS 
   subversioncheckout.ui 
   subversionswitch.ui 
   subversionlog.ui 
   subversiondiff.ui )

kde4_add_executable(kio_svn_helper ${kio_svn_helper_SRCS})

target_link_libraries(kio_svn_helper  ${KDE4_KDECORE_LIBS} kdeui kio kio )

install_targets(/bin kio_svn_helper )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/konqueror/servicemenus FILES subversion.desktop subversion_toplevel.desktop )

kde4_footer()



#original Makefile.am contents follow:

#bin_PROGRAMS = kio_svn_helper
#
#INCLUDES = $(all_includes)
#AM_LDFLAGS = $(all_libraries)
#
#kio_svn_helper_SOURCES = kio_svn_helper.cpp subversioncheckout.ui subversionswitch.ui subversionlog.ui subversiondiff.ui
#
#kio_svn_helper_LDFLAGS = $(KDE_RPATH) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KSYCOCA) $(LIB_KIO) $(all_libraries)
#
#servicemenudir = \
#    $(kde_datadir)/konqueror/servicemenus
#
#servicemenu_DATA = \
#    subversion.desktop subversion_toplevel.desktop
#
#
#METASOURCES = AUTO
#
