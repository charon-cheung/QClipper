#include "clearcmd.h"
#include <QDebug>
#include <QSound>
ClearCmd::ClearCmd(QListWidget* w)
{
    widget = w;
    for(int i=0; i<widget->count(); i++)
        list<<widget->item(i)->text();
}

void ClearCmd::undo()
{
    widget->addItems(list);
    QSound::play(":/Sound/Sound/Undo.wav");
}

void ClearCmd::redo()
{
    qDebug()<<"redo";
}
