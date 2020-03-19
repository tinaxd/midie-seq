#include "mainwindow.h"

#include <QApplication>
#include <QLoggingCategory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//#ifdef MIDIE_DEBUG
    QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, true);
//#endif
    MainWindow w;
    w.show();
    return a.exec();
}
