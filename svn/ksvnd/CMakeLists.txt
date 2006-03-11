kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kded_ksvnd_PART_SRCS ksvnd.cpp )

kde4_automoc(${kded_ksvnd_PART_SRCS})

kde4_add_ui_files(kded_ksvnd_PART_SRCS commitdlg.ui )

kde4_add_dcop_skels(kded_ksvnd_PART_SRCS ksvnd.h )

kde4_add_plugin(kded_ksvnd ${kded_ksvnd_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kded_ksvnd )

target_link_libraries(kded_ksvnd  ${QT_AND_KDECORE_LIBS} kio )

install_targets(${LIB_INSTALL_DIR}/kde4 kded_ksvnd )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR}/kded FILES ksvnd.desktop )

kde4_footer()



#original Makefile.am contents follow:

##INCLUDES= -I$(top_srcdir)/kwallet/client $(all_includes)
#INCLUDES= $(all_includes)
#
#kde_module_LTLIBRARIES = kded_ksvnd.la
#
#kded_ksvnd_la_SOURCES = commitdlg.ui ksvnd.cpp ksvnd.skel 
#kded_ksvnd_la_METASOURCES = AUTO
#kded_ksvnd_la_LDFLAGS = $(all_libraries) -module -avoid-version
##kded_ksvnd_la_LIBADD = $(top_builddir)/kwallet/client/libkwalletclient.la $(LIB_KIO)
#kded_ksvnd_la_LIBADD = $(LIB_KIO)
#
#kded_DATA = ksvnd.desktop
#kdeddir = $(kde_servicesdir)/kded
