#ifndef BEGINDIALOG_H
#define BEGINDIALOG_H

#include <QDialog>

namespace Ui {
class BeginDialog;
}

class BeginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BeginDialog(QWidget *parent = 0);
    ~BeginDialog();

private slots:
    void on_begin_cancel_clicked();

private:
    Ui::BeginDialog *ui;
    bool begin_cancel_status = false;
};

#endif // BEGINDIALOG_H
