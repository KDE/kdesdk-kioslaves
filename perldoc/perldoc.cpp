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

#include <QByteArray>
#include <QCoreApplication>
#include <QProcess>
#include <QStandardPaths>
#include <QStringList>
#include <QUrl>

#include <KAboutData>
#include <klocalizedstring.h>

class KIOPluginForMetaData : public QObject
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kio.slave.perldoc" FILE "perldoc.json")
};

// Embed version info.  Using const char[] instead of const char* const
// places it in a read-only section.
static const char
#ifdef __GNUC__ /* force this into final object files */
__attribute__((__used__))
#endif
kio_perldoc_version[] = "0.10.0";

PerldocProtocol::PerldocProtocol(const QByteArray &pool, const QByteArray &app)
    : KIO::SlaveBase("perldoc", pool, app)
{
    m_pod2htmlPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kio_perldoc/pod2html.pl");
    m_cssLocation = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kio_docfilter/kio_docfilter.css" );
}

PerldocProtocol::~PerldocProtocol()
{
}

void PerldocProtocol::get(const QUrl &url)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QStringList l = url.path().split('/', Qt::SkipEmptyParts);
#else
    QStringList l = url.path().split('/', QString::SkipEmptyParts);
#endif

    // Check for perldoc://foo
    if(!url.host().isEmpty()) {
        QUrl newURL(url);

        newURL.setPath(url.host() + url.path());
        newURL.setHost(QString());

        redirection(newURL);
        finished();
        return;
    }

    mimeType("text/html");

    if(l[0].isEmpty() || url.path() == "/") {
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
        finished();
        return;
    }

    if(l[0] != "functions" && l[0] != "faq") {
        // See if it exists first.
        if(!topicExists(l[0])) {
            // Failed
            QByteArray errstr =
                i18n("<html><head><title>No documentation for %1</title><body>"
                "Unable to find documentation for <b>%2</b></body></html>\n",
                l[0], l[0]).toLocal8Bit();

            data(errstr);
            finished();
            return;
        }
    }

    QStringList pod2htmlArguments;
    if (l[0] == "functions") {
        pod2htmlArguments << "-f" << l[1];
    } else if (l[0] == "faq") {
        pod2htmlArguments << "-q" << l[1];
    } else if (!l[0].isEmpty()) {
        pod2htmlArguments << l[0];
    }

    QProcess pod2htmlProcess;

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("KIO_PERLDOC_VERSION", kio_perldoc_version);
    env.insert("KIO_PERLDOC_CSSLOCATION", m_cssLocation);
    pod2htmlProcess.setProcessEnvironment(env);

    pod2htmlProcess.start(m_pod2htmlPath, pod2htmlArguments);
    if (!pod2htmlProcess.waitForFinished()) {
        failAndQuit();
        return;
    }

    if ((pod2htmlProcess.exitStatus() != QProcess::NormalExit) ||
        (pod2htmlProcess.exitCode() < 0)) {
        error(KIO::ERR_CANNOT_LAUNCH_PROCESS, m_pod2htmlPath);
    }

    data(pod2htmlProcess.readAllStandardOutput());
    finished();
}

void PerldocProtocol::failAndQuit()
{
    data(errorMessage());
    finished();
}

QByteArray PerldocProtocol::errorMessage()
{
    return QByteArray("<html><body bgcolor=\"#FFFFFF\">" +
           i18n("Error in perldoc").toLocal8Bit() +
           "</body></html>");
}

void PerldocProtocol::stat(const QUrl &/*url*/)
{
    KIO::UDSEntry uds_entry;
    uds_entry.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG | S_IRWXU | S_IRWXG | S_IRWXO);

    statEntry(uds_entry);
    finished();
}

void PerldocProtocol::listDir(const QUrl &url)
{
    error( KIO::ERR_CANNOT_ENTER_DIRECTORY, url.path() );
}

bool PerldocProtocol::topicExists(const QString &topic)
{
    // Run perldoc in query mode to see if the given manpage exists.
    QProcess perldocProcess;
    perldocProcess.start(QStringLiteral("perldoc"), QStringList() << "-l" << topic);
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
            i18n("perldoc KIOSlave"),
            kio_perldoc_version,
            i18n("KIOSlave to provide access to perldoc documentation"),
            KAboutLicense::GPL_V2,
            i18n("Copyright 2007, 2008 Michael Pyne"),
            i18n("Uses Pod::HtmlEasy by M. P. Graciliano and Geoffrey Leach")
        );

        aboutData.addAuthor(i18n("Michael Pyne"), i18n("Maintainer, port to KDE 4"),
            "michael.pyne@kdemail.net", "http://purinchu.net/wp/");
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

        PerldocProtocol slave(argv[2], argv[3]);
        slave.dispatchLoop();

        return 0;
    }
}

#include "perldoc.moc"

// vim: set et sw=4 ts=8:
