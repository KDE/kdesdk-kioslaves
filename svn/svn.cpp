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
#include <svn_ra.h>

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

static svn_error_t *
open_tmp_file (apr_file_t **fp, void *callback_baton) {
  kio_svn_callback_baton_t *cb = callback_baton;
  const char *truepath;
  const char *ignored_filename;

  if (cb->base_dir)
    truepath = apr_pstrdup (cb->pool, cb->base_dir);
  else
    /* ### TODO: need better tempfile support */
    truepath = "";

  /* Tack on a made-up filename. */
  truepath = svn_path_join (truepath, "tempfile", cb->pool);

  /* Open a unique file;  use APR_DELONCLOSE. */  
  SVN_ERR (svn_io_open_unique_file (fp, &ignored_filename, truepath, ".tmp", TRUE, cb->pool));

  return SVN_NO_ERROR;
}

static svn_error_t *write_to_string(void *baton, const char *data, apr_size_t *len) {
	kbaton *tb = ( kbaton* )baton;

	svn_stringbuf_appendbytes(tb->target_string, data, *len);
	//processedSize(*len);

	return SVN_NO_ERROR;
}

//don't implement mimeType() until we don't need to download the whole file

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

	QString target = url.url().replace( 0, 3, "http" );
	kdDebug() << "myURL: " << target << endl;
	
	//find the requested revision
	svn_opt_revision_t rev;
	int idx = target.findRev( "?rev=" );
	if ( idx != -1 ) {
		QString revstr = target.mid( idx+5 );
		kdDebug() << "revision string found " << revstr  << endl;
		if ( revstr == "HEAD" ) {
			rev.kind = svn_opt_revision_head;
			kdDebug() << "revision searched : HEAD" << endl;
		} else {
			rev.kind = svn_opt_revision_number;
			rev.value.number = revstr.toLong();
			kdDebug() << "revision searched : " << rev.value.number << endl;
		}
		target = target.left( idx );
		kdDebug() << "new target : " << target << endl;
	} else {
		kdDebug() << "no revision given. searching HEAD " << endl;
		rev.kind = svn_opt_revision_head;
	}

	svn_client_cat (bt->string_stream, target.local8Bit(),&rev,*ctx, subpool);

	// Send the mimeType as soon as it is known
	QByteArray *cp = new QByteArray();
	cp->setRawData( bt->target_string->data, bt->target_string->len );
	KMimeType::Ptr mt = KMimeType::findByContent(*cp);
	kdDebug() << "KMimeType returned : " << mt->name() << endl;
	mimeType( mt->name() );

	totalSize(bt->target_string->len);

	//send data
	data(*cp);

	data(QByteArray()); // empty array means we're done sending the data
	finished();
	svn_pool_destroy (subpool);
}

static int
compare_items_as_paths (const svn_item_t *a, const svn_item_t *b) {
  return svn_path_compare_paths ((const char *)a->key, (const char *)b->key);
}

void kio_svnProtocol::stat(const KURL & url){
	kdDebug() << "kio_svn::stat(const KURL& url) : " << url.url() << endl ;

	void *ra_baton, *session;
	svn_ra_plugin_t *ra_lib;
	svn_node_kind_t kind;
	const char *auth_dir;
	svn_revnum_t revnum;
	apr_pool_t *subpool = svn_pool_create (pool);

	QString target = url.url().replace( 0, 3, "http" );

	//find the requested revision
/*	svn_opt_revision_t rev;
	int idx = target.findRev( "?rev=" );
	if ( idx != -1 ) {
		QString revstr = target.mid( idx+5 );
		kdDebug() << "revision string found " << revstr  << endl;
		if ( revstr == "HEAD" ) {
			rev.kind = svn_opt_revision_head;
			kdDebug() << "revision searched : HEAD" << endl;
		} else {
			rev.kind = svn_opt_revision_number;
			rev.value.number = revstr.toLong();
			kdDebug() << "revision searched : " << rev.value.number << endl;
		}
		target = target.left( idx );
		kdDebug() << "new target : " << target << endl;
	} else {
		kdDebug() << "no revision given. searching HEAD " << endl;
		rev.kind = svn_opt_revision_head;
	}
*/
	//init
	svn_ra_init_ra_libs(&ra_baton,subpool);
	//find RA libs
	svn_ra_get_ra_library(&ra_lib,ra_baton,target,subpool);
	kdDebug() << "RA init completed" << endl;
	//start session
	//FIXME
	svn_ra_callbacks_t *cbtable = apr_pcalloc(subpool, sizeof(*cbtable));	
	kio_svn_callback_baton_t *callbackbt = apr_pcalloc(subpool, sizeof( *callbackbt ));

	//XXX hmmm ... maybe i should ask sussman about that part
	cbtable->open_tmp_file = open_tmp_file;
	cbtable->get_wc_prop = NULL;
	cbtable->set_wc_prop = NULL;
	cbtable->push_wc_prop = NULL;
	cbtable->auth_baton = ( *ctx )->auth_baton;

	callbackbt->base_dir = target;
	callbackbt->pool = subpool;
	callbackbt->config = ( *ctx )->config;
	
	ra_lib->open(&session,target,cbtable,callbackbt,( *ctx )->config,subpool);
	kdDebug() << "Session opened to " << target << endl;

	//find number for HEAD
	ra_lib->get_latest_revnum(session,&revnum,subpool);
	kdDebug() << "Got revnum" << endl;
	
	//get it
	ra_lib->check_path(&kind,session,"",revnum,subpool);
	kdDebug() << "Checked Path" << endl;
	
	UDSEntry entry;
	switch ( kind ) {
		case svn_node_file:
			kdDebug() << "::stat result : file" << endl;
			createUDSEntry(url.url(),"",0,false,0,entry);
			statEntry( entry );
			finished();
			break;
		case svn_node_dir:
			kdDebug() << "::stat result : directory" << endl;
			createUDSEntry(url.url(),"",0,true,0,entry);
			statEntry( entry );
			finished();
			break;
		case svn_node_unknown:
		case svn_node_none:
			//error XXX
		default:
			kdDebug() << "::stat result : UNKNOWN ==> WOW :)" << endl;
			;
	}
	svn_pool_destroy( subpool );
}

void kio_svnProtocol::listDir(const KURL & url){
	kdDebug() << "kio_svn::listDir(const KURL& url) : " << url.url() << endl ;

	apr_pool_t *subpool = svn_pool_create (pool);
	apr_hash_t *dirents;
	QString target = url.url().replace( 0, 3, "http" );
	
	//find the requested revision
	svn_opt_revision_t rev;
	int idx = target.findRev( "?rev=" );
	if ( idx != -1 ) {
		QString revstr = target.mid( idx+5 );
		kdDebug() << "revision string found " << revstr  << endl;
		if ( revstr == "HEAD" ) {
			rev.kind = svn_opt_revision_head;
			kdDebug() << "revision searched : HEAD" << endl;
		} else {
			rev.kind = svn_opt_revision_number;
			rev.value.number = revstr.toLong();
			kdDebug() << "revision searched : " << rev.value.number << endl;
		}
		target = target.left( idx );
		kdDebug() << "new target : " << target << endl;
	} else {
		kdDebug() << "no revision given. searching HEAD " << endl;
		rev.kind = svn_opt_revision_head;
	}

	svn_client_ls (&dirents, target, &rev, false, *ctx, subpool);

  apr_array_header_t *array;
  int i;

  array = apr_hash_sorted_keys (dirents, compare_items_as_paths, subpool);
  
	UDSEntry entry;
  for (i = 0; i < array->nelts; ++i) {
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

void kio_svnProtocol::copy(const KURL & src, const KURL& dest, int permissions, bool overwrite){
		kdDebug() << "kio_svnProtocol::copy() Source : " << src.url() << " Dest : " << dest << endl;

		finished();
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
