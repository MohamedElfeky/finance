#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql/QSqlDatabase>
#include <QSqlTableModel>
#include <QDate>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void refreshTable(void);
    void refreshGraph(void);
    void refreshOutgoings(void);

    Ui::MainWindow *ui;

    QStringList tableHeader;
    #define PRICE_HISTORY 31 // Show the last 31 values
    QVector<double> price_day, price_time; // Stores the value of the graph

private slots:
    void btn_clicked_newEntry(void);
    void table_valueChanged(int row, int col);
    void startDateChanged(QDateTime d);
    void dailyBudgetChanged(double newBudget);
};

#endif // MAINWINDOW_H
