#include "mavenasynparse.h"
#include "mavenitemkeeper.h"
#include "services/project/projectgenerator.h"

#include "common/common.h"

#include <QAction>
#include <QDebug>
#include <QtXml>

class MavenAsynParsePrivate
{
    friend  class MavenAsynParse;
    QDomDocument xmlDoc;
    QThread *thread {nullptr};
    QString rootPath;
    QList<QStandardItem *> rows {};
};

MavenAsynParse::MavenAsynParse()
    : d(new MavenAsynParsePrivate)
{
    QObject::connect(Inotify::globalInstance(), &Inotify::modified,
                     this, &MavenAsynParse::doDirWatchModify,
                     Qt::DirectConnection);
    QObject::connect(Inotify::globalInstance(), &Inotify::createdSub,
                     this, &MavenAsynParse::doWatchCreatedSub,
                     Qt::DirectConnection);
    QObject::connect(Inotify::globalInstance(), &Inotify::deletedSub,
                     this, &MavenAsynParse::doWatchDeletedSub,
                     Qt::DirectConnection);

    d->thread = new QThread();
    this->moveToThread(d->thread);
}

MavenAsynParse::~MavenAsynParse()
{
    if (d) {
        if (d->thread) {
            if (d->thread->isRunning())
                d->thread->quit();
            while (d->thread->isFinished());
            delete d->thread;
            d->thread = nullptr;
        }
        delete d;
    }
}

void MavenAsynParse::loadPoms(const dpfservice::ProjectInfo &info)
{
    QFile docFile(info.projectFilePath());

    if (!docFile.exists()) {
        parsedError({"Failed, maven pro not exists!: " + docFile.fileName(), false});
    }

    if (!docFile.open(QFile::OpenModeFlag::ReadOnly)) {;
        parsedError({docFile.errorString(), false});
    }

    if (!d->xmlDoc.setContent(&docFile)) {
        docFile.close();
    }
    docFile.close();
}

void MavenAsynParse::parseProject(const dpfservice::ProjectInfo &info)
{
    using namespace dpfservice;
    ParseInfo<QList<QStandardItem*>> itemsResult;
    itemsResult.result = generatorChildItem(info.sourceFolder());
    itemsResult.isNormal = true;
    emit parsedProject(itemsResult);
}

void MavenAsynParse::parseActions(const dpfservice::ProjectInfo &info)
{
    using namespace dpfservice;
    ProjectInfo proInfo = info;
    if (proInfo.isEmpty()) {
        return;
    }

    QFileInfo xmlFileInfo(proInfo.projectFilePath());
    if (!xmlFileInfo.exists())
        return;

    d->xmlDoc = QDomDocument(xmlFileInfo.fileName());
    QFile xmlFile(xmlFileInfo.filePath());
    if (!xmlFile.open(QIODevice::ReadOnly)) {
        qCritical() << "Failed, Can't open xml file: "
                    << xmlFileInfo.filePath();
        return;
    }
    if (!d->xmlDoc.setContent(&xmlFile)) {
        qCritical() << "Failed, Can't load to XmlDoc class: "
                    << xmlFileInfo.filePath();
        return;
    }

    QDomNode n = d->xmlDoc.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (!e.isNull() && e.tagName() == "project") {
            n = n.firstChild();
        }

        if (e.tagName() == "dependencies") {
            QDomNode depNode = e.firstChild();
            while (!depNode.isNull()) {
                QDomElement depElem = depNode.toElement();
                if (depElem.tagName() == "dependency") {
                    auto depcyNode = depElem.firstChild();
                    while (!depcyNode.isNull()) {
                        auto depcyElem = depcyNode.toElement();
                        qInfo() << "dependencies.dependency."
                                   + depcyElem.tagName()
                                   + ":" + depcyElem.text();
                        depcyNode = depcyNode.nextSibling();
                    }
                }
                depNode = depNode.nextSibling();
            }
        }

        if (e.tagName() == "build")
        {
            QDomNode buildNode = e.firstChild();
            while (!buildNode.isNull()) {
                auto buildElem = buildNode.toElement();
                QDomNode pmChildNode = buildElem.firstChild();
                if (buildElem.tagName() == "pluginManagement") {
                    while (!pmChildNode.isNull()) {
                        auto pmChildElem = pmChildNode.toElement();
                        QDomNode pluginsChild = pmChildElem.firstChild();
                        ProjectActionInfos actionInfos;
                        if (pmChildElem.tagName() == "plugins") {
                            while (!pluginsChild.isNull()) {
                                auto pluginsElem = pluginsChild.toElement();
                                QDomNode pluginChild = pluginsElem.firstChild();
                                if (pluginsElem.tagName() == "plugin") {
                                    while (!pluginChild.isNull()) {
                                        auto pluginElem = pluginChild.toElement();
                                        if ("artifactId" == pluginElem.tagName()) {
                                            ProjectActionInfo actionInfo;
                                            actionInfo.buildCommand = "mvn";
                                            actionInfo.workingDirectory = xmlFileInfo.filePath();
                                            QString buildArg = pluginElem.text()
                                                    .replace("maven-", "")
                                                    .replace("-plugin", "");
                                            actionInfo.buildArguments.append(buildArg);
                                            actionInfo.displyText = buildArg;
                                            actionInfo.tooltip = pluginElem.text();
                                            actionInfos.append(actionInfo);
                                        }
                                        pluginChild = pluginChild.nextSibling();
                                    }
                                } // pluginsElem.tagName() == "plugin"
                                pluginsChild = pluginsChild.nextSibling();
                            }
                            if (!actionInfos.isEmpty())
                                emit parsedActions({actionInfos, true});
                        } // pmChildElem.tagName() == "plugins"
                        pmChildNode = pmChildNode.nextSibling();
                    }
                } // buildElem.tagName() == "pluginManagement"
                buildNode = buildNode.nextSibling();
            }
        }// e.tagName() == "build"
        n = n.nextSibling();
    }
}

void MavenAsynParse::doDirWatchModify(const QString &path)
{
    //    QString pathTemp = path;
    //    auto changedItem = findItem(d->rows, pathTemp);
}

void MavenAsynParse::doWatchCreatedSub(const QString &path)
{
    //    if (!path.startsWith(d->rootPath))
    //        return;

    //    QString pathTemp = path;
    //    pathTemp = pathTemp.remove(0, d->rootPath.size());
    //    auto parent = findItem(d->rows, pathTemp);
    //    if (pathTemp.startsWith(QDir::separator()))
    //        pathTemp = pathTemp.remove(0, QString(QDir::separator()).size());

    //    auto insertItem = new QStandardItem(CustomIcons::icon(path), pathTemp);
    //    if (!parent) {
    //        int findRow = findRowWithDisplay(d->rows, pathTemp);
    //        if (findRow > 0) {
    //            d->rows.insert(findRow, insertItem);
    //        } else {
    //            d->rows.append(insertItem);
    //        }
    //    } else {
    //        int findRow = findRowWithDisplay(parent, pathTemp);
    //        if (findRow > 0) {
    //            parent->insertRow(findRow, insertItem);
    //        } else {
    //            parent->appendRow(insertItem);
    //        }
    //    }
}

void MavenAsynParse::doWatchDeletedSub(const QString &path)
{
    //    if (!path.startsWith(d->rootPath))
    //        return;

    //    QString pathTemp = path;
    //    pathTemp = pathTemp.remove(0, d->rootPath.size());
    //    auto parent = findItem(d->rows, pathTemp);
    //    if (parent && pathTemp.isEmpty()) {
    //        delete parent;
    //    }
}

bool MavenAsynParse::isSame(QStandardItem *t1, QStandardItem *t2, Qt::ItemDataRole role) const
{
    if (!t1 || !t2)
        return false;
    return t1->data(role) == t2->data(role);
}

QList<QStandardItem *> MavenAsynParse::generatorChildItem(const QString &path) const
{
    QString rootPath = path;
    if (rootPath.endsWith(QDir::separator())) {
        int separatorSize = QString(QDir::separator()).size();
        rootPath = rootPath.remove(rootPath.size() - separatorSize, separatorSize);
    }

    // 缓存当前工程目录
    d->rootPath = rootPath;
    Inotify::globalInstance()->addPath(d->rootPath);
    QList<QStandardItem *> result;

    {// 避免变量冲突 迭代文件夹
        QDir dir;
        dir.setPath(rootPath);
        dir.setFilter(QDir::NoDotAndDotDot | QDir::Dirs);
        dir.setSorting(QDir::Name);
        QDirIterator dirItera(dir, QDirIterator::Subdirectories);
        while (dirItera.hasNext()) {
            Inotify::globalInstance()->addPath(dirItera.filePath());
            QString childPath = dirItera.next().remove(0, rootPath.size());
            QStandardItem *item = findItem(result, childPath);
            QIcon icon = CustomIcons::icon(dirItera.fileInfo());
            auto newItem = new QStandardItem(icon, dirItera.fileName());
            newItem->setToolTip(dirItera.filePath());
            if (!item) {
                result.append(newItem);
            } else {
                item->appendRow(newItem);
            }
        }
    }
    {// 避免变量冲突 迭代文件
        QDir dir;
        dir.setPath(rootPath);
        dir.setFilter(QDir::NoDotAndDotDot | QDir::Files);
        dir.setSorting(QDir::Name);
        QDirIterator fileItera(dir, QDirIterator::Subdirectories);
        while (fileItera.hasNext()) {
            QString childPath = fileItera.next().remove(0, rootPath.size());
            QStandardItem *item = findItem(result, childPath);
            QIcon icon = CustomIcons::icon(fileItera.fileInfo());
            auto newItem = new QStandardItem(icon, fileItera.fileName());
            newItem->setToolTip(fileItera.filePath());
            if (!item) {
                result.append(newItem);
            } else {
                item->appendRow(newItem);
            }
        }
    }
    return result;
}

QString MavenAsynParse::path(QStandardItem *item) const
{
    if (!item)
        return "";

    QStandardItem *currItem = item;
    QString result = item->data(Qt::DisplayRole).toString();
    while (currItem->parent()) {
        result.insert(0, QDir::separator());
        result.insert(0, path(currItem->parent()));
    }
    return result;
}

QList<QStandardItem *> MavenAsynParse::rows(const QStandardItem *item) const
{
    QList<QStandardItem *> result;
    for (int i = 0; i < item->rowCount(); i++) {
        result << item->child(i);
    }
    return result;
}

int MavenAsynParse::findRowWithDisplay(QStandardItem *item, const QString &fileName)
{
    if (item->hasChildren()) {
        for (int i = 0; i < d->rows.size(); i++) {
            if (item->child(i)->data(Qt::DisplayRole) < fileName) {
                return i;
            }
        }
    }
    return -1;
}

int MavenAsynParse::findRowWithDisplay(QList<QStandardItem *> rowList, const QString &fileName)
{
    if (rowList.size() > 0) {
        for (int i = 0; i < d->rows.size(); i++) {
            if (rowList[i]->data(Qt::DisplayRole) < fileName) {
                return i;
            }
        }
    }
    return -1;
}

QStandardItem *MavenAsynParse::findItem(QList<QStandardItem *> rowList,
                                        QString &path,
                                        QStandardItem *parent) const
{
    if (path.endsWith(QDir::separator())) {
        int separatorSize = QString(QDir::separator()).size();
        path = path.remove(path.size() - separatorSize, separatorSize);
    }

    for (int i = 0; i < rowList.size(); i++) {
        QString pathSplit = QDir::separator() + rowList[i]->data(Qt::DisplayRole).toString();
        if (QFileInfo(rowList[i]->toolTip()).isDir() && path.startsWith(pathSplit)) {
            path = path.remove(0, pathSplit.size());
            return findItem(rows(rowList[i]), path, rowList[i]);
        }
    }

    return parent;
}