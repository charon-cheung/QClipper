#include "setting.h"
#include "ui_setting.h"
#include <QDebug>
#include <QDialog>

Setting::Setting(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Setting)
{
    ui->setupUi(this);
    this->setWindowTitle("设置");
    this->setWindowIcon(QIcon(":/Icon/setting_gear.png"));
    this->resize(670, 460);
    ui->filter->setPlaceholderText("filter");
    CheckSame = false;

    ListModel = new QStringListModel(this);
    QStringList list;
    list<<"行为"<<"界面"<<"声音"<<"快捷键";

    ListModel->setStringList(list);
    QFont font("Inconsolata");
    font.setPixelSize(20);
    QStandardItemModel *model = new QStandardItemModel();
    QStandardItem *Behavior = new QStandardItem("行为");
    Behavior->setFont(font);
    //    QPixmap p(150,50);
    //    p.fill("blue");
    Behavior->setIcon(QIcon(":/Setting/Setting/behavior.png"));

    QStandardItem *Interface = new QStandardItem("界面");
    Interface->setFont(font);
    Interface->setIcon(QIcon(":/Setting/Setting/interface.png"));

    QStandardItem *Sound = new QStandardItem("声音");
    Sound->setFont(font);
    Sound->setIcon(QIcon(":/Setting/Setting/sound.png"));

    QList<QStandardItem*> listItem;
    listItem<<Behavior<<Interface<<Sound;

    model->appendColumn(listItem);
    ui->listView->setModel(model);
    ui->listView->setFlow(QListView::TopToBottom);  //排列方式
    ui->listView->setEditTriggers(QAbstractItemView::NoEditTriggers); // 不允许编辑
    ui->listView->setGridSize(QSize(180,54));   // item大小
    ui->listView->setUniformItemSizes(true);    //所有item一样大小
    ui->listView->setViewMode(QListView::ListMode); //文字在图标右面
    ui->listView->setIconSize(QSize(48,48));    //图标大小

    ui->Behavior->setVisible(true);
    ui->Interface->setVisible(false);
    ui->Sound->setVisible(false);

    opacity=1.0;
    ui->OpacitySlider->setValue(SliderValue);
    connect(ui->OpacitySlider, SIGNAL(valueChanged(int)),
            this, SLOT(SetOpacity())  );

    ui->SaveRecord->setVisible(false);
    ui->WakeUp->installEventFilter(this);
    ui->Minimize->installEventFilter(this);
}

Setting::~Setting()
{
    delete ui;
    delete ListModel;
    delete model;
}
Setting* Setting::instance=0;
static QMutex mutex;
Setting *Setting::GetInstance()
{
    if(!instance)
    {
        QMutexLocker locker(&mutex);
        if(!instance)
            instance = new Setting;
    }
    return instance;
}

void Setting::on_Yes_clicked()
{
    this->accept();
}

void Setting::on_Cancel_clicked()
{
    this->reject();
}

bool Setting::GetCheckSame()
{
    if(ui->CheckSame->isChecked())
        return true;
    else return false;
}

bool Setting::GetMultiSel()
{
    if(ui->MultiSelect->isChecked())
        return true;
    else return false;
}
bool Setting::GetCheckMin()
{
    if(ui->KeepMin->isChecked())
        return true;
    else return false;
}

qreal Setting::GetOpacity()
{
    return opacity;
}

void Setting::on_listView_clicked(const QModelIndex &index)
{
    if(index.row() == Behavior)
    {
        ui->Behavior->setGeometry(250,60, 440,320);
        ui->Behavior->setVisible(true);
        ui->Interface->setVisible(false);
        ui->Sound->setVisible(false);
    }
    else if (index.row() == Interface )
    {
        ui->Interface->setGeometry(250,60, 400,320);
        ui->Interface->setVisible(true);
        ui->Behavior->setVisible(false);
        ui->Sound->setVisible(false);
    }
    else if(index.row() == Sound)
    {
        ui->Sound->setGeometry(250,60, 300,320);
        ui->Sound->setVisible(true);
        ui->Interface->setVisible(false);
        ui->Behavior->setVisible(false);
    }
}

void Setting::SetOpacity()
{
    SliderValue = ui->OpacitySlider->value();
    //注意不是100,而是100.0
    opacity = (100.0 - SliderValue)/100.0;
    ui->lineEdit->setText(QString::number(opacity) );
}

//设置两个快捷键, 只接受三个键的组合
bool Setting::eventFilter(QObject *obj, QEvent *e)
{
    if(e->type()==QEvent::KeyPress )
    {
        QKeyEvent *event = static_cast<QKeyEvent*>(e);
        if(obj==ui->WakeUp)
        {
            if(event->modifiers()== (Qt::AltModifier | Qt::ShiftModifier) &&
                    event->key()==Qt::Key_W)
            {
                ui->WakeUp->setText("Alt+Shift+W");
            }
            else if(event->modifiers()== (Qt::ControlModifier | Qt::ShiftModifier) &&
                    event->key()==Qt::Key_Z)
            {
                ui->WakeUp->setText("Ctrl+Shift+Z");
            }
            else ui->WakeUp->setText("");
            return true;
        }

        if(obj == ui->Minimize)
        {
            if(event->modifiers()== (Qt::AltModifier | Qt::ShiftModifier) &&
                    event->key()==Qt::Key_D)
            {
                ui->Minimize->setText("Alt+Shift+D");
            }
            ui->Minimize->setText("");
            return true;
        }
        else return false;
    }
    else return QDialog::eventFilter(obj,e);
}


QString Setting::GetMinimize()
{
    if(ui->Minimize->text().isEmpty())
        return "Alt+Shift+D";
    else return ui->Minimize->text();
}

QString Setting::GetWakeUp()
{
    if(ui->WakeUp->text().isEmpty())
        return "Alt+Shift+W";
    else return ui->WakeUp->text();
}
