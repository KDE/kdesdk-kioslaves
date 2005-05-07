/* This file is part of the KDE project
   Copyright (c) 2005 Mickael Marchand <marchand@kde.org>

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

#include <kcmdlineargs.h>
#include <klocale.h>
#include <kapplication.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <dcopclient.h>
#include <kdebug.h>
#include <kglobal.h>
#include <qtimer.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kio/netaccess.h>
#include <kpassivepopup.h>
#include <qpixmap.h>
#include <kmessagebox.h>

#include "kio_svn_helper.h"
#include "subversioncheckout.h"
#include <kurlrequester.h>
#include <qspinbox.h>

SvnHelper::SvnHelper():KApplication() {
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	KWinModule wm ( this );
	m_id = wm.activeWindow();

	KURL::List list;
	for ( int i = 0 ; i < args->count() ; i++ )
		list << args->url(i);

	if (args->isSet("u")) {
		kdDebug(7128) << "update " << list << endl;
		KURL servURL = "svn+http://this_is_a_fake_URL_and_this_is_normal/";
		//FIXME when 1.2 is out (move the loop inside kio_svn's ::update)
		for ( QValueListConstIterator<KURL> it = list.begin(); it != list.end() ; ++it ) {
			QByteArray parms;
			QDataStream s( parms, IO_WriteOnly );
			int cmd = 2;
			int rev = -1;
			kdDebug(7128) << "updating : " << (*it).prettyURL() << endl;
			s << cmd << *it << rev << QString( "HEAD" );
			KIO::SimpleJob * job = KIO::special(servURL, parms, true);
			connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );
			KIO::NetAccess::synchronousRun( job, 0 );
		}
	} else if (args->isSet("c")) {
		kdDebug(7128) << "commit " << list << endl;
		KURL servURL = "svn+http://this_is_a_fake_URL_and_this_is_normal/";
		QByteArray parms;
		QDataStream s( parms, IO_WriteOnly );
		int cmd = 3;
		s<<cmd;
		for ( QValueListConstIterator<KURL> it = list.begin(); it != list.end() ; ++it ) {
			kdDebug(7128) << "commiting : " << (*it).prettyURL() << endl;
			s << *it;
		}
		KIO::SimpleJob * job = KIO::special(servURL, parms, true);
		connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );
		KIO::NetAccess::synchronousRun( job, 0 );
	} else if (args->isSet("a")) {
		kdDebug(7128) << "add " << list << endl;
		KURL servURL = "svn+http://this_is_a_fake_URL_and_this_is_normal/";
		for ( QValueListConstIterator<KURL> it = list.begin(); it != list.end() ; ++it ) {
			QByteArray parms;
			QDataStream s( parms, IO_WriteOnly );
			int cmd = 6;
			kdDebug(7128) << "adding : " << (*it).prettyURL() << endl;
			s << cmd << *it;
			KIO::SimpleJob * job = KIO::special(servURL, parms, true);
			connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );
			KIO::NetAccess::synchronousRun( job, 0 );
		}
	} else if (args->isSet("d")) {
		kdDebug(7128) << "delete " << list << endl;
		KURL servURL = "svn+http://this_is_a_fake_URL_and_this_is_normal/";
		QByteArray parms;
		QDataStream s( parms, IO_WriteOnly );
		int cmd = 7;
		s<<cmd;
		for ( QValueListConstIterator<KURL> it = list.begin(); it != list.end() ; ++it ) {
			kdDebug(7128) << "deleting : " << (*it).prettyURL() << endl;
			s << *it;
		}
		KIO::SimpleJob * job = KIO::special(servURL, parms, true);
		connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );
		KIO::NetAccess::synchronousRun( job, 0 );
	} else if (args->isSet("r")) {
		kdDebug(7128) << "revert " << list << endl;
		KURL servURL = "svn+http://this_is_a_fake_URL_and_this_is_normal/";
		QByteArray parms;
		QDataStream s( parms, IO_WriteOnly );
		int cmd = 8;
		s<<cmd;
		for ( QValueListConstIterator<KURL> it = list.begin(); it != list.end() ; ++it ) {
			kdDebug(7128) << "reverting : " << (*it).prettyURL() << endl;
			s << *it;
		}
		KIO::SimpleJob * job = KIO::special(servURL, parms, true);
		connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );
		KIO::NetAccess::synchronousRun( job, 0 );
	} else if (args->isSet("C")) {
		kdDebug(7128) << "checkout " << list << endl;
		SubversionCheckout d;
		int result = d.exec();
		if ( result == QDialog::Accepted ) {
			for ( QValueListConstIterator<KURL> it = list.begin(); it != list.end() ; ++it ) {
				KURL servURL = "svn+http://this_is_a_fake_URL_and_this_is_normal/";
				QByteArray parms;
				QDataStream s( parms, IO_WriteOnly );
				int cmd = 1;
				int rev = -1;
				QString revkind = "HEAD";
				if ( d.revision->value() != 0 ) {
					rev = d.revision->value();
					revkind = "";
				}
				s<<cmd;
				s << KURL( d.url->url() );
				s << ( *it );
				s << rev;
				s << revkind;
				kdDebug(7128) << "checkouting : " << d.url->url() << " into " << (*it).prettyURL() << " at rev : " << rev << " or " << revkind << endl;
				KIO::SimpleJob * job = KIO::special(servURL, parms, true);
				connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );
				KIO::NetAccess::synchronousRun( job, 0 );
			}
		}
	}
	QTimer::singleShot( 0, this, SLOT( finished() ) );
}

void SvnHelper::slotResult( KIO::Job* job ) {
	if ( job->error() )
		job->showErrorDialog( );

	KIO::MetaData ma = job->metaData();
	QValueList<QString> keys = ma.keys();
	qHeapSort( keys );
	QValueList<QString>::Iterator begin = keys.begin(), end = keys.end(), it;

	QStringList message;
	for ( it = begin; it != end; ++it ) {
		kdDebug(7128) << "METADATA helper : " << *it << ":" << ma[ *it ] << endl;
		if ( ( *it ).endsWith( "string" ) ) {
			if ( ma[ *it ].length() > 2 ) {
				message << ma[ *it ];
			}
		}
	}
/*	KPassivePopup *pop = new KPassivePopup ( wm.activeWindow() );
	pop->setView( "Subversion", message );
	pop->setAutoDelete(true);
	pop->setTimeout( 10 );
	pop->show();*/
	KMessageBox::informationListWId(m_id, "", message, "Subversion");
}

void SvnHelper::finished() {
	kapp->quit();
}

static KCmdLineOptions options[] = {
	{ "u", I18N_NOOP("Update given URL"), 0 },
	{ "c", I18N_NOOP("Commit given URL"), 0 },
	{ "C", I18N_NOOP("Checkout in given directory"), 0 },
	{ "a", I18N_NOOP("Add given URL to the working copy"), 0 },
	{ "d", I18N_NOOP("Delete given URL from the working copy"), 0 },
	{ "s", I18N_NOOP("Switch given working copy to another branch"), 0 },
	{ "r", I18N_NOOP("Revert local changes"), 0 },
	{ "m", I18N_NOOP("Merge changes between two branches"), 0 },
	{"!+URL",   I18N_NOOP("URL to update/commit/add/delete from Subversion"), 0 },
	KCmdLineLastOption
};

int main(int argc, char **argv) {
	KCmdLineArgs::init(argc, argv, "kio_svn_helper", "kio_svn_helper", "kio_svn_helper", "0.1");

	KCmdLineArgs::addCmdLineOptions( options );
	KGlobal::locale()->setMainCatalogue("kio_svn");
	KApplication::addCmdLineOptions();

	if ( KCmdLineArgs::parsedArgs()->count()==0 )
		KCmdLineArgs::usage();
	KApplication *app = new SvnHelper();

//	app->dcopClient()->attach();
	app->exec();
}
