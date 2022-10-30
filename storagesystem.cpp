#include "storagesystem.h"
#include "cmakevars.h"

#include <QDebug>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

namespace Debug
{
bool execQuery(const QString &queryStr)
{
    QSqlQuery query;
    query.exec(queryStr);

    if (query.lastError().type() != QSqlError::NoError)
    {
        qDebug() << query.lastError();
        return false;
    }

    return true;
}
} // namespace Debug

bool StorageSystem::initDataBase()
{
    bool res = true;
    QSqlQuery query(getDb());

    // create Materials table
    res &= Debug::execQuery(
        "create table materials ("
        "id integer not null primary key,"
        "type varchar(255),"
        "amount integer,"
        "storageid integer)");

    // create Storages table
    res &= Debug::execQuery(
        "create table storages ("
        "id integer not null primary key,"
        "name varchar(255) unique,"
        "contain integer)");

    // create History table
//    res &= Debug::execQuery(
//        "create table history ("
//        "id integer not null primary key,"
//        "fromstorageid integer,"
//        "tostorageid integer,"
//        "materialid integer)");

    return res;
}

bool StorageSystem::clearDataBase()
{
    bool res = true;
    QSqlQuery query(getDb());

    res &= Debug::execQuery("delete from materials");
    res &= Debug::execQuery("delete from storages");

    return res;
}

bool StorageSystem::addStorage(const QString &storageName)
{
    QSqlQuery query(getDb());

    return Debug::execQuery(QStringLiteral("insert into storages (name, contain) "
                                           "values ('%0', %1)")
                                .arg(storageName)
                                .arg(0));
}

bool StorageSystem::addMaterial(const QString &materialName, int amount, const QString &storageName)
{
    QSqlQuery query(getDb());
    const int storageId = getStorageIdByName(storageName);

    query.exec(QStringLiteral("select name, contain "
                              "from storages where id=%0")
                   .arg(storageId));
    if (query.next() && !query.isValid())
    {
        qDebug() << "storageId does not exist!";
        return false;
    }

    bool res = true;
    res &= Debug::execQuery(QStringLiteral("update storages "
                                           "set contain=%0 "
                                           "where id=%1")
                                .arg(query.value(1).toInt() + amount)
                                .arg(storageId));

    res &= Debug::execQuery(QStringLiteral("insert into materials (type, amount, storageid) "
                                           "values ('%0', %1, %2)")
                                .arg(materialName)
                                .arg(amount)
                                .arg(storageId));

    return res;
}

bool StorageSystem::removeStorage(const QString &storageName)
{
    QSqlQuery query(getDb());

    // remove all materials from the storage
    query.exec(QStringLiteral("delete from materials "
                              "where storageid=%0")
                   .arg(getStorageIdByName(storageName)));

    bool res = true;

    // remove storage
    res &= Debug::execQuery(QStringLiteral("delete from storages "
                                           "where name='%0'")
                                .arg(storageName));

    return res;
}

bool StorageSystem::removeMaterial(const QString &materialName, int amount,
                                   const QString &storageName)
{
    QSqlQuery query(getDb());
    const int storageId = getStorageIdByName(storageName);

    query.exec(QStringLiteral("select name, contain "
                              "from storages where id=%0")
                   .arg(storageId));
    if (query.next() && !query.isValid())
    {
        qDebug() << "storageId does not exist!";
        return false;
    }

    bool res = true;

    res &= Debug::execQuery(QStringLiteral("update storages "
                                           "set contain=%0 "
                                           "where id=%1")
                                .arg(query.value(1).toInt() - amount)
                                .arg(storageId));

    res &= Debug::execQuery(QStringLiteral("delete from materials "
                                           "where type='%0' and amount=%1 and storageid='%2'")
                                .arg(materialName)
                                .arg(amount)
                                .arg(getStorageIdByName(storageName)));

    return res;
}

QSqlDatabase &StorageSystem::getDb()
{
    static QSqlDatabase db;

    if (!db.isValid())
    {
        const QString dbPath = QDir(DB_DIR_PATH)
                                   .absoluteFilePath(QStringLiteral("storage-system.db"));
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(dbPath);
        db.open();
    }

    return db;
}

int StorageSystem::getStorageIdByName(const QString &storageName)
{
    QSqlQuery query(getDb());

    query.exec(QStringLiteral("select id "
                              "from storages where name='%0'")
                   .arg(storageName));

    if (query.next() && !query.isValid())
    {
        qDebug() << "The storageName does not exist!";
        return false;
    }

    return query.value(0).toInt();
}
