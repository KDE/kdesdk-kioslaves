#ifndef _svn_H_
#define _svn_H_

#include <qstring.h>
#include <qcstring.h>

#include <kurl.h>
#include <kio/global.h>
#include <kio/slavebase.h>
#include <svn_pools.h>
#include <svn_auth.h>
#include <svn_client.h>
#include <svn_config.h>
#include <sys/stat.h>
#include <qvaluelist.h>

class QCString;

class kio_svnProtocol : public KIO::SlaveBase
{
	public:
		kio_svnProtocol(const QCString &pool_socket, const QCString &app_socket);
		virtual ~kio_svnProtocol();
		virtual void mimetype(const KURL& url);
		virtual void get(const KURL& url);
		virtual void listDir(const KURL& url);
		virtual void stat(const KURL& url);

	private:
		bool createUDSEntry( const QString& filename, const QString& user, long int size, bool isdir, time_t mtime, KIO::UDSEntry& entry);
		apr_pool_t *pool;
		svn_client_ctx_t **ctx;
		svn_auth_baton_t *auth_baton;
};

#endif
