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

#ifndef KSVND_H
#define KSVND_H

#include <dcopclient.h>
#include <kdedmodule.h>

class KSvnd : public KDEDModule
{
  Q_OBJECT
  K_DCOP
public:
  KSvnd(const QCString &);
  ~KSvnd();

k_dcop:
//  void addAuthInfo(KIO::AuthInfo, long);
  const QString commitDialog(const QString& comment);

public slots:

protected:
};

#endif
