/*
    This file is part of the KDE Project

    Copyright (C) 2003, 2004 Mickael Marchand <marchand@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    version 2 as published by the Free Software Foundation.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library; see the file COPYING. If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kapplication.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <qdir.h>
#include <qfile.h>

#include "config.h"

#include "ksvnd.h"
#include "commitdlg.h"

extern "C" {
    KDE_EXPORT KDEDModule *create_ksvnd(const QCString &name) {
       return new KSvnd(name);
    }
}

KSvnd::KSvnd(const QCString &name)
 : KDEDModule(name) {
}

KSvnd::~KSvnd() {
}

QString KSvnd::commitDialog(QString modifiedFiles) {
	CommitDlg commitDlg;
	commitDlg.setLog( modifiedFiles );
	int result = commitDlg.exec();
	if ( result == QDialog::Accepted ) {
		return commitDlg.logMessage();
	} else
		return QString::null;
}

<<<<<<< .mine
bool KSvnd::AreAnyFilesInSvn( const KURL::List& wclist ) {
	for ( QValueListConstIterator<KURL> it = wclist.begin(); it != wclist.end() ; ++it ) {
		if ( isFileInSvnEntries( ( *it ).fileName(), ( *it ).directory() + "/.svn/entries" ) )
			return true;
	}
	return false;
}

bool KSvnd::AreAnyFilesNotInSvn( const KURL::List& wclist ) {
	for ( QValueListConstIterator<KURL> it = wclist.begin(); it != wclist.end() ; ++it ) {
		if ( !isFileInSvnEntries( ( *it ).fileName(), ( *it ).directory() + "/.svn/entries" ) )
			return true;
	}
	return false;
}

bool KSvnd::AreAllFilesInSvn( const KURL::List& wclist ) {
	for ( QValueListConstIterator<KURL> it = wclist.begin(); it != wclist.end() ; ++it ) {
		if ( !isFileInSvnEntries( ( *it ).fileName(), ( *it ).directory() + "/.svn/entries" ) )
			return false;
	}
	return true;
}

bool KSvnd::AreAllFilesNotInSvn( const KURL::List& wclist ) {
	for ( QValueListConstIterator<KURL> it = wclist.begin(); it != wclist.end() ; ++it ) {
		if ( isFileInSvnEntries( ( *it ).fileName(), ( *it ).directory() + "/.svn/entries" ) )
			return false;
	}
	return true;
}

bool KSvnd::isFileInSvnEntries ( const QString filename, const QString entfile ) {
	QFile file( entfile );
	if ( file.open( IO_ReadOnly ) ) {
		QTextStream stream( &file );
		QString line;
		while ( !stream.atEnd() ) {
			line = stream.readLine().simplifyWhiteSpace();
			if ( line == "name=\""+ filename + "\"" )
				return true;
		}
		file.close();
	}
	return false;
}

bool KSvnd::anyNotValidWorkingCopy( const KURL::List& wclist ) {
	bool result = true; //one negative match is enough
	for ( QValueListConstIterator<KURL> it = wclist.begin(); it != wclist.end() ; ++it ) {
		//if is a directory check whether it contains a .svn/entries file
		QDir dir( ( *it ).path() );
		if ( dir.exists() ) { //it's a dir
			if ( !QFile::exists( ( *it ).path() + "/.svn/entries" ) )
				result = false;
		}

		//else check if ./.svn/entries exists
		if ( !QFile::exists( ( *it ).directory() + "/.svn/entries" ) )
			result = false;
	}
	return result;
}

bool KSvnd::anyValidWorkingCopy( const KURL::List& wclist ) {
	for ( QValueListConstIterator<KURL> it = wclist.begin(); it != wclist.end() ; ++it ) {
		//if is a directory check whether it contains a .svn/entries file
		QDir dir( ( *it ).path() );
		if ( dir.exists() ) { //it's a dir
			if ( QFile::exists( ( *it ).path() + "/.svn/entries" ) )
				return true;
		}

		//else check if ./.svn/entries exists
		if ( QFile::exists( ( *it ).directory() + "/.svn/entries" ) )
			return true;
	}
	return false;
}

#if 0
void KSvnd::notify(const QString& path, int action, int kind, const QString& mime_type, int content_state, int prop_state, long int revision, const QString& userstring) {
	kdDebug(7128) << "KDED/Subversion : notify " << path << " action : " << action << " mime_type : " << mime_type << " content_state : " << content_state << " prop_state : " << prop_state << " revision : " << revision << " userstring : " << userstring << endl; 
	QByteArray params;

	QDataStream stream(params, IO_WriteOnly);
	stream << path << action << kind << mime_type << content_state << prop_state << revision << userstring;

	emitDCOPSignal( "subversionNotify(QString,int,int,QString,int,int,long int,QString)", params );
}

void KSvnd::status(const QString& path, int text_status, int prop_status, int repos_text_status, int repos_prop_status, long int rev ) {
	kdDebug(7128) << "KDED/Subversion : status " << path << " " << text_status << " " << prop_status << " "
			<< repos_text_status << " " << repos_prop_status << " " << rev << endl;
	QByteArray params;

	QDataStream stream(params, IO_WriteOnly);
	stream << path << text_status << prop_status << repos_text_status << repos_prop_status << rev;

	emitDCOPSignal( "subversionStatus(QString,int,int,int,int,long int)", params );
}

void KSvnd::popupMessage( const QString& message ) {
	kdDebug(7128) << "KDED/Subversion : popupMessage" << message << endl;
	KMessageBox::information(0, message, i18n( "Subversion" ) );
}
#endif

#include "ksvnd.moc"
