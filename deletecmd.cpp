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
        //�?�?使�?�主�??��???��??
        QMessageBox::warning(0,"?��??�常?��???��?��?��??",f->errorString());
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
