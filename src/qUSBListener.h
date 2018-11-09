#ifndef QUSBLISTENER_H
#define QUSBLISTENER_H

#include <QWidget>
#include <QVector>
#include <windows.h>
#include <dbt.h>
#include "usbdevice.h"
//#include <QAbstractNativeEventFilter>

class qUSBListener : public QWidget
{
    Q_OBJECT
public:
    qUSBListener();
    bool start(const uint16_t vid = 0, const uint16_t pid = 0, const QString sn = "");
    bool stop();
    QVector<usbDevice> getDevList();

signals:
    void USBConnected(usbDevice name);
    void USBDisconnected(usbDevice name);
    void PortConnected(QString portName);
    void PortDisconnected(QString portName);

protected:
    virtual bool nativeEvent(const QByteArray & eventType,
                             void * message,
                             long * result);

    bool getDevData(LPARAM lParamDev, usbDevice &newDevice);
    inline QString getQstring(void* stringPtr);

private:
    HDEVNOTIFY      devNotify;
    usbDevice*  targetDev;
};

#endif // QUSBLISTENER_H
