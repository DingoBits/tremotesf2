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

#ifndef TREMOTESF_TORRENTSMODEL_H
#define TREMOTESF_TORRENTSMODEL_H

#include <memory>
#include <vector>

#include <QAbstractTableModel>

namespace libtremotesf
{
    class Torrent;
}

namespace tremotesf
{
    class Rpc;

    class TorrentsModel : public QAbstractTableModel
    {
        Q_OBJECT
        Q_PROPERTY(tremotesf::Rpc* rpc READ rpc WRITE setRpc)
    public:
#ifdef TREMOTESF_SAILFISHOS
        enum Role
        {
            NameRole = Qt::UserRole,
            StatusRole,
            TotalSizeRole,
            PercentDoneRole,
            EtaRole,
            RatioRole,
            AddedDateRole
        };
        Q_ENUMS(Role)
#else
        enum Column
        {
            NameColumn,
            SizeWhenDoneColumn,
            TotalSizeColumn,
            ProgressBarColumn,
            ProgressColumn,
            StatusColumn,
            PriorityColumn,
            QueuePositionColumn,
            SeedersColumn,
            LeechersColumn,
            DownloadSpeedColumn,
            UploadSpeedColumn,
            EtaColumn,
            RatioColumn,
            AddedDateColumn,
            DoneDateColumn,
            DownloadSpeedLimitColumn,
            UploadSpeedLimitColumn,
            TotalDownloadedColumn,
            TotalUploadedColumn,
            LeftUntilDoneColumn,
            DownloadDirectoryColumn,
            CompletedSizeColumn,
            ActivityDateColumn,
            ColumnCount
        };
        static const int SortRole = Qt::UserRole;
#endif

        explicit TorrentsModel(Rpc* rpc = nullptr, QObject* parent = nullptr);

        int columnCount(const QModelIndex& = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role) const override;
#ifndef TREMOTESF_SAILFISHOS
        QVariant headerData(int section, Qt::Orientation, int role = Qt::DisplayRole) const override;
#endif
        int rowCount(const QModelIndex&) const override;

        Rpc* rpc() const;
        void setRpc(Rpc* rpc);

        libtremotesf::Torrent* torrentAtIndex(const QModelIndex& index) const;
        Q_INVOKABLE libtremotesf::Torrent* torrentAtRow(int row) const;
        Q_INVOKABLE libtremotesf::Torrent* torrentByName(const QString& name) const;

        Q_INVOKABLE QVariantList idsFromIndexes(const QModelIndexList& indexes) const;

#ifdef TREMOTESF_SAILFISHOS
    protected:
        QHash<int, QByteArray> roleNames() const override;
#endif

    private:
        void update();

        std::vector<std::shared_ptr<libtremotesf::Torrent>> mTorrents;
        Rpc* mRpc;
    };
}

#endif // TREMOTESF_TORRENTSMODEL_H
