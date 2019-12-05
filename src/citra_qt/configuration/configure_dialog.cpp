// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <map>
#include <QListWidgetItem>
#include "citra_qt/configuration/config.h"
#include "citra_qt/configuration/configure_dialog.h"
#include "citra_qt/hotkeys.h"
#include "core/settings.h"
#include "ui_configure.h"

ConfigureDialog::ConfigureDialog(QWidget* parent, HotkeyRegistry& registry, const bool goto_web,
                                 bool enable_web_config)
    : QDialog(parent), ui(new Ui::ConfigureDialog), registry(registry) {
    ui->setupUi(this);
    ui->hotkeysTab->Populate(registry);
    ui->webTab->SetWebServiceConfigEnabled(enable_web_config);

    PopulateSelectionList();

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(ui->selectorList, &QListWidget::itemSelectionChanged, this,
            &ConfigureDialog::UpdateVisibleTabs);

    adjustSize();
    ui->selectorList->setCurrentRow(0);

    // Set up used key list synchronisation
    connect(ui->inputTab, &ConfigureInput::InputKeysChanged, ui->hotkeysTab,
            &ConfigureHotkeys::OnInputKeysChanged);
    connect(ui->hotkeysTab, &ConfigureHotkeys::HotkeysChanged, ui->inputTab,
            &ConfigureInput::OnHotkeysChanged);

    // Synchronise lists upon initialisation
    ui->inputTab->EmitInputKeysChanged();
    ui->hotkeysTab->EmitHotkeysChanged();

    if (goto_web) {
        ui->tabWidget->setCurrentWidget(ui->webTab);
    }
}

ConfigureDialog::~ConfigureDialog() = default;

void ConfigureDialog::SetConfiguration() {
    ui->generalTab->SetConfiguration();
    ui->systemTab->SetConfiguration();
    ui->inputTab->LoadConfiguration();
    ui->graphicsTab->SetConfiguration();
    ui->enhancementsTab->SetConfiguration();
    ui->customLayoutTab->SetConfiguration();
    ui->audioTab->SetConfiguration();
    ui->cameraTab->SetConfiguration();
    ui->debugTab->SetConfiguration();
    ui->webTab->SetConfiguration();
    ui->uiTab->SetConfiguration();
}

void ConfigureDialog::ApplyConfiguration() {
    ui->generalTab->ApplyConfiguration();
    ui->systemTab->ApplyConfiguration();
    ui->inputTab->ApplyConfiguration();
    ui->inputTab->ApplyProfile();
    ui->hotkeysTab->ApplyConfiguration(registry);
    ui->graphicsTab->ApplyConfiguration();
    ui->enhancementsTab->ApplyConfiguration();
    ui->customLayoutTab->ApplyConfiguration();
    ui->audioTab->ApplyConfiguration();
    ui->cameraTab->ApplyConfiguration();
    ui->debugTab->ApplyConfiguration();
    ui->webTab->ApplyConfiguration();
    ui->uiTab->ApplyConfiguration();

    Settings::Apply();
    Settings::LogSettings();
}

Q_DECLARE_METATYPE(QList<QWidget*>);

void ConfigureDialog::PopulateSelectionList() {
    ui->selectorList->clear();

    const std::array<std::pair<QString, QList<QWidget*>>, 4> items{
        {{QStringLiteral("General"),
          {ui->generalTab, ui->webTab, ui->debugTab, ui->uiTab, ui->versionsTab}},
         {QStringLiteral("System"), {ui->systemTab, ui->audioTab, ui->cameraTab}},
         {QStringLiteral("Graphics"), {ui->enhancementsTab, ui->graphicsTab, ui->customLayoutTab}},
         {QStringLiteral("Controls"), {ui->inputTab, ui->hotkeysTab}}}};

    for (const auto& entry : items) {
        auto* const item = new QListWidgetItem(entry.first);
        item->setData(Qt::UserRole, QVariant::fromValue(entry.second));

        ui->selectorList->addItem(item);
    }
}

void ConfigureDialog::UpdateVisibleTabs() {
    const auto items = ui->selectorList->selectedItems();
    if (items.isEmpty()) {
        return;
    }

    const std::map<QWidget*, QString> widgets = {
        {ui->generalTab, QStringLiteral("General")},
        {ui->systemTab, QStringLiteral("System")},
        {ui->inputTab, QStringLiteral("Input")},
        {ui->hotkeysTab, QStringLiteral("Hotkeys")},
        {ui->graphicsTab, QStringLiteral("Advanced")},
        {ui->enhancementsTab, QStringLiteral("Enhancements")},
        {ui->customLayoutTab, QStringLiteral("Custom Layout")},
        {ui->audioTab, QStringLiteral("Audio")},
        {ui->cameraTab, QStringLiteral("Camera")},
        {ui->debugTab, QStringLiteral("Debug")},
        {ui->webTab, QStringLiteral("Web")},
        {ui->uiTab, QStringLiteral("UI")},
        {ui->versionsTab, QStringLiteral("Versions")},
    };

    ui->tabWidget->clear();

    const QList<QWidget*> tabs = qvariant_cast<QList<QWidget*>>(items[0]->data(Qt::UserRole));

    for (const auto tab : tabs) {
        ui->tabWidget->addTab(tab, widgets.at(tab));
    }
}
