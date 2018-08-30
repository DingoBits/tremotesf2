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

#ifndef TREMOTESF_IPCSERVER_H
#define TREMOTESF_IPCSERVER_H

#include <QObject>

namespace tremotesf
{
    struct ArgumentsParseResult
    {
        QStringList files;
        QStringList urls;
    };

    class IpcServer : public QObject
    {
        Q_OBJECT
    public:
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
        static const QLatin1String serviceName;
        static const QLatin1String objectPath;
        static const QLatin1String interfaceName;
#endif

        explicit IpcServer(QObject* parent = nullptr);

        static bool tryToConnect();
        static void activateWindow();
        static void sendArguments(const QStringList& arguments);

        static void parseArgument(const QString& argument, ArgumentsParseResult& result);
        static ArgumentsParseResult parseArguments(const QStringList& arguments);
    signals:
        void windowActivationRequested();
        void filesReceived(const QStringList& files);
        void urlsReceived(const QStringList& urls);
#ifdef TREMOTESF_SAILFISHOS
        void torrentPropertiesPageRequested(const QString& hashString);
#endif
    };
}

#endif // TREMOTESF_IPCSERVER_H
