#ifndef SAVECMD_H
#define SAVECMD_H
#include <QUndoCommand>
#include <QListWidget>
#include <QListWidgetItem>

class SaveCmd : public QUndoCommand
{
public:
    SaveCmd(QListWidget* w);
    void undo();
    void redo();
private:
    QListWidget* widget;
    QStringList list;
};

#endif // SAVECMD_H
