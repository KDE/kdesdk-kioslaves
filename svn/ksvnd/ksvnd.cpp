/*
    This file is part of the KDE Project

    Copyright (C) 2003 Mickael Marchand <marchand@kde.org>

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

#include "config.h"

#include "ksvnd.h"
#include "commitdialog.h"

extern "C" {
    KDEDModule *create_ksvnd(const QCString &name) {
       return new KSvnd(name);
    }
}

KSvnd::KSvnd(const QCString &name)
 : KDEDModule(name) {
}

KSvnd::~KSvnd() {
}

QString KSvnd::commitDialog(QString comment) {
	CommitDialog commitDlg;
	commitDlg.setLog( comment );
	int result = commitDlg.exec();
	if ( result == QDialog::Accepted ) {
		return commitDlg.logMessage();
	} else
		return QString::null;
}

#include "ksvnd.moc"
