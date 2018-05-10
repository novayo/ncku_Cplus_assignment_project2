#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QHeaderView>
#include <QApplication>
#include <QProcess>
#include <QTableWidgetItem>
#include <QAbstractItemView>
#include <ctime>
#include <QMessageBox>

#define UNASSIGNED 0
#define N 9

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    rowCount(9), columnCount(9), gammingPeriod(0), pause(true), sudoku_start(false), undo(false), editing_mode(false), solve(false),
    finish_status(false)
{
    ui->setupUi(this);

    srand(time(NULL));
    // set text font
    user_text_font = 30;
    question_text_font = 20;

    // set user_sudoku
    user_sudoku_row.assign(rowCount, 0);
    user_sudoku.assign(columnCount, user_sudoku_row);

    // set ans_sudoku
    ans_sudoku_row.assign(rowCount, 0);
    ans_sudoku.assign(columnCount, ans_sudoku_row);

    // set difficulty
    difficulty = 50;
    give_hint_times = 0;

    // set undo
    undo_value.assign(100000, 0);
    undo_row.assign(100000, 0);
    undo_column.assign(100000, 0);
    undo_step = 0;

    // set clock
    ctime = QTime::currentTime();
    ui->Clock->setVisible(false);

    // start timer
    QTimer *timer = new QTimer(this);
    timer->start(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(showTime()));
    connect(ui->pause, SIGNAL(clicked()), this, SLOT(showTime()));
    connect(ui->NewPuzzle, SIGNAL(clicked()), this, SLOT(showTime()));

    // start new puzzle
    connect(ui->actionsetRandomly, SIGNAL(triggered()), this, SLOT(on_NewPuzzle_clicked()));

    // set default
    ui->pause->setEnabled(false);
    ui->Undo->setEnabled(false);
    ui->resultLabel->setVisible(false);
    ui->can_be_solve_lable->setVisible(false);
    for (int i=0; i<rowCount; ++i){
        for (int j=0; j<columnCount; ++j){
            Qt::ItemFlags eFlags = ui->tableWidget->item(i, j)->flags();
            eFlags &= ~Qt::ItemIsEditable;
            ui->tableWidget->item(i, j)->setFlags(eFlags);
        }
    }
    aboutdialog = new AboutDialog();
    begindialog = new BeginDialog();
    begindialog->exec();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Random sudoku //
void MainWindow::on_NewPuzzle_clicked()
{
    this->setWindowTitle("NCKU_Project_2 (Wait...)");

    /***** set default *****/
    // default sudoku_start
    sudoku_start = false;
    solve = false;
    finish_status = false;
    ui->NewPuzzle->setText("New Puzzle");

    // default pause button
    pause = false;
    ui->pause->setText("Pause");
    ui->pause->setEnabled(true);
    ui->Undo->setEnabled(true);

    // default timer time
    gammingPeriod = 0;
    ui->Clock->setVisible(true);

    // default text
    QFont fnt;
    fnt.setPointSize(user_text_font);
    for (int i=0; i<rowCount; ++i){
        for (int j=0; j<columnCount; ++j){
            ui->tableWidget->item(i, j)->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->item(i, j)->setTextColor(Qt::black);
            ui->tableWidget->item(i, j)->setFont(fnt);
            ui->tableWidget->item(i, j)->setBackgroundColor(QColor(255, 255, 255));
            Qt::ItemFlags eFlags = ui->tableWidget->item(i, j)->flags();
            eFlags |= Qt::ItemIsEditable;
            ui->tableWidget->item(i, j)->setFlags(eFlags);
        }
    }

    // Get user_sudoku
    int question[9][9];

    if (editing_mode == true){
        int n_zero = 0;
        for (int i=0; i<rowCount; ++i){
            for (int j=0; j<columnCount; ++j){
                if (ui->tableWidget->item(i, j)->text().toInt() == 0){
                    ++n_zero;
                }
            }
        }
        if (n_zero == 81){
            editing_mode = false;
        }
    }

    if (editing_mode == true){
        // get value from table
        for (int i=0; i<rowCount; ++i){
            for (int j=0; j<columnCount; ++j){
                user_sudoku[i][j] = ui->tableWidget->item(i, j)->text().toInt();
                question[i][j] = user_sudoku[i][j];
            }
        }
    }
    else{
        set_random_sudoku();
        for (int i=0; i<rowCount; ++i){
            for (int j=0; j<columnCount; ++j){
                question[i][j] = user_sudoku[i][j];
            }
        }
    }

    // set question to table //
    for (int i=0; i<rowCount; ++i){
        for (int j=0; j<columnCount; ++j){
            if (question[i][j] == 0){
                ui->tableWidget->item(i, j)->setText(QString(' '));
            }
            else{
                ui->tableWidget->item(i, j)->setText(QString::number(question[i][j]));
                fnt.setPointSize(question_text_font);
                ui->tableWidget->item(i, j)->setFont(fnt);
                ui->tableWidget->item(i, j)->setBackgroundColor(QColor(221,221,221));
                ui->tableWidget->item(i, j)->setTextColor(Qt::black);
                Qt::ItemFlags eFlags = ui->tableWidget->item(i, j)->flags();
                eFlags &= ~Qt::ItemIsEditable;
                ui->tableWidget->item(i, j)->setFlags(eFlags);
            }

        }
    }

    // check if the question is legal or not
    bool question_is_legal = true;

    // check whether exit 0 on table, if yes-> can solve , if not -> cant solve
    bool check_sudoku_exist_zero = false;
    for (int i=0; i<rowCount; ++i){
        for (int j=0; j<columnCount; ++j){
            user_sudoku[i][j] = ui->tableWidget->item(i, j)->text().toInt();
            if (user_sudoku[i][j] == 0){
                check_sudoku_exist_zero = true;
            }
        }
    }

    // solve it
    if (check_sudoku_exist_zero == true){
        if(SolveSudoku()){
            for (int i=0; i<rowCount; ++i){
                for (int j=0; j<columnCount; ++j){
                    // if question can solve, copy to ans_sudoku
                    ans_sudoku[i][j] = user_sudoku[i][j];
                    //qDebug() << i << " " << j << " " << ans_sudoku[i][j];
                }
            }
            //print solvable
            ui->can_be_solve_lable->setText("Solvable Sudoku");
            ui->can_be_solve_lable->setVisible(true);
            QPalette sample_palette;
            sample_palette.setColor(QPalette::WindowText, QColor(0,221,0));

            ui->can_be_solve_lable->setAutoFillBackground(true);
            ui->can_be_solve_lable->setPalette(sample_palette);
        }
        else{
            // print unsolvable
            ui->can_be_solve_lable->setText("Unsolvable Sudoku");
            ui->can_be_solve_lable->setVisible(true);

            QPalette sample_palette;
            sample_palette.setColor(QPalette::WindowText, QColor(0,221,0));

            ui->can_be_solve_lable->setAutoFillBackground(true);
            ui->can_be_solve_lable->setPalette(sample_palette);

            question_is_legal = false;
            //qDebug() << "no";
        }
    }
    else if (isCorrect()==false && check_sudoku_exist_zero==false){
        // print sudoku table can't be solved !
        ui->can_be_solve_lable->setText("Sudoku table can't be solved !");
        ui->can_be_solve_lable->setVisible(true);
        QPalette sample_palette;
        sample_palette.setColor(QPalette::WindowText, QColor(255,0,255));

        ui->can_be_solve_lable->setAutoFillBackground(true);
        ui->can_be_solve_lable->setPalette(sample_palette);
        question_is_legal = false;
    }
    else{
        // print sudoku table is solved !
        ui->can_be_solve_lable->setText("Sudoku table is already solved !");
        ui->can_be_solve_lable->setVisible(true);
        QPalette sample_palette;
        sample_palette.setColor(QPalette::WindowText, Qt::red);

        ui->can_be_solve_lable->setAutoFillBackground(true);
        ui->can_be_solve_lable->setPalette(sample_palette);
        question_is_legal = false;
    }

    /***** if question is not legal *****/
    if (question_is_legal == false){
        // set default
        ui->pause->setEnabled(false);
        ui->Undo->setEnabled(false);
        ui->Clock->setVisible(false);
        ui->resultLabel->setVisible(false);

        // set every item cant be modified
        for (int i=0; i<rowCount; ++i){
            for (int j=0; j<columnCount; ++j){
                Qt::ItemFlags eFlags = ui->tableWidget->item(i, j)->flags();
                eFlags &= ~Qt::ItemIsEditable;
                ui->tableWidget->item(i, j)->setFlags(eFlags);
            }
        }
        ui->can_be_solve_lable->setVisible(true);
    }
    else{
        // sudoku start !!!
        this->setWindowTitle("NCKU_Project_2 (Gamming)");
        sudoku_start = true;
    }
    editing_mode = false;
}

// Manual sudoku
void MainWindow::on_actionsetManually_triggered()
{
    editing_mode = true;
    this->setWindowTitle("NCKU_Project_2 (Editing)");
    ui->NewPuzzle->setText("Press to Start");

    for (int i=0; i<rowCount; ++i){
        for (int j=0; j<columnCount; ++j){
            // set text default
            QFont fnt;
            fnt.setPointSize(user_text_font);
            ui->tableWidget->item(i, j)->setFont(fnt);
            ui->tableWidget->item(i, j)->setTextColor(Qt::black);
            ui->tableWidget->item(i, j)->setText(QString(' '));
            ui->tableWidget->item(i, j)->setBackgroundColor(QColor(255,255,255));

            // set item editable
            Qt::ItemFlags eFlags = ui->tableWidget->item(i, j)->flags();
            eFlags |= Qt::ItemIsEditable;
            ui->tableWidget->item(i, j)->setFlags(eFlags);

            // set timer invisible
            ui->Clock->setVisible(false);

            // set pause button inenable
            ui->pause->setEnabled(false);
            ui->Undo->setEnabled(false);
        }
    }
}

// Finish button //
void MainWindow::on_actionCheck_triggered()
{
    if (sudoku_start && !solve){
        /***** get value into user_sudoku *****/
        // check if exist 0 on table
        bool is_zero = false;
        for (int i=0; i<rowCount; ++i){
            for (int j=0; j<columnCount; ++j){
                QString user_sudoku_value = ui->tableWidget->item(i, j)->text();
                if (user_sudoku_value.toInt() > 0 && user_sudoku_value.toInt() < 10){
                    user_sudoku[i][j] = user_sudoku_value.toInt();
                }
                else{
                    is_zero = true;
                }
            }
        }

        if (is_zero == false){
            if (isCorrect() == true){
                ui->resultLabel->setText("Bingo!");
                ui->resultLabel->setVisible(true);
                QPalette sample_palette;
                sample_palette.setColor(QPalette::WindowText, QColor(0,221,0));

                ui->resultLabel->setAutoFillBackground(true);
                ui->resultLabel->setPalette(sample_palette); 

                // set default
                finish_status = true;
                sudoku_start = false;
                pause = true;
                ui->pause->setEnabled(false);
                ui->Undo->setEnabled(false);
                for (int i=0; i<rowCount; ++i){
                    for (int j=0; j<columnCount; ++j){
                        Qt::ItemFlags eFlags = ui->tableWidget->item(i, j)->flags();
                        eFlags &= ~Qt::ItemIsEditable;
                        ui->tableWidget->item(i, j)->setFlags(eFlags);
                    }
                }

                // if you finish
                QTime zeroTime(0, 0, 0);
                QTime display;
                display = zeroTime.addSecs(gammingPeriod);
                QString time_text = display.toString("hh : mm : ss");
                QString str = "<font size = 5 ><b>Hint Times = </b></font><u><font size = 5 color = red>" + QString::number(give_hint_times)
                        + "</u></font><br><br><b><font size = 5>Time = </b><u></font><font size = 5 color = red>" + time_text  + "</u></font></br></br>";

                QPixmap exportSuccess(":/Resources/img/myGrade.png");
                QMessageBox wb("Congradulation !",str,
                        QMessageBox::NoIcon,QMessageBox::Cancel | QMessageBox::Default,QMessageBox::NoButton,QMessageBox::NoButton);
                wb.setIconPixmap(exportSuccess);
                wb.exec();

            }
            else{
                ui->resultLabel->setText("Wrong!");
                ui->resultLabel->setVisible(true);
                QPalette sample_palette;
                sample_palette.setColor(QPalette::WindowText, Qt::red);

                ui->resultLabel->setAutoFillBackground(true);
                ui->resultLabel->setPalette(sample_palette);
            }
        }
        else{
            ui->resultLabel->setText("Wrong!");
            ui->resultLabel->setVisible(true);
            QPalette sample_palette;
            sample_palette.setColor(QPalette::WindowText, Qt::red);

            ui->resultLabel->setAutoFillBackground(true);
            ui->resultLabel->setPalette(sample_palette);
        }
    }
}

// Reset button //
void MainWindow::on_actionReset_triggered()
{
    if (sudoku_start){
        pause = false;
        ui->pause->setEnabled(true);
        ui->Undo->setEnabled(true);
        ui->pause->setText("Pause");
        ui->resultLabel->setVisible(false);
        ui->can_be_solve_lable->setVisible(false);
        gammingPeriod = 0;
        while(undo_step >= 0){
            //qDebug() <<  undo_step;
            if (ui->tableWidget->item(undo_row[undo_step], undo_column[undo_step])->backgroundColor() == QColor(255, 255, 255)){
                ui->tableWidget->item(undo_row[undo_step], undo_column[undo_step])->setText(QString(' '));
            }
            undo_column[undo_step] = 0;
            undo_row[undo_step] = 0;
            undo_value[undo_step] = 0;
            undo_step--;
        }
        if (undo_step < 0){
            undo_step = 0;
        }
        solve = false;
    }
}

// Pause button //
void MainWindow::on_pause_clicked()
{
    pause = !pause;
    if (pause){
        ui->pause->setText("Continue");
        if (gammingPeriod > 0)
            gammingPeriod = static_cast<int>(gammingPeriod);
        for (int i=0; i<rowCount; ++i){
            for (int j=0; j<columnCount; ++j){
                Qt::ItemFlags eFlags = ui->tableWidget->item(i, j)->flags();
                eFlags &= ~Qt::ItemIsEditable;
                ui->tableWidget->item(i, j)->setFlags(eFlags);
                --undo_step;
            }
        }
    }
    else{
        ui->pause->setText("Pause");
        for (int i=0; i<rowCount; ++i){
            for (int j=0; j<columnCount; ++j){
                if (ui->tableWidget->item(i, j)->backgroundColor() == QColor(255, 255, 255)){
                    Qt::ItemFlags eFlags = ui->tableWidget->item(i, j)->flags();
                    eFlags |= Qt::ItemIsEditable;
                    ui->tableWidget->item(i, j)->setFlags(eFlags);
                    --undo_step;
                }
            }
        }
    }
}

// Undo button //
void MainWindow::on_tableWidget_cellChanged(int row, int column)
{
    if (finish_status == false){
        ui->resultLabel->setVisible(false);
        ui->can_be_solve_lable->setVisible(false);
    }

    if (ui->tableWidget->item(row, column)->textColor() == Qt::black){
        string str = ui->tableWidget->item(row, column)->text().toStdString();
        char c = str[0] - 48;
        char c1 = str[1] - 48;

        if (editing_mode == true){
            if (((int(c) > 10) || (int(c) == 0) || (int(c1) > -48) || ui->tableWidget->item(row, column)->text().toInt() > 10)){
                ui->tableWidget->item(row, column)->setText(QString(' '));
            }
        }
        else if (sudoku_start && editing_mode == false){
            if (((int(c) > 10) || (int(c) == 0) || (int(c1) > -48) || ui->tableWidget->item(row, column)->text().toInt() > 10)){
                ui->tableWidget->item(row, column)->setText(QString(' '));
                --undo_step;
            }else{
                if (!undo){
                    undo_value[undo_step] = ui->tableWidget->item(row, column)->text().toInt();
                    undo_row[undo_step] = row;
                    undo_column[undo_step] = column;
                    ++undo_step;
                }
                else{
                    undo = false;
                }
            }
        }
    }
    else{
        // do nothing
    }
}
void MainWindow::on_Undo_clicked()
{
    undo = true;

    if (undo_step != 0 && !pause){
        --undo_step;
        if (undo_value[undo_step] == 0){
            on_Undo_clicked();
            return;
        }
        else{
            ui->tableWidget->item(undo_row[undo_step], undo_column[undo_step])->setText(QString(' '));
        }
    }
    else{
        undo = false;
    }
    undo_value[undo_step] = 0;
    undo_row[undo_step] = 0;
    undo_column[undo_step] = 0;
    if (undo_step < 0){
        undo_step =0;
    }
}

// Solve button //
void MainWindow::on_actionSolve_triggered()
{
    /***** check if the user's answer can solve or not *****/
    if (sudoku_start){
        bool check_sudoku_is_zero = false;
        for (int i=0; i<rowCount; ++i){
            for (int j=0; j<columnCount; ++j){
                if (ui->tableWidget->item(i, j)->text().toInt() == 0){
                    check_sudoku_is_zero = true;
                }
                
            }
        }

        if (check_sudoku_is_zero == true){
            /*****(a) check whether exist 0 on the table, if yes -> do (b), if no -> do (c) *****/
            bool check_solvable_within_user = false;

            // get value to user_sudoku
            for (int i=0; i<rowCount; ++i){
                for (int j=0; j<columnCount; ++j){
                    user_sudoku[i][j] = ui->tableWidget->item(i, j)->text().toInt();
                }
            }

            // try to solve the puzzle with user's answer
            if (SolveSudoku()){
                check_solvable_within_user = true;
            }
            else{
                check_solvable_within_user = false;
            }

            if (check_solvable_within_user == true){
                for (int i=0; i<rowCount; ++i){
                    for (int j=0; j<columnCount; ++j){
                        if(ui->tableWidget->item(i, j)->backgroundColor() == QColor(255, 255, 255) && ui->tableWidget->item(i, j)->text().toInt() == 0){
                            ui->tableWidget->item(i, j)->setTextColor(Qt::red);
                            Qt::ItemFlags eFlags = ui->tableWidget->item(i, j)->flags();
                            eFlags &= ~Qt::ItemIsEditable;
                            ui->tableWidget->item(i, j)->setFlags(eFlags);
                            ui->tableWidget->item(i, j)->setText(QString::number(user_sudoku[i][j]));
                        }
                    }
                }
            }
            else{
                for (int i=0; i<rowCount; ++i){
                    for (int j=0; j<columnCount; ++j){
                        if(ui->tableWidget->item(i, j)->backgroundColor() == QColor(255, 255, 255)){
                            ui->tableWidget->item(i, j)->setTextColor(Qt::red);
                            Qt::ItemFlags eFlags = ui->tableWidget->item(i, j)->flags();
                            eFlags &= ~Qt::ItemIsEditable;
                            ui->tableWidget->item(i, j)->setFlags(eFlags);
                            ui->tableWidget->item(i, j)->setText(QString::number(ans_sudoku[i][j]));
                        }
                    }
                }
            }

            /// // print solved // ///
            ui->resultLabel->setText(" Solved ");
            ui->resultLabel->setVisible(true);
            QPalette sample_palette;
            sample_palette.setColor(QPalette::WindowText, QColor(0,221,0));

            ui->resultLabel->setAutoFillBackground(true);
            ui->resultLabel->setPalette(sample_palette);

            solve = true;
        }
        else if (isCorrect()==false && check_sudoku_is_zero==false){
            ///  // print sudoku table can't be solved ! // ///
            ui->can_be_solve_lable->setText("Sudoku table can't be solved !");
            ui->can_be_solve_lable->setVisible(true);
            QPalette sample_palette;
            sample_palette.setColor(QPalette::WindowText, QColor(255,0,255));

            ui->can_be_solve_lable->setAutoFillBackground(true);
            ui->can_be_solve_lable->setPalette(sample_palette);
        }
        else{
            ///  // print sudoku table is solved ! // ///
            ui->can_be_solve_lable->setText("Sudoku table is already solved !");
            ui->can_be_solve_lable->setVisible(true);
            QPalette sample_palette;
            sample_palette.setColor(QPalette::WindowText, QColor(255,0,255));

            ui->can_be_solve_lable->setAutoFillBackground(true);
            ui->can_be_solve_lable->setPalette(sample_palette);
        }
        pause = true;
        ui->pause->setEnabled(false);
        ui->Undo->setEnabled(false);
    }
}

// Get Hint //
void MainWindow::on_actionGet_Hint_triggered()
{
    if (sudoku_start){
        /*****(a) check whether exist 0 on the table, if yes -> do (b), if no -> do (c) *****/
        bool check_sudoku_exist_zero = false;
        for (int i=0; i<rowCount; ++i){
            for (int j=0; j<columnCount; ++j){
                user_sudoku[i][j] = ui->tableWidget->item(i, j)->text().toInt();
                if (user_sudoku[i][j] == 0){
                    check_sudoku_exist_zero = true;
                }
            }
        }

        /*****(b) Choose a random position that is not question and not user's answer *****/
        if (check_sudoku_exist_zero == true){

            // check if puzzle can solve or not with user's answer
            bool check_solvable_within_user = false;

            // get value to user_sudoku
            for (int i=0; i<rowCount; ++i){
                for (int j=0; j<columnCount; ++j){
                    user_sudoku[i][j] = ui->tableWidget->item(i, j)->text().toInt();
                }
            }

            // try to solve the puzzle with user's answer
            if (SolveSudoku()){
                check_solvable_within_user = true;
            }
            else{
                check_solvable_within_user = false;
            }

            if (check_solvable_within_user == true){
                // find null space, store x and y in a array
                int null_space[81][2];
                int null_space_number = 0;
                for (int i=0; i<rowCount; ++i){
                    for (int j=0; j<columnCount; ++j){
                        if(ui->tableWidget->item(i, j)->text().toInt() == 0){
                            null_space[null_space_number][0] = i;
                            null_space[null_space_number][1] = j;
                            ++null_space_number;
                        }
                    }
                }
                // randomly find a position
                int rando = rand()%null_space_number;

                // set position to table, and change textcolor to orange
                ui->tableWidget->item(null_space[rando][0], null_space[rando][1])->setTextColor(QColor(255, 136, 0));
                ui->tableWidget->item(null_space[rando][0], null_space[rando][1])->setText(QString::number(user_sudoku[null_space[rando][0]][null_space[rando][1]]));
                Qt::ItemFlags eFlags = ui->tableWidget->item(null_space[rando][0], null_space[rando][1])->flags();
                eFlags &= ~Qt::ItemIsEditable;
                ui->tableWidget->item(null_space[rando][0], null_space[rando][1])->setFlags(eFlags);
                ++give_hint_times;
            }
            else{
                ++give_hint_times;
                // randomly choose x and y, and set ans_sudoku with that position
                while(true){
                    int rando_row = rand()%rowCount;
                    int rando_col = rand()%columnCount;
                    if (ui->tableWidget->item(rando_row, rando_col)->backgroundColor() == QColor(255, 255, 255) && ui->tableWidget->item(rando_row, rando_col)->text().toInt() != 0){
                        ui->tableWidget->item(rando_row, rando_col)->setTextColor(QColor(255, 136, 0));
                        ui->tableWidget->item(rando_row, rando_col)->setText(QString::number(ans_sudoku[rando_row][rando_col]));
                        Qt::ItemFlags eFlags = ui->tableWidget->item(rando_row, rando_col)->flags();
                        eFlags &= ~Qt::ItemIsEditable;
                        ui->tableWidget->item(rando_row, rando_col)->setFlags(eFlags);
                        //qDebug() << ans_sudoku[rando_row][rando_col];
                        break;
                    }
                    else{
                        continue;
                    }
                }
            }
        }
        else{
            // check if puzzle is already correct, if yes-> goto finish(), if no-> git hint
            if (isCorrect()){
                // do nothing
            }
            else{
                ++give_hint_times;
                while(true){
                    int rando_row = rand()%rowCount;
                    int rando_col = rand()%columnCount;
                    if (ui->tableWidget->item(rando_row, rando_col)->backgroundColor() == QColor(255, 255, 255) && ui->tableWidget->item(rando_row, rando_col)->textColor() == Qt::black){
                        ui->tableWidget->item(rando_row, rando_col)->setTextColor(QColor(255, 136, 0));
                        ui->tableWidget->item(rando_row, rando_col)->setText(QString::number(ans_sudoku[rando_row][rando_col]));
                        Qt::ItemFlags eFlags = ui->tableWidget->item(rando_row, rando_col)->flags();
                        eFlags &= ~Qt::ItemIsEditable;
                        ui->tableWidget->item(rando_row, rando_col)->setFlags(eFlags);
                        //qDebug() << ans_sudoku[rando_row][rando_col];
                        break;
                    }
                    else{
                        continue;
                    }
                }
            }
        }
    }
}

// timer //
void MainWindow::showTime(){
    QTime zeroTime(0, 0, 0);
    QTime display;
    display = zeroTime.addSecs(gammingPeriod);

    QString time_text = display.toString("hh : mm : ss");

    ui->Clock->setText(time_text);
    if(!pause){
        ++gammingPeriod;
    }
}

// About //
void MainWindow::on_actionAbout_triggered()
{
    aboutdialog->show();
}

// Shortcut //
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (sudoku_start){
        if (event->key() == Qt::Key_P){
            on_pause_clicked();
        }
        else if (event->key() == Qt::Key_R){
            on_actionReset_triggered();
        }
        else if (event->key() == Qt::Key_F){
            on_actionCheck_triggered();
        }
        else if (event->key() == Qt::Key_H){
            on_actionGet_Hint_triggered();
        }
        else if (event->key() == Qt::Key_S){
            on_actionSolve_triggered();
        }
        else if (event->key() == Qt::Key_U){
            on_Undo_clicked();
        }
        else{
            // do nothing
        }
    }
}

/****** tool ******/

// check sudoku valid
bool MainWindow::isCorrect()
{
    bool check_result;
    int check_arr[9];

    // check rows
    for(int i=0; i<rowCount; ++i)
    {
        for(int j=0; j<columnCount; ++j){
            check_arr[j] = user_sudoku[i][j];
        }
        check_result = checkUnity(check_arr);
        if(!check_result){
            return false;
        }
    }

    // check columns
    for(int i=0; i<rowCount; ++i)
    {
        for(int j=0; j<columnCount; ++j){
            check_arr[j] = user_sudoku[j][i];
        }
        check_result = checkUnity(check_arr);
        if(!check_result){
            return false;
        }
    }

    // check cells
    for(int k=0; k<3; ++k){
        for(int x=0; x<rowCount; x+=3)
        {
            for(int y=0; y<columnCount; ++y)
            {
                int x_tmp = x + y/3;
                int y_tmp = y%3 + k*3;
                check_arr[y] = user_sudoku[y_tmp][x_tmp];
            }
            check_result = checkUnity(check_arr);
            if(!check_result)
                return false;
        }
     }
    return true;
}

bool MainWindow::checkUnity(int arr[])
{
    int arr_unity[9]; // counters
    for(int i=0; i<9; ++i)
        arr_unity[i] = 0; // initialize
    for(int i=0; i<9; ++i)
        ++arr_unity[arr[i]-1]; // count
    for(int i=0; i<9; ++i)
        if(arr_unity[i] > 1) // all element
            return false; // must be 1
    return true;
}

void MainWindow::set_random_sudoku()
{
    // create first line number
    int arrayFrom_1_to_9[rowCount];
    for (int i=0; i<rowCount; ++i){
        arrayFrom_1_to_9[i] = 1;
    }

    int f[9];

    // set first line number
    for (int i=0; i<rowCount; ++i){
        int rand_i = rand()%rowCount;
        if (arrayFrom_1_to_9[rand_i] == 1){
            f[i] = rand_i + 1;
            --arrayFrom_1_to_9[rand_i];
        }
        else{
            --i;
            continue;
        }
    }

    // finish the random sudoku
    int tmp_sudoku[9][9] = {f[0],f[1],f[2],f[3],f[4],f[5],f[6],f[7],f[8],
                            f[3],f[4],f[5],f[6],f[7],f[8],f[0],f[1],f[2],
                            f[6],f[7],f[8],f[0],f[1],f[2],f[3],f[4],f[5],

                            f[7],f[8],f[0],f[1],f[2],f[3],f[4],f[5],f[6],
                            f[1],f[2],f[3],f[4],f[5],f[6],f[7],f[8],f[0],
                            f[4],f[5],f[6],f[7],f[8],f[0],f[1],f[2],f[3],

                            f[5],f[6],f[7],f[8],f[0],f[1],f[2],f[3],f[4],
                            f[8],f[0],f[1],f[2],f[3],f[4],f[5],f[6],f[7],
                            f[2],f[3],f[4],f[5],f[6],f[7],f[8],f[0],f[1]
                           };
    // set null space
    int null_space_number = difficulty + 1;
    while(--null_space_number){
        int rand_row = rand()%rowCount;
        int rand_col = rand()%columnCount;

        // check if the random position is already 0 or not
        if (tmp_sudoku[rand_row][rand_col] == 0){
            ++null_space_number;
            continue;
        }
        else{
            tmp_sudoku[rand_row][rand_col] = 0;
        }
    }




    for (int i=0; i<rowCount; ++i){
        for (int j=0; j<columnCount; ++j){
             user_sudoku[i][j] = tmp_sudoku[i][j];
        }
    }


    /*
    int rando;
    while(true){
        for(int i=0; i<9; ++i){
            for(int j=0; j<9; ++j){
                user_sudoku[i][j]=0;
            }
        }
        for(int i =0;i<=8;i++){
            rando=rand()%9;
            rando+=1;
            user_sudoku[i][i]=rando;

        }
        for(int i =0;i<=45;i++){
            ranrow=rand()%9;
            rancol=rand()%9;
            rando=rand()%9;
            rando+=1;
            if(isSafe(ranrow, rancol, rando)){
                user_sudoku[ranrow][rancol]=rando;
            }
        }

        if(SolveSudoku()){
            for(int i =0;i<=50;i++){
                int ranrow=rand()%9;
                int randcol=rand()%9;
                rando=rand()%9;
                rando+=1;
                user_sudoku[ranrow][randcol]=0;
            }
            for(int i=0; i<9; ++i){
                for(int j=0; j<9; ++j){
                    ui->tableWidget->item(i,j)->setText(QString::number(user_sudoku[i][j]));
                }
            }
            return;
        }
        else{
            continue;
        }
    }
    */
}

bool MainWindow::SolveSudoku()
{
    if(isCorrect()){
        int row, col, i=0;
        if (!FindUnassignedLocation(row, col))
           return true;

        for (int num = 1; num <= 9; num++)
        {
            if (isSafe(row, col, num))
            {
                user_sudoku[row][col] = num;

                if (SolveSudoku())
                    return true;

                user_sudoku[row][col] = UNASSIGNED;
            }
        }
        return false;
    }
    else{
        return false;
    }
}

bool MainWindow::FindUnassignedLocation(int &row, int &col)
{
    for (row = 0; row < N; row++)
            for (col = 0; col < N; col++)
                if (user_sudoku[row][col] == UNASSIGNED)
                    return true;
        return false;
}

bool MainWindow::UsedInRow(int row, int num)
{
    for (int col = 0; col < N; col++)
            if (user_sudoku[row][col] == num)
                return true;
    return false;
}

bool MainWindow::UsedInCol(int col, int num)
{
    for (int row = 0; row < N; row++)
            if (user_sudoku[row][col] == num)
                return true;
    return false;
}

bool MainWindow::UsedInBox(int boxStartRow, int boxStartCol, int num)
{
    for (int row = 0; row < 3; row++)
            for (int col = 0; col < 3; col++)
                if (user_sudoku[row+boxStartRow][col+boxStartCol] == num)
                    return true;
    return false;
}

bool MainWindow::isSafe(int row, int col, int num)
{
    return !UsedInRow(row,num) && !UsedInCol(col,num) && !UsedInBox(row - row%3 , col - col%3, num);
}

