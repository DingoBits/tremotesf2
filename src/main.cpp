#include <csignal>

#include <QCommandLineParser>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

#include "ipcserver.h"
#include "utils.h"

#ifdef TREMOTESF_SAILFISHOS
#include <memory>
#include <QGuiApplication>
#include <QQmlContext>
#include <QQuickView>
#include <sailfishapp.h>
#include "accounts.h"
#else
#include <QApplication>
#include <QIcon>
#include "desktop/mainwindow.h"
#endif

#ifdef Q_OS_WIN
#include <windows.h>
#endif

int main(int argc, char** argv)
{
#ifdef Q_OS_WIN
    AllowSetForegroundWindow(ASFW_ANY);
#endif

#ifdef TREMOTESF_SAILFISHOS
    std::unique_ptr<QGuiApplication> app(SailfishApp::application(argc, argv));
    std::unique_ptr<QQuickView> view(SailfishApp::createView());
#else
    QApplication app(argc, argv);
    app.setOrganizationName(app.applicationName());
    app.setWindowIcon(QIcon::fromTheme("tremotesf"));
    app.setQuitOnLastWindowClosed(false);
#endif
    qApp->setApplicationVersion("1.1.0");

    QCommandLineParser parser;
#ifdef TREMOTESF_SAILFIDHOS
    parser.addPositionalArgument("torrent", "Torrent file or URL");
#else
    parser.addPositionalArgument("torrents", "Torrent files or URLs");
#endif
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(qApp->arguments());
    const QStringList arguments(parser.positionalArguments());

    signal(SIGINT, [](int) { qApp->quit(); });
    signal(SIGTERM, [](int) { qApp->quit(); });

    if (tremotesf::IpcServer::tryToConnect()) {
        qDebug() << "Only one instance of Tremotesf can be run at the same time";
        if (arguments.isEmpty()) {
            tremotesf::IpcServer::ping();
        } else {
            tremotesf::IpcServer::sendArguments(arguments);
        }
        return 0;
    }

    tremotesf::IpcServer ipcServer;

    QTranslator qtTranslator;
    qtTranslator.load(QLocale(), "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    qApp->installTranslator(&qtTranslator);

    QTranslator appTranslator;
#ifdef Q_OS_WIN
    appTranslator.load(QLocale().name(), QString("%1/translations").arg(app.applicationDirPath()));
#else
    appTranslator.load(QLocale().name(), TRANSLATIONS_PATH);
#endif
    qApp->installTranslator(&appTranslator);

#ifdef TREMOTESF_SAILFISHOS
    tremotesf::Accounts::migrate();

    tremotesf::Utils::registerTypes();

    view->rootContext()->setContextProperty("ipcServer", &ipcServer);

    tremotesf::ArgumentsParseResult result(tremotesf::IpcServer::parseArguments(arguments));
    view->rootContext()->setContextProperty("files", result.files);
    view->rootContext()->setContextProperty("urls", result.urls);

    view->setSource(SailfishApp::pathTo("qml/main.qml"));
    view->show();
#else
    tremotesf::MainWindow window(&ipcServer, arguments);
    window.showIfNeeded();
#endif

    return qApp->exec();
}
