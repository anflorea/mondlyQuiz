#include "StoreProvider.h"

#include "GeneralStore.h"

StoreProvider::StoreProvider(IDispatcher* dispatcher)
{
    m_dispatcher = dispatcher;

    GeneralStore *generalStore = new GeneralStore(StoreIdsHelper::GENERAL_STORE, m_dispatcher);
    m_stores[StoreIdsHelper::GENERAL_STORE] = generalStore;

}

Store *StoreProvider::getStoreById(StoreIdsHelper::StoreId id)
{
    return m_stores[id];
}

StoreProvider::~StoreProvider()
{
    for (Store* store: m_stores.values()) {
        delete store;
    }
}
