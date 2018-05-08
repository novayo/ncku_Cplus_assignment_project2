#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QTableWidget>
#include <vector>
#include <QTimer>
#include <QDateTime>
#include <string>

using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    bool isCorrect();
    bool checkUnity(int arr[]);
    bool SolveSudoku();
    bool FindUnassignedLocation(int &row, int &col);
    bool UsedInRow(int row, int num);
    bool UsedInCol(int col,int num);
    bool UsedInBox(int boxStartRow, int boxStartCol, int num);
    bool isSafe(int row, int col, int num);
    void set_random_sudoku();

    ~MainWindow();

private slots:
    void showTime();
    void on_tableWidget_cellChanged(int row, int column);
    void on_actionCheck_triggered();
    void on_actionSolve_triggered();
    void on_actionReset_triggered();
    void on_actionsetManually_triggered();
    void on_pause_clicked();
    void on_NewPuzzle_clicked();
    void on_Undo_clicked();

    void on_actionGet_Hint_triggered();

private:
    Ui::MainWindow *ui;
    int user_text_font;
    int question_text_font;
    int gammingPeriod;
    int undo_step;
    int ranrow;
    int rancol;
    bool pause;
    bool sudoku_start;
    bool undo;
    bool solve;
    bool editing_mode;
    const int rowCount;
    const int columnCount;
    vector<int> user_sudoku_row;
    vector< vector<int> > user_sudoku;
    vector<int> undo_value;
    vector<int> undo_row;
    vector<int> undo_column;
    QTime ctime;
    QTimer *timer;
};

#endif // MAINWINDOW_H
