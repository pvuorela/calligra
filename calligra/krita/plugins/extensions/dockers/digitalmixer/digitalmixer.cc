/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "digitalmixer.h"

#include <stdlib.h>

#include <QTimer>

#include <kactioncollection.h>
#include <kcomponentdata.h>
#include <kis_debug.h>
#include <kpluginfactory.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <KoDockFactoryBase.h>

#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_view2.h"

#include "digitalmixer_dock.h"
#include <KoDockRegistry.h>

K_PLUGIN_FACTORY_WITH_JSON(DigitalMixerPluginFactory, "krita_digitalmixer.json", registerPlugin<DigitalMixerPlugin>();)
//K_EXPORT_PLUGIN(DigitalMixerPluginFactory( "krita" ) )

class DigitalMixerDockFactory : public KoDockFactoryBase {
public:
    DigitalMixerDockFactory()
    {
    }

    virtual QString id() const
    {
        return QString( "DigitalMixer" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    virtual QDockWidget* createDockWidget()
    {
        DigitalMixerDock * dockWidget = new DigitalMixerDock();
        
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const
    {
        return DockMinimized;
    }
private:


};


DigitalMixerPlugin::DigitalMixerPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new DigitalMixerDockFactory());
}

DigitalMixerPlugin::~DigitalMixerPlugin()
{
    m_view = 0;
}

#include "digitalmixer.moc"
