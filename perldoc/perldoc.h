#ifndef KIO_PERLDOC_H_
#define KIO_PERLDOC_H_

/*
 * perldoc.h
 *
 * Borrowed from KDevelop's perldoc ioslave, and improved.
 * Copyright 2007 Michael Pyne <michael.pyne@kdemail.net>
 *
 * No copyright header was present in KDevelop's perldoc io slave source
 * code.  However, source code revision history indicates it was written and
 * imported by Bernd Gehrmann <bernd@mail.berlios.de>.  KDevelop is distributed
 * under the terms of the GNU General Public License v2.  Therefore, so is
 * this software.
 *
 * All changes made by Michael Pyne are licensed under the terms of the GNU
 * GPL version 2 or (at your option) any later version.
 *
 * Uses the Pod::HtmlEasy Perl module by M. P. Graciliano and
 * Geoffrey Leach.  It is distributed under the same terms as Perl.
 * See pod2html.pl for more information.
 */

// KF
#include <KIO/WorkerBase>

class PerldocProtocol : public KIO::WorkerBase
{
public:
    PerldocProtocol(const QByteArray &pool, const QByteArray &app);
    ~PerldocProtocol() override;

    KIO::WorkerResult get(const QUrl &url) override;
    KIO::WorkerResult stat(const QUrl &url) override;
    KIO::WorkerResult listDir(const QUrl &url) override;

    bool topicExists(const QString &topic);

protected:
    QByteArray errorMessage();
    KIO::WorkerResult failAndQuit();

    QString m_pod2htmlPath;
    QString m_cssLocation;
};

#endif

// vim: set et sw=4 ts=8:
