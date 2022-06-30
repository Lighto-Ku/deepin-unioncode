/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
#ifndef PROJECTCMAKEOPEN_H
#define PROJECTCMAKEOPEN_H

#include <QObject>
#include <QAction>

class CmakeOpenHandler : public QObject
{
    Q_OBJECT
public:
    explicit CmakeOpenHandler(QObject *parent = nullptr);
    static CmakeOpenHandler *instance();
    QAction *openAction();

public slots:
    void doProjectOpen(const QString &name, const QString &language,
                       const QString &filePath);
};

#endif // PROJECTCMAKEOPEN_H