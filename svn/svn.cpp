/* Copyright (C) 2003 Mickael Marchand <marchand@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qcstring.h>
#include <qsocket.h>
#include <qdatetime.h>
#include <qbitarray.h>

#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kinstance.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kurl.h>
#include <ksock.h>

#include <svn_sorts.h>
#include <svn_path.h>
#include <svn_utf.h>

#include <kmimetype.h>

#include "svn.h"

using namespace KIO;

kio_svnProtocol::kio_svnProtocol(const QCString &pool_socket, const QCString &app_socket)
	: SlaveBase("kio_svn", pool_socket, app_socket){
		kdDebug() << "kio_svnProtocol::kio_svnProtocol()" << endl;
		svn_error_t *err;
		apr_initialize();
		pool = svn_pool_create (NULL);
		err = svn_config_ensure (pool);
		if (err) {
			//FIXME
			svn_pool_destroy (pool);
		}
		*ctx = apr_pcalloc(pool, sizeof(*ctx));
		svn_config_get_config (&(*ctx)->config,pool);

		apr_array_header_t *providers = apr_array_make(pool, 1, sizeof(svn_auth_provider_object_t *));

		svn_auth_provider_object_t *username_wc_provider;

		svn_client_get_username_provider (&username_wc_provider,pool);

		*(svn_auth_provider_object_t **)apr_array_push(providers) = username_wc_provider;

		svn_auth_open(&auth_baton, providers, pool);

		( *ctx )->auth_baton = auth_baton;

}

kio_svnProtocol::~kio_svnProtocol(){
	kdDebug() << "kio_svnProtocol::~kio_svnProtocol()" << endl;
	svn_pool_destroy(pool);
	apr_terminate();
}

static svn_error_t *write_to_string(void *baton, const char *data, apr_size_t *len) {
	kbaton *tb = ( kbaton* )baton;

	svn_stringbuf_appendbytes(tb->target_string, data, *len);

	return SVN_NO_ERROR;
}


void kio_svnProtocol::get(const KURL& url ){
	kdDebug() << "kio_svn::get(const KURL& url)" << endl ;

	QString remoteServer = url.host();
	infoMessage(i18n("Looking for %1...").arg( remoteServer ) );

	apr_pool_t *subpool = svn_pool_create (pool);
	kbaton *bt;
	bt = apr_pcalloc(subpool, sizeof(*bt));
	bt->target_string = svn_stringbuf_create("", subpool);
	bt->string_stream = svn_stream_create(bt,subpool);
	svn_stream_set_write(bt->string_stream,write_to_string);

	svn_opt_revision_t rev;
	rev.kind = svn_opt_revision_head;
	QString target = url.url().replace( 0, 3, "http" );
	kdDebug() << "myURL: " << target << endl;

	svn_client_cat (bt->string_stream, target.local8Bit(),&rev,*ctx, subpool);

	// Send the mimeType as soon as it is known
	QByteArray *cp = new QByteArray();
	cp->setRawData( bt->target_string->data, bt->target_string->len );
	KMimeType::Ptr mt = KMimeType::findByContent(*cp);
	kdDebug() << "KMimeType returned : " << mt->name() << endl;
	mimeType( mt->name() );

	//send data
	data(*cp);

	data(QByteArray()); // empty array means we're done sending the data
	finished();
	svn_pool_destroy (subpool);
}

static int
compare_items_as_paths (const svn_item_t *a, const svn_item_t *b)
{
  return svn_path_compare_paths ((const char *)a->key, (const char *)b->key);
}

void kio_svnProtocol::stat(const KURL & url){
	kdDebug() << "kio_svn::stat(const KURL& url) : " << url.url() << endl ;
	UDSEntry entry;

	createUDSEntry(url.url(), "",0,true,0,entry);
	
	statEntry( entry );
	finished();
}

void kio_svnProtocol::listDir(const KURL & url){
	kdDebug() << "kio_svn::listDir(const KURL& url) : " << url.url() << endl ;

	apr_pool_t *subpool = svn_pool_create (pool);
	apr_hash_t *dirents;
	svn_opt_revision_t rev;
	rev.kind = svn_opt_revision_head;
	QString target = url.url().replace( 0, 3, "http" );

	svn_client_ls (&dirents, target, &rev, false, *ctx, subpool);

  apr_array_header_t *array;
  int i;

  array = apr_hash_sorted_keys (dirents, compare_items_as_paths, subpool);
  
	UDSEntry entry;
  for (i = 0; i < array->nelts; ++i)
    {
			entry.clear();
      const char *utf8_entryname, *native_entryname;
      svn_dirent_t *dirent;
      svn_item_t *item;
     
      item = &APR_ARRAY_IDX (array, i, svn_item_t);

      utf8_entryname = item->key;

      dirent = apr_hash_get (dirents, utf8_entryname, item->klen);

      svn_utf_cstring_from_utf8 (&native_entryname, utf8_entryname, subpool);
			const char *native_author = NULL;

			if (dirent->last_author)
				svn_utf_cstring_from_utf8 (&native_author, dirent->last_author, subpool);

			if ( createUDSEntry(QString( native_entryname ), QString( native_author ), dirent->size,
						dirent->kind==svn_node_dir ? true : false, dirent->time, entry) )
				listEntry( entry, false );
			
/*
			printf ("%c %7"SVN_REVNUM_T_FMT" %8.8s "
					"%8"SVN_FILESIZE_T_FMT" %12s %s%s\n",
					dirent->has_props ? 'P' : '_',
					dirent->created_rev,
					native_author ? native_author : "      ? ",
					dirent->size,
					timestr,
					native_entryname,
					(dirent->kind == svn_node_dir) ? "/" : "");*/
		}
	listEntry( entry, true );

	finished();
	svn_pool_destroy (subpool);
}

bool kio_svnProtocol::createUDSEntry( const QString& filename, const QString& user, long int size, bool isdir, time_t mtime, UDSEntry& entry) {
	UDSAtom atom;
	atom.m_uds = KIO::UDS_NAME;
	atom.m_str = filename;
	entry.append( atom );

	atom.m_uds = KIO::UDS_FILE_TYPE;
	atom.m_long = isdir ? S_IFDIR : S_IFREG;
	entry.append( atom );

	atom.m_uds = KIO::UDS_SIZE;
	atom.m_long = size;
	entry.append( atom );
	
	atom.m_uds = KIO::UDS_MODIFICATION_TIME;
	atom.m_long = mtime;
	entry.append( atom );
	
	atom.m_uds = KIO::UDS_USER;
	atom.m_str = user;
	entry.append( atom );

	return true;
}

extern "C"
{
	int kdemain(int argc, char **argv)    {
		KInstance instance( "kio_svn" );

		kdDebug(7101) << "*** Starting kio_svn " << endl;

		if (argc != 4) {
			kdDebug(7101) << "Usage: kio_svn  protocol domain-socket1 domain-socket2" << endl;
			exit(-1);
		}

		kio_svnProtocol slave(argv[2], argv[3]);
		slave.dispatchLoop();

		kdDebug(7101) << "*** kio_svn Done" << endl;
		return 0;
	}
}
