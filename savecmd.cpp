#include "savecmd.h"
#include <QDebug>

SaveCmd::SaveCmd(QListWidget* w)
{
    widget = w;
}

void SaveCmd::undo()
{
    widget->takeItem(0);
}

void SaveCmd::redo()
{
    qDebug()<<"redo";
}
