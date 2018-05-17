#include "mainwindow.h"
#include <QApplication>
#include <QStyle>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
    Qt::WindowFlags flags = w.windowFlags();
    flags |= Qt::WindowContextHelpButtonHint;
    w.setWindowFlags (flags);
    w.show();

    return a.exec();
}
