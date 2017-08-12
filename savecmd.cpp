#include "savecmd.h"
#include <QDebug>
#include <QSound>
SaveCmd::SaveCmd(QListWidget* w)
{
    widget = w;
    for(int i=0; i<widget->count(); i++)
        list<<widget->item(i)->text();
}

void SaveCmd::undo()
{
    widget->addItems(list);
    QSound::play(":/Sound/Sound/Undo.wav");
}

void SaveCmd::redo()
{
    qDebug()<<"redo";
}
