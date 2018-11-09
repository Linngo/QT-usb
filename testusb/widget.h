#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

#include <QDebug>
#include "usbtmcdevice.h"
#include "qUSBListener.h"
#include <QButtonGroup>

#include <QTimer>

class QFile;

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

signals:
    void testsend(qint64);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private slots:
    void USBConnect(usbDevice dev);
    void USBDisconnect(usbDevice dev);
    void on_send_clicked();
    void on_open_clicked();

    bool openFile(QString &fileName);
    void send();
    bool startTransfer();
    void updateClientProgress(qint64);

    void handleTimeout();

    void on_pushButton_clicked();

private:
    Ui::Widget *ui;
    UsbTmcDevice *tmcDevice;
    qUSBListener usbAlert;

    QFile *localFile;
    qint64 totalBytes;
    qint64 bytesWritten;
    qint64 bytesToWrite;
    qint64 payloadSize;
    QString fileName;
    QByteArray outBlock;

    QButtonGroup *group;
    QTimer *timer;
};

#endif // WIDGET_H
