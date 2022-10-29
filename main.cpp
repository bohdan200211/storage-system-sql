#include "cmakevars.h"

#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <QDebug>

void initDataBase() {}

void execQuery(const QString &queryStr)
{
    QSqlQuery query;
    query.exec(queryStr);

    if (query.lastError().type() != QSqlError::NoError)
        qDebug() << query.lastError();
}

int main(int argc, char *argv[])
{
    //    initDataBase();

    const QString dbPath = QDir(DB_DIR_PATH).absoluteFilePath(QStringLiteral("books.db"));

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);
    db.open();

    execQuery("delete from library");

    execQuery(
        "create table library ("
        "id integer not null primary key,"
        "name varchar(255),"
        "cost integer)");

    execQuery(
        "insert into library (name, cost) "
        "values ('qwerty1', 25)");

    execQuery(
        "insert into library (name, cost) "
        "values ('qwerty2', 50)");

    execQuery(
        "insert into library (name, cost) "
        "values ('qwerty3', 75)");

    execQuery(
        "insert into library (name, cost) "
        "values ('qwerty4', 77)");

    execQuery(
        "insert into library (name, cost) "
        "values ('qwerty5', 80)");

    execQuery(
        "delete from library "
        "where cost>=75 and cost<80");

    execQuery(
        "update library "
        "set name='changed' "
        "where id=1");

    QSqlQuery query;
    query.exec("select * from library");

    while (query.next())
    {
        qDebug() << query.value(0).toString();
        qDebug() << query.value(1).toString();
        qDebug() << query.value(2).toString();
    }

    qDebug() << dbPath;
}
