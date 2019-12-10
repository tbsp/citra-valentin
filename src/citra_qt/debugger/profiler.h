// Copyright 2015 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <QDialog>
#include "common/profiler.h"

namespace Ui {
class Profiler;
} // namespace Ui

class QtProfiler : public QDialog, public Common::Profiler {
    Q_OBJECT

public:
    QtProfiler(QWidget* parent = nullptr);
    ~QtProfiler();

    void Stop();

public slots:
    void Update();

private slots:
    Q_INVOKABLE void Set(const std::string category, const std::string scope,
                         const long long milliseconds) override;

signals:
    void Stopped();

private:
    struct ScopeData {
        int row = -1;
        const std::string category;
        const std::string scope;
        long long current;
        long long minimum;
        long long maximum;
    };

    std::unique_ptr<Ui::Profiler> ui;

    std::mutex mutex;
    std::unordered_map<std::string, ScopeData> current;

    bool stop_requested = false;
    bool stopping = false;
};
