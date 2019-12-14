// Copyright 2015 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QApplication>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QModelIndex>
#include <QProcess>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QThreadPool>
#include <QToolButton>
#include <QTreeView>
#include <QtConcurrent/QtConcurrent>
#include <fmt/format.h>
#include "citra_qt/debugger/console.h"
#include "citra_qt/game_list.h"
#include "citra_qt/game_list_p.h"
#include "citra_qt/game_list_worker.h"
#include "citra_qt/main.h"
#include "citra_qt/uisettings.h"
#include "common/common_paths.h"
#include "common/logging/log.h"
#include "core/file_sys/archive_extsavedata.h"
#include "core/file_sys/archive_source_sd_savedata.h"
#include "core/hle/service/fs/archive.h"

GameListSearchField::KeyReleaseEater::KeyReleaseEater(GameList* gamelist) : gamelist{gamelist} {}

// EventFilter in order to process systemkeys while editing the searchfield
bool GameListSearchField::KeyReleaseEater::eventFilter(QObject* obj, QEvent* event) {
    // If it isn't a KeyRelease event then continue with standard event processing
    if (event->type() != QEvent::KeyRelease) {
        return QObject::eventFilter(obj, event);
    }

    QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
    QString edit_filter_text = gamelist->search_field->edit_filter->text().toLower();

    // If the searchfield's text hasn't changed special function keys get checked
    // If no function key changes the searchfield's text the filter doesn't need to get reloaded
    if (edit_filter_text == edit_filter_text_old) {
        switch (key_event->key()) {
        // Escape: Resets the searchfield
        case Qt::Key_Escape: {
            if (edit_filter_text_old.isEmpty()) {
                return QObject::eventFilter(obj, event);
            } else {
                gamelist->search_field->edit_filter->clear();
                edit_filter_text.clear();
            }
            break;
        }
        // Return and Enter
        // If the enter key gets pressed first checks how many and which entry is visible
        // If there is only one result launch this game
        case Qt::Key_Return:
        case Qt::Key_Enter: {
            if (gamelist->search_field->visible == 1) {
                const QString file_path = gamelist->getLastFilterResultItem();

                // To avoid loading error dialog loops while confirming them using enter
                // Also users usually want to run a different game after closing one
                gamelist->search_field->edit_filter->clear();
                edit_filter_text.clear();
                emit gamelist->GameChosen(file_path);
            } else {
                return QObject::eventFilter(obj, event);
            }
            break;
        }
        default:
            return QObject::eventFilter(obj, event);
        }
    }
    edit_filter_text_old = edit_filter_text;
    return QObject::eventFilter(obj, event);
}

void GameListSearchField::setFilterResult(int visible, int total) {
    this->visible = visible;
    this->total = total;

    const QString result_of_text = QStringLiteral("of");
    QString result_text;
    if (total == 1) {
        result_text = QStringLiteral("result");
    } else {
        result_text = QStringLiteral("results");
    }
    label_filter_result->setText(
        QStringLiteral("%1 %2 %3 %4").arg(visible).arg(result_of_text).arg(total).arg(result_text));
}

QString GameList::getLastFilterResultItem() const {
    QStandardItem* folder;
    QStandardItem* child;
    QString file_path;
    const int folderCount = item_model->rowCount();
    for (int i = 0; i < folderCount; ++i) {
        folder = item_model->item(i, 0);
        const QModelIndex folder_index = folder->index();
        const int children_count = folder->rowCount();
        for (int j = 0; j < children_count; ++j) {
            if (!tree_view->isRowHidden(j, folder_index)) {
                child = folder->child(j, 0);
                file_path = child->data(GameListItemPath::FullPathRole).toString();
            }
        }
    }
    return file_path;
}

void GameListSearchField::clear() {
    edit_filter->clear();
}

void GameListSearchField::setFocus() {
    if (edit_filter->isVisible()) {
        edit_filter->setFocus();
    }
}

GameListSearchField::GameListSearchField(GameList* parent) : QWidget{parent} {
    KeyReleaseEater* key_release_eater = new KeyReleaseEater(parent);
    layout_filter = new QHBoxLayout;
    layout_filter->setMargin(8);
    label_filter = new QLabel;
    label_filter->setText(QStringLiteral("Filter:"));
    edit_filter = new QLineEdit;
    edit_filter->clear();
    edit_filter->setPlaceholderText(QStringLiteral("Enter pattern to filter"));
    edit_filter->installEventFilter(key_release_eater);
    edit_filter->setClearButtonEnabled(true);
    connect(edit_filter, &QLineEdit::textChanged, parent, &GameList::onTextChanged);
    label_filter_result = new QLabel;
    button_filter_close = new QToolButton(this);
    button_filter_close->setText(QStringLiteral("X"));
    button_filter_close->setCursor(Qt::ArrowCursor);
    button_filter_close->setStyleSheet(
        QStringLiteral("QToolButton { border: none; padding: 0px; color: "
                       "#000000; font-weight: bold; background: #F0F0F0 }"
                       "QToolButton:hover { border: none; padding: 0px; color: "
                       "#EEEEEE; font-weight: bold; background: #E81123 }"));
    connect(button_filter_close, &QToolButton::clicked, parent, &GameList::onFilterCloseClicked);
    layout_filter->setSpacing(10);
    layout_filter->addWidget(label_filter);
    layout_filter->addWidget(edit_filter);
    layout_filter->addWidget(label_filter_result);
    layout_filter->addWidget(button_filter_close);
    setLayout(layout_filter);
}

/**
 * Checks if all words separated by spaces are contained in another string
 * This offers a word order insensitive search function
 *
 * @param String that gets checked if it contains all words of the userinput string
 * @param String containing all words getting checked
 * @return true if the haystack contains all words of userinput
 */
static bool ContainsAllWords(const QString& haystack, const QString& userinput) {
    const QStringList userinput_split =
        userinput.split(QLatin1Char{' '}, QString::SplitBehavior::SkipEmptyParts);

    return std::all_of(userinput_split.begin(), userinput_split.end(),
                       [&haystack](const QString& s) { return haystack.contains(s); });
}

// Syncs the expanded state of Game Directories with settings to persist across sessions
void GameList::onItemExpanded(const QModelIndex& item) {
    const GameListItemType type = item.data(GameListItem::TypeRole).value<GameListItemType>();
    if (type == GameListItemType::CustomDir || type == GameListItemType::InstalledDir ||
        type == GameListItemType::SystemDir) {
        item.data(GameListDir::GameDirRole).value<UISettings::GameDir*>()->expanded =
            tree_view->isExpanded(item);
    }
}

// Event in order to filter the gamelist after editing the searchfield
void GameList::onTextChanged(const QString& new_text) {
    const int folder_count = tree_view->model()->rowCount();
    QString edit_filter_text = new_text.toLower();
    QStandardItem* folder;
    int children_total = 0;

    // If the searchfield is empty every item is visible
    // Otherwise the filter gets applied
    if (edit_filter_text.isEmpty()) {
        for (int i = 0; i < folder_count; ++i) {
            folder = item_model->item(i, 0);
            const QModelIndex folder_index = folder->index();
            const int children_count = folder->rowCount();
            for (int j = 0; j < children_count; ++j) {
                ++children_total;
                tree_view->setRowHidden(j, folder_index, false);
            }
        }
        search_field->setFilterResult(children_total, children_total);
    } else {
        int result_count = 0;
        for (int i = 0; i < folder_count; ++i) {
            folder = item_model->item(i, 0);
            const QModelIndex folder_index = folder->index();
            const int children_count = folder->rowCount();
            for (int j = 0; j < children_count; ++j) {
                ++children_total;
                const QStandardItem* child = folder->child(j, 0);
                const QString file_path =
                    child->data(GameListItemPath::FullPathRole).toString().toLower();
                const QString file_long_title =
                    child->data(GameListItemPath::LongTitleRole).toString().toLower();
                const QString file_program_id =
                    child->data(GameListItemPath::ProgramIdRole).toString().toLower();

                // Only items which filename in combination with its title contains all words
                // that are in the searchfield will be visible in the gamelist
                // The search is case insensitive because of toLower()
                // I decided not to use Qt::CaseInsensitive in containsAllWords to prevent
                // multiple conversions of edit_filter_text for each game in the gamelist
                const QString file_name =
                    file_path.mid(file_path.lastIndexOf(QLatin1Char{'/'}) + 1) + QLatin1Char{' '} +
                    file_long_title;
                if (ContainsAllWords(file_name, edit_filter_text) ||
                    (file_program_id.count() == 16 && edit_filter_text.contains(file_program_id))) {
                    tree_view->setRowHidden(j, folder_index, false);
                    ++result_count;
                } else {
                    tree_view->setRowHidden(j, folder_index, true);
                }
                search_field->setFilterResult(result_count, children_total);
            }
        }
    }
}

void GameList::onUpdateThemedIcons() {
    for (int i = 0; i < item_model->invisibleRootItem()->rowCount(); i++) {
        QStandardItem* child = item_model->invisibleRootItem()->child(i);

        switch (child->data(GameListItem::TypeRole).value<GameListItemType>()) {
        case GameListItemType::InstalledDir:
            child->setData(QIcon::fromTheme(QStringLiteral("sd_card")).pixmap(48),
                           Qt::DecorationRole);
            break;
        case GameListItemType::SystemDir:
            child->setData(QIcon::fromTheme(QStringLiteral("chip")).pixmap(48), Qt::DecorationRole);
            break;
        case GameListItemType::CustomDir: {
            const UISettings::GameDir* game_dir =
                child->data(GameListDir::GameDirRole).value<UISettings::GameDir*>();
            const QString icon_name = QFileInfo::exists(game_dir->path)
                                          ? QStringLiteral("folder")
                                          : QStringLiteral("bad_folder");
            child->setData(QIcon::fromTheme(icon_name).pixmap(48), Qt::DecorationRole);
            break;
        }
        case GameListItemType::AddDir:
            child->setData(QIcon::fromTheme(QStringLiteral("plus")).pixmap(48), Qt::DecorationRole);
            break;
        }
    }
}

void GameList::onFilterCloseClicked() {
    main_window->filterBarSetChecked(false);
}

GameList::GameList(GMainWindow* parent) : QWidget(parent) {
    this->main_window = parent;
    layout = new QVBoxLayout;
    tree_view = new QTreeView;
    search_field = new GameListSearchField(this);
    item_model = new QStandardItemModel(tree_view);
    tree_view->setModel(item_model);

    tree_view->setAlternatingRowColors(true);
    tree_view->setSelectionMode(QHeaderView::SingleSelection);
    tree_view->setSelectionBehavior(QHeaderView::SelectRows);
    tree_view->setVerticalScrollMode(QHeaderView::ScrollPerPixel);
    tree_view->setHorizontalScrollMode(QHeaderView::ScrollPerPixel);
    tree_view->setSortingEnabled(true);
    tree_view->setEditTriggers(QHeaderView::NoEditTriggers);
    tree_view->setContextMenuPolicy(Qt::CustomContextMenu);
    tree_view->setStyleSheet(QStringLiteral("QTreeView { border: none }"));

    item_model->insertColumns(0, COLUMN_COUNT);
    item_model->setHeaderData(COLUMN_NAME, Qt::Horizontal, QStringLiteral("Name"));
    item_model->setHeaderData(COLUMN_REGION, Qt::Horizontal, QStringLiteral("Region"));
    item_model->setHeaderData(COLUMN_FILE_TYPE, Qt::Horizontal, QStringLiteral("File type"));
    item_model->setHeaderData(COLUMN_SIZE, Qt::Horizontal, QStringLiteral("Size"));
    item_model->setSortRole(GameListItemPath::SortRole);

    connect(main_window, &GMainWindow::UpdateThemedIcons, this, &GameList::onUpdateThemedIcons);
    connect(tree_view, &QTreeView::activated, this, &GameList::ValidateEntry);
    connect(tree_view, &QTreeView::customContextMenuRequested, this, &GameList::PopupContextMenu);
    connect(tree_view, &QTreeView::expanded, this, &GameList::onItemExpanded);
    connect(tree_view, &QTreeView::collapsed, this, &GameList::onItemExpanded);

    // We must register all custom types with the Qt Automoc system so that we are able to use
    // it with signals/slots. In this case, QList falls under the umbrells of custom types.
    qRegisterMetaType<QList<QStandardItem*>>("QList<QStandardItem*>");

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(tree_view);
    layout->addWidget(search_field);
    setLayout(layout);
}

GameList::~GameList() {
    emit ShouldCancelWorker();
}

void GameList::setFilterFocus() {
    if (tree_view->model()->rowCount() > 0) {
        search_field->setFocus();
    }
}

void GameList::setFilterVisible(bool visibility) {
    search_field->setVisible(visibility);
}

void GameList::SetDirectoryWatcherEnabled(bool enabled) {
    if (enabled) {
        watcher = std::make_unique<QFileSystemWatcher>(this);
        connect(watcher.get(), &QFileSystemWatcher::directoryChanged, this,
                &GameList::RefreshGameDirectory, Qt::UniqueConnection);
    } else {
        watcher.reset();
    }
}

void GameList::clearFilter() {
    search_field->clear();
}

void GameList::AddDirEntry(GameListDir* entry_items) {
    item_model->invisibleRootItem()->appendRow(entry_items);
    tree_view->setExpanded(
        entry_items->index(),
        entry_items->data(GameListDir::GameDirRole).value<UISettings::GameDir*>()->expanded);
}

void GameList::AddEntry(const QList<QStandardItem*>& entry_items, GameListDir* parent) {
    parent->appendRow(entry_items);
}

void GameList::ValidateEntry(const QModelIndex& item) {
    const QModelIndex selected = item.sibling(item.row(), 0);

    switch (selected.data(GameListItem::TypeRole).value<GameListItemType>()) {
    case GameListItemType::Game: {
        const QString file_path = selected.data(GameListItemPath::FullPathRole).toString();
        if (file_path.isEmpty()) {
            return;
        }
        const QFileInfo file_info(file_path);
        if (!file_info.exists() || file_info.isDir()) {
            return;
        }
        // Users usually want to run a different game after closing one
        search_field->clear();
        emit GameChosen(file_path);
        break;
    }
    case GameListItemType::AddDir:
        emit AddDirectory();
        break;
    }
}

bool GameList::isEmpty() const {
    for (int i = 0; i < item_model->rowCount(); i++) {
        const QStandardItem* child = item_model->invisibleRootItem()->child(i);
        const GameListItemType type = static_cast<GameListItemType>(child->type());
        if (!child->hasChildren() &&
            (type == GameListItemType::InstalledDir || type == GameListItemType::SystemDir)) {
            item_model->invisibleRootItem()->removeRow(child->row());
            i--;
        };
    }
    return !item_model->invisibleRootItem()->hasChildren();
}

void GameList::DonePopulating(QStringList watch_list) {
    emit ShowList(!isEmpty());

    item_model->invisibleRootItem()->appendRow(new GameListAddDir());

    // Clear out the old directories to watch for changes and add the new ones
    const QStringList watch_dirs = watcher->directories();
    if (!watch_dirs.isEmpty()) {
        watcher->removePaths(watch_dirs);
    }
    // Workaround: Add the watch paths in chunks to allow the gui to refresh
    // This prevents the UI from stalling when a large number of watch paths are added
    // Also artificially caps the watcher to a certain number of directories
    constexpr int LIMIT_WATCH_DIRECTORIES = 5000;
    constexpr int SLICE_SIZE = 25;
    const int len = std::min(watch_list.length(), LIMIT_WATCH_DIRECTORIES);
    for (int i = 0; i < len; i += SLICE_SIZE) {
        watcher->addPaths(watch_list.mid(i, i + SLICE_SIZE));
        QCoreApplication::processEvents();
    }
    tree_view->setEnabled(true);
    const int folderCount = tree_view->model()->rowCount();
    int children_total = 0;
    for (int i = 0; i < folderCount; ++i) {
        children_total += item_model->item(i, 0)->rowCount();
    }
    search_field->setFilterResult(children_total, children_total);
    if (children_total > 0) {
        search_field->setFocus();
    }

    emit PopulatingCompleted();
}

void GameList::PopupContextMenu(const QPoint& menu_location) {
    const QModelIndex item = tree_view->indexAt(menu_location);
    if (!item.isValid()) {
        return;
    }

    const QModelIndex selected = item.sibling(item.row(), 0);
    QMenu context_menu;
    switch (selected.data(GameListItem::TypeRole).value<GameListItemType>()) {
    case GameListItemType::Game:
        AddGamePopup(context_menu, selected.data(GameListItemPath::FullPathRole).toString(),
                     selected.data(GameListItemPath::ProgramIdRole).toULongLong(),
                     selected.data(GameListItemPath::ExtdataIdRole).toULongLong());
        break;
    case GameListItemType::CustomDir:
        AddPermDirPopup(context_menu, selected);
        AddCustomDirPopup(context_menu, selected);
        break;
    case GameListItemType::InstalledDir:
    case GameListItemType::SystemDir:
        AddPermDirPopup(context_menu, selected);
        break;
    }
    context_menu.exec(tree_view->viewport()->mapToGlobal(menu_location));
}

void GameList::AddGamePopup(QMenu& context_menu, const QString& path, u64 program_id,
                            u64 extdata_id) {
    context_menu.addAction(QStringLiteral("Start Using Game Settings"), [this, path] {
        emit Hide();

        QtConcurrent::run([path] {
            const std::string& config_dir = FileUtil::GetUserPath(FileUtil::UserPath::ConfigDir);
            const std::string general_config_path = config_dir + DIR_SEP + "qt-config.ini";
            const std::string game_config_path = path.toStdString() + ".cv-ini";

            std::string general_config;
            FileUtil::ReadFileToString(true, general_config_path, general_config);

            if (FileUtil::Exists(game_config_path)) {
                FileUtil::Copy(game_config_path, general_config_path);
            } else {
                FileUtil::WriteStringToFile(true, general_config_path, "");
            }

            const bool show_console = UISettings::values.show_console;
            UISettings::values.show_console = false;
            Debugger::ToggleConsole();
            QProcess::execute(QCoreApplication::applicationFilePath(), QStringList() << path);
            UISettings::values.show_console = show_console;

            std::string game_config;
            FileUtil::ReadFileToString(true, general_config_path, game_config);
            FileUtil::WriteStringToFile(true, game_config_path, game_config);

            FileUtil::WriteStringToFile(true, general_config_path, general_config);
            QCoreApplication::quit();
        });
    });

    const std::string sdmc_dir = FileUtil::GetUserPath(FileUtil::UserPath::SDMCDir);
    const bool is_application =
        0x0004000000000000 <= program_id && program_id <= 0x00040000FFFFFFFF;

    if (is_application && FileUtil::Exists(FileSys::ArchiveSource_SDSaveData::GetSaveDataPathFor(
                              sdmc_dir, program_id))) {
        context_menu.addAction(QStringLiteral("Open Save Data Location"), [this, program_id] {
            emit OpenFolderRequested(program_id, GameListOpenTarget::SAVE_DATA);
        });
    }

    if (extdata_id && is_application &&
        FileUtil::Exists(FileSys::GetExtDataPathFromId(sdmc_dir, extdata_id))) {
        context_menu.addAction(QStringLiteral("Open Extra Data Location"), [this, extdata_id] {
            emit OpenFolderRequested(extdata_id, GameListOpenTarget::EXT_DATA);
        });
    }

    const Service::FS::MediaType media_type = Service::AM::GetTitleMediaType(program_id);
    const bool is_installed =
        path.toStdString() == Service::AM::GetTitleContentPath(media_type, program_id);

    if (is_installed) {
        context_menu.addAction(QStringLiteral("Open Application Location"), [this, program_id] {
            emit OpenFolderRequested(program_id, GameListOpenTarget::APPLICATION);
        });
    }

    if (is_application && FileUtil::Exists(Service::AM::GetTitlePath(Service::FS::MediaType::SDMC,
                                                                     program_id + 0xe00000000) +
                                           "content/")) {
        context_menu.addAction(QStringLiteral("Open Update Data Location"), [this, program_id] {
            emit OpenFolderRequested(program_id, GameListOpenTarget::UPDATE_DATA);
        });
    }

    if (program_id != 0) {
        context_menu.addAction(QStringLiteral("Open Texture Dump Location"), [this, program_id] {
            if (FileUtil::CreateFullPath(
                    fmt::format("{}textures/{:016X}/",
                                FileUtil::GetUserPath(FileUtil::UserPath::DumpDir), program_id))) {
                emit OpenFolderRequested(program_id, GameListOpenTarget::TEXTURE_DUMP);
            }
        });
        context_menu.addAction(QStringLiteral("Open Custom Texture Location"), [this, program_id] {
            if (FileUtil::CreateFullPath(
                    fmt::format("{}textures/{:016X}/",
                                FileUtil::GetUserPath(FileUtil::UserPath::LoadDir), program_id))) {
                emit OpenFolderRequested(program_id, GameListOpenTarget::TEXTURE_LOAD);
            }
        });
    }

    if (is_installed) {
        context_menu.addAction(QStringLiteral("Delete Application"), [=] {
            const bool directory_watcher_was_enabled = watcher != nullptr;

            if (directory_watcher_was_enabled) {
                SetDirectoryWatcherEnabled(false);
            }

            FileUtil::DeleteDirRecursively(Service::AM::GetTitlePath(media_type, program_id));

            if (directory_watcher_was_enabled) {
                SetDirectoryWatcherEnabled(true);
                PopulateAsync(UISettings::values.game_dirs);
            }
        });
    }
};

void GameList::AddCustomDirPopup(QMenu& context_menu, QModelIndex selected) {
    UISettings::GameDir& game_dir =
        *selected.data(GameListDir::GameDirRole).value<UISettings::GameDir*>();

    QAction* deep_scan =
        context_menu.addAction(QStringLiteral("Scan Subfolders"), [this, &game_dir] {
            game_dir.deep_scan = !game_dir.deep_scan;
            emit SettingsChanged();
            PopulateAsync(UISettings::values.game_dirs);
        });

    deep_scan->setCheckable(true);
    deep_scan->setChecked(game_dir.deep_scan);

    context_menu.addAction(QStringLiteral("Remove Game Directory"), [this, &game_dir, selected] {
        UISettings::values.game_dirs.removeOne(game_dir);
        emit SettingsChanged();
        item_model->invisibleRootItem()->removeRow(selected.row());
    });
}

void GameList::AddPermDirPopup(QMenu& context_menu, QModelIndex selected) {
    UISettings::GameDir& game_dir =
        *selected.data(GameListDir::GameDirRole).value<UISettings::GameDir*>();

    const int row = selected.row();

    if (row > 0) {
        context_menu.addAction(u8"\U000025b2 Move Up", [this, selected, row, &game_dir] {
            // find the indices of the items in settings and swap them
            std::swap(UISettings::values.game_dirs[UISettings::values.game_dirs.indexOf(game_dir)],
                      UISettings::values.game_dirs[UISettings::values.game_dirs.indexOf(
                          *selected.sibling(row - 1, 0)
                               .data(GameListDir::GameDirRole)
                               .value<UISettings::GameDir*>())]);
            // move the treeview items
            QList<QStandardItem*> item = item_model->takeRow(row);
            item_model->invisibleRootItem()->insertRow(row - 1, item);
            tree_view->setExpanded(selected, game_dir.expanded);
        });
    }

    if (row < item_model->rowCount() - 2) {
        context_menu.addAction(u8"\U000025bc Move Down ", [this, selected, row, &game_dir] {
            // find the indices of the items in settings and swap them
            std::swap(UISettings::values.game_dirs[UISettings::values.game_dirs.indexOf(game_dir)],
                      UISettings::values.game_dirs[UISettings::values.game_dirs.indexOf(
                          *selected.sibling(row + 1, 0)
                               .data(GameListDir::GameDirRole)
                               .value<UISettings::GameDir*>())]);
            // move the treeview items
            const QList<QStandardItem*> item = item_model->takeRow(row);
            item_model->invisibleRootItem()->insertRow(row + 1, item);
            tree_view->setExpanded(selected, game_dir.expanded);
        });
    }

    context_menu.addAction(QStringLiteral("Open Directory Location"),
                           [this, game_dir] { emit OpenDirectory(game_dir.path); });
}

QStandardItemModel* GameList::GetModel() const {
    return item_model;
}

void GameList::PopulateAsync(QVector<UISettings::GameDir>& game_dirs) {
    tree_view->setEnabled(false);
    // Delete any rows that might already exist if we're repopulating
    item_model->removeRows(0, item_model->rowCount());
    search_field->clear();

    emit ShouldCancelWorker();

    GameListWorker* worker = new GameListWorker(game_dirs);

    connect(worker, &GameListWorker::EntryReady, this, &GameList::AddEntry, Qt::QueuedConnection);
    connect(worker, &GameListWorker::DirEntryReady, this, &GameList::AddDirEntry,
            Qt::QueuedConnection);
    connect(worker, &GameListWorker::Finished, this, &GameList::DonePopulating,
            Qt::QueuedConnection);
    // Use DirectConnection here because worker->Cancel() is thread-safe and we want it to
    // cancel without delay.
    connect(this, &GameList::ShouldCancelWorker, worker, &GameListWorker::Cancel,
            Qt::DirectConnection);

    QThreadPool::globalInstance()->start(worker);
    current_worker = std::move(worker);
}

void GameList::SaveInterfaceLayout() {
    UISettings::values.gamelist_header_state = tree_view->header()->saveState();
}

void GameList::LoadInterfaceLayout() {
    QHeaderView* header = tree_view->header();
    if (!header->restoreState(UISettings::values.gamelist_header_state)) {
        // We are using the name column to display icons and titles
        // so make it as large as possible as default.
        header->resizeSection(COLUMN_NAME, header->width());
    }

    item_model->sort(header->sortIndicatorSection(), header->sortIndicatorOrder());
}

const QStringList GameList::supported_file_extensions = {
    QStringLiteral("3ds"), QStringLiteral("3dsx"), QStringLiteral("elf"), QStringLiteral("axf"),
    QStringLiteral("cci"), QStringLiteral("cxi"),  QStringLiteral("app")};

void GameList::RefreshGameDirectory() {
    if (!UISettings::values.game_dirs.isEmpty() && current_worker != nullptr) {
        LOG_INFO(Frontend, "Change detected in the games directory. Reloading game list.");
        PopulateAsync(UISettings::values.game_dirs);
    }
}

QString GameList::FindGameByProgramID(u64 program_id) {
    return FindGameByProgramID(item_model->invisibleRootItem(), program_id);
}

QString GameList::FindGameByProgramID(QStandardItem* current_item, u64 program_id) {
    if (current_item->type() == static_cast<int>(GameListItemType::Game) &&
        current_item->data(GameListItemPath::ProgramIdRole).toULongLong() == program_id) {
        return current_item->data(GameListItemPath::FullPathRole).toString();
    } else if (current_item->hasChildren()) {
        for (int child_id = 0; child_id < current_item->rowCount(); child_id++) {
            QString path = FindGameByProgramID(current_item->child(child_id, 0), program_id);
            if (!path.isEmpty()) {
                return path;
            }
        }
    }
    return QString();
}

GameListPlaceholder::GameListPlaceholder(GMainWindow* parent) : QWidget{parent} {
    connect(parent, &GMainWindow::UpdateThemedIcons, this,
            &GameListPlaceholder::onUpdateThemedIcons);

    layout = new QVBoxLayout;
    image = new QLabel;
    text = new QLabel;
    layout->setAlignment(Qt::AlignCenter);
    image->setPixmap(QIcon::fromTheme(QStringLiteral("plus_folder")).pixmap(200));

    text->setText(QStringLiteral("Double-click to add a new folder to the game list"));
    QFont font = text->font();
    font.setPointSize(20);
    text->setFont(font);
    text->setAlignment(Qt::AlignHCenter);
    image->setAlignment(Qt::AlignHCenter);

    layout->addWidget(image);
    layout->addWidget(text);
    setLayout(layout);
}

GameListPlaceholder::~GameListPlaceholder() = default;

void GameListPlaceholder::onUpdateThemedIcons() {
    image->setPixmap(QIcon::fromTheme(QStringLiteral("plus_folder")).pixmap(200));
}

void GameListPlaceholder::mouseDoubleClickEvent(QMouseEvent* event) {
    emit GameListPlaceholder::AddDirectory();
}
