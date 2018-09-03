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

#include "ipcserver.h"

#include <QDBusAbstractAdaptor>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusMessage>
#include <QDebug>

namespace tremotesf
{
    namespace
    {
        class IpcServerDBusAdaptor : public QDBusAbstractAdaptor
        {
            Q_OBJECT
            Q_CLASSINFO("D-Bus Interface", "org.equeim.Tremotesf")
        public:
            IpcServerDBusAdaptor(IpcServer* ipcServer)
                : QDBusAbstractAdaptor(ipcServer),
                  mIpcServer(ipcServer)
            {

            }
        public slots:
            void ActivateWindow()
            {
                qDebug() << "Window activation requested";
                emit mIpcServer->windowActivationRequested();
            }

            void SetArguments(const QStringList& files, const QStringList& urls)
            {
                qDebug() << "Received arguments" << files << urls;
                if (!files.isEmpty()) {
                    emit mIpcServer->filesReceived(files);
                }
                if (!urls.isEmpty()) {
                    emit mIpcServer->urlsReceived(urls);
                }
            }

#ifdef TREMOTESF_SAILFISHOS
            void OpenTorrentPropertiesPage(const QString& hashString)
            {
                qDebug() << "TorrentPropertiesPage requested";
                emit mIpcServer->torrentPropertiesPageRequested(hashString);
            }
#endif

        private:
            IpcServer* mIpcServer;
        };
    }

    const QLatin1String IpcServer::serviceName("org.equeim.Tremotesf");
    const QLatin1String IpcServer::objectPath("/org/equeim/Tremotesf");
    const QLatin1String IpcServer::interfaceName("org.equeim.Tremotesf");

    IpcServer::IpcServer(QObject* parent)
        : QObject(parent)
    {
        if (QDBusConnection::sessionBus().registerService(serviceName)) {
            qDebug() << "Registered D-Bus service";
            if (QDBusConnection::sessionBus().registerObject(objectPath, interfaceName, new IpcServerDBusAdaptor(this), QDBusConnection::ExportAllSlots)) {
                qDebug() << "Registered D-Bus object";
            } else {
                qWarning() << "Failed to register D-Bus object";
            }
        } else {
            qWarning() << "Failed to register D-Bus service";
        }
    }

    bool IpcServer::tryToConnect()
    {
        const QDBusMessage reply(QDBusConnection::sessionBus().call(QDBusMessage::createMethodCall(serviceName,
                                                                                                   objectPath,
                                                                                                   QLatin1String("org.freedesktop.DBus.Peer"),
                                                                                                   QLatin1String("Ping"))));
        return reply.type() == QDBusMessage::ReplyMessage;
    }

    void IpcServer::activateWindow()
    {
        qInfo() << "Requesting window activation";
        const QDBusMessage reply(QDBusConnection::sessionBus().call(QDBusMessage::createMethodCall(serviceName,
                                                                                                   objectPath,
                                                                                                   interfaceName,
                                                                                                   QLatin1String("ActivateWindow"))));
        if (reply.type() != QDBusMessage::ReplyMessage) {
            qWarning() << "D-Bus method call failed, error string" << reply.errorMessage();
        }
    }

    void IpcServer::sendArguments(const QStringList& arguments)
    {
        qInfo() << "Sending arguments";
        QDBusMessage message(QDBusMessage::createMethodCall(serviceName,
                                                            objectPath,
                                                            interfaceName,
                                                            QLatin1String("SetArguments")));
        const ArgumentsParseResult result(parseArguments(arguments));
        message.setArguments({result.files, result.urls});
        const QDBusMessage reply(QDBusConnection::sessionBus().call(message));
        if (reply.type() != QDBusMessage::ReplyMessage) {
            qWarning() << "D-Bus method call failed, error string" << reply.errorMessage();
        }
    }
}

#include "ipcserver_dbus.moc"
