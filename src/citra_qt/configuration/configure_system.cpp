// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QMessageBox>
#include "citra_qt/configuration/configure_system.h"
#include "citra_qt/uisettings.h"
#include "core/core.h"
#include "core/hle/service/cfg/cfg.h"
#include "core/hle/service/ptm/ptm.h"
#include "core/settings.h"
#include "ui_configure_system.h"

static const std::array<int, 12> days_in_month = {{
    31,
    29,
    31,
    30,
    31,
    30,
    31,
    31,
    30,
    31,
    30,
    31,
}};

static const std::array<QString, 187> country_names = {
    QStringLiteral(""),
    QStringLiteral("Japan"),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral("Anguilla"),
    QStringLiteral("Antigua and Barbuda"), // 0-9
    QStringLiteral("Argentina"),
    QStringLiteral("Aruba"),
    QStringLiteral("Bahamas"),
    QStringLiteral("Barbados"),
    QStringLiteral("Belize"),
    QStringLiteral("Bolivia"),
    QStringLiteral("Brazil"),
    QStringLiteral("British Virgin Islands"),
    QStringLiteral("Canada"),
    QStringLiteral("Cayman Islands"), // 10-19
    QStringLiteral("Chile"),
    QStringLiteral("Colombia"),
    QStringLiteral("Costa Rica"),
    QStringLiteral("Dominica"),
    QStringLiteral("Dominican Republic"),
    QStringLiteral("Ecuador"),
    QStringLiteral("El Salvador"),
    QStringLiteral("French Guiana"),
    QStringLiteral("Grenada"),
    QStringLiteral("Guadeloupe"), // 20-29
    QStringLiteral("Guatemala"),
    QStringLiteral("Guyana"),
    QStringLiteral("Haiti"),
    QStringLiteral("Honduras"),
    QStringLiteral("Jamaica"),
    QStringLiteral("Martinique"),
    QStringLiteral("Mexico"),
    QStringLiteral("Montserrat"),
    QStringLiteral("Netherlands Antilles"),
    QStringLiteral("Nicaragua"), // 30-39
    QStringLiteral("Panama"),
    QStringLiteral("Paraguay"),
    QStringLiteral("Peru"),
    QStringLiteral("Saint Kitts and Nevis"),
    QStringLiteral("Saint Lucia"),
    QStringLiteral("Saint Vincent and the Grenadines"),
    QStringLiteral("Suriname"),
    QStringLiteral("Trinidad and Tobago"),
    QStringLiteral("Turks and Caicos Islands"),
    QStringLiteral("United States"), // 40-49
    QStringLiteral("Uruguay"),
    QStringLiteral("US Virgin Islands"),
    QStringLiteral("Venezuela"),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral(""), // 50-59
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral("Albania"),
    QStringLiteral("Australia"),
    QStringLiteral("Austria"),
    QStringLiteral("Belgium"),
    QStringLiteral("Bosnia and Herzegovina"),
    QStringLiteral("Botswana"), // 60-69
    QStringLiteral("Bulgaria"),
    QStringLiteral("Croatia"),
    QStringLiteral("Cyprus"),
    QStringLiteral("Czech Republic"),
    QStringLiteral("Denmark"),
    QStringLiteral("Estonia"),
    QStringLiteral("Finland"),
    QStringLiteral("France"),
    QStringLiteral("Germany"),
    QStringLiteral("Greece"), // 70-79
    QStringLiteral("Hungary"),
    QStringLiteral("Iceland"),
    QStringLiteral("Ireland"),
    QStringLiteral("Italy"),
    QStringLiteral("Latvia"),
    QStringLiteral("Lesotho"),
    QStringLiteral("Liechtenstein"),
    QStringLiteral("Lithuania"),
    QStringLiteral("Luxembourg"),
    QStringLiteral("Macedonia"), // 80-89
    QStringLiteral("Malta"),
    QStringLiteral("Montenegro"),
    QStringLiteral("Mozambique"),
    QStringLiteral("Namibia"),
    QStringLiteral("Netherlands"),
    QStringLiteral("New Zealand"),
    QStringLiteral("Norway"),
    QStringLiteral("Poland"),
    QStringLiteral("Portugal"),
    QStringLiteral("Romania"), // 90-99
    QStringLiteral("Russia"),
    QStringLiteral("Serbia"),
    QStringLiteral("Slovakia"),
    QStringLiteral("Slovenia"),
    QStringLiteral("South Africa"),
    QStringLiteral("Spain"),
    QStringLiteral("Swaziland"),
    QStringLiteral("Sweden"),
    QStringLiteral("Switzerland"),
    QStringLiteral("Turkey"), // 100-109
    QStringLiteral("United Kingdom"),
    QStringLiteral("Zambia"),
    QStringLiteral("Zimbabwe"),
    QStringLiteral("Azerbaijan"),
    QStringLiteral("Mauritania"),
    QStringLiteral("Mali"),
    QStringLiteral("Niger"),
    QStringLiteral("Chad"),
    QStringLiteral("Sudan"),
    QStringLiteral("Eritrea"), // 110-119
    QStringLiteral("Djibouti"),
    QStringLiteral("Somalia"),
    QStringLiteral("Andorra"),
    QStringLiteral("Gibraltar"),
    QStringLiteral("Guernsey"),
    QStringLiteral("Isle of Man"),
    QStringLiteral("Jersey"),
    QStringLiteral("Monaco"),
    QStringLiteral("Taiwan"),
    QStringLiteral(""), // 120-129
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral("South Korea"),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral(""), // 130-139
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral("Hong Kong"),
    QStringLiteral("Macau"),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral(""), // 140-149
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral("Indonesia"),
    QStringLiteral("Singapore"),
    QStringLiteral("Thailand"),
    QStringLiteral("Philippines"),
    QStringLiteral("Malaysia"),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral(""), // 150-159
    QStringLiteral("China"),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral("United Arab Emirates"),
    QStringLiteral("India"), // 160-169
    QStringLiteral("Egypt"),
    QStringLiteral("Oman"),
    QStringLiteral("Qatar"),
    QStringLiteral("Kuwait"),
    QStringLiteral("Saudi Arabia"),
    QStringLiteral("Syria"),
    QStringLiteral("Bahrain"),
    QStringLiteral("Jordan"),
    QStringLiteral(""),
    QStringLiteral(""), // 170-179
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral(""),
    QStringLiteral("San Marino"),
    QStringLiteral("Vatican City"),
    QStringLiteral("Bermuda"), // 180-186
};

ConfigureSystem::ConfigureSystem(QWidget* parent) : QWidget(parent), ui(new Ui::ConfigureSystem) {
    ui->setupUi(this);
    connect(ui->combo_birthmonth,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &ConfigureSystem::UpdateBirthdayComboBox);
    connect(ui->combo_init_clock,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &ConfigureSystem::UpdateInitTime);
    connect(ui->button_regenerate_console_id, &QPushButton::clicked, this,
            &ConfigureSystem::RefreshConsoleID);
    for (u8 i = 0; i < country_names.size(); i++) {
        if (!country_names[i].isEmpty()) {
            ui->combo_country->addItem(country_names.at(i), i);
        }
    }

    ConfigureTime();
}

ConfigureSystem::~ConfigureSystem() = default;

void ConfigureSystem::SetConfiguration() {
    enabled = !Core::System::GetInstance().IsPoweredOn();

    ui->combo_init_clock->setCurrentIndex(static_cast<u8>(Settings::values.init_clock));
    QDateTime date_time;
    date_time.setTime_t(Settings::values.init_time);
    ui->edit_init_time->setDateTime(date_time);

    if (!enabled) {
        cfg = Service::CFG::GetModule(Core::System::GetInstance());
        ASSERT_MSG(cfg, "CFG Module missing!");
        ReadSystemSettings();
        ui->group_system_settings->setEnabled(false);
    } else {
        // This tab is enabled only when game is not running (i.e. all service are not initialized).
        cfg = std::make_shared<Service::CFG::Module>();
        ReadSystemSettings();

        ui->label_disable_info->hide();
    }
}

void ConfigureSystem::ReadSystemSettings() {
    // set username
    username = cfg->GetUsername();
    // TODO(wwylele): Use this when we move to Qt 5.5
    // ui->edit_username->setText(QString::fromStdU16String(username));
    ui->edit_username->setText(
        QString::fromUtf16(reinterpret_cast<const ushort*>(username.data())));

    // set birthday
    std::tie(birthmonth, birthday) = cfg->GetBirthday();
    ui->combo_birthmonth->setCurrentIndex(birthmonth - 1);
    UpdateBirthdayComboBox(
        birthmonth -
        1); // explicitly update it because the signal from setCurrentIndex is not reliable
    ui->combo_birthday->setCurrentIndex(birthday - 1);

    // set system language
    language_index = cfg->GetSystemLanguage();
    ui->combo_language->setCurrentIndex(language_index);

    // set sound output mode
    sound_index = cfg->GetSoundOutputMode();
    ui->combo_sound->setCurrentIndex(sound_index);

    // set the country code
    country_code = cfg->GetCountryCode();
    ui->combo_country->setCurrentIndex(ui->combo_country->findData(country_code));

    // set the console id
    u64 console_id = cfg->GetConsoleUniqueId();
    ui->label_console_id->setText(
        QStringLiteral("Console ID: 0x%1").arg(QString::number(console_id, 16).toUpper()));

    // set play coin
    play_coin = Service::PTM::Module::GetPlayCoins();
    ui->spinBox_play_coins->setValue(play_coin);
}

void ConfigureSystem::ApplyConfiguration() {
    if (!enabled) {
        return;
    }

    bool modified = false;

    // apply username
    // TODO(wwylele): Use this when we move to Qt 5.5
    // std::u16string new_username = ui->edit_username->text().toStdU16String();
    std::u16string new_username(
        reinterpret_cast<const char16_t*>(ui->edit_username->text().utf16()));
    if (new_username != username) {
        cfg->SetUsername(new_username);
        modified = true;
    }

    // apply birthday
    int new_birthmonth = ui->combo_birthmonth->currentIndex() + 1;
    int new_birthday = ui->combo_birthday->currentIndex() + 1;
    if (birthmonth != new_birthmonth || birthday != new_birthday) {
        cfg->SetBirthday(new_birthmonth, new_birthday);
        modified = true;
    }

    // apply language
    int new_language = ui->combo_language->currentIndex();
    if (language_index != new_language) {
        cfg->SetSystemLanguage(static_cast<Service::CFG::SystemLanguage>(new_language));
        modified = true;
    }

    // apply sound
    int new_sound = ui->combo_sound->currentIndex();
    if (sound_index != new_sound) {
        cfg->SetSoundOutputMode(static_cast<Service::CFG::SoundOutputMode>(new_sound));
        modified = true;
    }

    // apply country
    u8 new_country = static_cast<u8>(ui->combo_country->currentData().toInt());
    if (country_code != new_country) {
        cfg->SetCountryCode(new_country);
        modified = true;
    }

    // apply play coin
    u16 new_play_coin = static_cast<u16>(ui->spinBox_play_coins->value());
    if (play_coin != new_play_coin) {
        Service::PTM::Module::SetPlayCoins(new_play_coin);
    }

    // update the config savegame if any item is modified.
    if (modified) {
        cfg->UpdateConfigNANDSavegame();
    }

    Settings::values.init_clock =
        static_cast<Settings::InitClock>(ui->combo_init_clock->currentIndex());
    Settings::values.init_time = ui->edit_init_time->dateTime().toTime_t();
    Settings::Apply();
}

void ConfigureSystem::UpdateBirthdayComboBox(int birthmonth_index) {
    if (birthmonth_index < 0 || birthmonth_index >= 12)
        return;

    // store current day selection
    int birthday_index = ui->combo_birthday->currentIndex();

    // get number of days in the new selected month
    int days = days_in_month[birthmonth_index];

    // if the selected day is out of range,
    // reset it to 1st
    if (birthday_index < 0 || birthday_index >= days)
        birthday_index = 0;

    // update the day combo box
    ui->combo_birthday->clear();
    for (int i = 1; i <= days; ++i) {
        ui->combo_birthday->addItem(QString::number(i));
    }

    // restore the day selection
    ui->combo_birthday->setCurrentIndex(birthday_index);
}

void ConfigureSystem::ConfigureTime() {
    ui->edit_init_time->setCalendarPopup(true);
    QDateTime dt;
    dt.fromString(QStringLiteral("2000-01-01 00:00:01"), QStringLiteral("yyyy-MM-dd hh:mm:ss"));
    ui->edit_init_time->setMinimumDateTime(dt);

    SetConfiguration();

    UpdateInitTime(ui->combo_init_clock->currentIndex());
}

void ConfigureSystem::UpdateInitTime(int init_clock) {
    const bool is_fixed_time =
        static_cast<Settings::InitClock>(init_clock) == Settings::InitClock::FixedTime;
    ui->label_init_time->setVisible(is_fixed_time);
    ui->edit_init_time->setVisible(is_fixed_time);
}

void ConfigureSystem::RefreshConsoleID() {
    QMessageBox::StandardButton reply;
    QString warning_text =
        QStringLiteral("This will replace your current virtual 3DS with a new one. "
                       "Your current virtual 3DS will not be recoverable. "
                       "This might have unexpected effects in games. This might fail, "
                       "if you use an outdated config savegame. Continue?");
    reply = QMessageBox::critical(this, QStringLiteral("Warning"), warning_text,
                                  QMessageBox::No | QMessageBox::Yes);
    if (reply == QMessageBox::No) {
        return;
    }

    u32 random_number;
    u64 console_id;
    cfg->GenerateConsoleUniqueId(random_number, console_id);
    cfg->SetConsoleUniqueId(random_number, console_id);
    cfg->UpdateConfigNANDSavegame();
    ui->label_console_id->setText(
        QStringLiteral("Console ID: 0x%1").arg(QString::number(console_id, 16).toUpper()));
}