#include "storagesystem.h"

int main(int argc, char *argv[])
{
    bool doFillDb = false;
    if (doFillDb)
    {
        // clear data
        StorageSystem::clearDataBase();

        StorageSystem::initDataBase();

        // add storages
        StorageSystem::addStorage("st-1");
        StorageSystem::addStorage("st-2");
        StorageSystem::addStorage("st-3");

        // add some primary materials
        StorageSystem::moveMaterials("", "st-1", "AAA", 10, 0);
        StorageSystem::moveMaterials("", "st-1", "BBB", 20, 0);
        StorageSystem::moveMaterials("", "st-1", "CCC", 30, 0);

        // st-2
        StorageSystem::moveMaterials("st-1", "st-2", "AAA", 8, 4);
        StorageSystem::moveMaterials("st-1", "st-2", "BBB", 6, 8);

        // st-3
        StorageSystem::moveMaterials("st-1", "st-3", "BBB", 12, 16);
        StorageSystem::moveMaterials("st-1", "st-3", "CCC", 18, 32);

    }
    // print
    StorageSystem::printAllStorageStatus();

    // filter
    const auto f0 = StorageSystem::Filter();
    f0.print();

    const auto f1 = StorageSystem::Filter().setPrice(">=8");
    f1.print();

    const auto f2 = StorageSystem::Filter().setPrice(">=8").setMaterialName("BBB");
    f2.print();

    const auto f3 = StorageSystem::Filter().setPrice(">=8").setMaterialName("BBB").setActionType(
        StorageSystem::DataActionType::Add);
    f3.print();

    const auto f4 = StorageSystem::Filter()
                        .setUtcTime("between 1670774172633 and 1670774172673")
                        .setActionType(StorageSystem::DataActionType::Add);
    f4.print();

    const auto f5 = StorageSystem::Filter()
                        .setActionType(StorageSystem::DataActionType::Remove)
                        .setMaterialAmout("<=12")
                        .setMaterialName("BBB");
    f5.print();

    return 0;
}
