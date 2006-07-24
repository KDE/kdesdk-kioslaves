add_definitions(${SVN_INCLUDES})
add_subdirectory( ksvnd ) 
add_subdirectory( svnhelper ) 
add_subdirectory( icons ) 




########### next target ###############

set(kio_svn_PART_SRCS svn.cpp )

kde4_automoc(${kio_svn_PART_SRCS})

kde4_add_plugin(kio_svn ${kio_svn_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kio_svn )

target_link_libraries(kio_svn  ${KDE4_KIO_LIBS} )

install(TARGETS kio_svn  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES svn+http.protocol svn+https.protocol svn+file.protocol svn+ssh.protocol svn.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )




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
