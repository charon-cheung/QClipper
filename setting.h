#ifndef SETTING_H
#define SETTING_H

#include <QDialog>
#include <QStringListModel>
#include <QStandardItem>
#include <QKeyEvent>
#include <QMutex>
#include <QMutexLocker>

namespace Ui {
class Setting;
}

enum {
    Behavior,
    Interface,
    Sound
};

class Setting : public QDialog
{
    Q_OBJECT
    
public:
    explicit Setting(QWidget *parent = 0);
    ~Setting();
    static Setting* GetInstance();

    bool CheckSame;
    bool GetCheckSame();
    bool GetMultiSel();
    bool GetCheckMin();
    QString GetWakeUp();
    QString GetMinimize();
    qreal GetOpacity();
private slots:
    void on_Yes_clicked();

    void on_Cancel_clicked();

    void on_listView_clicked(const QModelIndex &index);

    void SetOpacity();

private:
    Ui::Setting *ui;
    static Setting* instance;

    QStringListModel* ListModel;
    QStandardItemModel *model;
    int SliderValue;
    qreal opacity;
//    QString minimize;
//    QString wakeup;
protected:
    bool eventFilter(QObject *obj, QEvent *e);
};

#endif // SETTING_H
