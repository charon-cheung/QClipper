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
    if(!f->open(QIODevice::WriteOnly | QIODevice::Append) )
    {
        //å¦?ä½?ä½¿ç?¨ä¸»çª??£ç???·å??
        QMessageBox::warning(0,"?¢å??°å¸¸?¨æ???¬æ?¶å?ºé??",f->errorString());
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
