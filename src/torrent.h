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

#ifndef TREMOTESF_TORRENT_H
#define TREMOTESF_TORRENT_H

#include <memory>
#include <vector>

#include <QDateTime>
#include <QObject>
#include <QVariantMap>

#include "torrentfilesmodelentry.h"

namespace tremotesf
{
    class Rpc;
    class Tracker;

    class Torrent : public QObject
    {
        Q_OBJECT

        Q_ENUMS(Status)
        Q_ENUMS(RatioLimitMode)
        Q_ENUMS(Priority)
        Q_ENUMS(IdleSeedingLimitMode)

        Q_PROPERTY(int id READ id CONSTANT)
        Q_PROPERTY(QString hashString READ hashString CONSTANT)
        Q_PROPERTY(QString name READ name CONSTANT)

        Q_PROPERTY(Status status READ status NOTIFY updated)
        Q_PROPERTY(QString statusString READ statusString NOTIFY updated)
        Q_PROPERTY(int queuePosition READ queuePosition NOTIFY updated)

        Q_PROPERTY(long long totalSize READ totalSize NOTIFY updated)
        Q_PROPERTY(long long completedSize READ completedSize NOTIFY updated)
        Q_PROPERTY(long long leftUntilDone READ leftUntilDone NOTIFY updated)
        Q_PROPERTY(long long sizeWhenDone READ sizeWhenDone NOTIFY updated)
        Q_PROPERTY(float percentDone READ percentDone NOTIFY updated)
        Q_PROPERTY(float recheckProgress READ recheckProgress NOTIFY updated)
        Q_PROPERTY(int eta READ eta NOTIFY updated)

        Q_PROPERTY(long long downloadSpeed READ downloadSpeed NOTIFY updated)
        Q_PROPERTY(long long uploadSpeed READ uploadSpeed NOTIFY updated)

        Q_PROPERTY(bool downloadSpeedLimited READ isDownloadSpeedLimited WRITE setDownloadSpeedLimited)
        Q_PROPERTY(int downloadSpeedLimit READ downloadSpeedLimit WRITE setDownloadSpeedLimit)
        Q_PROPERTY(bool uploadSpeedLimited READ isUploadSpeedLimited WRITE setUploadSpeedLimited)
        Q_PROPERTY(int uploadSpeedLimit READ uploadSpeedLimit WRITE setUploadSpeedLimit)

        Q_PROPERTY(long long totalDownloaded READ totalDownloaded NOTIFY updated)
        Q_PROPERTY(long long totalUploaded READ totalUploaded NOTIFY updated)
        Q_PROPERTY(float ratio READ ratio NOTIFY updated)
        Q_PROPERTY(RatioLimitMode ratioLimitMode READ ratioLimitMode WRITE setRatioLimitMode NOTIFY updated)
        Q_PROPERTY(float ratioLimit READ ratioLimit WRITE setRatioLimit NOTIFY updated)

        Q_PROPERTY(int seeders READ seeders NOTIFY updated)
        Q_PROPERTY(int leechers READ leechers NOTIFY updated)
        Q_PROPERTY(int peersLimit READ peersLimit WRITE setPeersLimit NOTIFY updated)

        Q_PROPERTY(QDateTime addedDate READ addedDate CONSTANT)
        Q_PROPERTY(QDateTime activityDate READ activityDate NOTIFY updated)
        Q_PROPERTY(QDateTime doneDate READ doneDate NOTIFY updated)

        Q_PROPERTY(bool honorSessionLimits READ honorSessionLimits WRITE setHonorSessionLimits)
        Q_PROPERTY(Priority bandwidthPriority READ bandwidthPriority WRITE setBandwidthPriority)
        Q_PROPERTY(IdleSeedingLimitMode idleSeedingLimitMode READ idleSeedingLimitMode WRITE setIdleSeedingLimitMode)
        Q_PROPERTY(int idleSeedingLimit READ idleSeedingLimit WRITE setIdleSeedingLimit)
        Q_PROPERTY(QString downloadDirectory READ downloadDirectory NOTIFY updated)
        Q_PROPERTY(QString creator READ creator NOTIFY updated)
        Q_PROPERTY(QDateTime creationDate READ creationDate NOTIFY updated)
        Q_PROPERTY(QString comment READ comment NOTIFY updated)

    public:
        enum Status
        {
            Paused,
            Downloading,
            Seeding,
            StalledDownloading,
            StalledSeeding,
            QueuedForDownloading,
            QueuedForSeeding,
            Checking,
            QueuedForChecking,
            Errored
        };

        enum RatioLimitMode
        {
            GlobalRatioLimit,
            SingleRatioLimit,
            UnlimitedRatio,
        };

        enum Priority
        {
            LowPriority = -1,
            NormalPriority,
            HighPriority
        };

        enum IdleSeedingLimitMode
        {
            GlobalIdleSeedingLimit,
            SingleIdleSeedingLimit,
            UnlimitedIdleSeeding
        };

        static const QString idKey;

        explicit Torrent(int id, const QVariantMap& torrentMap, Rpc* rpc);

        int id() const;
        const QString& hashString() const;
        const QString& name() const;

        Status status() const;
        QString statusString() const;
        int queuePosition() const;

        long long totalSize() const;
        long long completedSize() const;
        long long leftUntilDone() const;
        long long sizeWhenDone() const;
        float percentDone() const;
        float recheckProgress() const;
        int eta() const;

        long long downloadSpeed() const;
        long long uploadSpeed() const;

        bool isDownloadSpeedLimited() const;
        void setDownloadSpeedLimited(bool limited);
        int downloadSpeedLimit() const;
        void setDownloadSpeedLimit(int limit);

        bool isUploadSpeedLimited() const;
        void setUploadSpeedLimited(bool limited);
        int uploadSpeedLimit() const;
        void setUploadSpeedLimit(int limit);

        long long totalDownloaded() const;
        long long totalUploaded() const;
        float ratio() const;
        RatioLimitMode ratioLimitMode() const;
        void setRatioLimitMode(RatioLimitMode mode);
        float ratioLimit() const;
        void setRatioLimit(float limit);

        int seeders() const;
        int leechers() const;
        int peersLimit() const;
        void setPeersLimit(int limit);

        const QDateTime& addedDate() const;
        const QDateTime& activityDate() const;
        const QDateTime& doneDate() const;

        bool honorSessionLimits() const;
        void setHonorSessionLimits(bool honor);
        Priority bandwidthPriority() const;
        void setBandwidthPriority(Priority priority);
        IdleSeedingLimitMode idleSeedingLimitMode() const;
        void setIdleSeedingLimitMode(IdleSeedingLimitMode mode);
        int idleSeedingLimit() const;
        void setIdleSeedingLimit(int limit);
        const QString& downloadDirectory() const;
        const QString& creator() const;
        const QDateTime& creationDate() const;
        const QString& comment() const;

        bool isFilesEnabled() const;
        void setFilesEnabled(bool enabled);
        bool isFilesUpdated() const;
        const QVariantList& files() const;
        const QVariantList& filesStats() const;

        void setFilesWanted(const QVariantList& files, bool wanted);
        void setFilesPriority(const QVariantList& files, TorrentFilesModelEntryEnums::Priority priority);

        void renameFile(const QString& path, const QString& newName);

        const std::vector<std::shared_ptr<Tracker>>& trackers() const;
        Q_INVOKABLE void addTracker(const QString& announce);
        Q_INVOKABLE void setTracker(int trackerId, const QString& announce);
        Q_INVOKABLE void removeTrackers(const QVariantList& ids);

        bool isPeersEnabled() const;
        void setPeersEnabled(bool enabled);
        bool isPeersUpdated() const;

        bool isUpdated() const;

        void update(const QVariantMap& torrentMap);
        void updateFiles(const QVariantMap& torrentMap);
        void updatePeers(const QVariantMap& torrentMap);

    private:
        int mId = 0;
        QString mHashString;
        QString mName;

        Status mStatus = Paused;
        QString mErrorString;
        int mQueuePosition = 0;

        long long mTotalSize = 0;
        long long mCompletedSize = 0;
        long long mLeftUntilDone = 0;
        long long mSizeWhenDone = 0;
        float mPercentDone = 0.0f;
        float mRecheckProgress = 0.0f;
        int mEta = 0;

        long long mDownloadSpeed = 0;
        long long mUploadSpeed = 0;

        bool mDownloadSpeedLimited = false;
        int mDownloadSpeedLimit = 0; // KiB/s
        bool mUploadSpeedLimited = false;
        int mUploadSpeedLimit = 0; // KiB/s

        long long mTotalDownloaded = 0;
        long long mTotalUploaded = 0;
        float mRatio = 0.0f;
        RatioLimitMode mRatioLimitMode = GlobalRatioLimit;
        float mRatioLimit = 0.0f;

        int mSeeders = 0;
        int mLeechers = 0;
        int mPeersLimit = 0;

        QDateTime mAddedDate;
        QDateTime mActivityDate;
        QDateTime mDoneDate;

        bool mHonorSessionLimits = false;
        Priority mBandwidthPriority = NormalPriority;
        IdleSeedingLimitMode mIdleSeedingLimitMode = GlobalIdleSeedingLimit;
        int mIdleSeedingLimit = 0;
        QString mDownloadDirectory;
        QString mComment;
        QString mCreator;
        QDateTime mCreationDate;

        bool mFilesEnabled = false;
        bool mFilesUpdated = false;
        QVariantList mFiles;
        QVariantList mFileStats;

        bool mPeersEnabled = false;
        bool mPeersUpdated = false;

        std::vector<std::shared_ptr<Tracker>> mTrackers;

        Rpc* mRpc;
    signals:
        void updated();
        void filesUpdated(const QVariantList& files, const QVariantList& fileStats);
        void fileRenamed(const QString& filePath, const QString& newName);
        void peersUpdated(const QVariantList& peers);
        void limitsEdited();
    };
}

#endif // TREMOTESF_TORRENT_H
