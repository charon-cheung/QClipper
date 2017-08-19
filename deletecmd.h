#ifndef DELETECMD_H
#define DELETECMD_H
#include <QUndoCommand>
#include <QListWidget>
#include "qclipper.h"

class DeleteCmd : public QUndoCommand
{
public:
    DeleteCmd(QListWidget* w);
    void undo();
    void redo();
private:
    QListWidget* widget;
    QString text;
    int row;
};

#endif // DELETECMD_H
