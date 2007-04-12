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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <kcmdlineargs.h>
#include <klocale.h>
#include <kapplication.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kglobal.h>
#include <qtimer.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kio/netaccess.h>
#include <qpixmap.h>
#include <kmessagebox.h>

#include "kio_svn_helper.h"
#include <kurlrequester.h>
#include <qspinbox.h>
#include <QProcess>
#include <ktemporaryfile.h>
#include <qtextstream.h>
#include <q3textedit.h>
#include <kstandarddirs.h>
#include <q3textbrowser.h>
#include <qtextcodec.h>

SubversionCheckout::SubversionCheckout(QWidget *parent )
: QDialog(parent)
{
   setupUi( this );
   connect(buttonOk, SIGNAL(clicked()), this, SLOT(accept()));
   connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
}

SubversionSwitch::SubversionSwitch(QWidget *parent )
: QDialog(parent)
{
   setupUi( this );
   connect(buttonOk, SIGNAL(clicked()), this, SLOT(accept()));
   connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
}

Subversion_Diff::Subversion_Diff(QWidget *parent )
: QDialog(parent)
{
   setupUi( this );
   connect(buttonOk, SIGNAL(clicked()), this, SLOT(accept()));
}



SvnHelper::SvnHelper():KApplication() {
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
#ifdef Q_WS_X11
	m_id=KWM::activeWindow();
	KWM::activateWindow(m_id);
#else
	m_id = 0;
#endif

	KUrl::List list;
	for ( int i = 0 ; i < args->count() ; i++ )
		list << args->url(i);

	if (args->isSet("u")) {
		kDebug(7128) << "update " << list << endl;
		const KUrl servURL("svn+http://this_is_a_fake_URL_and_this_is_normal/");
		//FIXME when 1.2 is out (move the loop inside kio_svn's ::update)
		for ( QList<KUrl>::const_iterator it = list.begin(); it != list.end() ; ++it ) {
			QByteArray parms;
			QDataStream s( &parms, QIODevice::WriteOnly );
			int cmd = 2;
			int rev = -1;
			kDebug(7128) << "updating : " << (*it).prettyUrl() << endl;
			s << cmd << *it << rev << QString( "HEAD" );
			KIO::SimpleJob * job = KIO::special(servURL, parms, true);
			connect( job, SIGNAL( result( KJob * ) ), this, SLOT( slotResult( KJob * ) ) );
			KIO::NetAccess::synchronousRun( job, 0 );
		}
	} else if (args->isSet("c")) {
		kDebug(7128) << "commit " << list << endl;
		const KUrl servURL("svn+http://this_is_a_fake_URL_and_this_is_normal/");
		QByteArray parms;
		QDataStream s( &parms, QIODevice::WriteOnly );
		int cmd = 3;
		s<<cmd;
		for ( QList<KUrl>::const_iterator it = list.begin(); it != list.end() ; ++it ) {
			kDebug(7128) << "commiting : " << (*it).prettyUrl() << endl;
			s << *it;
		}
		KIO::SimpleJob * job = KIO::special(servURL, parms, true);
		connect( job, SIGNAL( result( KJob * ) ), this, SLOT( slotResult( KJob * ) ) );
		KIO::NetAccess::synchronousRun( job, 0 );
	} else if (args->isSet("a")) {
		kDebug(7128) << "add " << list << endl;
		const KUrl servURL("svn+http://this_is_a_fake_URL_and_this_is_normal/");
		for ( QList<KUrl>::const_iterator it = list.begin(); it != list.end() ; ++it ) {
			QByteArray parms;
			QDataStream s( &parms, QIODevice::WriteOnly );
			int cmd = 6;
			kDebug(7128) << "adding : " << (*it).prettyUrl() << endl;
			s << cmd << *it;
			KIO::SimpleJob * job = KIO::special(servURL, parms, true);
			connect( job, SIGNAL( result( KJob * ) ), this, SLOT( slotResult( KJob * ) ) );
			KIO::NetAccess::synchronousRun( job, 0 );
		}
	} else if (args->isSet("D")) {
		kDebug(7128) << "diff " << list << endl;
		const KUrl servURL("svn+http://this_is_a_fake_URL_and_this_is_normal/");
		for ( QList<KUrl>::const_iterator it = list.begin(); it != list.end() ; ++it ) {
			QByteArray parms;
			QDataStream s( &parms, QIODevice::WriteOnly );
			int cmd = 13;
			kDebug(7128) << "diffing : " << (*it).prettyUrl() << endl;
			int rev1=-1;
			int rev2=-1;
			QString revkind1 = "BASE";
			QString revkind2 = "WORKING";
			s << cmd << *it << *it << rev1 << revkind1 << rev2 << revkind2 << true ;
			KIO::SimpleJob * job = KIO::special(servURL, parms, true);
			connect( job, SIGNAL( result( KJob * ) ), this, SLOT( slotResult( KJob * ) ) );
			KIO::NetAccess::synchronousRun( job, 0 );
			if ( diffresult.count() > 0 ) {
				//check kompare is available
				if ( !KStandardDirs::findExe( "kompare" ).isNull() ) {
					KTemporaryFile *tmp = new KTemporaryFile; //TODO: Found while porting: This is never deleted! Needs fixed.
					tmp->open();
					QTextStream stream ( tmp );
					stream.setCodec( QTextCodec::codecForName( "utf8" ) );
					for ( QStringList::Iterator it2 = diffresult.begin();it2 != diffresult.end() ; ++it2 ) {
						stream << ( *it2 ) << "\n";
					}
					stream.flush();
					QProcess *p = new QProcess;
					QStringList arguments;
					arguments << "-n" << "-o" << tmp->fileName();
					p->start("kompare", arguments);
				} else { //else do it with message box
					Subversion_Diff df;
					for ( QStringList::Iterator it2 = diffresult.begin();it2 != diffresult.end() ; ++it2 ) {
						df.text->append( *it2 );
					}
					QFont f = df.font();
					f.setFixedPitch( true );
					df.text->setFont( f );
					df.show();
				}
			}
			diffresult.clear();
		}
	} else if (args->isSet("d")) {
		kDebug(7128) << "delete " << list << endl;
		const KUrl servURL("svn+http://this_is_a_fake_URL_and_this_is_normal/");
		QByteArray parms;
		QDataStream s( &parms, QIODevice::WriteOnly );
		int cmd = 7;
		s<<cmd;
		for ( QList<KUrl>::const_iterator it = list.begin(); it != list.end() ; ++it ) {
			kDebug(7128) << "deleting : " << (*it).prettyUrl() << endl;
			s << *it;
		}
		KIO::SimpleJob * job = KIO::special(servURL, parms, true);
		connect( job, SIGNAL( result( KJob * ) ), this, SLOT( slotResult( KJob * ) ) );
		KIO::NetAccess::synchronousRun( job, 0 );
	} else if (args->isSet("s")) {
		kDebug(7128) << "switch " << list << endl;
		SubversionSwitch d;
		int result = d.exec();
		if ( result == QDialog::Accepted ) {
			for ( QList<KUrl>::const_iterator it = list.begin(); it != list.end() ; ++it ) {
				kDebug(7128) << "switching : " << (*it).prettyUrl() << endl;
				const KUrl servURL("svn+http://this_is_a_fake_URL_and_this_is_normal/");
				QByteArray parms;
				QDataStream s( &parms, QIODevice::WriteOnly );
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
				s << KUrl( d.url->url() );
				s << recurse;
				s << revnumber;
				s << revkind;
				KIO::SimpleJob * job = KIO::special(servURL, parms, true);
				connect( job, SIGNAL( result( KJob * ) ), this, SLOT( slotResult( KJob * ) ) );
				KIO::NetAccess::synchronousRun( job, 0 );
			}
		}
	} else if (args->isSet("r")) {
		kDebug(7128) << "revert " << list << endl;
		const KUrl servURL("svn+http://this_is_a_fake_URL_and_this_is_normal/");
		QByteArray parms;
		QDataStream s( &parms, QIODevice::WriteOnly );
		int cmd = 8;
		s<<cmd;
		for ( QList<KUrl>::const_iterator it = list.begin(); it != list.end() ; ++it ) {
			kDebug(7128) << "reverting : " << (*it).prettyUrl() << endl;
			s << *it;
		}
		KIO::SimpleJob * job = KIO::special(servURL, parms, true);
		connect( job, SIGNAL( result( KJob * ) ), this, SLOT( slotResult( KJob * ) ) );
		KIO::NetAccess::synchronousRun( job, 0 );
	} else if (args->isSet("C")) {
		kDebug(7128) << "checkout " << list << endl;
		SubversionCheckout d;
		int result = d.exec();
		if ( result == QDialog::Accepted ) {
			for ( QList<KUrl>::const_iterator it = list.begin(); it != list.end() ; ++it ) {
				const KUrl servURL("svn+http://this_is_a_fake_URL_and_this_is_normal/");
				QByteArray parms;
				QDataStream s( &parms, QIODevice::WriteOnly );
				int cmd = 1;
				int rev = -1;
				QString revkind = "HEAD";
				if ( d.revision->value() != 0 ) {
					rev = d.revision->value();
					revkind = "";
				}
				s<<cmd;
				s << KUrl( d.url->url() );
				s << ( *it );
				s << rev;
				s << revkind;
				kDebug(7128) << "checkouting : " << d.url->url() << " into " << (*it).prettyUrl() << " at rev : " << rev << " or " << revkind << endl;
				KIO::SimpleJob * job = KIO::special(servURL, parms, true);
				connect( job, SIGNAL( result( KJob * ) ), this, SLOT( slotResult( KJob * ) ) );
				KIO::NetAccess::synchronousRun( job, 0 );
			}
		}
	} else {
		KMessageBox::sorry(0, "Sorry, request not recognised.  Perhaps not implemented yet?", "Feature Not Implemented");
	}
	QTimer::singleShot( 0, this, SLOT( finished() ) );
}

void SvnHelper::slotResult( KJob* job ) {
	if ( job->error() )
		static_cast<KIO::Job*>( job )->showErrorDialog( );

	KIO::MetaData ma = static_cast<KIO::Job*>(job )->metaData();
	QList<QString> keys = ma.keys();
	qSort( keys );
	QList<QString>::Iterator begin = keys.begin(), end = keys.end(), it;

	QStringList message;
	for ( it = begin; it != end; ++it ) {
	//	kDebug(7128) << "METADATA helper : " << *it << ":" << ma[ *it ] << endl;
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
	KGlobal::locale()->setMainCatalog("kio_svn");
	KCmdLineArgs::addStdCmdLineOptions();

	if ( KCmdLineArgs::parsedArgs()->count()==0 )
		KCmdLineArgs::usage();
	KApplication *app = new SvnHelper();

	app->exec();
}

#include "kio_svn_helper.moc"
