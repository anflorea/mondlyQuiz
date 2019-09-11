#include "Store.h"
#include "IDispatcher.h"

Store::Store(QObject *parent): QObject(parent)
{
}

Store::Store(StoreIdsHelper::StoreId id, IDispatcher *dispatcher)
{
    m_id = id;
    m_dispatcher = dispatcher;
    m_dispatcher->registerStore(m_id, this);
}

Store::Store(StoreIdsHelper::StoreId id, IDispatcher *dispatcher, IStoreProvider *storeProvider)
{
    m_id = id;
    m_dispatcher = dispatcher;
    m_dispatcher->registerStore(m_id, this);
    m_storeProvider = storeProvider;
}

StoreIdsHelper::StoreId Store::getId()
{
    return m_id;
}

Store::~Store()
{
    if (m_dispatcher) {
        m_dispatcher->removeStore(m_id);
    }
}
