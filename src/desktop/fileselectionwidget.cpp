/*
 * Tremotesf
 * Copyright (C) 2015-2016 Alexey Rochev <equeim@gmail.com>
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

#include "fileselectionwidget.h"

#include <QCoreApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QIcon>
#include <QLineEdit>
#include <QPushButton>

namespace tremotesf
{
    FileSelectionWidget::FileSelectionWidget(bool directory,
                                             const QString& filter,
                                             bool autoSetText,
                                             QWidget* parent)
        : QWidget(parent)
    {
        auto layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);

        mLineEdit = new QLineEdit(this);
        layout->addWidget(mLineEdit);

        mSelectionButton = new QPushButton(QIcon::fromTheme(QStringLiteral("document-open")), QString(), this);
        layout->addWidget(mSelectionButton);

        QObject::connect(mSelectionButton, &QPushButton::clicked, this, [=]() {
            QFileDialog* dialog;
            if (directory) {
                dialog = new QFileDialog(this, qApp->translate("tremotesf", "Select Directory"), mLineEdit->text());
                dialog->setFileMode(QFileDialog::Directory);
                dialog->setOptions(QFileDialog::ShowDirsOnly);
            } else {
                dialog = new QFileDialog(this, qApp->translate("tremotesf", "Select File"), QString(), filter);
                dialog->setFileMode(QFileDialog::ExistingFile);
            }
            dialog->setAttribute(Qt::WA_DeleteOnClose);

            QObject::connect(dialog, &QFileDialog::accepted, this, [=]() {
                if (autoSetText) {
                    mLineEdit->setText(dialog->selectedFiles().first());
                } else {
                    emit fileSelected(dialog->selectedFiles().first());
                }
            });

#ifdef Q_OS_WIN
            dialog->open();
#else
            dialog->show();
#endif
        });
    }

    QLineEdit* FileSelectionWidget::lineEdit() const
    {
        return mLineEdit;
    }

    QPushButton* FileSelectionWidget::selectionButton() const
    {
        return mSelectionButton;
    }
}
