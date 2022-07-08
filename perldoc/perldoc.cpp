/*
 * perldoc.cpp
 *
 * Borrowed from KDevelop's perldoc ioslave, and improved.
 * Copyright 2007 Michael Pyne <michael.pyne@kdemail.net>
 * Copyright 2017 Luigi Toscano <luigi.toscano@tiscali.it>
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

#include "perldoc.h"

// KIO worker
#include "version.h"
// KF
#include <KAboutData>
#include <KLocalizedString>
// Qt
#include <QByteArray>
#include <QCoreApplication>
#include <QProcess>
#include <QStandardPaths>
#include <QStringList>
#include <QUrl>

class KIOPluginForMetaData : public QObject
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kio.worker.perldoc" FILE "perldoc.json")
};

PerldocProtocol::PerldocProtocol(const QByteArray &pool, const QByteArray &app)
    : KIO::WorkerBase("perldoc", pool, app)
{
    m_pod2htmlPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kio_perldoc/pod2html.pl"));
    m_cssLocation = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kio_docfilter/kio_docfilter.css"));
}

PerldocProtocol::~PerldocProtocol()
{
}

KIO::WorkerResult PerldocProtocol::get(const QUrl &url)
{
    const QStringList l = url.path().split(QLatin1Char('/'), Qt::SkipEmptyParts);

    // Check for perldoc://foo
    if(!url.host().isEmpty()) {
        QUrl newURL(url);

        newURL.setPath(url.host() + url.path());
        newURL.setHost(QString());

        redirection(newURL);
        return KIO::WorkerResult::pass();
    }

    mimeType(QStringLiteral("text/html"));

    if(l[0].isEmpty() || url.path() == QLatin1String("/")) {
        QByteArray output = i18n("<html><head><title>No page requested</title>"
            "<body>No page was requested.  You can search for:<ul><li>functions "
            "using perldoc:/functions/foo</li>\n\n"
            "<li>faq entries using perldoc:/faq/search_terms</li>"
            "<li>All other perldoc documents using the name of the document, like"
            "<ul><li><a href='perldoc:/perlreftut'>perldoc:/perlreftut</a></li>"
            "<li>or <a href='perldoc:/Net::HTTP'>perldoc:/Net::HTTP</a></li></ul>"
            "</li></ul>\n\n</body></html>\n"
        ).toLocal8Bit();

        data(output);
        return KIO::WorkerResult::pass();
    }

    if(l[0] != QLatin1String("functions") && l[0] != QLatin1String("faq")) {
        // See if it exists first.
        if(!topicExists(l[0])) {
            // Failed
            QByteArray errstr =
                i18n("<html><head><title>No documentation for %1</title><body>"
                "Unable to find documentation for <b>%2</b></body></html>\n",
                l[0], l[0]).toLocal8Bit();

            data(errstr);
            return KIO::WorkerResult::pass();
        }
    }

    QStringList pod2htmlArguments;
    if (l[0] == QLatin1String("functions")) {
        pod2htmlArguments = QStringList{QStringLiteral("-f"), l[1]};
    } else if (l[0] == QLatin1String("faq")) {
        pod2htmlArguments = QStringList{QStringLiteral("-q"), l[1]};
    } else if (!l[0].isEmpty()) {
        pod2htmlArguments = QStringList{l[0]};
    }

    QProcess pod2htmlProcess;

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("KIO_PERLDOC_VERSION"), QStringLiteral(KIO_PERLDOC_VERSION_STRING));
    env.insert(QStringLiteral("KIO_PERLDOC_CSSLOCATION"), m_cssLocation);
    pod2htmlProcess.setProcessEnvironment(env);

    pod2htmlProcess.start(m_pod2htmlPath, pod2htmlArguments);
    if (!pod2htmlProcess.waitForFinished()) {
        return failAndQuit();
    }

    if ((pod2htmlProcess.exitStatus() != QProcess::NormalExit) ||
        (pod2htmlProcess.exitCode() < 0)) {
        return KIO::WorkerResult::fail(KIO::ERR_CANNOT_LAUNCH_PROCESS, m_pod2htmlPath);
    }

    data(pod2htmlProcess.readAllStandardOutput());
    return KIO::WorkerResult::pass();
}

KIO::WorkerResult PerldocProtocol::failAndQuit()
{
    data(errorMessage());
    return KIO::WorkerResult::pass();
}

QByteArray PerldocProtocol::errorMessage()
{
    return QByteArray("<html><body bgcolor=\"#FFFFFF\">" +
           i18n("Error in perldoc").toLocal8Bit() +
           "</body></html>");
}

KIO::WorkerResult PerldocProtocol::stat(const QUrl &/*url*/)
{
    KIO::UDSEntry uds_entry;
    uds_entry.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG | S_IRWXU | S_IRWXG | S_IRWXO);

    statEntry(uds_entry);
    return KIO::WorkerResult::pass();
}

KIO::WorkerResult PerldocProtocol::listDir(const QUrl &url)
{
    return KIO::WorkerResult::fail( KIO::ERR_CANNOT_ENTER_DIRECTORY, url.path() );
}

bool PerldocProtocol::topicExists(const QString &topic)
{
    // Run perldoc in query mode to see if the given manpage exists.
    QProcess perldocProcess;
    perldocProcess.start(QStringLiteral("perldoc"), QStringList{QStringLiteral("-l"), topic});
    if (!perldocProcess.waitForFinished()) {
        return false;
    }

    if ((perldocProcess.exitStatus() != QProcess::NormalExit) ||
        (perldocProcess.exitCode() < 0)) {
        return false;
    }

    return true;
}

extern "C" {

    int Q_DECL_EXPORT kdemain(int argc, char **argv)
    {
        QCoreApplication app(argc, argv);

        KAboutData aboutData(
            QStringLiteral("kio_perldoc"),
            i18n("perldoc KIO worker"),
            QStringLiteral(KIO_PERLDOC_VERSION_STRING),
            i18n("KIO worker to provide access to perldoc documentation"),
            KAboutLicense::GPL_V2,
            i18n("Copyright 2007, 2008 Michael Pyne"),
            i18n("Uses Pod::HtmlEasy by M. P. Graciliano and Geoffrey Leach")
        );

        aboutData.addAuthor(i18n("Michael Pyne"), i18n("Maintainer, port to KDE 4"),
            QStringLiteral("michael.pyne@kdemail.net"), QStringLiteral("http://purinchu.net/wp/"));
        aboutData.addAuthor(i18n("Bernd Gehrmann"), i18n("Initial implementation"));
        aboutData.addCredit(i18n("M. P. Graciliano"), i18n("Pod::HtmlEasy"));
        aboutData.addCredit(i18n("Geoffrey Leach"), i18n("Pod::HtmlEasy current maintainer"));
        aboutData.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"),
            i18nc("EMAIL OF TRANSLATORS", "Your emails"));

        app.setOrganizationDomain(QStringLiteral("kde.org"));
        app.setOrganizationName(QStringLiteral("KDE"));

        KAboutData::setApplicationData(aboutData);

        if (argc != 4) {
            fprintf(stderr, "Usage: kio_perldoc protocol domain-socket1 domain-socket2\n");
            exit(5);
        }

        PerldocProtocol worker(argv[2], argv[3]);
        worker.dispatchLoop();

        return 0;
    }
}

#include "perldoc.moc"

// vim: set et sw=4 ts=8:
