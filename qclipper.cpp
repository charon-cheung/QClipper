#include "qclipper.h"
#include "ui_qclipper.h"
#include <QDebug>
#include <QSizePolicy>
#include <QSettings>
#include <QMimeData>
#include <QAction>
#include <QFileDialog>
#include <QPushButton>
#include <QSound>
#include <QProcess>
#include <QDesktopWidget>
#include <QSqlDatabase>
#include <QSql>
#include <QSqlQuery>
#include <QSqlError>

QClipper::QClipper(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QClipper)
{
    ui->setupUi(this);

    this->InitUi();
    this->InitOther();
    this->CreateTray();
    this->SetAutoRun(true);     //开机启动
    this->SetShortCut();
    this->LoadSaveText();
    this->on_ShowCenter();
    QSound::play(":/Sound/Sound/Run.wav");

    undoStack = new QUndoStack(this);
    //两个action其实一样
    UnClear = undoStack->createUndoAction(this,"撤销清空");
    UnClear->setIcon(QIcon(":/Icon/unclear.png"));
    UnDelete= undoStack->createUndoAction(this,"撤销删除");
    UnDelete->setIcon(QIcon(":/Icon/undelete.png"));

    //注意itemClicked和itemPressed的不同,itemClicked只识别鼠标左键
    foreach (QListWidget* w, findChildren<QListWidget*>())
        connect(w, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(ClickText())  );

    connect(qApp->clipboard(), SIGNAL(dataChanged()), this, SLOT(addText())  );
    connect(ui->ShowNormal,SIGNAL(triggered(bool)), this, SLOT(on_ShowCenter()) );
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
    this->setWindowTitle("QClipper 1.5.2");

    ui->filter->setGeometry(0,1, WIDTH, FILTER_H);
    ui->filter->setPlaceholderText("Filter");
    ui->label->setGeometry(W*0.7,4, 110,30);
    //    列表属性设置
    ui->list->setFrameStyle(QFrame::Plain);
    ui->list->setFrameShape(QFrame::NoFrame);
    ui->list->setGeometry(0, FILTER_H+1,WIDTH, HEIGHT+5);
    ui->list->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->stored->setFocus();
    ui->list->installEventFilter(this);

    ui->stored->setFrameStyle(QFrame::Plain);
    ui->stored->setFrameShape(QFrame::NoFrame);
    ui->stored->setGeometry(WIDTH+5, FILTER_H+1, WIDTH, HEIGHT+5);
    ui->stored->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->stored->installEventFilter(this);

    ui->filter->setPlaceholderText(QString("输入进程名,回车键关闭"));
    ui->filter->setFont(QFont("Inconsolata",20));
}

void QClipper::InitOther()
{
//    字体
    font = new QFont();
    font->setBold(true);
    font->setPixelSize(FONT_SIZE);
    font->setFamily("Inconsolata");

    hasText = false;        //开始没有剪贴内容
    m_CheckSame = false;    //默认相同文本也插入记录
    m_KeepMin = false;      //默认关闭时不会最小化
    m_show = false;         // 不要总是显示
    ui->ShowNormal->setEnabled(false);

    dirPath=QApplication::applicationDirPath();
    QDir d(dirPath);
    if(!d.entryList().contains("Style"))
    {
        d.mkdir("Style");
    }
}

void QClipper::CreateTray()
{
    //    托盘菜单
    trayMenu = new QMenu(this);
    trayMenu->addAction(ui->ShowNormal);
    trayMenu->addAction(ui->Reboot);
    trayMenu->addAction(ui->Exit);
    //    托盘图标
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/Icon/qclipper.png"));
    trayIcon->setToolTip("QClipper 1.5.2");
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
    connect(WakeUp, SIGNAL(activated()), this, SLOT(on_ShowCursor()) );

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
//    if(!text.isEmpty())
//    qDebug()<<"复制的文本: "<<text<<qrand()%50;
    if(text.isEmpty())  return;     // empty不处理
    if(IsBlank(text))  return;      // 若复制的全是空白字符，则不处理

    if( ui->list->count() >= MAX_ROW)
    {
        QListWidgetItem *last=ui->list->takeItem(ui->list->count()-1);
        delete last;
    }
    //加索引，例如： 1.
    int index = ui->list->count();
    QString ItemText = QString::number( index +1 ) + ". " + text;

    if(ItemText.indexOf("file:///")==3)
        return;     //排除复制文件名的情况
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
        if(i>=9)
            ui->list->item(i)->setText(QString("%1.").arg(i+1) +
                         ui->list->item(i)->text().remove(0,4));
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

    QString ItemText;
    if(obj->objectName()=="list")
        ItemText = w->currentItem()->text().remove(0,3);
    else ItemText = w->currentItem()->text();

    qApp->clipboard()->blockSignals(true);  //很关键，防止下一句代码中剪贴板发出dataChange信号
    QSound::play(":/Sound/Sound/MouseClick.wav");
    qApp->clipboard()->setText(ItemText, QClipboard::Clipboard);
    if(m_show)  return;
    this->on_ShowMini();
}

void QClipper::LoadSaveText()
{
    StoredFile = new QFile(this);
    StoredFile->setPermissions(QFileDevice::ReadOwner);
    QDir::setCurrent(QCoreApplication::applicationDirPath());
    StoredFile->setFileName("save.txt");
    QTextStream load(StoredFile);
    if(!StoredFile->exists())
        QMessageBox::information(this, "注意!","没有在bin文件夹发现save.text, 已经自动创建文件");

    if(!StoredFile->open(QIODevice::ReadWrite))
    {
        QMessageBox::warning(this, "加载保存的文本时出错",StoredFile->errorString());
        return;
    }
    load.setCodec("UTF-8");
    QString s = load.readAll();
    loadText = s.split("\n", QString::SkipEmptyParts);
    for(int i=0; i<loadText.count(); i++)
    {
        ui->stored->insertItem(0, loadText.at(loadText.count()-1-i));
        ui->stored->item(0)->setFont(*font);
    }
    delete StoredFile;
}

void QClipper::SetAutoRun(bool on)
{
    QSettings *reg=new QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                                 QSettings::NativeFormat);
    //开机启动
    if (on)
    {
        reg->setValue("QClipper","D:\\Build_My_Projet\\QClipper_static\\release\\QClipper.exe");
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
            return false;
    }
    return true;
}

void QClipper::on_list_customContextMenuRequested(const QPoint &pos)
{
    Q_UNUSED(pos);

    this->LoadChildMenu();
    menu.addAction(ui->Clear);
    menu.addAction(ui->Save);
    menu.addAction(UnClear);
    if(ui->list->count()!=0)
        UnClear->setDisabled(true);
    menu.addAction(ui->AddTemplate);
    menu.addAction(ui->Close);
    menu.addAction(ui->Help);
    menu.addAction(ui->About_QClipper);
    menu.exec(QCursor::pos());      // 菜单跟随鼠标位置
}

void QClipper::LoadChildMenu()
{
    if(m)   return; // 否则每次点右键都会添加二级菜单
    m = menu.addMenu(QIcon(":/Icon/theme.png"),"加载主题");

    QAction* darcula = m->addAction("darcula");
    QAction* black = m->addAction("black");
    QAction* white = m->addAction("white");

    darcula->setObjectName("darcula");
    black->setObjectName("black");
    white->setObjectName("white");

    connect(darcula,SIGNAL(triggered(bool)), this, SLOT(ReadTheme()));
    connect(black,SIGNAL(triggered(bool)), this, SLOT(ReadTheme()));
    connect(white,SIGNAL(triggered(bool)), this, SLOT(ReadTheme()));
}

void QClipper::ReadTheme()
{
    QString fileName = dirPath + "/Style/";
    if(sender()->objectName()=="darcula")
        fileName = fileName+"darcula.qss";
    else if(sender()->objectName()=="black")
        fileName = fileName+"black.qss";
    else if(sender()->objectName()=="white")
        fileName = fileName+"white.qss";

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
//    动作执行的时候创建此子类的对象，并放入QUndoStack
    ClearCmd *clear_cmd = new ClearCmd(ui->list);
    undoStack->push(clear_cmd);

    ui->list->clear();
    v.clear();
    qApp->clipboard()->clear(QClipboard::Clipboard);
    qApp->clipboard()->clear(QClipboard::Selection);
    qApp->clipboard()->clear(QClipboard::FindBuffer);
    QSound::play(":/Sound/Sound/Clear.wav");
}

void QClipper::Export()
{
    if(hasText)
    {
        f = new QFile(this);
        QDir::setCurrent(QCoreApplication::applicationDirPath());
        f->setFileName("save.txt");
        QTextStream write(f);
        if(! f->open(QIODevice::WriteOnly | QIODevice::Append) ) // 加这句才生成 save.txt
        {
            QMessageBox::warning(this,"追加到常用文本时出错",f->errorString());
            return;
        }
        write.setCodec("UTF-8");
        write<<saveText<<"\n";
        f->close();
    }
    delete f;
}

void QClipper::InsertIntoDB()
{
    QSqlQuery query;
    query.prepare("INSERT INTO text(content) VALUES(:content)");
    query.bindValue(":content",saveText);
    bool flag = query.exec();
    if(!flag)
    {
        QMessageBox::warning(NULL,"数据库写入失败",query.lastError().text());
    }
}

void QClipper::DeleteFromDB()
{
    QSqlQuery query;
    query.prepare("DELETE FROM text WHERE content = ?");
    query.addBindValue(deleteText);
    bool flag = query.exec();
    if(!flag)
    {
        QMessageBox::warning(NULL,"从数据库删除数据失败！",query.lastError().text());
    }
}

void QClipper::on_About_QClipper_triggered()
{
    // parent用this, 则对话框也采用QClipper的样式表
    QMessageBox::about(this, "QClipper 1.5.2", "QClipper是我自己开发的一个剪贴板工具");
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
        if(ui->ShowNormal->isEnabled())
            this->on_ShowCenter();
        break;
    default:
        break;
    }
}

void QClipper::on_ShowCursor()
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

void QClipper::on_ShowCenter()
{
    if(this->size() != QSize(0,0))      return;
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    this->showNormal();
    QSound::play(":/Sound/Sound/Run.wav");
    //鼠标在哪个屏幕，就判断是哪个屏幕
    QDesktopWidget *desk=QApplication::desktop();
    int w = desk->screenGeometry().width();
    int h = desk->screenGeometry().height();
    StartAnimation(QRect(0,0,0,0), QRect(w/2-W/2, h/2-H/2, W,H));
    ui->ShowNormal->setEnabled(false);
}

void QClipper::on_Exit_triggered()
{
    qApp->quit();   // 退出
}
//安装filter的obj不能再使用上下键选择文本
bool QClipper::eventFilter(QObject *obj, QEvent *e)
{
    if(obj == ui->stored)
    {
        if(ui->stored->count()==0)    return false;
        if(e->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent* >(e);
            QString keyText;
            int StoredRow = ui->stored->currentRow();
            int StoredRowCount = ui->stored->count();
            switch(keyEvent->key())
            {
            case Qt::Key_Space :
                keyText = ui->stored->currentItem()->text();

                QSound::play(":/Sound/Sound/MouseClick.wav");
                this->on_ShowMini();
                break;
            case Qt::Key_W :
                keyText = ui->stored->item(0)->text();
                QSound::play(":/Sound/Sound/MouseClick.wav");
                this->on_ShowMini();
                break;
            case Qt::Key_S :
                keyText = ui->stored->item(StoredRowCount - 1)->text();
                QSound::play(":/Sound/Sound/MouseClick.wav");
                this->on_ShowMini();
                break;
            case Qt::Key_Up :
                if(StoredRow)
                    ui->stored->setCurrentRow(StoredRow-1);
                else
                    ui->stored->setCurrentRow(StoredRowCount-1);
                QSound::play(":/Sound/Sound/KeyPress.wav");
                break;
            case Qt::Key_Down :
                if(StoredRow!=StoredRowCount-1)
                    ui->stored->setCurrentRow(StoredRow+1);
                else
                    ui->stored->setCurrentRow(0);
                QSound::play(":/Sound/Sound/KeyPress.wav");
                break;
            default :
                break;
            }
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
    else if(obj == ui->list)
    {
        if(ui->list->count()==0)    return false;
        if(e->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent* >(e);
            QString keyText;
            int ListRow = ui->list->currentRow();
            int ListRowCount = ui->list->count();
            switch(keyEvent->key())
            {
            case Qt::Key_Space :
                keyText = ui->stored->currentItem()->text();
                this->on_ShowMini();
                QSound::play(":/Sound/Sound/MouseClick.wav");
                break;
            case Qt::Key_Up :
                if(ListRow)
                    ui->stored->setCurrentRow(ListRow-1);
                else
                    ui->stored->setCurrentRow(ListRowCount-1);
                QSound::play(":/Sound/Sound/KeyPress.wav");
                break;
            case Qt::Key_Down :
                if(ListRow!=ListRowCount-1)
                    ui->stored->setCurrentRow(ListRow+1);
                else
                    ui->stored->setCurrentRow(0);
                QSound::play(":/Sound/Sound/KeyPress.wav");
                break;
            default :
                break;
            }
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
#if 0
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
#endif
}

void QClipper::on_AddTemplate_triggered()
{
    tem = new Template(this);
    tem->show();
    if(tem->exec()==QDialog::Accepted && ! tem->getExp().isEmpty())
    {
        qApp->clipboard()->setText(tem->getExp());
    }
    else return;
}

void QClipper::on_Save_triggered()
{
    if(v.size()==0)     return;
    saveText = ui->list->currentItem()->text().remove(0,3);
    QSound::play(":/Sound/Sound/Save.wav");

    for(int i=0;i<ui->stored->count();i++)
    {
        if(saveText==ui->stored->item(i)->text())
        {
            QMessageBox::information(this,"重复!","常用文本中已经存在!");
            return;
        }
        else continue;
    }
    QListWidgetItem* saveItem = new QListWidgetItem(0);
    saveItem->setText(saveText);
    ui->stored->insertItem(0, saveText);
    ui->stored->item(0)->setFont(*font);
    delete saveItem;

    this->Export();
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

void QClipper::on_Help_triggered()
{
    QMessageBox::information(this,"使用说明",
                             "Alt+Shift+W : 呼唤                       \n\n"
                             "Alt+Shift+D : 隐藏                       \n\n"
                             "在托盘点击右键可以退出                   \n\n"
                             "可用鼠标点击选择                         \n\n"
                             "可保存剪贴记录到常用文本                 \n\n"
                             "键盘上下键选择常用文本,空格可复制        \n\n"
                             "常用的剪贴文本                               ");
}

void QClipper::on_Close_triggered()
{
    if(m_KeepMin)
    {
        this->hide();
        return;
    }
    QMessageBox msg(this);
    msg.setText(tr("      请选择需要的操作"));

    QPushButton* SetMin = msg.addButton(tr("最小化"), QMessageBox::ActionRole);
    QPushButton* Exit = msg.addButton(tr("退出"), QMessageBox::ActionRole);
    QPushButton* Cancel = msg.addButton(tr("取消"), QMessageBox::ActionRole);
//    不显示最大化和最小化按钮,必须用两行完成
    msg.setWindowFlags(msg.windowFlags() & ~Qt::WindowMinMaxButtonsHint);
    msg.setWindowFlags(msg.windowFlags() | Qt::WindowStaysOnTopHint);
    msg.exec();

    if(msg.clickedButton() == SetMin)
    {
        this->on_ShowMini();
    }
    else if (msg.clickedButton() == Exit)
    {
        qApp->quit();
    }
    else if (msg.clickedButton() == Cancel)
        msg.close();
}

void QClipper::on_Delete_triggered()
{
    DeleteCmd *delete_cmd = new DeleteCmd(ui->stored);
    undoStack->push(delete_cmd);

    deleteText = ui->stored->currentItem()->text();
    loadText.removeOne(deleteText);

    QFile *file = new QFile(this);
    QDir::setCurrent(QCoreApplication::applicationDirPath());
    file->setFileName("save.txt");
    QTextStream write(file);
    //打开方式是只写和覆盖已有内容,QIODevice::Truncate会直接将文件内容清空,但不能只用Truncate
    if(! file->open(QIODevice::WriteOnly | QIODevice::Truncate) )
    {
        QMessageBox::warning(this,"删除常用文本时出错",file->errorString());
        return;
    }
    write.setCodec("UTF-8");
    foreach (QString s, loadText)
        write<<s<<"\n";
    file->close();

    delete file;
    delete ui->stored->currentItem();
}

void QClipper::on_stored_customContextMenuRequested(const QPoint &pos)
{
    Q_UNUSED(pos);
    QMenu m;
    m.addAction(ui->Delete);
    m.addAction(UnDelete);
    m.exec(QCursor::pos());
}

void QClipper::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Escape)
    {
        qDebug()<<"escape pressed";
        return;
    }
}

void QClipper::on_filter_returnPressed()
{
    QString process = ui->filter->text();
    if(process.isEmpty() || process.at(process.count()-1)==' ')
        return;
    QString cmd="taskkill /f /im "+ui->filter->text()+".exe";
    std::string str = cmd.toStdString();
    const char *s= str.c_str();
    system(s);
}
