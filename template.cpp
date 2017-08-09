#include "template.h"
#include "ui_template.h"
#include <QMessageBox>
Template::Template(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Template)
{
    ui->setupUi(this);
    this->setWindowTitle("构建模板");
    this->setWindowIcon(QIcon(":/Icon/template.png"));
}

Template::~Template()
{
    delete ui;
}

QString Template::getExp()
{
    return exp;
}

void Template::on_Cancel_clicked()
{
    this->reject();
}

void Template::on_Yes_clicked()
{

    if(! ui->input->text().contains("*") || ui->input->text().isEmpty())
    {
        QMessageBox::warning(0, "注意！","请输入合法的模板");
        return;
    }
    else exp=ui->input->text();
    this->accept();
}

void Template::on_Help_clicked()
{
    QMessageBox::information(0, "模板说明","输入的文本至少包含一个 * ");
}
