#include "qclipper.h"
#include "ui_qclipper.h"
#include <QDebug>
#include <QSizePolicy>
#include <QIcon>
#include <QSettings>
#include <QMimeData>
#include <QAction>
#include <QFileDialog>
#include <QPushButton>
#include <QMap>
#include <QSound>
#include <QProcess>

QClipper::QClipper(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QClipper)
{
    ui->setupUi(this);
    this->InitUi();
    this->SetTray();
    this->SetAutoRun(true);     //开机启动
    this->SetShortCut();
    LoadText();     //    加载常用的文本项
    QSound::play(":/Sound/Sound/Run.wav");
    QCursor c;
    StartAnimation(QRect(0,0, 0,0), QRect(c.pos().x()-W/2,c.pos().y()-H/2, W,H));
    undoStack = new QUndoStack(this);
    //注意itemClicked和itemPressed的不同,itemClicked只识别鼠标左键
    QList<QListWidget* > ListWidget = this->findChildren<QListWidget*>();
    foreach (QListWidget* w, ListWidget) {
        connect(w, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(ClickText())  );
    }
    connect(qApp->clipboard(), SIGNAL(dataChanged()),
            this, SLOT(addText())  );
}

QClipper::~QClipper()
{
    delete ui;
    delete font;
}

void QClipper::InitUi()
{
    this->resize(W, H);
    this->setMaximumSize(W, H);
    this->setMouseTracking(true);
    this->setWindowIcon(QIcon(":/Icon/QClipper"));
    this->setWindowFlags(Qt::FramelessWindowHint);
    //    this->setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    this->setWindowTitle("QClipper 1.0");

    ui->filter->setGeometry(0,1, WIDTH, FILTER_H);
    ui->filter->setPlaceholderText("Filter");
    ui->label->setGeometry(W*0.7,4, 110,30);
    //    列表属性设置
    ui->list->setFrameStyle(QFrame::Plain);
    ui->list->setFrameShape(QFrame::NoFrame);
    ui->list->setGeometry(0, FILTER_H+1,WIDTH, HEIGHT+5);
    ui->list->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->list->setFocus();
    ui->list->installEventFilter(this);

    ui->stored->setFrameStyle(QFrame::Plain);
    ui->stored->setFrameShape(QFrame::NoFrame);
    ui->stored->setGeometry(WIDTH+5, FILTER_H+1, WIDTH, HEIGHT+5);
    ui->stored->setContextMenuPolicy(Qt::CustomContextMenu);

    //    字体
    font = new QFont();
    font->setBold(true);
    font->setPixelSize(FONT_SIZE);
    font->setFamily("Inconsolata");
    //    if( this->locale().language() == QLocale::Chinese)
    //        qDebug()<<"当前使用中文";
    hasText = false;        //开始没有剪贴内容
    m_MultiSel = false;     //默认关闭多选模式
    m_CheckSame = false;    //默认相同文本也插入记录
    m_KeepMin = false;      //默认关闭时不会最小化
    m_show = false;         // 不要总是显示
    ui->ShowNormal->setEnabled(false);
}

void QClipper::SetTray()
{
    //    托盘菜单
    trayMenu = new QMenu(this);
    trayMenu->addAction(ui->ShowNormal);
    trayMenu->addAction(ui->Reboot);
    trayMenu->addAction(ui->Setting);
    trayMenu->addAction(ui->Exit);

    //    托盘图标
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/Icon/qclipper.png"));
    trayIcon->setToolTip("QClipper 1.0");
    trayIcon->setContextMenu(trayMenu);
    trayIcon->showMessage("托盘标题", "托盘内容", QSystemTrayIcon::Information, 3000);
    trayIcon->show();

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(TrayIconClicked(QSystemTrayIcon::ActivationReason))  );
}

void QClipper::SetShortCut()
{
    // 全局热键，托盘显示时，恢复正常窗口，不能与现有全局热键冲突,
    //    否则报错 QxtGlobalShortcut failed to register
    wakeup= "Alt+Shift+W";
    QxtGlobalShortcut* WakeUp = new QxtGlobalShortcut(this);
    WakeUp->setShortcut(QKeySequence("Alt+Shift+W"));
    connect(WakeUp, SIGNAL(activated()), this, SLOT(on_ShowNormal_triggered()) );

    QxtGlobalShortcut* MiNi = new QxtGlobalShortcut(this);
    MiNi->setShortcut(QKeySequence("Alt+Shift+D"));
    connect(MiNi, SIGNAL(activated()), this, SLOT(on_ShowMini()) );
}

void QClipper::StartAnimation(QRect begin, QRect end)
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(ANIMATION_TIME);
    animation->setStartValue(begin);
    animation->setEndValue(end);
    animation->start();
}

void QClipper::addText()
{
    qApp->clipboard()->blockSignals(false);   //解除点击和按键盘时的信号封锁
    QString text=qApp->clipboard()->text(QClipboard::Clipboard);
//    QString text = qApp->clipboard()->mimeData()->text();
    qDebug()<<"复制到剪贴板的文本: "<<text<<qrand()%50;
    if(text.isEmpty())  return;     // 空，不处理
    if(IsBlank(text))  return;      // 若复制的全是空白字符，则不处理

    if( ui->list->count() >= MAX_ROW)
    {
        QListWidgetItem *last=ui->list->takeItem(ui->list->count()-1);
        delete last;
    }
    //加索引，例如： 1.
    int index = ui->list->count();

    QString ItemText;
    ItemText = QString::number( index +1 ) + ". " + text;
    v.prepend(ItemText);

    if(m_CheckSame)
    {
        for(int i=0; i<index;i++)
        {
            if(ItemText.remove(0,3)==ui->list->item(i)->text().remove(0,3))
            {
                QMessageBox box;
                box.setWindowIcon(QIcon(":/Icon/warning.png"));
                box.setWindowTitle(tr("重复！"));
                box.setText("当前文本已经在剪贴板");
                box.setWindowFlags(Qt::WindowStaysOnTopHint);
                //                消息窗口弹出到桌面最前，只能用WindowStaysOnTopHint
                //                        box.activateWindow();
                box.exec();
                return;
            }
        }
    }
    ui->list->insertItem(0,v.at(0));    //最关键的插入文本
    //调整索引
    for(int i=0; i<index+1;i++)
    {
        ui->list->item(i)->setText(QString("%1. ").arg(i+1) +
                                   ui->list->item(i)->text().remove(0,3));
    }
    //每个item的样式
    hasText = true;
    ui->list->item(0)->setFont(*font);
    ui->list->item(0)->setSizeHint(QSize(290,30));   // 每一行的大小
    ui->list->item(0)->setToolTip(v.at(0));
    ui->list->item(0)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
}

void QClipper::ClickText()
{
    QObject* obj = sender();
    QListWidget* w = qobject_cast<QListWidget*>(obj);
    QString ItemText = w->currentItem()->text().remove(0,3);
    qApp->clipboard()->blockSignals(true);  //很关键，防止下一句代码中剪贴板发出dataChange信号
    QSound::play(":/Sound/Sound/MouseClick.wav");
    if(m_MultiSel)  //多选模式不会最小化窗口
    {
        w->setSelectionMode(QAbstractItemView::ExtendedSelection);
        MultiText+=ItemText;
        qApp->clipboard()->setText(MultiText);
    }
    else {
        qApp->clipboard()->setText(ItemText, QClipboard::Clipboard);
        if(m_show)  return;
        this->on_ShowMini();
    }
}

void QClipper::LoadText()
{
    StoredFile = new QFile(this);

    QDir::setCurrent(QCoreApplication::applicationDirPath());
    StoredFile->setFileName("save.txt");
    QTextStream load(StoredFile);
    if(!StoredFile->open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, "加载保存的文本出错",StoredFile->errorString());
        return;
    }
    load.setCodec("UTF-8");
    QString s = load.readAll();
    QStringList ll = s.split("\n", QString::SkipEmptyParts);
    for(int i=0; i<ll.count(); i++)
    {
        ui->stored->insertItem(0, ll.at(ll.count()-1-i));
        ui->stored->item(0)->setFont(*font);
    }
}

void QClipper::SetAutoRun(bool on)
{
    QSettings *reg=new QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                                 QSettings::NativeFormat);
    //开机启动
    if (on)
    {   reg->setValue("QClipper","D:\\Build_My_Projet\\QClipper_static\\release\\QClipper.exe");
        //        qDebug()<<"value: "<<reg->value("QClipper").toString();
    }
    else
    {
        reg->setValue("QClipper","null");
    }
    delete reg;
}

bool QClipper::IsBlank(QString text)
{
    for(int i=0;i <text.size(); i++)
    {
        if( text.at(i) != ' ' )
        {
            return false;
        }
    }
    return true;
}

void QClipper::on_list_customContextMenuRequested(const QPoint &pos)
{
    Q_UNUSED(pos);
//    qDebug()<<pos.x()<<"  "<<pos.y();
    QMenu menu;
    menu.addAction(ui->LoadTheme);
    menu.addAction(ui->Clear);
    menu.addAction(ui->Save);
    menu.addAction(ui->Undo);
    menu.addAction(ui->Export);
    menu.addAction(ui->AddTemplate);
    menu.addAction(ui->Help);
    menu.addAction(ui->Setting);
    if(m_MultiSel)
        menu.addAction(ui->clearMult);
    menu.addAction(ui->About_QClipper);
    menu.exec(QCursor::pos());      // 菜单跟随鼠标位置
}

void QClipper::on_LoadTheme_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "","",
                                                    tr("样式表文件(*.qss)") );
    if(fileName.isEmpty())  return;   // 不加这句，若选取消，窗口会恢复为默认样式
    QFile f(fileName);
    QTextStream read(&f);
    if(! f.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, "加载主题文件出错",f.errorString());
        return;
    }
    QString theme = read.readAll();
    f.close();
    this->setStyleSheet(theme);
}

void QClipper::on_Clear_triggered()
{
    ui->list->clear();
    v.clear();
    qApp->clipboard()->clear(QClipboard::Clipboard);
    qApp->clipboard()->clear(QClipboard::Selection);
    qApp->clipboard()->clear(QClipboard::FindBuffer);
    QSound::play(":/Sound/Sound/Clear.wav");
}

void QClipper::on_Setting_triggered()
{
    //    if(!m_setting)
    m_setting = new Setting(0);
    m_setting->show();
    if(m_setting->exec() != QDialog::Accepted)    return;
    m_CheckSame = m_setting->GetCheckSame();
    m_MultiSel = m_setting->GetMultiSel();
    opacity = m_setting->GetOpacity();
    m_KeepMin = m_setting->GetCheckMin();
    this->setWindowOpacity(opacity);

    minimize = m_setting->GetMinimize();
    wakeup = m_setting->GetWakeUp();

}

void QClipper::on_Export_triggered()
{
    if(hasText)
    {
        //        QString saveFile = QFileDialog::getSaveFileName(0, "导出剪贴板记录",
        //                                                        "export",
        //                                                        tr("导出文件(*.txt)"));
        f = new QFile(this);
        QDir::setCurrent(QCoreApplication::applicationDirPath());
        f->setFileName("save.txt");
        QTextStream write(f);
        if(! f->open(QIODevice::WriteOnly) )    // 加这句才生成export.txt
        {
            QMessageBox::warning(this,"导出到常用文本时出错",f->errorString());
            return;
        }
        write.setCodec("UTF-8");
        for(int i=0; i<ui->stored->count(); i++)
        {
            QString text = ui->stored->item(i)->text();
            write<<text<<"\n";
        }
        f->close();
    }
    delete f;
}

void QClipper::on_About_QClipper_triggered()
{
    // parent用this, 则对话框也采用QClipper的样式表
    QMessageBox::about(0, "QClipper 1.0", "QClipper是我自己开发的一个剪贴板工具。");
}

void QClipper::closeEvent(QCloseEvent *e)
{
    e->ignore();
    if(m_KeepMin)
    {
        this->hide();
        return;
    }
    QMessageBox msg;
    msg.setWindowTitle(tr("请注意！"));
    msg.setText(tr("选择需要的操作"));

    QPushButton* SetMin = msg.addButton(tr("最小化到托盘"), QMessageBox::ActionRole);
    QPushButton* Exit = msg.addButton(tr("退出"), QMessageBox::ActionRole);
    QPushButton* Cancel = msg.addButton(tr("取消"), QMessageBox::ActionRole);
    msg.setWindowFlags(Qt::WindowStaysOnTopHint);
    msg.exec();

    if(msg.clickedButton() == SetMin)
    {
        this->hide();
    }
    else if (msg.clickedButton() == Exit)
    {
        qApp->quit();
    }
    else if (msg.clickedButton() == Cancel)
        this->close();
}

void QClipper::changeEvent(QEvent *e)
{
    if( (e->type() == QEvent::WindowStateChange) && (this->isMinimized()) )
    {
        this->hide();
    }
}

void QClipper::TrayIconClicked(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
    case QSystemTrayIcon::DoubleClick :
        this->setWindowState(Qt::WindowActive);
        this->show();
        break;
    default:
        break;
    }
}

void QClipper::on_ShowNormal_triggered()
{
    //两行的顺序不能交换
    if(this->size() != QSize(0,0))      return;
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    this->showNormal();
    QSound::play(":/Sound/Sound/Run.wav");
    QCursor c;
    StartAnimation(QRect(0,0,0,0), QRect(c.pos().x()-W/2, c.pos().y()-H/2, W,H));
    ui->ShowNormal->setEnabled(false);
}

void QClipper::on_ShowMini()
{
    if(this->size() == QSize(0,0))      return;
    QSound::play(":/Sound/Sound/Min.wav");
    StartAnimation(QRect(this->pos().x(), this->pos().y(), W,H), QRect(0,0, 0,0));
    ui->ShowNormal->setEnabled(true);
}

void QClipper::on_Exit_triggered()
{
    qApp->quit();   // 退出
}

bool QClipper::eventFilter(QObject *obj, QEvent *e)
{
    if(obj == ui->list)
    {
        if(ui->list->count()==0)    return false;
        if(e->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent* >(e);
            QString keyText;

            switch(keyEvent->key())
            {
            case Qt::Key_1 :
                keyText = ui->list->item(0)->text().remove(0,3);
                break;
            case Qt::Key_2 :
                keyText = ui->list->item(1)->text().remove(0,3);
                break;
            case Qt::Key_3 :
                keyText = ui->list->item(2)->text().remove(0,3);
                break;
            case Qt::Key_4 :
                keyText = ui->list->item(3)->text().remove(0,3);
                break;
            case Qt::Key_5 :
                keyText = ui->list->item(4)->text().remove(0,3);
                break;
            case Qt::Key_6 :
                keyText = ui->list->item(5)->text().remove(0,3);
                break;
            case Qt::Key_7 :
                keyText = ui->list->item(6)->text().remove(0,3);
                break;
            case Qt::Key_8 :
                keyText = ui->list->item(7)->text().remove(0,3);
                break;
            case Qt::Key_9 :
                keyText = ui->list->item(8)->text().remove(0,3);
                break;
            case Qt::Key_0 :
                keyText = ui->list->item(9)->text().remove(0,3);
                break;
            default :
                break;
            }
            QSound::play(":/Sound/Sound/KeyPress.wav");
            qApp->clipboard()->blockSignals(true);
            qApp->clipboard()->setText(keyText, QClipboard::Clipboard);
            return true;
        }
        else{
            //只能在这里解除键盘事件的信号封锁   ???
            qApp->clipboard()->blockSignals(false);
            return false;
        }
    }
    else    return QDialog::eventFilter(obj,e);
}

void QClipper::on_filter_textChanged(const QString &arg1)
{
    count = ui->list->count();
    for(int i=0; i<count; i++)
    {
        if(arg1.isEmpty())
            ui->list->item(i)->setHidden(false);
        if(! v.at(i).contains(arg1, Qt::CaseSensitive))
        {
            ui->list->item(i)->setHidden(true);
        }
        else  ui->list->item(i)->setHidden(false);
    }
}

void QClipper::on_AddTemplate_triggered()
{
    tem = new Template(this);
    tem->show();
    if(tem->exec()==QDialog::Accepted && ! tem->exp.isEmpty())
    {
        qApp->clipboard()->setText(tem->exp);
    }
    else return;
}

void QClipper::on_Save_triggered()
{
    if(v.size()==0)     return;
    saveText = ui->list->currentItem()->text();
    QSound::play(":/Sound/Sound/Save.wav");
    QListWidgetItem* saveItem = new QListWidgetItem(0);
    saveItem->setText(saveText);
    ui->stored->insertItem(0, saveText);
    ui->stored->item(0)->setFont(*font);
    delete saveItem;

    SaveCmd *save_cmd = new SaveCmd(ui->stored);
    undoStack->push(save_cmd);
}

void QClipper::on_clearMult_triggered()
{
    qApp->clipboard()->clear(QClipboard::Clipboard);
    MultiText.clear();
}

void QClipper::on_Reboot_triggered()
{
    QString program = QApplication::applicationFilePath();
    QStringList args = QApplication::arguments();
    QString dir = QDir::currentPath();
    QProcess::startDetached(program, args, dir);
    qApp->quit();
}

void QClipper::on_Undo_triggered()
{
    undoStack->undo();
}

void QClipper::on_Help_triggered()
{
    QMessageBox::information(0,"使用说明",
                             "Alt+Shift+W : 呼唤                       \n\n"
                             "Alt+Shift+D : 隐藏                       \n\n"
                             "在托盘点击右键可以退出                   \n\n"
                             "可用鼠标点击和键盘选择                   \n\n"
                             "可保存剪贴文本到常用再导出               \n\n"
                             "在exe所在文件夹建save.text可以保存         \n"
                             "常用的剪贴文本                               ");
//    QMessageBox::setWindowFlags(Qt::WindowStaysOnTopHint);
}
