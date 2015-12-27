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

    connect(ui->date_start, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(startDateChanged(QDateTime)));
    connect(ui->spn_dailyBudget, SIGNAL(valueChanged(double)), this, SLOT(dailyBudgetChanged(double)));

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
    // Create configuration table
    qry.prepare(
        "CREATE TABLE IF NOT EXISTS"
        "`config` ("
            "`startdate` INTEGER UNIQUE,"
            "`dailybudget` REAL"
        ")"
    );
    if( !qry.exec() )
        qDebug() << qry.lastError();
    else
        qDebug() << "Initial config table created!";

    qry.prepare( "SELECT * FROM `config`");
    if( !qry.exec() )
        qDebug() << qry.lastError();
    else
    {
        qDebug( "Selected config!" );

        QSqlRecord rec = qry.record();

        if(rec.count() == 0)
        {
            qry.prepare(
                "INSERT INTO"
                "`config`("
                    "`startdate`,`dailybudget`"
                ") VALUES ("
                    "0, 0.0"
                ")"
            );
            if( !qry.exec() )
                qDebug() << qry.lastError();
            else
                qDebug() << "Initial config table entry created!";
        }
        else
        {
            qry.next();
            QDateTime dt = QDateTime::fromTime_t(qry.value(0).toInt());
            ui->date_start->setDateTime(dt);
            ui->spn_dailyBudget->setValue(qry.value(1).toDouble());
        }
    }

    // Create data table
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
        qDebug() << "Initial data table created!";

    refreshTable();

    // Graph
    price_time = QVector<double>(PRICE_HISTORY);
    price_time = QVector<double>(PRICE_HISTORY);
    for(int i = 0; i < PRICE_HISTORY; i++)
    {
        price_time[i] = -i;
    }
    ui->plot_month->addGraph();
    ui->plot_month->graph(0)->setData(price_time, price_day);
    ui->plot_month->xAxis->setLabel("Days");
    ui->plot_month->xAxis->setRange(-PRICE_HISTORY, 0);
    ui->plot_month->yAxis->setVisible(false); // Display the price at the right side
    ui->plot_month->yAxis2->setVisible(true);
    ui->plot_month->yAxis2->setLabel("Price");
    ui->plot_month->setBackground(Qt::transparent);
    ui->plot_month->setAttribute(Qt::WA_OpaquePaintEvent, false);
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
 * Refresh table content based on database
 */
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

                    if(tableHeader.at(c) == "Timestamp") // Make column Timestamp non-editable
                    {
                        ui->table_entrys->item(r, c)->setFlags(ui->table_entrys->item(r, c)->flags() & ~Qt::ItemIsEditable);
                    }
                }
            }
            QDateTime t;
            t.setTime_t(qry.value(0).toInt());
            ui->table_entrys->item(r, 0)->setText(t.toString("dd.MM.yyyy hh:mm:ss"));
            ui->table_entrys->item(r, 1)->setText(qry.value(1).toString());
            ui->table_entrys->item(r, 2)->setText(qry.value(2).toString());
            ui->table_entrys->item(r, 3)->setText(qry.value(3).toString());
            ui->table_entrys->item(r, 4)->setText(qry.value(4).toString());
        }

        ui->table_entrys->blockSignals(false);
    }
}

/*
 * Refresh graph with the current budget calculation
 */
void MainWindow::refreshGraph(void)
{

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
    QString tableText = ui->table_entrys->item(row, col)->text();

    QSqlQuery qry;
    qry.prepare(
        "UPDATE `revenues`"
        "SET `"+ tableHeader.at(col) +"`='" + tableText + "'"
        "WHERE `_rowid_`='" + QString::number(row + 1) + "'"
    );
    if(!qry.exec())
        qDebug() << qry.lastError();
    else
        qDebug("Updated!");
}

/*
 * Slot: Config: Start Date for budget calculation changed
 */
void MainWindow::startDateChanged(QDateTime d)
{
    QSqlQuery qry;
    qry.prepare(
        "UPDATE `config`"
        "SET `startdate`='" + QString::number(d.toTime_t()) + "'"
        "WHERE `_rowid_`='1'"
    );
    if(!qry.exec())
        qDebug() << qry.lastError();
    else
        qDebug("Updated config: startdate!");
}

/*
 * Slot: Config: Daily budget changed
 */
void MainWindow::dailyBudgetChanged(double newBudget)
{
    QSqlQuery qry;
    qry.prepare(
        "UPDATE `config`"
        "SET `dailybudget`='" + QString::number(newBudget) + "'"
        "WHERE `_rowid_`='1'"
    );
    if(!qry.exec())
        qDebug() << qry.lastError();
    else
        qDebug("Updated config: dalyBudget!");
}
