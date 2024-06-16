#include "widget.h"

#include "ftd2xx.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");
    QApplication a(argc, argv);
    Widget w;
    w.show();

    return a.exec();
}
