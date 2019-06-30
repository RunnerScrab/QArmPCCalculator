#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("QArmSkills");
    a.setOrganizationName("RunnerScrab");
    a.setOrganizationDomain("N/A");
    MainWindow w;
    w.show();

    return a.exec();
}
