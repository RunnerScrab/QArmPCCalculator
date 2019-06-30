#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>

static void ErrorMsg(const char* msg)
{
    QMessageBox mb;
    mb.setText(msg);
    mb.exec();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("QArmSkills");
    a.setOrganizationName("RunnerScrab");
    a.setOrganizationDomain("N/A");
    MainWindow* pW = nullptr;
    try
    {
        pW = new MainWindow();
    }
    catch(std::exception ex)
    {
        ErrorMsg(ex.what());
        return 0;
    }

    pW->show();

    int retval = a.exec();

    delete pW;
    return retval;
}
