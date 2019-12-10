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
#include <iostream>

#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

#ifdef TREMOTESF_SAILFISHOS
#include <memory>
#include <QGuiApplication>
#include <QQmlContext>
#include <QQuickView>
#include <sailfishapp.h>
#else
#include <QApplication>
#include <QIcon>
#ifdef Q_OS_WIN
#include <windows.h>
#endif // Q_OS_WIN
#endif // TREMOTESF_SAILFISHOS

#include "3rdparty/cxxopts.hpp"

#include "ipcserver.h"
#include "servers.h"
#include "signalhandler.h"
#include "utils.h"

#ifndef TREMOTESF_SAILFISHOS
#include "desktop/mainwindow.h"
#endif

namespace {
    inline std::string parseAppName(const char* arg)
    {
        const char* sep = strrchr(arg, '/');
        return std::string(sep ? sep + 1 : arg);
    }

    inline QStringList toStringList(const std::vector<std::string>& strings)
    {
        QStringList list;
        list.reserve(strings.size());
        for (const std::string& str : strings) {
            list.push_back(QString::fromStdString(str));
        }
        return list;
    }
}

int main(int argc, char** argv)
{
    // Setup handler for UNIX signals or Windows console handler
    tremotesf::SignalHandler::setupHandlers();

    //
    // Command line parsing
    //
#ifndef TREMOTESF_SAILFISHOS
    bool minimizedFlag = false;
#endif
    QStringList torrents;
    {
        const std::string appName(parseAppName(argv[0]));
        const std::string versionString(appName + " " TREMOTESF_VERSION);
        cxxopts::Options opts(appName, versionString);
        opts.add_options()
            ("v,version", "display version information", cxxopts::value<bool>())
            ("h,help", "display this help", cxxopts::value<bool>())
#ifdef TREMOTESF_SAILFISHOS
            ("torrent", "", cxxopts::value<std::string>()->default_value(""));
            opts.parse_positional("torrent");
            opts.positional_help("torrent");
#else
            ("m,minimized", "start minimized in notification area", cxxopts::value<bool>(minimizedFlag))
            ("torrents", "", cxxopts::value<std::vector<std::string>>()->default_value(""));
            opts.parse_positional("torrents");
            opts.positional_help("torrents");
#endif
        try {
            const auto result(opts.parse(argc, argv));
            if (result["help"].as<bool>()) {
                std::cout << opts.help() << std::endl;
                return 0;
            }
            if (result["version"].as<bool>()) {
                std::cout << versionString << std::endl;
                return 0;
            }
#ifdef TREMOTESF_SAILFISHOS
            const std::string torrent(result["torrent"].as<std::string>());
            if (!torrent.empty()) {
                torrents.push_back(QString::fromStdString(torrent));
            }
#else
            torrents = toStringList(result["torrents"].as<std::vector<std::string>>());
#endif
        } catch (const cxxopts::OptionException& e) {
            std::cerr << e.what() << std::endl;
            return 1;
        }
    }
    //
    // End of command line parsing
    //

    //
    // Q(Gui)Application initialization
    //
#ifdef Q_OS_WIN
    AllowSetForegroundWindow(ASFW_ANY);
#endif

#ifdef TREMOTESF_SAILFISHOS
    std::unique_ptr<QGuiApplication> app(SailfishApp::application(argc, argv));
    std::unique_ptr<QQuickView> view(SailfishApp::createView());
#else
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication app(argc, argv);
#endif
    QCoreApplication::setApplicationVersion(QLatin1String(TREMOTESF_VERSION));
    //
    // End of QApplication initialization
    //

    // Setup socket notifier for UNIX signals
    tremotesf::SignalHandler::setupNotifier();

    // Send command to another instance
    if (tremotesf::IpcServer::tryToConnect()) {
        qWarning() << "Only one instance of Tremotesf can be run at the same time";
        if (torrents.isEmpty()) {
            tremotesf::IpcServer::activateWindow();
        } else {
            tremotesf::IpcServer::sendArguments(torrents);
        }
        return 0;
    }

#ifndef TREMOTESF_SAILFISHOS
    qApp->setOrganizationName(qApp->applicationName());
    qApp->setWindowIcon(QIcon::fromTheme(QLatin1String("org.equeim.Tremotesf")));
    qApp->setQuitOnLastWindowClosed(false);
#ifdef Q_OS_WIN
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    QIcon::setThemeSearchPaths({QLatin1String("icons")});
    QIcon::setThemeName(QLatin1String("breeze"));
#endif
#endif

    tremotesf::IpcServer ipcServer;

    QTranslator qtTranslator;
    qtTranslator.load(QLocale(), QLatin1String("qt"), QLatin1String("_"), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    qApp->installTranslator(&qtTranslator);

    QTranslator appTranslator;
#if defined(Q_OS_WIN) && !defined(TEST_BUILD)
    appTranslator.load(QLocale().name(), QString::fromLatin1("%1/translations").arg(qApp->applicationDirPath()));
#else
    appTranslator.load(QLocale().name(), QLatin1String(TRANSLATIONS_PATH));
#endif
    qApp->installTranslator(&appTranslator);

    tremotesf::Utils::registerTypes();

    tremotesf::Servers::migrate();

    if (tremotesf::SignalHandler::exitRequested) {
        return 0;
    }

#ifdef TREMOTESF_SAILFISHOS
    view->rootContext()->setContextProperty(QLatin1String("ipcServer"), &ipcServer);
    view->rootContext()->setContextProperty(QLatin1String("ipcServerServiceName"), tremotesf::IpcServer::serviceName);
    view->rootContext()->setContextProperty(QLatin1String("ipcServerObjectPath"), tremotesf::IpcServer::objectPath);
    view->rootContext()->setContextProperty(QLatin1String("ipcServerInterfaceName"), tremotesf::IpcServer::interfaceName);

    tremotesf::ArgumentsParseResult result(tremotesf::IpcServer::parseArguments(torrents));
    view->rootContext()->setContextProperty(QLatin1String("files"), result.files);
    view->rootContext()->setContextProperty(QLatin1String("urls"), result.urls);

    view->setSource(SailfishApp::pathTo(QLatin1String("qml/main.qml")));
    if (tremotesf::SignalHandler::exitRequested) {
        return 0;
    }
    view->show();
#else
    tremotesf::MainWindow window(&ipcServer, torrents);
    if (tremotesf::SignalHandler::exitRequested) {
        return 0;
    }
    window.showMinimized(minimizedFlag);
#endif

    if (tremotesf::SignalHandler::exitRequested) {
        return 0;
    }

    const int exitCode = qApp->exec();
#ifdef Q_OS_WIN
    CoUninitialize();
#endif
    return exitCode;
}
