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

#include "mainwindowstatusbar.h"

#include <QCoreApplication>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>

#include <KSeparator>

#include "../accounts.h"
#include "../rpc.h"
#include "../serverstats.h"
#include "../utils.h"

namespace tremotesf
{
    MainWindowStatusBar::MainWindowStatusBar(const Rpc* rpc, QWidget* parent)
        : QStatusBar(parent),
          mRpc(rpc)
    {
        setSizeGripEnabled(false);

        delete layout();

        auto layout = new QHBoxLayout(this);
        layout->setContentsMargins(8, 4, 8, 4);

        mNoAccountsErrorImage = new QLabel(this);
        mNoAccountsErrorImage->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-error")).pixmap(16, 16));
        mNoAccountsErrorImage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        layout->addWidget(mNoAccountsErrorImage);

        mAccountLabel = new QLabel(this);
        layout->addWidget(mAccountLabel);

        mFirstSeparator = new KSeparator(Qt::Vertical, this);
        layout->addWidget(mFirstSeparator);

        mStatusLabel = new QLabel(this);
        layout->addWidget(mStatusLabel);

        mSecondSeparator = new KSeparator(Qt::Vertical, this);
        layout->addWidget(mSecondSeparator);

        mDownloadSpeedImage = new QLabel(this);
        mDownloadSpeedImage->setPixmap(QIcon::fromTheme(QStringLiteral("go-down")).pixmap(16, 16));
        mDownloadSpeedImage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        layout->addWidget(mDownloadSpeedImage);

        mDownloadSpeedLabel = new QLabel(this);
        layout->addWidget(mDownloadSpeedLabel);

        mThirdSeparator = new KSeparator(Qt::Vertical, this);
        layout->addWidget(mThirdSeparator);

        mUploadSpeedImage = new QLabel(this);
        mUploadSpeedImage->setPixmap(QIcon::fromTheme(QStringLiteral("go-up")).pixmap(16, 16));
        mUploadSpeedImage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        layout->addWidget(mUploadSpeedImage);

        mUploadSpeedLabel = new QLabel(this);
        layout->addWidget(mUploadSpeedLabel);

        updateLayout();
        QObject::connect(mRpc, &Rpc::statusChanged, this, &MainWindowStatusBar::updateLayout);
        QObject::connect(Accounts::instance(), &Accounts::hasAccountsChanged, this, &MainWindowStatusBar::updateLayout);

        updateAccountLabel();
        QObject::connect(Accounts::instance(), &Accounts::currentAccountChanged, this, &MainWindowStatusBar::updateAccountLabel);
        QObject::connect(Accounts::instance(), &Accounts::hasAccountsChanged, this, &MainWindowStatusBar::updateAccountLabel);

        updateStatusLabels();
        QObject::connect(mRpc, &Rpc::statusStringChanged, this, &MainWindowStatusBar::updateStatusLabels);

        QObject::connect(mRpc->serverStats(), &ServerStats::updated, this, [=]() {
            mDownloadSpeedLabel->setText(Utils::formatByteSpeed(mRpc->serverStats()->downloadSpeed()));
            mUploadSpeedLabel->setText(Utils::formatByteSpeed(mRpc->serverStats()->uploadSpeed()));
        });
    }

    void MainWindowStatusBar::updateLayout()
    {
        if (Accounts::instance()->hasAccounts()) {
            mNoAccountsErrorImage->hide();
            mAccountLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
            mFirstSeparator->show();
            mStatusLabel->show();
            if (mRpc->status() == Rpc::Connected) {
                mSecondSeparator->show();
                mDownloadSpeedImage->show();
                mDownloadSpeedLabel->show();
                mThirdSeparator->show();
                mUploadSpeedImage->show();
                mUploadSpeedLabel->show();
            } else {
                mSecondSeparator->hide();
                mDownloadSpeedImage->hide();
                mDownloadSpeedLabel->hide();
                mThirdSeparator->hide();
                mUploadSpeedImage->hide();
                mUploadSpeedLabel->hide();
            }
        } else {
            mNoAccountsErrorImage->show();
            mAccountLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            mFirstSeparator->hide();
            mStatusLabel->hide();
            mSecondSeparator->hide();
            mDownloadSpeedImage->hide();
            mDownloadSpeedLabel->hide();
            mThirdSeparator->hide();
            mUploadSpeedImage->hide();
            mUploadSpeedLabel->hide();
        }
    }

    void MainWindowStatusBar::updateAccountLabel()
    {
        if (Accounts::instance()->hasAccounts()) {
            mAccountLabel->setText(QStringLiteral("%1 (%2)").arg(Accounts::instance()->currentAccountName()).arg(Accounts::instance()->currentAccountAddress()));
        } else {
            mAccountLabel->setText(qApp->translate("tremotesf", "No accounts"));
        }
    }

    void MainWindowStatusBar::updateStatusLabels()
    {
        mStatusLabel->setText(mRpc->statusString());
    }
}
