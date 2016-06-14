#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("RamanBaselineCorrector");
    a.setOrganizationName("VU_LTC");
    a.setOrganizationDomain("lasercenter.vu.lt");

    MainWindow w;
    w.show();
    
    return a.exec();
}
