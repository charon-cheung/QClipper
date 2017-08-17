#include "deletecmd.h"

DeleteCmd::DeleteCmd(QListWidget *w)
{
    widget = w;
    row = widget->currentRow();
    text = widget->currentItem()->text();
}

void DeleteCmd::undo()
{
    widget->insertItem(row, text);
}

void DeleteCmd::redo()
{

}
