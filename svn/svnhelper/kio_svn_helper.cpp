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
#include <qpixmap.h>
#include <kmessagebox.h>

#include "kio_svn_helper.h"
#include "subversioncheckout.h"
#include "subversionswitch.h"
#include "subversiondiff.h"
#include <kurlrequester.h>
#include <qspinbox.h>
#include <kprocess.h>
#include <ktempfile.h>
#include <qtextstream.h>
#include <qtextedit.h>
#include <kstandarddirs.h>
#include <qtextbrowser.h>
#include <qtextcodec.h>

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
	} else if (args->isSet("D")) {
		kdDebug(7128) << "diff " << list << endl;
		KURL servURL = "svn+http://this_is_a_fake_URL_and_this_is_normal/";
		for ( QValueListConstIterator<KURL> it = list.begin(); it != list.end() ; ++it ) {
			QByteArray parms;
			QDataStream s( parms, IO_WriteOnly );
			int cmd = 13;
			kdDebug(7128) << "diffing : " << (*it).prettyURL() << endl;
			int rev1=-1;
			int rev2=-1;
			QString revkind1 = "BASE";
			QString revkind2 = "WORKING";
			s << cmd << *it << *it << rev1 << revkind1 << rev2 << revkind2 << true ;
			KIO::SimpleJob * job = KIO::special(servURL, parms, true);
			connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );
			KIO::NetAccess::synchronousRun( job, 0 );
			if ( diffresult.count() > 0 ) {
				//check kompare is available
				if ( !KStandardDirs::findExe( "kompare" ).isNull() ) {
					KTempFile *tmp = new KTempFile;
					tmp->setAutoDelete(true);
					QTextStream *stream = tmp->textStream();
					stream->setCodec( QTextCodec::codecForName( "utf8" ) );
					for ( QStringList::Iterator it2 = diffresult.begin();it2 != diffresult.end() ; ++it2 ) {
						( *stream ) << ( *it2 ) << "\n";
					}
					tmp->close();
					KProcess *p = new KProcess;
					*p << "kompare" << "-n" << "-o" << tmp->name();
					p->start();
				} else { //else do it with message box
					Subversion_Diff df;
					for ( QStringList::Iterator it2 = diffresult.begin();it2 != diffresult.end() ; ++it2 ) {
						df.text->append( *it2 );
					}
					QFont f = df.font();
					f.setFixedPitch( true );
					df.text->setFont( f );
					df.exec();
				}
			}
			diffresult.clear();
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
	} else if (args->isSet("s")) {
		kdDebug(7128) << "switch " << list << endl;
		SubversionSwitch d;
		int result = d.exec();
		if ( result == QDialog::Accepted ) {
			for ( QValueListConstIterator<KURL> it = list.begin(); it != list.end() ; ++it ) {
				kdDebug(7128) << "switching : " << (*it).prettyURL() << endl;
				KURL servURL = "svn+http://this_is_a_fake_URL_and_this_is_normal/";
				QByteArray parms;
				QDataStream s( parms, IO_WriteOnly );
				int revnumber = -1;
				QString revkind = "HEAD";
				if ( d.revision->value() != 0 ) {
					revnumber = d.revision->value();
					revkind = "";
				}
				bool recurse=true;
				int cmd = 12;
				s << cmd;
				s << *it;
				s << KURL( d.url->url() );
				s << recurse;
				s << revnumber;
				s << revkind;
				KIO::SimpleJob * job = KIO::special(servURL, parms, true);
				connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );
				KIO::NetAccess::synchronousRun( job, 0 );
			}
		}
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
	} else {
		KMessageBox::sorry(0, "Sorry, request not recognised.  Perhaps not implemented yet?", "Feature Not Implemented");
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
	//	kdDebug(7128) << "METADATA helper : " << *it << ":" << ma[ *it ] << endl;
		if ( ( *it ).endsWith( "string" ) ) {
			if ( ma[ *it ].length() > 2 ) {
				message << ma[ *it ];
			}
		}
		//extra check to retrieve the diff output in case with run a diff command
		if ( ( *it ).endsWith( "diffresult" ) ) {
				diffresult << ma[ *it ];
		}
	}
	if ( message.count() > 0 )
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
	{ "D", I18N_NOOP("Show locally made changements with diff"), 0 },
	{"!+URL",   I18N_NOOP("URL to update/commit/add/delete from Subversion"), 0 },
	KCmdLineLastOption
};

int main(int argc, char **argv) {
	KCmdLineArgs::init(argc, argv, "kio_svn_helper", I18N_NOOP("Subversion Helper"), "KDE frontend for SVN", "0.1");

	KCmdLineArgs::addCmdLineOptions( options );
	KGlobal::locale()->setMainCatalogue("kio_svn");
	KApplication::addCmdLineOptions();

	if ( KCmdLineArgs::parsedArgs()->count()==0 )
		KCmdLineArgs::usage();
	KApplication *app = new SvnHelper();

//	app->dcopClient()->attach();
	app->exec();
}

#include "kio_svn_helper.moc"
