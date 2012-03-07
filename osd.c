#include "osd.h"

#include "common.h"
#include "monitor.h"

#include <sys/time.h>

#include <png++/image.hpp>
#include <png++/rgba_pixel.hpp>

#include <vdr/videodir.h>

#define DBUSOSDDIR "/tmp/dbus2vdr"
//#define DBUSOSDDIR VideoDirectory


int   cDBusOsd::osd_number = 0;

cDBusOsd::cDBusOsd(cDBusOsdProvider& Provider, int Left, int Top, uint Level)
 :cOsd(Left, Top, Level)
 ,provider(Provider)
 ,osd_index(osd_number++)
 ,counter(0)
{
  osd_dir = cString::sprintf("%s/dbusosd-%04x", DBUSOSDDIR, osd_index);
  if (!MakeDirs(*osd_dir, true))
     esyslog("dbus2vdr: can't create %s", *osd_dir);
  provider.SendMessage(new cDbusOsdMsg("Open", osd_dir, Left, Top, 0, 0));
}

cDBusOsd::~cDBusOsd()
{
  provider.SendMessage(new cDbusOsdMsg("Close", osd_dir, 0, 0, 0, 0));
}

void cDBusOsd::Flush(void)
{
  if (!cOsd::Active())
      return;

  struct timeval start;
  struct timeval end;
  struct timezone timeZone;
  gettimeofday(&start, &timeZone);

  bool write = false;
  if (IsTrueColor()) {
    LOCK_PIXMAPS;
    int left = Left();
    int top = Top();
    const cRect* vp;
    int vx, vy, vw, vh;
    int x, y;
    const uint8_t *pixel;
    png::image<png::rgba_pixel> *pngfile;
    while (cPixmapMemory *pm = RenderPixmaps()) {
          write = true;
          vp = &pm->ViewPort();
          vx = vp->X();
          vy = vp->Y();
          vw = vp->Width();
          vh = vp->Height();
          pixel = pm->Data();
          pngfile = new png::image<png::rgba_pixel>(vw, vh);
          for (y = 0; y < vh ; y++) {
              for (x = 0; x < vw; x++) {
                  (*pngfile)[y][x] = png::rgba_pixel(pixel[2], pixel[1], pixel[0], pixel[3]);
                  pixel += 4;
                  }
              }
          cString filename = cString::sprintf("%s/%04x-%d-%d-%d-%d.png", *osd_dir, counter, left, top, vx, vy);
          pngfile->write(*filename);
          provider.SendMessage(new cDbusOsdMsg("Display", filename, left, top, vx, vy));

          counter++;
          delete pngfile;
          delete pm;
          }
  }

  if (write) {
     gettimeofday(&end, &timeZone);
     int timeNeeded = end.tv_usec - start.tv_usec;
     timeNeeded += (end.tv_sec - start.tv_sec) * 1000000;
     isyslog("dbus2vdr: flushing osd %d needed %d\n", osd_index, timeNeeded);
     }
}


cDBusOsdProvider::cDBusOsdProvider(void)
{
  isyslog("dbus2vdr: new osd provider");
  SetDescription("dbus2vdr: osd provider signal");
  Start();
}

cDBusOsdProvider::~cDBusOsdProvider()
{
  isyslog("dbus2vdr: delete osd provider");
  msgCond.Broadcast();
  Cancel(10);
}

cOsd *cDBusOsdProvider::CreateOsd(int Left, int Top, uint Level)
{
  return new cDBusOsd(*this, Left, Top, Level);
}

void cDBusOsdProvider::Action(void)
{
  while (Running() || (msgQueue.Count() > 0)) {
        cDbusOsdMsg *dbmsg = NULL;
        { // for short lock
          cMutexLock MutexLock(&msgMutex);
          dbmsg = msgQueue.First();
          if (dbmsg != NULL)
             msgQueue.Del(dbmsg, false);
        }
        if (dbmsg != NULL) {
           DBusMessage *msg = dbus_message_new_signal("/OSD", DBUS_VDR_OSD_INTERFACE, dbmsg->action);
           if (msg != NULL) {
              DBusMessageIter args;
              dbus_message_iter_init_append(msg, &args);
              const char *file = *dbmsg->file;
              if (dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &file)) {
                 if (dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &dbmsg->left)) {
                    if (dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &dbmsg->top)) {
                       if (dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &dbmsg->vx)) {
                          if (dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &dbmsg->vy)) {
                             if (cDBusMonitor::SendSignal(msg))
                                msg = NULL;
                             }
                          }
                       }
                    }
                 }
              if (msg != NULL)
                 dbus_message_unref(msg);
              }
           delete dbmsg;
           }
        cMutexLock MutexLock(&msgMutex);
        if (msgQueue.Count() == 0)
           msgCond.TimedWait(msgMutex, 1000);
        }
}

void cDBusOsdProvider::SendMessage(cDbusOsdMsg *Msg)
{
  cMutexLock MutexLock(&msgMutex);
  msgQueue.Add(Msg);
  msgCond.Broadcast();
}


cDbusOsdMsg::~cDbusOsdMsg(void)
{
  if (strcmp(action, "Close") == 0) {
     isyslog("dbus2vdr: deleting osd files at %s", *file);
     RemoveFileOrDir(*file, false);
     }
}
