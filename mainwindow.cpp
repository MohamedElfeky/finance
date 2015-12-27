#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QDebug>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>
#include <QDateTime>
#include <QSqlTableModel>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qcustomplot.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->btn_entry, SIGNAL(clicked()), this, SLOT(btn_clicked_newEntry()));
    connect(ui->table_entrys, SIGNAL(cellChanged(int,int)), this, SLOT(table_valueChanged(int,int)));

    // Create global database object, open and connect
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName( "./finance.db" );

    if(!db.open())
    {
        qDebug() << db.lastError();
        qFatal("Failed to connect to database.");
    }

    qDebug("Connected!");

    tableHeader << "Timestamp" << "Business" << "Category" << "Article" << "Price";
    ui->table_entrys->setColumnCount(tableHeader.count());
    ui->table_entrys->setHorizontalHeaderLabels(tableHeader);

    QSqlQuery qry;
    qry.prepare(
        "CREATE TABLE IF NOT EXISTS"
        "`revenues` ("
            "`"+ tableHeader.at(0) +"` INTEGER,"
            "`"+ tableHeader.at(1) +"` TEXT,"
            "`"+ tableHeader.at(2) +"` TEXT,"
            "`"+ tableHeader.at(3) +"` TEXT,"
            "`"+ tableHeader.at(4) +"` REAL"
        ")"
    );
    if( !qry.exec() )
        qDebug() << qry.lastError();
    else
        qDebug() << "Initial table created!";

    refreshTable();
}

void MainWindow::refreshTable(void)
{
    QSqlQuery qry;
    qry.prepare( "SELECT * FROM revenues" );
    if( !qry.exec() )
        qDebug() << qry.lastError();
    else
    {
        qDebug( "Selected!" );

        ui->table_entrys->blockSignals(true); // Block signals so that the signal only gets fired during user input

        QSqlRecord rec = qry.record();
        QTableWidgetItem *itab;

        for(int r = 0; qry.next(); r++)
        {
            ui->table_entrys->setRowCount(r + 1);
            for(int c = 0; c < rec.count(); c++)
            {
                itab = ui->table_entrys->item(r, c);
                if(itab == NULL)
                {
                    itab = new QTableWidgetItem;
                    ui->table_entrys->setItem(r, c, itab);
                }
            }
            QDateTime t;
            t.setTime_t(qry.value(0).toInt());
            ui->table_entrys->item(r, 0)->setText(t.toString("dd.MM.yyyy"));
            ui->table_entrys->item(r, 1)->setText(qry.value(1).toString());
            ui->table_entrys->item(r, 2)->setText(qry.value(2).toString());
            ui->table_entrys->item(r, 3)->setText(qry.value(3).toString());
            ui->table_entrys->item(r, 4)->setText(qry.value(4).toString());
        }

        ui->table_entrys->blockSignals(false);
    }
}

MainWindow::~MainWindow()
{
    ui->table_entrys->blockSignals(true);
    QTableWidgetItem *itab;
    for(int r = 0; r < ui->table_entrys->rowCount(); r++)
    {
        for(int c = 0; c < ui->table_entrys->rowCount(); c++)
        {
            itab = ui->table_entrys->item(r, c);
            delete itab;
        }
    }
    delete ui;
    qDebug() << "destroyed mainwindow";
}

/*
 * Slot: Button "create new entry" clicked
 */
void MainWindow::btn_clicked_newEntry(void)
{
    QSqlQuery qry;
    qry.prepare(
        "INSERT INTO"
        "`revenues`("
            "`"+ tableHeader.at(0) +"`, `"+ tableHeader.at(1) +"`, `"+ tableHeader.at(2) +"`, `"+ tableHeader.at(3) +"`, `"+ tableHeader.at(4) +"`"
        ") VALUES ("
            + QString::number(QDateTime::currentDateTime().toTime_t()) + ",'"
            + ui->ln_business->text() + "','"
            + ui->ln_category->text() + "','"
            + ui->ln_article->text() + "',"
            + QString::number(ui->spn_price->value()) +
        ")"
    );
    if( !qry.exec() )
        qDebug() << qry.lastError();
    else
        qDebug("Inserted!");

    refreshTable();
}

/*
 * Slot: Table value changed
 */

void MainWindow::table_valueChanged(int row, int col)
{
    QSqlQuery qry;
    qry.prepare(
        "UPDATE `revenues`"
        "SET `"+ tableHeader.at(col) +"`='" + ui->table_entrys->item(row, col)->text() + "'"
        "WHERE `_rowid_`='" + QString::number(row + 1) + "'"
    );
    if(!qry.exec())
        qDebug() << qry.lastError();
    else
        qDebug("Updated!");
}
