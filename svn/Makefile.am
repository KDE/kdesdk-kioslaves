INCLUDES = $(SVNINC) $(all_includes)

kde_module_LTLIBRARIES = kio_svn.la

kio_svn_la_SOURCES = svn.cpp
kio_svn_la_LIBADD = $(SVNLIBS) -lkio
kio_svn_la_LDFLAGS = -avoid-version -module $(SVNLD) $(all_libraries) $(KDE_PLUGIN) 

protocol_DATA = svn+http.protocol
protocoldir = $(kde_servicesdir)

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kio_svn.pot
