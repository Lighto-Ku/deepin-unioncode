/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BUILDMANAGER_H
#define BUILDMANAGER_H

#include "services/builder/builderglobals.h"
#include "services/builder/task.h"
#include "services/project/projectinfo.h"
#include "common/widget/outputpane.h"

#include <QObject>
#include <QSharedPointer>
#include <QMutex>

namespace dpfservice {
class WindowService;
}

class CompileOutputPane;
class ProblemOutputPane;
class BuildManagerPrivate;
class BuildManager : public QObject
{
    Q_OBJECT
public:
    static BuildManager *instance();

    CompileOutputPane *getCompileOutputPane() const;
    ProblemOutputPane *getProblemOutputPane() const;

    void setActivedProjectInfo(const QString &kitName, const QString &workingDir);
    void clearActivedProjectInfo();

    void handleCommand(const QList<BuildCommandInfo> &info, bool isSynchronous);

signals:
    void sigOutputCompileInfo(const QString &content, const OutputPane::OutputFormat format);
    void sigOutputProblemInfo(const QString &content);
    void sigBuildState(const BuildState &buildstate);
    void sigOutputNotify(const BuildState &state, const BuildCommandInfo &commandInfo);
    void sigResetBuildUI();

public slots:
    void slotOutputCompileInfo(const QString &content, const OutputPane::OutputFormat format);
    void slotOutputProblemInfo(const QString &content);
    void addOutput(const QString &content, const OutputPane::OutputFormat format);
    void buildProject();
    void rebuildProject();
    void cleanProject();
    void slotBuildState(const BuildState &buildstate);
    void slotOutputNotify(const BuildState &state, const BuildCommandInfo &commandInfo);
    void slotResetBuildUI();

private:
    explicit BuildManager(QObject *parent = nullptr);
    virtual ~BuildManager();

    void addMenu();

    void outputLog(const QString &content, const OutputPane::OutputFormat format);
    void outputError(const QString &content);
    void outputNotify(const BuildState &state, const BuildCommandInfo &commandInfo);

    bool execCommands(const QList<BuildCommandInfo> &commandList, bool isSynchronous);
    bool execCommand(const BuildCommandInfo &info);

    void execBuildStep(QList<BuildMenuType> menuTypelist);
    void outBuildState(const BuildState &buildState);
    bool canStartBuild();

    QMutex releaseMutex;

    BuildManagerPrivate *const d;
};

#endif // BUILDMANAGER_H
