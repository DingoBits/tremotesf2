// SPDX-FileCopyrightText: 2015-2022 Alexey Rochev
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "aboutdialog.h"

#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QGuiApplication>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <QTextBrowser>
#include <QVBoxLayout>

#include <KTitleWidget>

#include "libtremotesf/literals.h"
#include "tremotesf/utils.h"

namespace tremotesf {
    AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent) {
        setWindowTitle(qApp->translate("tremotesf", "About"));

        auto layout = new QVBoxLayout(this);

        auto titleWidget = new KTitleWidget(this);
        titleWidget->setIcon(qApp->windowIcon(), KTitleWidget::ImageLeft);
        titleWidget->setText(QString::fromLatin1("%1 %2").arg(TREMOTESF_APP_NAME ""_l1, qApp->applicationVersion()));
        layout->addWidget(titleWidget);

        auto tabWidget = new QTabWidget(this);

        auto aboutPage = new QWidget(this);
        auto aboutPageLayout = new QVBoxLayout(aboutPage);
        auto aboutPageLabel = new QLabel(
            qApp->translate(
                "tremotesf",
                "<p>&#169; 2015-2020 Alexey Rochev &lt;<a "
                "href=\"mailto:equeim@gmail.com\">equeim@gmail.com</a>&gt;</p>\n"
                "<p>Source code: <a "
                "href=\"https://github.com/equeim/tremotesf2\">https://github.com/equeim/tremotesf2</a></p>\n"
                "<p>Translations: <a "
                "href=\"https://www.transifex.com/equeim/tremotesf\">https://www.transifex.com/equeim/tremotesf</a></p>"
            ),
            this
        );
        QObject::connect(aboutPageLabel, &QLabel::linkActivated, this, &QDesktopServices::openUrl);
        aboutPageLayout->addWidget(aboutPageLabel);

        tabWidget->addTab(aboutPage, qApp->translate("tremotesf", "About"));

        auto authorsPage = new QWidget(this);
        auto authorsPageLayout = new QVBoxLayout(authorsPage);
        auto authorsWidget = new QTextBrowser(this);
        authorsWidget->setHtml(Utils::readTextResource(":/authors.html"));
        authorsWidget->setOpenExternalLinks(true);
        authorsPageLayout->addWidget(authorsWidget);
        authorsPageLayout->addStretch();
        tabWidget->addTab(authorsPage, qApp->translate("tremotesf", "Authors"));

        auto translatorsWidget = new QTextBrowser(this);
        translatorsWidget->setHtml(Utils::readTextResource(":/translators.html"));
        translatorsWidget->setOpenExternalLinks(true);
        tabWidget->addTab(translatorsWidget, qApp->translate("tremotesf", "Translators"));

        auto licenseWidget = new QTextBrowser(this);
        licenseWidget->setHtml(Utils::readTextResource(":/license.html"));
        licenseWidget->setOpenExternalLinks(true);
        tabWidget->addTab(licenseWidget, qApp->translate("tremotesf", "License"));

        layout->addWidget(tabWidget);

        auto dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
        QObject::connect(dialogButtonBox, &QDialogButtonBox::rejected, this, &AboutDialog::reject);
        layout->addWidget(dialogButtonBox);

        dialogButtonBox->button(QDialogButtonBox::Close)->setDefault(true);

        setMinimumSize(minimumSizeHint());
    }

    QSize AboutDialog::sizeHint() const { return minimumSizeHint().expandedTo(QSize(420, 384)); }
}
