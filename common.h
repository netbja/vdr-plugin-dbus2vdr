#ifndef __DBUS2VDR_COMMON_H
#define __DBUS2VDR_COMMON_H

#define DBUS_VDR_BUSNAME                  "de.tvdr.vdr"
#define DBUS_VDR_CHANNEL_INTERFACE        DBUS_VDR_BUSNAME".channel"
#define DBUS_VDR_EPG_INTERFACE            DBUS_VDR_BUSNAME".epg"
#define DBUS_VDR_OSD_INTERFACE            DBUS_VDR_BUSNAME".osd"
#define DBUS_VDR_PLUGIN_INTERFACE         DBUS_VDR_BUSNAME".plugin"
#define DBUS_VDR_PLUGINMANAGER_INTERFACE  DBUS_VDR_BUSNAME".pluginmanager"
#define DBUS_VDR_RECORDING_INTERFACE      DBUS_VDR_BUSNAME".recording"
#define DBUS_VDR_REMOTE_INTERFACE         DBUS_VDR_BUSNAME".remote"
#define DBUS_VDR_SETUP_INTERFACE          DBUS_VDR_BUSNAME".setup"
#define DBUS_VDR_SHUTDOWN_INTERFACE       DBUS_VDR_BUSNAME".shutdown"
#define DBUS_VDR_SKIN_INTERFACE           DBUS_VDR_BUSNAME".skin"
#define DBUS_VDR_STATUS_INTERFACE         DBUS_VDR_BUSNAME".status"
#define DBUS_VDR_TIMER_INTERFACE          DBUS_VDR_BUSNAME".timer"
#define DBUS_VDR_VDR_INTERFACE            DBUS_VDR_BUSNAME".vdr"

enum eBusType { busSystem = 0, busNetwork = 1};

#endif
