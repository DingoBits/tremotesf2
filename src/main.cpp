/*
 * Tremotesf
 * Copyright (C) 2015-2018 Alexey Rochev <equeim@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <csignal>

#include <QCommandLineParser>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

#include "ipcserver.h"
#include "servers.h"
#include "utils.h"

#ifdef TREMOTESF_SAILFISHOS
#include <memory>
#include <QGuiApplication>
#include <QQmlContext>
#include <QQuickView>
#include <sailfishapp.h>
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
    app.setWindowIcon(QIcon::fromTheme(QLatin1String("org.equeim.Tremotesf")));
    app.setQuitOnLastWindowClosed(false);
#ifdef Q_OS_WIN
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    QIcon::setThemeSearchPaths({QLatin1String("icons")});
    QIcon::setThemeName(QLatin1String("breeze"));
#endif
#endif // TREMOTESF_SAILFISHOS

    qApp->setApplicationVersion(QLatin1String(TREMOTESF_VERSION));

    QCommandLineParser parser;
#ifdef TREMOTESF_SAILFISHOS
    parser.addPositionalArgument(QLatin1String("torrent"), QLatin1String("Torrent file or URL"));
#else
    parser.addPositionalArgument(QLatin1String("torrents"), QLatin1String("Torrent files or URLs"));
    parser.addOption(QCommandLineOption(QLatin1String("minimized"), QLatin1String("Start minimized in notification area")));
#endif
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(qApp->arguments());
    const QStringList arguments(parser.positionalArguments());

    signal(SIGINT, [](int) { qApp->quit(); });
    signal(SIGTERM, [](int) { qApp->quit(); });
#ifdef Q_OS_UNIX
    signal(SIGHUP, [](int) { qApp->quit(); });
    signal(SIGQUIT, [](int) { qApp->quit(); });
#endif

    if (tremotesf::IpcServer::tryToConnect()) {
        qWarning() << "Only one instance of Tremotesf can be run at the same time";
        if (arguments.isEmpty()) {
            tremotesf::IpcServer::ping();
        } else {
            tremotesf::IpcServer::sendArguments(arguments);
        }
        return 0;
    }

    tremotesf::IpcServer ipcServer;

    QTranslator qtTranslator;
    qtTranslator.load(QLocale(), QLatin1String("qt"), QLatin1String("_"), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    qApp->installTranslator(&qtTranslator);

    QTranslator appTranslator;
#if defined(Q_OS_WIN) && !defined(TEST_BUILD)
    appTranslator.load(QLocale().name(), QString::fromLatin1("%1/translations").arg(app.applicationDirPath()));
#else
    appTranslator.load(QLocale().name(), QLatin1String(TRANSLATIONS_PATH));
#endif
    qApp->installTranslator(&appTranslator);

    tremotesf::Utils::registerTypes();

    tremotesf::Servers::migrate();

#ifdef TREMOTESF_SAILFISHOS
    view->rootContext()->setContextProperty(QLatin1String("ipcServer"), &ipcServer);

    tremotesf::ArgumentsParseResult result(tremotesf::IpcServer::parseArguments(arguments));
    view->rootContext()->setContextProperty(QLatin1String("files"), result.files);
    view->rootContext()->setContextProperty(QLatin1String("urls"), result.urls);

    view->setSource(SailfishApp::pathTo(QLatin1String("qml/main.qml")));
    view->show();
#else
    tremotesf::MainWindow window(&ipcServer, arguments);
    window.showMinimized(parser.isSet(QLatin1String("minimized")));

#endif

    const int exitCode = qApp->exec();
#ifdef Q_OS_WIN
    CoUninitialize();
#endif
    return exitCode;
}
