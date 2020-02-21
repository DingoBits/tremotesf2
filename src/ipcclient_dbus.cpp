/*
 * Tremotesf
 * Copyright (C) 2015-2019 Alexey Rochev <equeim@gmail.com>
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

#include "ipcclient.h"

#include <QDBusInterface>
#include <QDebug>

#include "ipcserver.h"

namespace tremotesf
{
    class IpcClient::Private
    {
    public:
        QDBusInterface interface;
    };

    IpcClient::IpcClient() : d(new Private{{IpcServer::serviceName(),
                                            IpcServer::objectPath(),
                                            IpcServer::interfaceName()}})
    {

    }

    IpcClient::~IpcClient() = default;

    bool IpcClient::isConnected() const
    {
        return d->interface.isValid();
    }

    void IpcClient::activateWindow()
    {
        qInfo("Requesting window activation");
        const QDBusMessage reply(d->interface.call(QLatin1String("ActivateWindow")));
        if (reply.type() != QDBusMessage::ReplyMessage) {
            qWarning() << "D-Bus method call failed, error string:" << reply.errorMessage();
        }
    }

    void IpcClient::sendArguments(const QStringList& files, const QStringList& urls)
    {
        qInfo("Sending arguments");
        const QDBusMessage reply(d->interface.call(QLatin1String("SetArguments"), files, urls));
        if (reply.type() != QDBusMessage::ReplyMessage) {
            qWarning() << "D-Bus method call failed, error string:" << reply.errorMessage();
        }
    }
}
