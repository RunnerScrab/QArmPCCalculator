#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>

#include <QMainWindow>
#include <QSettings>
#include "SkillsDB.h"
#include "PC_Class.h"
#include "PC_Class_Skill.h"

class QComboBox;
class QPlainTextEdit;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void comboBoxesChanged();
    void on_actionSet_Font_triggered();

private:
    void LoadSettings();
    void SaveSettings();
    void closeEvent(QCloseEvent* event) override;

    Ui::MainWindow *ui;
    std::unique_ptr<SkillsDB> m_spSkillsDb;
    QComboBox* m_pMainComboBox;
    QComboBox* m_pSubComboBox;
    QPlainTextEdit* m_pReportText;
    QString m_settingsfilepath;
};

#endif // MAINWINDOW_H
