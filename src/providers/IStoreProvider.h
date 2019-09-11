#ifndef ISTOREPROVIDER_H
#define ISTOREPROVIDER_H

#include <QObject>
#include "StoreIds.h"

class Store;

class IStoreProvider : public QObject
{
    Q_OBJECT
public:
    virtual Store *getStoreById(StoreIdsHelper::StoreId id) = 0;
};

#endif // ISTOREPROVIDER_H
