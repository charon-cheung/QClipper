#ifndef QCLIPPER_H
#define QCLIPPER_H

#include <QDialog>
#include <QClipboard>
#include <QVector>
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
#include <QPropertyAnimation>

#include <template.h>
#include <clearcmd.h>
#include <deletecmd.h>

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
    void InitOther();
    void CreateTray();
    void SetShortCut();
    void StartAnimation(QRect begin, QRect end);
private:
    Ui::QClipper *ui;

    QVector<QString> v;
    QClipboard* board;
    QFont* font;
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QxtGlobalShortcut* shortcut, WakeUp, Mini;
    bool hasText;

    QString MultiText;
    QFile* f;

    int count;
    Template* tem;

    bool m_CheckSame;
    bool m_show;
    bool m_KeepMin;
    qreal opacity;

    QString minimize;
    QString wakeup;

    QMenu menu;
    QMenu *m=NULL;
    QFile* StoredFile;
    QString saveText,deleteText;
    QStringList loadText;
    QUndoStack *undoStack;
    QAction *UnClear, *UnDelete;

    QPropertyAnimation *animation;
    QString dirPath;
    enum{
        MAX_ROW = 10,
        FONT_SIZE = 24,
        WIDTH = 320,
        HEIGHT = 300,
        FILTER_H = 35,
        W = WIDTH*2+5,
        H = HEIGHT+5+FILTER_H,
        ANIMATION_TIME=200
    };
private slots:
    void addText();
    void ClickText();
    void LoadSaveText();    //加载常用的文本项

    void SetAutoRun(bool on);

    bool IsBlank(QString text);

    void on_list_customContextMenuRequested(const QPoint &pos);

    void LoadChildMenu();

    void ReadTheme();

    void on_Clear_triggered();

    void Export();

    void InsertIntoDB();

    void DeleteFromDB();

    void on_About_QClipper_triggered();

    void TrayIconClicked(QSystemTrayIcon::ActivationReason reason);

    void on_ShowCursor();
    void on_ShowMini();
    void on_ShowCenter();

    void on_Exit_triggered();

    void on_filter_textChanged(const QString &arg1);

    void on_AddTemplate_triggered();

    void on_Save_triggered();

    void on_clearMult_triggered();

    void on_Reboot_triggered();

    void on_Help_triggered();

    void on_Close_triggered();

    void on_Delete_triggered();

    void on_stored_customContextMenuRequested(const QPoint &pos);

    void on_filter_returnPressed();

protected:
    void keyPressEvent(QKeyEvent *event);
    void changeEvent(QEvent *e);
    bool eventFilter(QObject *obj, QEvent *e);
};

#endif // QCLIPPER_H
