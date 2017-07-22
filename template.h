#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <QDialog>

namespace Ui {
class Template;
}

class Template : public QDialog
{
    Q_OBJECT
    
public:
    explicit Template(QWidget *parent = 0);
    ~Template();
    QString exp;    //需要在QClipper类中访问

private slots:

    void on_Cancel_clicked();

    void on_Yes_clicked();

    void on_Help_clicked();

private:
    Ui::Template *ui;

};

#endif // TEMPLATE_H
