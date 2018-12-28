#include "widget.h"
#include "ui_widget.h"
#include <QMessageBox>

#include <QFileDialog>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
//#include <QtEndian>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
    ,totalBytes(0)
    ,bytesWritten(0)
    ,bytesToWrite(0)
    ,payloadSize(1024)
    ,fileName("")
{
    ui->setupUi(this);
//    this->setWindowTitle("Title");

    tmcDevice = new UsbTmcDevice(0x067B, 0x2305, "");
//    tmcDevice = new UsbTmcDevice(0x0451, 0x16B4, "");

    if (!tmcDevice->open()) {
        ui->label->setText("Device open failed.");
        ui->label->setStyleSheet("QLabel {font-weight: bold; color: red}");
        qDebug() << "Device open failed.";
    }else{
        ui->label->setText("Device connect.");
        ui->label->setStyleSheet("QLabel {font-weight: bold; color: green}");
    }
    usbAlert.start();
    connect(&this->usbAlert, &qUSBListener::USBConnected,
            this, &this->USBConnect);//
    connect(&this->usbAlert, &qUSBListener::USBDisconnected,
            this, &this->USBDisconnect);

//    connect(tmcDevice, SIGNAL(bytesWritten(qint64)),
//            this, SLOT(updateClientProgress(qint64)));
    connect(this, SIGNAL(testsend(qint64)),
            this, SLOT(updateClientProgress(qint64)));

//    ui->progressBar->reset();
//    ui->send->setEnabled(false);
    setAcceptDrops(true);

    group = new QButtonGroup();
    group->addButton(ui->gujian,0);
    group->addButton(ui->ziku,1);
    group->addButton(ui->qita,2);
}

Widget::~Widget()
{
    usbAlert.stop();
    tmcDevice->close();
    delete tmcDevice;
    delete ui;
}

void Widget::USBConnect(usbDevice dev) {
    qDebug()<<"USB DEVICE CONNECTED:"<<dev.VID<<dev.PID<<dev.serialNum;
    if (!tmcDevice->open()) {
        ui->label->setText("Device open failed.");
        ui->label->setStyleSheet("QLabel {font-weight: bold; color: green}");
        qDebug() << "Device open failed.";
    }else{
        ui->label->setText("Device connect.");
        ui->label->setStyleSheet("QLabel {font-weight: bold; color: green}");
    }
}

void Widget::USBDisconnect(usbDevice dev) {
    qDebug()<<"USB DEVICE DISCONNECTED:"<<dev.VID<<dev.PID<<dev.serialNum;
    ui->label->setText("Device disconnect.");
    ui->label->setStyleSheet("QLabel {font-weight: bold; color: red}");
    tmcDevice->close();
}
//转字符串
/*QByteArray HexStringToByteArray(QString HexString)
{
    bool ok;
    QByteArray ret;
    HexString = HexString.trimmed();
    HexString = HexString.simplified();
    QStringList sl = HexString.split(" ");

    foreach (QString s, sl) {
        if(!s.isEmpty())
        {
            char c = s.toInt(&ok,16)&0xFF;
            if(ok){
                ret.append(c);
            }else{
                qDebug()<<"非法字符："<<s;
            }
        }
    }
    qDebug()<<ret;
    return ret;
}*/

void Widget::on_send_clicked()
{
    if(tmcDevice->isOpen())
    {
        if(fileName.isEmpty())
        {
//            if( ui->lineEdit->text().isEmpty())
//            {
                QMessageBox::warning(this, tr("Warning!"), tr("数据为空"));
//                return;
//            }
//            QByteArray sendData;
//            sendData.append(ui->lineEdit->text());
//            if (ui->checkBox->isChecked()) {
//                sendData.append('\r');
//            }
//            tmcDevice->write(sendData);
        }
        else
        {
            this->send();
            this->startTransfer();
        }
    }
    else
       QMessageBox::warning(this, tr("Warning!"), tr("未连接设备"));
}

void Widget::on_open_clicked()
{
    ui->progressBar->reset();
//    ui->label_open->setText(QStringLiteral("等待打開文件！"));
    ui->edit_open->setText(QStringLiteral("open file！"));

    fileName = QFileDialog::getOpenFileName(this);
    if (!openFile(fileName)) {
        qDebug() << "Error: openFile failed";
        return;
    }
}

bool Widget::openFile(QString &fileName)
{
    if (fileName.isEmpty())
        return false;

    ui->send->setEnabled(true);
//    ui->label_open->setText(QStringLiteral("%1").arg(fileName));
//    ui->label_open->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ui->edit_open->setText(QStringLiteral("%1").arg(fileName));
    return true;
}

void Widget::send()
{
    ui->send->setEnabled(false);
//    ui->progressBar->reset();
    // 初始化已發送字節為0
    bytesWritten = 0;
}

void Widget::handleTimeout()
{
    QByteArray buf;
    switch (group->checkedId())
    {
    case 0:/*gujian*/
        {
            buf = QByteArray::fromHex("1D6C7F");
        }
        break;
    case 1:/*ziku*/
        {
            buf = QByteArray::fromHex("1D6C0D");
            buf.append((char)(totalBytes&0xff));
            buf.append((char)((totalBytes>>8)&0xff));
            buf.append((char)((totalBytes>>16)&0xff));
            buf.append((char)((totalBytes>>24)&0xff));
//            qDebug() << buf.toHex();
        }
        break;
    default: ;
    }

    tmcDevice->write(buf);
    buf.resize(0);
    ui->progressBar->setMaximum((qint32)totalBytes);
    timer->stop();

    this->updateClientProgress(0);
}

bool Widget::startTransfer()
{
    localFile = new QFile(fileName);
    if (!localFile->open(QFile::ReadOnly)) {
        qDebug() << "client: open file error!";
        QMessageBox::warning(this, tr("Warning!"), tr("%1 不存在").arg(fileName));
        return false;
    }
    // 獲取文件大小
    totalBytes = localFile->size();
    bytesToWrite = totalBytes;

 /*   QDataStream sendOut(&outBlock, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_5_10);
    QString currentFileName = fileName.right(fileName.size() - fileName.lastIndexOf('/')-1);
    // 保留總大小信息空間、文件名大小信息空間，然後輸入文件名
    sendOut << qint64(0) << qint64(0) << currentFileName;

    // 這裡的總大小是總大小信息、文件名大小信息、文件名和實際文件大小的總和
    totalBytes += outBlock.size();
    sendOut.device()->seek(0);

    // 返回outBolock的開始，用實際的大小信息代替兩個qint64(0)空間
    sendOut << totalBytes << qint64((outBlock.size() - sizeof(qint64)*2));

    // 發送完文件頭結構後剩餘數據的大小
    bytesToWrite = totalBytes - tmcDevice->write(outBlock);

    outBlock.resize(0);*/

    switch (group->checkedId())
    {
    case 0:/*gujian*/
        {
            //do something
            QByteArray buf = QByteArray::fromHex("1D6C087F");
            qDebug() << buf.toHex();
            tmcDevice->write(buf);
            buf.resize(0);
            ui->progressBar->setMinimum(0);
            ui->progressBar->setMaximum(0);
            timer = new QTimer(this);
            connect(timer, SIGNAL(timeout()), this, SLOT(handleTimeout()));
            timer->start(4000);
        }
        break;
    case 1:/*ziku*/
        {
            QByteArray buf = QByteArray::fromHex("1D6C07A3");
            qDebug() << buf.toHex();
            tmcDevice->write(buf);
            buf.resize(0);
            ui->progressBar->setMinimum(0);
            ui->progressBar->setMaximum(0);
            /************延时1***************/
            //        QTime _Timer = QTime::currentTime().addMSecs(4000);
            //        while( QTime::currentTime() < _Timer );
            //        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
            /************延时2***************/
            timer = new QTimer(this);
            connect(timer, SIGNAL(timeout()), this, SLOT(handleTimeout()));
            timer->start(4000);
            /************延时3***************/
            //        QElapsedTimer t;
            //        t.start();
            //        while(t.elapsed()<4000);

//            Sleep(4000);
//            buf = QByteArray::fromHex("1D6C0D");
//            qDebug() <<  QByteArray::number(totalBytes,16);
//            buf.append((char)(totalBytes&0xff));
//            buf.append((char)((totalBytes>>8)&0xff));
//            buf.append((char)((totalBytes>>16)&0xff));
//            buf.append((char)((totalBytes>>24)&0xff));
//            qDebug() << buf.toHex();
//            tmcDevice->write(buf);
//            buf.resize(0);
        }
        break;
    default:
        ui->progressBar->setMaximum((qint32)totalBytes);
        this->updateClientProgress(0);
    }
//    if (ui->ziku->isChecked()) {}

    return true;
}

void Widget::updateClientProgress(qint64 numBytes)
{
    // 已經發送數據的大小
    bytesWritten += numBytes;

    qint64 hhhh = 0;
    // 如果已經發送了數據
    if (bytesToWrite > 0) {
        // 每次發送payloadSize大小的數據，這裡設置為64KB，如果剩餘的數據不足64KB，
        // 就發送剩餘數據的大小
        outBlock = localFile->read(qMin(bytesToWrite, payloadSize));

        // 發送完一次數據後還剩餘數據的大小
        hhhh = tmcDevice->write(outBlock);
        bytesToWrite -= hhhh;
//        bytesToWrite -= tmcDevice->write(outBlock);

        // 清空發送緩衝區
        outBlock.resize(0);
    } else { // 如果沒有發送任何數據，則關閉文件
        localFile->close();
    }
    // 更新進度條
    QCoreApplication::processEvents();
    ui->progressBar->setValue((qint32)bytesWritten);
    // 如果發送完畢

    if (bytesWritten == totalBytes) {
        localFile->close();
        ui->send->setEnabled(true);
        qDebug() << "Send complete";
    }
    else
        emit testsend(hhhh);
}

void Widget::dragEnterEvent(QDragEnterEvent *event)
{
    qDebug() << "dragEnterEvent";
    if (event->mimeData()->hasFormat("text/uri-list")) {
        event->acceptProposedAction();
    }
}

void Widget::dropEvent(QDropEvent *event)
{
    qDebug() << "dropEvent";

    ui->progressBar->reset();

    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) {
        qDebug() << "Error: urls is empty";
        return;
    }

    fileName = urls.first().toLocalFile();
    if (!openFile(fileName)) {
        qDebug() << "Error: openFile failed";
        return;
    }
}

void Widget::on_pushButton_clicked()
{
    if(tmcDevice->isOpen())
    {
        if(ui->textEdit->toPlainText().isEmpty())
        {
            QMessageBox::warning(this, tr("Warning!"), tr("数据为空"));
            return;
        }
        QByteArray sendData;

        if (ui->isHex->isChecked()) {
            sendData = QByteArray::fromHex(ui->textEdit->toPlainText().toLatin1());
        }
        else
            sendData.append(ui->textEdit->toPlainText());
        if (ui->addCR->isChecked()) {
            sendData.append('\r');
        }
        tmcDevice->write(sendData);

    }
    else
       QMessageBox::warning(this, tr("Warning!"), tr("未连接设备"));
}
