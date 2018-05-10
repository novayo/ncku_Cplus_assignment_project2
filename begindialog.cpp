#include "begindialog.h"
#include "ui_begindialog.h"

BeginDialog::BeginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BeginDialog)
{
    ui->setupUi(this);
}

BeginDialog::~BeginDialog()
{
    delete ui;
}

void BeginDialog::on_begin_cancel_clicked()
{
    begin_cancel_status = !begin_cancel_status;
    if (begin_cancel_status){
        ui->label_5->setVisible(false);
        ui->label_6->setVisible(false);
        ui->label_7->setVisible(false);
        ui->label_3->setGeometry(-400, 125, 800, 75);
        QFont fnt;
        fnt.setPointSize(30);
        ui->label_3->setFont(fnt);
    }
    else{
        reject();
    }

}
