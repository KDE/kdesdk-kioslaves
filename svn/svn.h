/* This file is part of the KDE project
   Copyright (C) 2003 Mickael Marchand <marchand@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

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

typedef struct kbaton {
	svn_stream_t *target_stream;
	svn_stringbuf_t *target_string;
	svn_stream_t *string_stream;
} kbaton;

typedef struct kio_svn_callback_baton_t {
	const char* base_dir;
	apr_hash_t *config;
	apr_pool_t *pool;
} kio_svn_callback_baton_t;


class kio_svnProtocol : public KIO::SlaveBase
{
	public:
		kio_svnProtocol(const QCString &pool_socket, const QCString &app_socket);
		virtual ~kio_svnProtocol();
		virtual void special( const QByteArray& data );
		virtual void get(const KURL& url);
		virtual void listDir(const KURL& url);
		virtual void stat(const KURL& url);
		virtual void mkdir(const KURL& url, int permissions);
		virtual void del( const KURL& url, bool isfile );
		virtual void copy(const KURL & src, const KURL& dest, int permissions, bool overwrite);
		virtual void rename(const KURL& src, const KURL& dest, bool overwrite);
		void checkout( const KURL& repos, const KURL& wc, int revnumber, const QString& revkind );
		void update( const KURL& wc, int revnumber, const QString& revkind );
		static svn_error_t* checkAuth(const char **info, const char *prompt, svn_boolean_t hide, void *baton, apr_pool_t *pool); 
		void recordCurrentURL(const KURL& url);
		KURL myURL;
		svn_client_ctx_t ctx;
		KIO::AuthInfo info;

		enum SVN_METHOD { 
SVN_CHECKOUT=1, //KURL repository, KURL workingcopy, int revnumber=-1, QString revkind(HEAD, ...) //revnumber==-1 => use of revkind
SVN_UPDATE=2, // KURL wc (svn:///tmp/test, int revnumber=-1, QString revkind(HEAD, ...) // revnumber==-1 => use of revkind
SVN_COMMIT=3, 
SVN_LOG=4, 
SVN_IMPORT=5
		};

	private:
		bool createUDSEntry( const QString& filename, const QString& user, long int size, bool isdir, time_t mtime, KIO::UDSEntry& entry);
		apr_pool_t *pool;
};

#endif
