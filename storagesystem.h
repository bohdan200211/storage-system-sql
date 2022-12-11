#pragma once

#include <QString>

class QSqlDatabase;

class StorageSystem
{
public:
    enum class DataActionType : int
    {
        Add = 0,
        Remove,
        None,
    };

public:
    StorageSystem() = delete;
    StorageSystem(const StorageSystem &) = delete;

    class Filter
    {
    public:
        Filter();
        ~Filter();

        Filter &setActionType(DataActionType);
        Filter &setMaterialName(const QString &);
        Filter &setStorageName(const QString &);
        Filter &setMaterialAmout(const QString &);
        Filter &setPrice(const QString &);
        Filter &setUtcTime(const QString &);

        void print() const;

    private:
        QString buildQuery() const;

    private:
        DataActionType _actionType;
        QString _materialName;
        QString _storageName;

        QString _materialAmout; // example: >=1, =2, BETWEEN 50 AND 60
        QString _price;
        QString _utcTime;
    };

    // print functions
    static void printAllStorageStatus();
    static void printStorageStatus(const QString &storageName);

    // db modification tools
    static bool initDataBase();
    static bool clearDataBase();

    static bool addStorage(const QString &storageName);
    static bool moveMaterials(const QString &fromStorage, const QString &toStorage,
                              const QString &materialName, int amount, int sellPrice);

private:
    static QSqlDatabase &getDb();

    static bool addMaterial(const QString &materialName, int amount, const QString &storageName);
    static bool removeMaterial(const QString &materialName, int amount, const QString &storageName);

    static int getStorageIdByName(const QString &storageName);
    static int getMaterialIdByName(const QString &materialName, int amount,
                                   const QString &storageName);

    static bool addHistoryRecord(DataActionType actionType, const QString &materialName, int amount,
                                 const QString &storageName, int price);
};
