SUBDIRS = ksvnd svnhelper icons

INCLUDES = $(SVN_INCLUDE) $(all_includes)

kde_module_LTLIBRARIES = kio_svn.la

kio_svn_la_SOURCES = svn.cpp
kio_svn_la_LIBADD = -lkio
kio_svn_la_LDFLAGS = -avoid-version -module $(all_libraries) $(KDE_PLUGIN) $(SVNLD) $(SVN_LIB)

protocol_DATA = svn+http.protocol svn+https.protocol svn+file.protocol svn+ssh.protocol svn.protocol
protocoldir = $(kde_servicesdir)

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kio_svn.pot
