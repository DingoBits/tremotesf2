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

#include "peersmodel.h"

#include <QCoreApplication>

#include "modelutils.h"
#include "utils.h"
#include "libtremotesf/stdutils.h"
#include "libtremotesf/torrent.h"
#include "libtremotesf/torrent_qdebug.h"

namespace tremotesf
{
#ifndef TREMOTESF_SAILFISHOS
    const int PeersModel::SortRole = Qt::UserRole;
#endif

    PeersModel::PeersModel(libtremotesf::Torrent* torrent, QObject* parent)
        : QAbstractTableModel(parent),
          mTorrent(nullptr),
          mLoaded(false)
    {
        setTorrent(torrent);
    }

    PeersModel::~PeersModel()
    {
        if (mTorrent) {
            mTorrent->setPeersEnabled(false);
        }
    }

    int PeersModel::columnCount(const QModelIndex&) const
    {
#ifdef TREMOTESF_SAILFISHOS
        return 1;
#else
        return ColumnCount;
#endif
    }

    QVariant PeersModel::data(const QModelIndex& index, int role) const
    {
        const libtremotesf::Peer& peer = mPeers[static_cast<size_t>(index.row())];
#ifdef TREMOTESF_SAILFISHOS
        switch (role) {
        case Address:
            return peer.address;
        case DownloadSpeed:
            return peer.downloadSpeed;
        case UploadSpeed:
            return peer.uploadSpeed;
        case Progress:
            return peer.progress;
        case Flags:
            return peer.flags;
        case Client:
            return peer.client;
        }
#else
        switch (role) {
        case Qt::DisplayRole:
            switch (index.column()) {
            case AddressColumn:
                return peer.address;
            case DownloadSpeedColumn:
                return Utils::formatByteSpeed(peer.downloadSpeed);
            case UploadSpeedColumn:
                return Utils::formatByteSpeed(peer.uploadSpeed);
            case ProgressColumn:
                return Utils::formatProgress(peer.progress);
            case FlagsColumn:
                return peer.flags;
            case ClientColumn:
                return peer.client;
            }
            break;
        case SortRole:
            switch (index.column()) {
            case DownloadSpeedColumn:
                return peer.downloadSpeed;
            case UploadSpeedColumn:
                return peer.uploadSpeed;
            case ProgressBarColumn:
            case ProgressColumn:
                return peer.progress;
            default:
                return data(index, Qt::DisplayRole);
            }
        }
#endif
        return QVariant();
    }

#ifndef TREMOTESF_SAILFISHOS
    QVariant PeersModel::headerData(int section, Qt::Orientation, int role) const
    {
        if (role == Qt::DisplayRole) {
            switch (section) {
            case AddressColumn:
                return qApp->translate("tremotesf", "Address");
            case DownloadSpeedColumn:
                return qApp->translate("tremotesf", "Down Speed");
            case UploadSpeedColumn:
                return qApp->translate("tremotesf", "Up Speed");
            case ProgressBarColumn:
                return qApp->translate("tremotesf", "Progress Bar");
            case ProgressColumn:
                return qApp->translate("tremotesf", "Progress");
            case FlagsColumn:
                return qApp->translate("tremotesf", "Flags");
            case ClientColumn:
                return qApp->translate("tremotesf", "Client");
            }
        }
        return QVariant();
    }
#endif

    int PeersModel::rowCount(const QModelIndex&) const
    {
        return static_cast<int>(mPeers.size());
    }

    bool PeersModel::removeRows(int row, int count, const QModelIndex& parent)
    {
        beginRemoveRows(parent, row, row + count - 1);
        const auto first(mPeers.begin() + row);
        mPeers.erase(first, first + count);
        endRemoveRows();
        return true;
    }

    libtremotesf::Torrent* PeersModel::torrent() const
    {
        return mTorrent;
    }

    void PeersModel::setTorrent(libtremotesf::Torrent* torrent)
    {
        if (torrent != mTorrent) {
            mTorrent = torrent;

            if (mTorrent) {
                QObject::connect(mTorrent, &libtremotesf::Torrent::peersUpdated, this, &PeersModel::update);
                if (mTorrent->isPeersEnabled()) {
                    qWarning() << mTorrent << "already has enabled peers, this shouldn't happen";
                }
                mTorrent->setPeersEnabled(true);
            } else {
                beginResetModel();
                mPeers.clear();
                endResetModel();
            }
        }
    }

    bool PeersModel::isLoaded() const
    {
        return mLoaded;
    }

#ifdef TREMOTESF_SAILFISHOS
    QHash<int, QByteArray> PeersModel::roleNames() const
    {
        return {{Address, "address"},
                {DownloadSpeed, "downloadSpeed"},
                {UploadSpeed, "uploadSpeed"},
                {Progress, "progress"},
                {Flags, "flags"},
                {Client, "client"}};
    }
#endif

    void PeersModel::update(const std::vector<int>& removed, const std::vector<int>& changed, int added)
    {
        if (!removed.empty()) {
            ModelBatchRemover remover{this};
            for (int index : removed) {
                remover.remove(index);
            }
            remover.remove();
        }

        const auto& newPeers = mTorrent->peers();

        if (!changed.empty()) {
            ModelBatchChanger changer{this};
            for (int index : changed) {
                const size_t i = static_cast<size_t>(index);
                mPeers[i] = newPeers[i];
                changer.changed(index);
            }
            changer.changed();
        }

        if (added > 0) {
            const int first = static_cast<int>(mPeers.size());
            const int last = first + added - 1;
            beginInsertRows(QModelIndex(), first, last);
            mPeers.reserve(static_cast<size_t>(last + 1));
            for (auto end = newPeers.end(), i = end - added; i != end; ++i) {
                mPeers.push_back(*i);
            }
            endInsertRows();
        }

        if (!mLoaded) {
            mLoaded = true;
            emit loadedChanged();
        }
    }
}
