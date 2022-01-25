#include "workspacedata.h"
#include "workspaceobject.h"
#include "common/util/custompaths.h"
#include "common/util/supportfile.h"

#include <QSet>
#include <QFileInfo>
#include <QFile>
#include <QDir>

class WorkspaceDataPrivate
{
    friend class WorkspaceData;
    QSet<QString> workspaceFolders;
};

WorkspaceData::WorkspaceData(QObject *parent)
    : QObject(parent)
    , d(new WorkspaceDataPrivate)
{

}

WorkspaceData::~WorkspaceData()
{
    if (d) {
        if (!d->workspaceFolders.isEmpty())
            d->workspaceFolders.clear();
        delete d;
    }
}

WorkspaceData *WorkspaceData::globalInstance() {
    static WorkspaceData ins;
    return &ins;
}

void WorkspaceData::addWorkspace(const QString &dirPath)
{
    d->workspaceFolders.insert(dirPath);
}

void WorkspaceData::delWorkspace(const QString &dirPath)
{
    d->workspaceFolders.remove(dirPath);
}

QStringList WorkspaceData::findWorkspace(const QString &filePath)
{
    QStringList result;
    QFileInfo info(filePath);
    if (info.isFile()) {
        info = QFileInfo(info.dir().absolutePath());
    }

    if (info.isDir()) {
        if (d->workspaceFolders.contains(info.filePath())) {
            result << info.filePath();
        }

        QDir dir(info.filePath());
        while(!dir.cdUp()) {
            if (d->workspaceFolders.contains(dir.absolutePath())) {
                result << dir.absolutePath();
            }
        }
    }
    return result;
}

QString WorkspaceData::targetPath(const QString &dirPath)
{
    if (QFileInfo(dirPath).isDir()) {
        if (dirPath.endsWith(QDir::separator())) {
            return dirPath + ".unioncode";
        } else {
            return dirPath + QDir::separator()+ ".unioncode";
        }
    }
    return "";
}

/*!
 * \brief WorkspaceData::doGenerate
 * \param dirPath
 *  这里将产生三条不同的分支逻辑
 *  1. 即有一个工程被扫描到则支持一种
 *  2. 多种工程需要用户进行选择，建议使用分组checkbox更为直观
 *  3. 如果没有相关工程支持，则将其视为文件浏览进行打开
 */
void WorkspaceData::doGenerate(const QString &dirPath)
{
    auto buildInfos = SupportFile::Builder::buildInfos(dirPath);

    if (buildInfos.size() == 1) {
        qInfo() << "build system: " << buildInfos[0].buildSystem;
        qInfo() << "project system: " << buildInfos[0].projectPath;
    } else if (buildInfos.size() > 1) {
        qInfo() << "need to select project do generate: ";
        for (auto val : buildInfos) {
            qInfo() << "    build system: " << val.buildSystem;
            qInfo() << "    project system: " << val.projectPath;
        }
    } else {
        qInfo() << "can't found any project support, open show file browser: "
                << dirPath;
    }
}