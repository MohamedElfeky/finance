#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql/QSqlDatabase>
#include <QSqlTableModel>

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

    Ui::MainWindow *ui;

    QStringList tableHeader;

private slots:
    void btn_clicked_newEntry(void);
    void table_valueChanged(int row, int col);
};

#endif // MAINWINDOW_H
