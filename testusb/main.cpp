#include "widget.h"
#include <QApplication>
#include <QIcon>
#include <QTabWidget>
#include <QLabel>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.setWindowTitle("updata");
//    w.setWindowIcon(QIcon(":/1.ico"));
    w.show();

    return a.exec();
}
