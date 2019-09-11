#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <QQueue>
#include "Store.h"
#include "IDispatcher.h"
#include "StoreIds.h"

// THIS MUST NOT BE CALLED BY MULTIPLE THREADS EVER!!!

class Dispatcher: public QObject, public IDispatcher
{
    Q_OBJECT
public:
    Dispatcher(QObject *parent = 0): QObject(parent) { m_dispatching = false; }
    
public:
    virtual void waitFor(QList<StoreIdsHelper::StoreId> ids);
    virtual void registerStore(StoreIdsHelper::StoreId id, Store *);
    virtual void removeStore(StoreIdsHelper::StoreId id);
    virtual void dispatch(const Action& a);
    virtual bool isDispatching() const;

private:
    void emitDispatch(const Action& a);
    QQueue <Action> m_queue;
    QHash <StoreIdsHelper::StoreId, Store*> m_stores;
    QHash <StoreIdsHelper::StoreId, bool> done;
    Action currentAction;
    StoreIdsHelper::StoreId dispatchingStoreId;
    bool m_dispatching;
};

#endif // DISPATCHER_H
