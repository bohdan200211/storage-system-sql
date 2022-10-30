#include "storagesystem.h"

#include <QString>

int main(int argc, char *argv[])
{
    StorageSystem::initDataBase();

    // clear data
    StorageSystem::clearDataBase();

    StorageSystem::addStorage("sg-1");
    StorageSystem::addMaterial("mt-1", 15, "sg-1");
    StorageSystem::addMaterial("mt-2", 20, "sg-1");

    StorageSystem::removeStorage("sg-1");

    StorageSystem::addStorage("sg-2");
    StorageSystem::addMaterial("mt-3", 25, "sg-2");
    StorageSystem::addMaterial("mt-3", 30, "sg-2");

    StorageSystem::removeMaterial("mt-3", 30, "sg-2");

    return 0;
}
