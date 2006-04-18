kde4_header()

add_subdirectory( ksvnd ) 
add_subdirectory( svnhelper ) 
add_subdirectory( icons ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kio_svn_PART_SRCS svn.cpp )

kde4_automoc(${kio_svn_PART_SRCS})

kde4_add_plugin(kio_svn ${kio_svn_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kio_svn )

target_link_libraries(kio_svn  ${KDE4_KIO_LIBS} )

install_targets(${PLUGIN_INSTALL_DIR} kio_svn )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES svn+http.protocol svn+https.protocol svn+file.protocol svn+ssh.protocol svn.protocol )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = ksvnd svnhelper icons
#
#INCLUDES = $(SVN_INCLUDE) $(all_includes)
#
#kde_module_LTLIBRARIES = kio_svn.la
#
#kio_svn_la_SOURCES = svn.cpp
#kio_svn_la_LIBADD = -lkio
#kio_svn_la_LDFLAGS = -avoid-version -module $(all_libraries) $(KDE_PLUGIN) $(SVNLD) $(SVN_LIB)
#
#protocol_DATA = svn+http.protocol svn+https.protocol svn+file.protocol svn+ssh.protocol svn.protocol
#protocoldir = $(kde_servicesdir)
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kio_svn.pot
