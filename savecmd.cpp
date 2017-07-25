#include "savecmd.h"
#include <QDebug>

SaveCmd::SaveCmd(QListWidget* w)
{
    widget = w;
}

void SaveCmd::undo()
{
    QListWidgetItem* item= widget->takeItem(0);
    delete item;
}

void SaveCmd::redo()
{
    qDebug()<<"redo";
}
