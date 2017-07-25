#include "savecmd.h"
#include <QDebug>
#include <QSound>
SaveCmd::SaveCmd(QListWidget* w)
{
    widget = w;
}

void SaveCmd::undo()
{
    QListWidgetItem* item= widget->takeItem(0);
    delete item;
    QSound::play(":/Sound/Sound/Undo.wav");
}

void SaveCmd::redo()
{
    qDebug()<<"redo";
}
