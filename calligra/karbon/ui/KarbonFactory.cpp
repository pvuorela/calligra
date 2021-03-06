/* This file is part of the KDE project
 * Copyright (C) 2001-2003 Lennart Kudling <kudling@kde.org>
 * Copyright (C) 2002-2003 Rob Buis <buis@kde.org>
 * Copyright (C) 2002 Tomislav Lukman <tomislav.lukman@ck.t-com.hr>
 * Copyright (C) 2002,2004-2005,2007 David Faure <faure@kde.org>
 * Copyright (C) 2002 Benoît Vautrin <benoit.vautrin@free.fr>
 * Copyright (C) 2003 Lukáš Tinkl <lukas@kde.org>
 * Copyright (C) 2004,2006 Laurent Montel <montel@kde.org>
 * Copyright (C) 2005-2006 Tim Beaulen <tbscope@gmail.com>
 * Copyright (C) 2005,2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2007 Matthias Kretz <kretz@kde.org>
 * Copyright (C) 2007 Stephan Kulow <coolo@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KarbonFactory.h"

#include "KarbonPart.h"
#include "KarbonDocument.h"
#include "KarbonAboutData.h"

#include <kaboutdata.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <kdebug.h>

KComponentData* KarbonFactory::s_instance = 0L;
KAboutData* KarbonFactory::s_aboutData = 0L;

KarbonFactory::KarbonFactory(QObject* parent)
        : KPluginFactory(*aboutData(), parent)
{
    componentData();
}

KarbonFactory::~KarbonFactory()
{
    delete s_instance;
    s_instance = 0L;
}

QObject* KarbonFactory::create(const char* /*iface*/, QWidget* /*parentWidget*/, QObject *parent, const QVariantList& args, const QString& keyword)
{
    Q_UNUSED(args);
    Q_UNUSED(keyword);

    KarbonPart *part = new KarbonPart(parent);
    KarbonDocument* doc = new KarbonDocument(part);
    part->setDocument(doc);;
    return part;
}

KAboutData * KarbonFactory::aboutData()
{
    if (!s_aboutData)
        s_aboutData = newKarbonAboutData();
    return s_aboutData;
}

const KComponentData &KarbonFactory::componentData()
{
    if (!s_instance) {
        s_instance = new KComponentData(aboutData());
        // Add any application-specific resource directories here

        s_instance->dirs()->addResourceType("kis_brushes", "data", "krita/brushes/");
        s_instance->dirs()->addResourceType("kis_pattern", "data", "krita/patterns/");
        s_instance->dirs()->addResourceType("karbon_gradient", "data", "krita/gradients/");
        s_instance->dirs()->addResourceType("karbon_clipart", "data", "karbon/cliparts/");
        s_instance->dirs()->addResourceType("karbon_template", "data", "karbon/templates/");
        s_instance->dirs()->addResourceType("karbon_effects", "data", "karbon/effects/");
        // Tell the iconloader about share/apps/calligra/icons
        KIconLoader::global()->addAppDir("calligra");
    }

    return *s_instance;
}

#include "KarbonFactory.moc"

