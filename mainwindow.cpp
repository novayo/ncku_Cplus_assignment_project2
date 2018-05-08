#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QHeaderView>
#include <QApplication>
#include <QProcess>
#include <QTableWidgetItem>
#include <QAbstractItemView>
#include <ctime>

#define UNASSIGNED 0
#define N 9

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), rowCount(9), columnCount(9), gammingPeriod(0), pause(true), sudoku_start(false), undo(false), editing_mode(false)
{
    ui->setupUi(this);

    // set text font
    user_text_font = 30;
    question_text_font = 20;

    // set user_sudoku
    user_sudoku_row.assign(rowCount, 0);
    user_sudoku.assign(columnCount, user_sudoku_row);

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
    ui->resultLabel->setVisible(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Finish button->setText(QString::number(user_sudoku[i][j]));
void MainWindow::on_actionCheck_triggered()
{
    if (sudoku_start){
        // get table widget value into vector
        for (int i=0; i<rowCount; ++i){
            for (int j=0; j<columnCount; ++j){
                QString user_sudoku_value = ui->tableWidget->item(i, j)->text();
                if (user_sudoku_value.toInt() > 0 && user_sudoku_value.toInt() < 10){
                    user_sudoku[i][j] = user_sudoku_value.toInt();
                }
            }
        }

        if (isCorrect() == true){
            ui->resultLabel->setText("Bingo");
            ui->resultLabel->setVisible(true);
            QPalette sample_palette;
            sample_palette.setColor(QPalette::WindowText, Qt::green);

            ui->resultLabel->setAutoFillBackground(true);
            ui->resultLabel->setPalette(sample_palette);

            qDebug() << "yeeeeeeeeeeeeeeeeeeeeeeeeeeeah!";
        }
        else{
            ui->resultLabel->setText("Wrong");
            ui->resultLabel->setVisible(true);
            QPalette sample_palette;
            sample_palette.setColor(QPalette::WindowText, Qt::red);

            ui->resultLabel->setAutoFillBackground(true);
            ui->resultLabel->setPalette(sample_palette);
            qDebug() << "Wrongggggggggggggggggggggggggg!";
        }
    }
}

void MainWindow::on_actionsetManually_triggered()
{
    editing_mode = true;
    this->setWindowTitle("NCKU_Project_2(Editing)");

    for (int i=0; i<rowCount; ++i){
        for (int j=0; j<columnCount; ++j){
            // set text default
            QFont fnt;
            fnt.setPointSize(user_text_font);
            ui->tableWidget->item(i, j)->setFont(fnt);
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
        }
    }
}

void MainWindow::on_actionSolve_triggered()
{
    bool check_sudoku_is_zero = false;
    for (int i=0; i<rowCount; ++i){
        for (int j=0; j<columnCount; ++j){
            user_sudoku[i][j] = ui->tableWidget->item(i, j)->text().toInt();
            if (user_sudoku[i][j] == 0)
                check_sudoku_is_zero = true;
        }
    }

    if (check_sudoku_is_zero == true){
        if(SolveSudoku()){
            for (int i=0; i<rowCount; ++i){
                for (int j=0; j<columnCount; ++j){
                    ui->tableWidget->item(i, j)->setText(QString::number(user_sudoku[i][j]));
                }
            }
           /// // print solvable // ///
            qDebug() << "solvable";
        }
        else{
            ///  // print unsolvable // ///
            qDebug() << "unsolvable";
        }
    }
    else if (isCorrect()==false && check_sudoku_is_zero==false){
        ///  // print sudoku table can't be solved ! // ///
        qDebug() << "sudoku table can't be solved !";
    }
    else{
        ///  // print sudoku table is solved ! // ///
        qDebug() << "sudoku table is already solved !";
    }
}

void MainWindow::on_actionReset_triggered()
{
    if (sudoku_start){
        pause = false;
        ui->pause->setText("pause");
        gammingPeriod = 0;
        while(undo_step >= 0){
            qDebug() <<  undo_step;
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
    }
}

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

// start new game //
void MainWindow::on_NewPuzzle_clicked()
{
    this->setWindowTitle("NCKU_Project_2(Gamming)");
    srand(time(NULL));
    /***** set default *****/
    // default sudoku_start
    sudoku_start = false;

    // default pause button
    pause = false;
    ui->pause->setText("Pause");
    ui->pause->setEnabled(true);

    // default timer time
    gammingPeriod = 0;
    ui->Clock->setVisible(true);

    // default text
    QFont fnt;
    fnt.setPointSize(user_text_font);
    for (int i=0; i<rowCount; ++i){
        for (int j=0; j<columnCount; ++j){
            ui->tableWidget->item(i, j)->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->item(i, j)->setFont(fnt);
            ui->tableWidget->item(i, j)->setBackgroundColor(QColor(255, 255, 255));
            Qt::ItemFlags eFlags = ui->tableWidget->item(i, j)->flags();
            eFlags |= Qt::ItemIsEditable;
            ui->tableWidget->item(i, j)->setFlags(eFlags);
        }
    }
/*
    // default sudoku table
    int s[9][9] = {0,0,4,0,3,0,0,0,0,   // difficult
                   0,0,5,0,0,1,0,7,0,
                   0,9,0,0,0,0,0,0,6,
                   0,1,0,0,0,0,0,0,3,
                   3,0,0,0,0,7,0,0,1,
                   2,0,0,5,0,0,0,6,0,
                   7,0,0,0,0,0,0,8,0,
                   0,5,0,6,0,0,7,0,0,
                   0,0,0,0,8,0,4,0,0
                  };
    int s1[9][9] = {3,0,0,0,0,1,0,2,4,  // mid
                    0,5,0,3,0,0,0,0,1,
                    0,1,0,0,0,8,0,7,0,
                    0,7,0,4,0,5,0,0,8,
                    0,0,3,2,0,0,9,0,0,
                    6,0,0,0,3,0,0,4,0,
                    0,8,0,9,0,0,0,6,0,
                    4,0,0,0,0,2,0,8,0,
                    7,9,0,8,0,0,0,0,2
                   };
    int s2[9][9] = {0,7,9,0,3,4,1,0,6,  // easy
                    4,0,0,0,0,5,0,9,0,
                    0,5,0,0,9,0,0,0,7,
                    0,0,5,9,0,1,0,4,0,
                    7,6,0,0,0,8,0,3,5,
                    0,2,0,6,0,0,7,0,0,
                    6,0,0,0,1,0,0,8,0,
                    0,4,0,5,0,0,0,0,9,
                    5,0,2,4,6,0,3,7,0
                   };*/
    int question[9][9];
    int choice = rand()%3;

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
        /*
        switch (choice) {
        case 0:
            for (int i=0; i<rowCount; ++i){
                for (int j=0; j<columnCount; ++j){
                    question[i][j] = s[i][j];
                }
            }
            break;
        case 1:
            for (int i=0; i<rowCount; ++i){
                for (int j=0; j<columnCount; ++j){
                    question[i][j] = s1[i][j];
                }
            }
            break;
        case 2:
            for (int i=0; i<rowCount; ++i){
                for (int j=0; j<columnCount; ++j){
                    question[i][j] = s2[i][j];
                }
            }
            break;
        default:
            break;
        }
        */
    }

    //
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

    // sudoku start !!!
    sudoku_start = true;
    editing_mode = false;
}

void MainWindow::set_random_sudoku()
{
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
            qDebug("%d",rando);
        }
        for(int i =0;i<=45;i++){
            ranrow=rand()%9;
            rancol=rand()%9;
            rando=rand()%9;
            rando+=1;
            if(isSafe(ranrow)){
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
}

void MainWindow::on_pause_clicked()
{
    pause = !pause;
    if (pause){
        ui->pause->setText("Continue");
        --gammingPeriod;
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
                Qt::ItemFlags eFlags = ui->tableWidget->item(i, j)->flags();
                eFlags |= Qt::ItemIsEditable;
                ui->tableWidget->item(i, j)->setFlags(eFlags);
                --undo_step;
            }
        }
    }
}

// set undo
void MainWindow::on_tableWidget_cellChanged(int row, int column)
{
    ui->resultLabel->setVisible(false);
    string str = ui->tableWidget->item(row, column)->text().toStdString();
    char c = str[0] - 48;

    if (!editing_mode){
        if (sudoku_start && ((int(c) > 10) || ui->tableWidget->item(row, column)->text().toInt() > 10)){
            ui->tableWidget->item(row, column)->setText(QString(' '));
            --undo_step;
        }else{
            if (sudoku_start && !undo){
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
        if(arr_unity[i] != 1) // all element
            return false; // must be 1
    return true;
}

bool MainWindow::SolveSudoku()
{
    if (!FindUnassignedLocation())
       return true;

    for (int num = 1; num <= 9; num++)
    {
        if (isSafe(num))
        {
            user_sudoku[row][col] = num;

            if (SolveSudoku())
                return true;

            user_sudoku[row][col] = UNASSIGNED;
        }
    }
    return false;
}

bool MainWindow::FindUnassignedLocation()
{
    for (row = 0; row < N; row++)
        for (col = 0; col < N; col++)
            if (user_sudoku[row][col] == UNASSIGNED)
                return true;
    return false;
}

bool MainWindow::UsedInRow(int rrow, int num)
{
    for (int ccol = 0; ccol < N; ccol++)
        if (user_sudoku[rrow][ccol] == num)
            return true;
    return false;
}

bool MainWindow::UsedInCol(int ccol, int num)
{
    for (int rrow = 0; rrow < N; rrow++)
        if (user_sudoku[rrow][ccol] == num)
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

bool MainWindow::isSafe(int num)
{
    return !UsedInRow(row,num) && !UsedInCol(col,num) && !UsedInBox(row - row%3 , col - col%3, num);
}










