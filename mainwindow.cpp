#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLayout>
#include <QMessageBox>
#include <QFontDialog>
#include <QScrollBar>
#include <QTextCursor>
#include <QStandardPaths>
#include <QDir>

#include <exception>
#include "SkillsDB.h"
#include "Character.h"
#include "PC_Class.h"
#include "Skill.h"

static void ErrorMsg(const char* msg)
{
    QMessageBox mb;
    mb.setText(msg);
    mb.exec();
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    try
    {
        m_spSkillsDb = std::make_unique<SkillsDB>();
    }
    catch(std::exception ex)
    {
        ErrorMsg(ex.what());
    }

    m_settingsfilepath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);

    m_pMainComboBox = findChild<QComboBox*>(QString("mainguildComboBox"));
    m_pSubComboBox = findChild<QComboBox*>(QString("subguildComboBox"));
    m_pReportText = findChild<QPlainTextEdit*>(QString("reportText"));
    connect(m_pMainComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(comboBoxesChanged()));
    connect(m_pSubComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(comboBoxesChanged()));

    LoadSettings();

    if(m_pMainComboBox && m_pSubComboBox)
    {
        const auto& rClassMap = m_spSkillsDb->GetClassMap();
        for (auto& clpair : rClassMap)
        {
            if (!clpair.second.IsSubclass())
            {
                m_pMainComboBox->addItem(clpair.first.c_str());
            }
            else
            {
                m_pSubComboBox->addItem(clpair.first.c_str());
            }
        }
    }

}


void MainWindow::LoadSettings()
{
    QDir settingsdir(m_settingsfilepath);
    if(!settingsdir.exists())
    {
        settingsdir.mkpath(m_settingsfilepath);
    }

    QSettings appsettings;
    appsettings.beginGroup("QArmSkills");
    QVariant fontface = appsettings.value("fontface");
    QVariant fontsize = appsettings.value("fontsize");
    QVariant window_geometry = appsettings.value("geometry");
    appsettings.endGroup();

    QString strfontface = fontface.isNull() ? "Courier" : fontface.toString();
    int intfontsize = fontsize.isNull() ? 8 : fontsize.toInt();
    restoreGeometry(window_geometry.toByteArray());
    m_pReportText->setFont(QFont(strfontface, intfontsize));
}

void MainWindow::SaveSettings()
{
    QSettings appsettings;
    appsettings.beginGroup("QArmSkills");
    appsettings.setValue("fontface", m_pReportText->font().family());
    appsettings.setValue("fontsize", m_pReportText->font().pointSize());
    appsettings.setValue("geometry", saveGeometry());
    appsettings.endGroup();
}

void MainWindow::comboBoxesChanged()
{
    QString guildstr;
    QString sguildstr;
    std::string mainguildstr = m_pMainComboBox->currentText().toStdString();
    std::string subguildstr =  m_pSubComboBox->currentText().toStdString();

    std::shared_ptr<Character> pChar = m_spSkillsDb->CreateCharacter(
        mainguildstr != "*NONE" ? mainguildstr.c_str() : nullptr,
        subguildstr != "*NONE" ? subguildstr.c_str() : nullptr);
    if (pChar)
    {
        std::string report = pChar->GetCombinedReport();
        m_pReportText->document()->setPlainText(report.c_str());
        m_pReportText->moveCursor(QTextCursor::Start);
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    SaveSettings();
    event->accept();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionSet_Font_triggered()
{
    m_pReportText->setFont(QFontDialog::getFont(0, m_pReportText->font()));
}
