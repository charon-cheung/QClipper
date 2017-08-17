#ifndef CLEARCMD_H
#define CLEARCMD_H
#include <QUndoCommand>
#include <QListWidget>
#include <QListWidgetItem>

class ClearCmd : public QUndoCommand
{
public:
    ClearCmd(QListWidget* w);
    void undo();
    void redo();
private:
    QListWidget* widget;
    QStringList list;
};

#endif // SAVECMD_H
