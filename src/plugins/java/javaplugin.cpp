/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
 *             hongjinchuan<hongjinchuan@uniontech.com>
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
#include "javaplugin.h"

#include "maven/mavengenerator.h"
#include "gradle/gradlegenerator.h"

#include "services/language/languageservice.h"

using namespace dpfservice;

void JavaPlugin::initialize()
{
    qInfo() << __FUNCTION__;
    auto &ctx = dpfInstance.serviceContext();
    LanguageService *languageService = ctx.service<LanguageService>(LanguageService::name());
    if (languageService) {
        QString errorString;
        bool ret = languageService->regClass<MavenGenerator>(MavenGenerator::toolKitName(), &errorString);
        if (!ret) {
            qCritical() << errorString;
        } else {
            ret = languageService->create<MavenGenerator>(MavenGenerator::toolKitName(), &errorString);
            if (!ret) {
                qCritical() << errorString;
            }
        }

        ret = languageService->regClass<GradleGenerator>(GradleGenerator::toolKitName(), &errorString);
        if (!ret) {
            qCritical() << errorString;
        } else {
            ret = languageService->create<GradleGenerator>(GradleGenerator::toolKitName(), &errorString);
            if (!ret) {
                qCritical() << errorString;
            }
        }
    }
}

bool JavaPlugin::start()
{
    qInfo() << __FUNCTION__;
    return true;
}

dpf::Plugin::ShutdownFlag JavaPlugin::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}

