#include "deletecmd.h"
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QCoreApplication>
#include <QTextStream>

DeleteCmd::DeleteCmd(QListWidget *w)
{
    widget = w;
    row = widget->currentRow();
    text = widget->currentItem()->text();
}

void DeleteCmd::undo()
{
    widget->insertItem(row, text);
    QFile *f = new QFile();
    QDir::setCurrent(QCoreApplication::applicationDirPath());
    f->setFileName("save.txt");
    QTextStream write(f);
    if(! f->open(QIODevice::WriteOnly | QIODevice::Append) ) // 加这句才生成 save.txt
    {
        //如何使用主窗口的样式?
        QMessageBox::warning(0,"恢复到常用文本时出错",f->errorString());
        return;
    }
    write.setCodec("UTF-8");
    write<<text<<"\n";
    f->close();
    delete f;
}

void DeleteCmd::redo()
{

}
