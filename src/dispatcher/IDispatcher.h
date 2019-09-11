#ifndef IDISPATCHER_H
#define IDISPATCHER_H

#include "Store.h"
#include "Action.h"
#include "StoreIds.h"

class IDispatcher
{
public:
    virtual void waitFor(QList<StoreIdsHelper::StoreId> ids) = 0;
    virtual void registerStore(StoreIdsHelper::StoreId id, Store *) = 0;
    virtual void removeStore(StoreIdsHelper::StoreId id) = 0;
    virtual void dispatch(const Action& a) = 0;
    virtual bool isDispatching() const = 0;
};

#endif // IDISPATCHER_H
