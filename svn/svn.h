
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


class kio_svnProtocol : public KIO::SlaveBase
{
	public:
		kio_svnProtocol(const QCString &pool_socket, const QCString &app_socket);
		virtual ~kio_svnProtocol();
		virtual void get(const KURL& url);
		virtual void listDir(const KURL& url);
		virtual void stat(const KURL& url);
		void copy(const KURL & src, const KURL& dest, int permissions, bool overwrite);

	private:
		bool createUDSEntry( const QString& filename, const QString& user, long int size, bool isdir, time_t mtime, KIO::UDSEntry& entry);
		apr_pool_t *pool;
		svn_client_ctx_t **ctx;
		svn_auth_baton_t *auth_baton;
};

#endif
