INCLUDES = -I/usr/include/subversion-1 -I/usr/include/apr-0/ $(all_includes)

kde_module_LTLIBRARIES = kio_svn.la

kio_svn_la_SOURCES = svn.cpp
kio_svn_la_LIBADD = -lsvn_client-1 -lkio
kio_svn_la_LDFLAGS = -avoid-version -module $(all_libraries) $(KDE_PLUGIN)

protocol_DATA = svn.protocol
protocoldir = $(kde_servicesdir)

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kio_svn.pot
