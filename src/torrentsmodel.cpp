/*
 * Tremotesf
 * Copyright (C) 2015-2017 Alexey Rochev <equeim@gmail.com>
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

#include "torrentsmodel.h"

#include <QCoreApplication>
#include <QPixmap>

#include "rpc.h"
#include "torrent.h"
#include "utils.h"

namespace tremotesf
{
    TorrentsModel::TorrentsModel(Rpc* rpc, QObject* parent)
        : QAbstractTableModel(parent),
          mRpc(nullptr)
    {
        setRpc(rpc);
    }

    int TorrentsModel::columnCount(const QModelIndex&) const
    {
#ifdef TREMOTESF_SAILFISHOS
        return 1;
#else
        return ColumnCount;
#endif
    }

    QVariant TorrentsModel::data(const QModelIndex& index, int role) const
    {
        const Torrent* torrent = mTorrents.at(index.row()).get();

#ifdef TREMOTESF_SAILFISHOS
        switch (role) {
        case NameRole:
            return torrent->name();
        case StatusRole:
            return torrent->status();
        case TotalSizeRole:
            return torrent->totalSize();
        case PercentDoneRole:
            return torrent->percentDone();
        case EtaRole:
            return torrent->eta();
        case RatioRole:
            return torrent->ratio();
        case AddedDateRole:
            return torrent->addedDate();
        }
#else
        switch (role) {
        case Qt::DecorationRole:
            if (index.column() == NameColumn) {
                switch (torrent->status()) {
                case Torrent::Paused:
                    return QPixmap(Utils::statusIconPath(Utils::PausedIcon));
                case Torrent::Seeding:
                    return QPixmap(Utils::statusIconPath(Utils::SeedingIcon));
                case Torrent::Downloading:
                    return QPixmap(Utils::statusIconPath(Utils::DownloadingIcon));
                case Torrent::StalledDownloading:
                    return QPixmap(Utils::statusIconPath(Utils::StalledDownloadingIcon));
                case Torrent::StalledSeeding:
                    return QPixmap(Utils::statusIconPath(Utils::StalledSeedingIcon));
                case Torrent::QueuedForDownloading:
                case Torrent::QueuedForSeeding:
                    return QPixmap(Utils::statusIconPath(Utils::QueuedIcon));
                case Torrent::Checking:
                case Torrent::QueuedForChecking:
                    return QPixmap(Utils::statusIconPath(Utils::CheckingIcon));
                case Torrent::Errored:
                    return QPixmap(Utils::statusIconPath(Utils::ErroredIcon));
                }
            }
            break;
        case Qt::DisplayRole:
            switch (index.column()) {
            case NameColumn:
                return torrent->name();
            case SizeWhenDoneColumn:
                return Utils::formatByteSize(torrent->sizeWhenDone());
            case TotalSizeColumn:
                return Utils::formatByteSize(torrent->sizeWhenDone());
            case ProgressColumn:
                if (torrent->status() == Torrent::Checking) {
                    return Utils::formatProgress(torrent->recheckProgress());
                }
                return Utils::formatProgress(torrent->percentDone());
            case StatusColumn:
                return torrent->statusString();
            case QueuePositionColumn:
                return torrent->queuePosition();
            case SeedersColumn:
                return torrent->seeders();
            case LeechersColumn:
                return torrent->leechers();
            case DownloadSpeedColumn:
                return Utils::formatByteSpeed(torrent->downloadSpeed());
            case UploadSpeedColumn:
                return Utils::formatByteSpeed(torrent->uploadSpeed());
            case EtaColumn:
                return Utils::formatEta(torrent->eta());
            case RatioColumn:
                return Utils::formatRatio(torrent->ratio());
            case AddedDateColumn:
                return torrent->addedDate();
            case DoneDateColumn:
                return torrent->doneDate();
            case DownloadSpeedLimitColumn:
                if (torrent->isDownloadSpeedLimited()) {
                    return Utils::formatSpeedLimit(torrent->downloadSpeedLimit());
                }
                break;
            case UploadSpeedLimitColumn:
                if (torrent->isUploadSpeedLimited()) {
                    return Utils::formatSpeedLimit(torrent->uploadSpeedLimit());
                }
                break;
            case TotalDownloadedColumn:
                return Utils::formatByteSize(torrent->totalDownloaded());
            case TotalUploadedColumn:
                return Utils::formatByteSize(torrent->totalUploaded());
            case LeftUntilDoneColumn:
                return Utils::formatByteSize(torrent->leftUntilDone());
            case DownloadDirectoryColumn:
                return torrent->downloadDirectory();
            case CompletedSizeColumn:
                return Utils::formatByteSize(torrent->completedSize());
            case ActivityDateColumn:
                return torrent->activityDate();
            }
            break;
        case SortRole:
            switch (index.column()) {
            case SizeWhenDoneColumn:
                return torrent->sizeWhenDone();
            case TotalSizeColumn:
                return torrent->totalSize();
            case ProgressBarColumn:
            case ProgressColumn:
                if (torrent->status() == Torrent::Checking) {
                    return torrent->recheckProgress();
                }
                return torrent->percentDone();
            case StatusColumn:
                return torrent->status();
            case DownloadSpeedColumn:
                return torrent->downloadSpeed();
            case UploadSpeedColumn:
                return torrent->uploadSpeed();
            case EtaColumn:
                return torrent->eta();
            case RatioColumn:
                return torrent->ratio();
            case AddedDateColumn:
                return torrent->addedDate();
            case DoneDateColumn:
                return torrent->doneDate();
            case DownloadSpeedLimitColumn:
                if (torrent->isDownloadSpeedLimited()) {
                    return torrent->downloadSpeedLimit();
                }
                return -1;
            case UploadSpeedLimitColumn:
                if (torrent->isUploadSpeedLimited()) {
                    return torrent->uploadSpeedLimit();
                }
                return -1;
            case TotalDownloadedColumn:
                return torrent->totalDownloaded();
            case TotalUploadedColumn:
                return torrent->totalUploaded();
            case LeftUntilDoneColumn:
                return torrent->leftUntilDone();
            case CompletedSizeColumn:
                return torrent->completedSize();
            case ActivityDateColumn:
                return torrent->activityDate();
            default:
                return data(index, Qt::DisplayRole);
            }
        }
#endif

        return QVariant();
    }

#ifndef TREMOTESF_SAILFISHOS
    QVariant TorrentsModel::headerData(int section, Qt::Orientation, int role) const
    {
        if (role == Qt::DisplayRole) {
            switch (section) {
            case NameColumn:
                //: Column header
                return qApp->translate("tremotesf", "Name");
            case SizeWhenDoneColumn:
                //: Column header
                return qApp->translate("tremotesf", "Size");
            case TotalSizeColumn:
                //: Column header
                return qApp->translate("tremotesf", "Total Size");
            case ProgressBarColumn:
                //: Column header
                return qApp->translate("tremotesf", "Progress Bar");
            case ProgressColumn:
                //: Column header
                return qApp->translate("tremotesf", "Progress");
            case PriorityColumn:
                //: Column header
                return qApp->translate("tremotesf", "Priority");
            case QueuePositionColumn:
                //: Column header
                return qApp->translate("tremotesf", "Queue Position");
            case StatusColumn:
                //: Column header
                return qApp->translate("tremotesf", "Status");
            case SeedersColumn:
                //: Column header
                return qApp->translate("tremotesf", "Seeders");
            case LeechersColumn:
                //: Column header
                return qApp->translate("tremotesf", "Leechers");
            case DownloadSpeedColumn:
                //: Column header
                return qApp->translate("tremotesf", "Down Speed");
            case UploadSpeedColumn:
                //: Column header
                return qApp->translate("tremotesf", "Up Speed");
            case EtaColumn:
                //: Column header
                return qApp->translate("tremotesf", "ETA");
            case RatioColumn:
                //: Column header
                return qApp->translate("tremotesf", "Ratio");
            case AddedDateColumn:
                //: Column header
                return qApp->translate("tremotesf", "Added on");
            case DoneDateColumn:
                //: Column header
                return qApp->translate("tremotesf", "Completed on");
            case DownloadSpeedLimitColumn:
                //: Column header
                return qApp->translate("tremotesf", "Down Limit");
            case UploadSpeedLimitColumn:
                //: Column header
                return qApp->translate("tremotesf", "Up Limit");
            case TotalDownloadedColumn:
                //: Column header
                return qApp->translate("tremotesf", "Downloaded");
            case TotalUploadedColumn:
                //: Column header
                return qApp->translate("tremotesf", "Uploaded");
            case LeftUntilDoneColumn:
                //: Column header
                return qApp->translate("tremotesf", "Remaining");
            case DownloadDirectoryColumn:
                //: Column header
                return qApp->translate("tremotesf", "Download Directory");
            case CompletedSizeColumn:
                //: Column header
                return qApp->translate("tremotesf", "Completed");
            case ActivityDateColumn:
                //: Column header
                return qApp->translate("tremotesf", "Last Activity");
            }
        }

        return QVariant();
    }
#endif

    int TorrentsModel::rowCount(const QModelIndex&) const
    {
        return mTorrents.size();
    }

    Rpc* TorrentsModel::rpc() const
    {
        return mRpc;
    }

    void TorrentsModel::setRpc(Rpc* rpc)
    {
        if (rpc && !mRpc) {
            mRpc = rpc;
            update();
            QObject::connect(mRpc, &Rpc::torrentsUpdated, this, &TorrentsModel::update);
        }
    }

    Torrent* TorrentsModel::torrentAtIndex(const QModelIndex& index) const
    {
        return torrentAtRow(index.row());
    }

    Torrent* TorrentsModel::torrentAtRow(int row) const
    {
        return mTorrents.at(row).get();
    }

    Torrent* TorrentsModel::torrentByName(const QString& name) const
    {
        for (const std::shared_ptr<Torrent>& torrent : mTorrents) {
            if (torrent->name() == name) {
                return torrent.get();
            }
        }
        return nullptr;
    }

    QVariantList TorrentsModel::idsFromIndexes(const QModelIndexList& indexes) const
    {
        QVariantList ids;
        for (const QModelIndex& index : indexes) {
            ids.append(mTorrents.at(index.row())->id());
        }
        return ids;
    }

#ifdef TREMOTESF_SAILFISHOS
    QHash<int, QByteArray> TorrentsModel::roleNames() const
    {
        return {{NameRole, QByteArrayLiteral("name")},
                {StatusRole, QByteArrayLiteral("status")},
                {TotalSizeRole, QByteArrayLiteral("totalSize")},
                {PercentDoneRole, QByteArrayLiteral("percentDone")},
                {EtaRole, QByteArrayLiteral("eta")},
                {RatioRole, QByteArrayLiteral("ratio")},
                {AddedDateRole, QByteArrayLiteral("addedDate")}};
    }
#endif

    void TorrentsModel::update()
    {
        const QList<std::shared_ptr<Torrent>>& torrents = mRpc->torrents();

        if (torrents.isEmpty()) {
            if (!mTorrents.isEmpty()) {
                beginRemoveRows(QModelIndex(), 0, mTorrents.size() - 1);
                mTorrents.clear();
                endRemoveRows();
            }
            return;
        }

        for (int i = 0, max = mTorrents.size(); i < max; ++i) {
            if (!torrents.contains(mTorrents.at(i))) {
                beginRemoveRows(QModelIndex(), i, i);
                mTorrents.removeAt(i);
                endRemoveRows();
                i--;
                max--;
            }
        }

        for (const std::shared_ptr<Torrent>& torrent : torrents) {
            if (!mTorrents.contains(torrent)) {
                const int row = mTorrents.size();
                beginInsertRows(QModelIndex(), row, row);
                mTorrents.append(torrent);
                endInsertRows();

                const Torrent* torrentPointer = torrent.get();
                QObject::connect(torrentPointer, &Torrent::limitsEdited, this, [=]() {
                    for (int i = 0, max = mTorrents.size(); i < max; i++) {
                        if (mTorrents.at(i).get() == torrentPointer) {
                            emit dataChanged(index(i, 0), index(i, columnCount() - 1));
                            break;
                        }
                    }
                });
            }
        }

        emit dataChanged(index(0, 0), index(mTorrents.size() - 1, columnCount() - 1));
    }
}
