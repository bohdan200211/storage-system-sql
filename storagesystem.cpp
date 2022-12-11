#include "storagesystem.h"
#include "cmakevars.h"
#include "fort.hpp"

#include <iostream>

#include <QDateTime>
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

void StorageSystem::printAllStorageStatus()
{
    QSqlQuery query(getDb());
    query.exec(QStringLiteral("select name from storages"));

    QVector<QString> storageNameList{};
    while (query.next())
        storageNameList.append(query.value(0).toString());

    fort::char_table table;

    table << fort::header;

    table[0][0] = "mt\\st";
    table[0][0].set_cell_content_text_style(fort::text_style::bold);
    table[0][0].set_cell_text_align(fort::text_align::center);

    table[1][0] = "Name";
    table[2][0] = "Amount";

    table[1][0].set_cell_content_fg_color(fort::color::red);
    table[2][0].set_cell_content_fg_color(fort::color::red);

    table[1][0].set_cell_text_align(fort::text_align::center);
    table[2][0].set_cell_text_align(fort::text_align::center);

    int columsOffset = 1;
    for (int i = 0; i < storageNameList.size(); ++i)
    {
        query.exec(QStringLiteral("select type, amount "
                                  "from materials where storageid=%0")
                       .arg(getStorageIdByName(storageNameList.at(i))));

        table[0][columsOffset] = storageNameList.at(i).toStdString();
        table[0][columsOffset].set_cell_content_fg_color(fort::color::cyan);
        table[0][columsOffset].set_cell_text_align(fort::text_align::center);

        int columsCounter = 0;
        while (query.next())
        {
            int absoluteOffset = columsCounter + columsOffset;
            table[1][absoluteOffset] = query.value(0).toString().toStdString();
            table[2][absoluteOffset] = std::to_string(query.value(1).toInt());

            table[1][absoluteOffset].set_cell_content_fg_color(fort::color::light_green);
            table[2][absoluteOffset].set_cell_content_fg_color(fort::color::light_green);

            table[1][absoluteOffset].set_cell_text_align(fort::text_align::center);
            table[2][absoluteOffset].set_cell_text_align(fort::text_align::center);

            ++columsCounter;
        }
        if (columsCounter == 0)
            columsCounter = 1;

        table[0][columsOffset].set_cell_span(columsCounter);
        columsOffset += columsCounter;
    }

    std::cout << table.to_string() << std::endl;
}

void StorageSystem::printStorageStatus(const QString &storageName)
{
    QSqlQuery query(getDb());
    query.exec(QStringLiteral("select type, amount "
                              "from materials where storageid=%0")
                   .arg(getStorageIdByName(storageName)));

    fort::char_table table;

    table << fort::header;

    table[0][0] = storageName.toStdString();
    table[0][0].set_cell_content_text_style(fort::text_style::bold);
    table[0][0].set_cell_content_fg_color(fort::color::cyan);
    table[0][0].set_cell_text_align(fort::text_align::center);

    table[1][0] = "Name";
    table[2][0] = "Amount";

    table[1][0].set_cell_content_fg_color(fort::color::light_red);
    table[2][0].set_cell_content_fg_color(fort::color::light_red);

    table[1][0].set_cell_text_align(fort::text_align::center);
    table[2][0].set_cell_text_align(fort::text_align::center);

    int columsCounter = 1;
    while (query.next())
    {
        table[1][columsCounter] = query.value(0).toString().toStdString();
        table[2][columsCounter] = std::to_string(query.value(1).toInt());

        table[1][columsCounter].set_cell_content_fg_color(fort::color::light_green);
        table[2][columsCounter].set_cell_content_fg_color(fort::color::light_green);

        table[1][columsCounter].set_cell_text_align(fort::text_align::center);
        table[2][columsCounter].set_cell_text_align(fort::text_align::center);

        ++columsCounter;
    }

    table[0][0].set_cell_span(columsCounter);

    std::cout << table.to_string() << std::endl;
}

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
        "name varchar(255) unique)");

    // create History tables
    res &= Debug::execQuery(
        "create table history ("
        "id integer not null primary key,"
        "dataactiontype interger,"
        // material part
        "type varchar(255),"
        "amount integer,"
        "storagename varchar(255),"
        "price integer,"
        // ~material part
        "timestamp integer)");

    return res;
}

bool StorageSystem::clearDataBase()
{
    bool res = true;
    QSqlQuery query(getDb());

    res &= Debug::execQuery("drop table materials");
    res &= Debug::execQuery("drop table storages");
    res &= Debug::execQuery("drop table history");

    return res;
}

bool StorageSystem::addStorage(const QString &storageName)
{
    QSqlQuery query(getDb());

    return Debug::execQuery(QStringLiteral("insert into storages (name) "
                                           "values ('%0')")
                                .arg(storageName));
}

bool StorageSystem::moveMaterials(const QString &fromStorage, const QString &toStorage,
                                  const QString &materialName, int amount, int sellPrice)
{
    if (fromStorage.isEmpty())
    {
        addMaterial(materialName, amount, toStorage);
    }
    else if (removeMaterial(materialName, amount, fromStorage))
    {
        addMaterial(materialName, amount, toStorage);

        // history track
        addHistoryRecord(DataActionType::Remove, materialName, amount, fromStorage, sellPrice);
        addHistoryRecord(DataActionType::Add, materialName, amount, toStorage, sellPrice);
    }
    else
    {
        return false;
    }
    return true;
}

bool StorageSystem::addMaterial(const QString &materialName, int amount, const QString &storageName)
{
    QSqlQuery query(getDb());
    const int storageId = getStorageIdByName(storageName);

    query.exec(QStringLiteral("select name "
                              "from storages where id=%0")
                   .arg(storageId));

    if (!query.next() && !query.isValid())
    {
        qDebug() << "addMaterial(): storageId does not exist!";
        return false;
    }

    bool res = true;

    query.exec(QStringLiteral("select id, amount from materials "
                              "where type='%0' and storageid='%2'")
                   .arg(materialName)
                   .arg(getStorageIdByName(storageName)));

    if (!query.next() && !query.isValid())
    {
        res &= Debug::execQuery(QStringLiteral("insert into materials (type, amount, storageid) "
                                               "values ('%0', %1, %2)")
                                    .arg(materialName)
                                    .arg(amount)
                                    .arg(storageId));
    }
    else
    {
        res &= Debug::execQuery(QStringLiteral("update materials set amount=%0 "
                                               "where id=%1")
                                    .arg(query.value(1).toInt() + amount)
                                    .arg(query.value(0).toInt()));
    }

    return res;
}

bool StorageSystem::removeMaterial(const QString &materialName, int amount,
                                   const QString &storageName)
{
    QSqlQuery query(getDb());
    const int storageId = getStorageIdByName(storageName);

    query.exec(QStringLiteral("select name "
                              "from storages where id=%0")
                   .arg(storageId));
    if (!query.next() && !query.isValid())
    {
        qDebug() << "storageId does not exist!";
        return false;
    }

    bool res = true;

    // is enough amount?
    query.exec(QStringLiteral("select id, amount from materials "
                              "where type='%0' and storageid='%2'")
                   .arg(materialName)
                   .arg(getStorageIdByName(storageName)));

    if (!query.next() && !query.isValid())
    {
        qDebug() << "material does not exist!";
        return false;
    }

    if (query.value(1).toInt() < amount)
    {
        qDebug() << "Not enough material!";
        return false;
    }

    res &= Debug::execQuery(QStringLiteral("update materials set amount=%0 "
                                           "where id=%1")
                                .arg(query.value(1).toInt() - amount)
                                .arg(query.value(0).toInt()));

    if (query.value(1).toInt() - amount == 0)
    {
        query.exec(QStringLiteral("delete from materials where id=%1").arg(query.value(0).toInt()));
    }

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

    if (!query.next() && !query.isValid())
    {
        qDebug() << "The storageName does not exist!";
        return false;
    }

    return query.value(0).toInt();
}

int StorageSystem::getMaterialIdByName(const QString &materialName, int amount,
                                       const QString &storageName)
{
    QSqlQuery query(getDb());

    query.exec(QStringLiteral("select id from materials "
                              "where type='%0' and amount=%1 and storageid='%2'")
                   .arg(materialName)
                   .arg(amount)
                   .arg(getStorageIdByName(storageName)));

    if (!query.next() && !query.isValid())
    {
        qDebug() << "The material does not exist!";
        return false;
    }

    return query.value(0).toInt();
}

bool StorageSystem::addHistoryRecord(DataActionType actionType, const QString &materialName,
                                     int amount, const QString &storageName, int price)
{
    QSqlQuery query(getDb());

    const auto currTimeToUtc = QDateTime::currentDateTime().toMSecsSinceEpoch();

    return Debug::execQuery(QStringLiteral("insert into history (dataactiontype, type, "
                                           "amount, storagename, price, timestamp) "
                                           "values (%0, '%1', %2, '%3', %4, %5)")
                                .arg(static_cast<int>(actionType))
                                .arg(materialName)
                                .arg(amount)
                                .arg(storageName)
                                .arg(price)
                                .arg(currTimeToUtc));
}

StorageSystem::Filter::Filter()
    : _actionType(DataActionType::None),
      _materialName(""),
      _storageName(""),
      _materialAmout(""),
      _price(""),
      _utcTime("")
{
}

StorageSystem::Filter &StorageSystem::Filter::setActionType(DataActionType actionType)
{
    _actionType = actionType;
    return *this;
}

StorageSystem::Filter &StorageSystem::Filter::setMaterialName(const QString &materialName)
{
    _materialName = materialName;
    return *this;
}

StorageSystem::Filter &StorageSystem::Filter::setStorageName(const QString &storageName)
{
    _storageName = storageName;
    return *this;
}

StorageSystem::Filter &StorageSystem::Filter::setMaterialAmout(const QString &materialAmout)
{
    _materialAmout = materialAmout;
    return *this;
}

StorageSystem::Filter &StorageSystem::Filter::setPrice(const QString &price)
{
    _price = price;
    return *this;
}

StorageSystem::Filter &StorageSystem::Filter::setUtcTime(const QString &utcTime)
{
    _utcTime = utcTime;
    return *this;
}

StorageSystem::Filter::~Filter() = default;

void StorageSystem::Filter::print() const
{
    QSqlQuery query(getDb());

    query.exec(buildQuery());

    // form table
    fort::char_table table;

    table << fort::header;

    table[0][0] = "Id";
    table[0][1] = "ActionType";
    table[0][2] = "MtrlType";
    table[0][3] = "MtrlAmount";
    table[0][4] = "StrgName";
    table[0][5] = "SellPrice";
    table[0][6] = "TimeStamp";

    for (int i = 0; i <= 6; ++i)
    {
        table[0][i].set_cell_content_fg_color(fort::color::cyan);
        table[0][i].set_cell_text_align(fort::text_align::center);
        table[0][i].set_cell_content_text_style(fort::text_style::bold);
    }

    int rowCounter = 1;
    while (query.next())
    {
        table[rowCounter][0] = std::to_string(query.value(0).toInt());
        table[rowCounter][1] = query.value(1).toInt() == 0 ? "Buy" : "Sell";
        table[rowCounter][2] = query.value(2).toString().toStdString();
        table[rowCounter][3] = std::to_string(query.value(3).toInt());
        table[rowCounter][4] = query.value(4).toString().toStdString();
        table[rowCounter][5] = std::to_string(query.value(5).toInt());
        table[rowCounter][6] = std::to_string(query.value(6).toInt());

        ++rowCounter;
    }

    std::cout << table.to_string() << std::endl;
}

QString StorageSystem::Filter::buildQuery() const
{
    QSqlQuery query(getDb());

    // build query
    QString queryBase = "select * from history where";
    bool isFirstCondition = true;

    if (_actionType != DataActionType::None)
    {
        if (isFirstCondition)
            isFirstCondition = false;

        queryBase += QStringLiteral(" dataactiontype = %0").arg(static_cast<int>(_actionType));
    }

    if (!_materialName.isEmpty())
    {
        if (isFirstCondition)
            isFirstCondition = false;
        else
            queryBase += QStringLiteral(" and");

        queryBase += QStringLiteral(" type = '%0'").arg(_materialName);
    }

    if (!_storageName.isEmpty())
    {
        if (isFirstCondition)
            isFirstCondition = false;
        else
            queryBase += QStringLiteral(" and");

        queryBase += QStringLiteral(" storagename = '%0'").arg(_storageName);
    }

    if (!_materialAmout.isEmpty())
    {
        if (isFirstCondition)
            isFirstCondition = false;
        else
            queryBase += QStringLiteral(" and");

        queryBase += QStringLiteral(" amount %0").arg(_materialAmout);
    }

    if (!_price.isEmpty())
    {
        if (isFirstCondition)
            isFirstCondition = false;
        else
            queryBase += QStringLiteral(" and");

        queryBase += QStringLiteral(" price %0").arg(_price);
    }

    if (!_utcTime.isEmpty())
    {
        if (isFirstCondition)
            isFirstCondition = false;
        else
            queryBase += QStringLiteral(" and");

        queryBase += QStringLiteral(" timestamp %0").arg(_utcTime);
    }

    if (isFirstCondition)
        queryBase = "select * from history";

    std::cout << queryBase.toStdString() << std::endl;;

    return queryBase;
}
