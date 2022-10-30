#pragma once

class QSqlDatabase;
class QString;

class StorageSystem
{
public:
    StorageSystem() = delete;
    StorageSystem(const StorageSystem &) = delete;

    static bool initDataBase();
    static bool clearDataBase();

    static bool addStorage(const QString &storageName);
    static bool addMaterial(const QString &materialName, int amount, const QString &storageName);

    static bool removeStorage(const QString &storageName);
    static bool removeMaterial(const QString &materialName, int amount, const QString &storageName);

private:
    static QSqlDatabase &getDb();
    static int getStorageIdByName(const QString &storageName);
};
