#ifndef STORECONTAINER_H
#define STORECONTAINER_H

#include <QHash>
#include "IStoreProvider.h"

class IDispatcher;
class Store;

class StoreProvider : public IStoreProvider
{
    Q_OBJECT
public:
    StoreProvider(IDispatcher*);

    Q_INVOKABLE virtual Store *getStoreById(StoreIdsHelper::StoreId id);

    ~StoreProvider();

private:
    IDispatcher *m_dispatcher;

    QHash<StoreIdsHelper::StoreId, Store*> m_stores;
};

#endif // STORECONTAINER_H
