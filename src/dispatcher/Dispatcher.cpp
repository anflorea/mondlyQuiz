#include "Dispatcher.h"

void Dispatcher::registerStore(StoreIdsHelper::StoreId id, Store * store)
{
    m_stores[id] = store;
}

void Dispatcher::removeStore(StoreIdsHelper::StoreId id)
{
    if(m_stores.contains(id) == false) {
        qDebug() << "No such store to remove";
        return ;
    }
    m_stores.remove(id);
}

void Dispatcher::emitDispatch(const Action& a)
{
    currentAction = a;
    done.clear();

    for(auto store_id : m_stores.keys()) {

        if (!done[store_id]) {
            dispatchingStoreId = store_id;
            done[store_id] = 1;
            m_stores[store_id]->notify(a);
        }
    }

    currentAction = Action();
}

void Dispatcher::dispatch(const Action& a)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<ActionType::Type>();
    QString actionTypeString(metaEnum.valueToKey(a.getType()));
    QVariantMap logData{
        { "action_type", actionTypeString },
    };

    if(m_dispatching) {
        m_queue.enqueue(a);
        return ;
    }

    m_dispatching = true;
    this->emitDispatch(a);

    while(m_queue.size() > 0) {
        Action a = m_queue.dequeue();
        this->emitDispatch(a);
    }

    m_dispatching = false;
}

void Dispatcher::waitFor(const QList<StoreIdsHelper::StoreId> m_dep)
{
    if(!m_dispatching || m_dep.size() == 0)
        return;
    for(auto store_id : m_dep) {
        if(!done[store_id]) {
            done[store_id] = 1;
            m_stores[store_id]->notify(currentAction);
        }
    }
}

bool Dispatcher::isDispatching() const {
    return m_dispatching;
}
