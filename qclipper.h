#ifndef QCLIPPER_H
#define QCLIPPER_H

#include <QDialog>
#include <QClipboard>
#include <QVector>
#include <QFont>
#include <QListWidgetItem>
#include <QCloseEvent>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QTextStream>
#include <QFile>
#include <QPointer>
#include <qxtglobalshortcut.h>
#include <QMessageBox>
#include <QUndoStack>

#include <template.h>
#include <setting.h>
#include <savecmd.h>
namespace Ui {
class QClipper;
}

class QClipper : public QDialog
{
    Q_OBJECT

public:
    explicit QClipper(QWidget *parent = 0);
    ~QClipper();

private:
    void InitUi();
    void SetTray();
    void SetShortCut();

    Ui::QClipper *ui;

    QVector<QString> v;
    QClipboard* board;
    QFont* font;
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QxtGlobalShortcut* shortcut;
    bool hasText;

    QString MultiText;
    QFile* f;

    int count;
    Template* tem;

//    QPointer<Setting> m_setting;
    Setting* m_setting;
    bool m_CheckSame;
    bool m_MultiSel;
    bool m_show;
    bool m_KeepMin;
    qreal opacity;

    QString minimize;
    QString wakeup;

    QFile* StoredFile;
    QString saveText;
    QUndoStack *undoStack;
    enum{
        MAX_ROW = 10,
        FONT_SIZE = 24,
        WIDTH = 300,
        HEIGHT = 300,
        FILTER_H = 30
    };
signals:
    void StoreText(QString text);
private slots:
    void addText();
    void ClickText();
    void LoadText();

    void test();
    void SetAutoRun(bool on);
    bool IsBlank(QString text);

    void on_list_customContextMenuRequested(const QPoint &pos);
    void on_LoadTheme_triggered();
    void on_Clear_triggered();
    void on_Setting_triggered();
    void on_Export_triggered();
    void on_About_QClipper_triggered();
    void TrayIconClicked(QSystemTrayIcon::ActivationReason reason);
    void on_ShowNormal_triggered();
    void on_Exit_triggered();
    void on_filter_textChanged(const QString &arg1);
    void on_AddTemplate_triggered();
    void on_Save_triggered();
    void on_clearMult_triggered();
    void on_AlwaysShow_triggered();
    void on_Reboot_triggered();

    void on_page_clicked();

    void on_Undo_triggered();

protected:
    void closeEvent(QCloseEvent *e);
    void changeEvent(QEvent *e);
    bool eventFilter(QObject *obj, QEvent *e);
};

#endif // QCLIPPER_H
