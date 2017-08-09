#include "qclipper.h"
#include <QApplication>
#include <QLockFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QClipper w;
    w.show();
    QLockFile lock("lock.lck");
    if(!lock.tryLock())
    {
        QMessageBox::warning(&w,"警告！","QClipper已经运行！");
        return 0;
    }
    return a.exec();
}
