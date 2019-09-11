#ifndef STORE_H
#define STORE_H

#include <QObject>
#include "QQmlVarPropertyHelpers.h"
#include "Action.h"
#include "StoreIds.h"

class IDispatcher;
class IStoreProvider;

class Store : public QObject
{
    Q_OBJECT

    QML_READONLY_VAR_PROPERTY(bool, isInvalidated, false)

public:
    Store(QObject*);
    Store(StoreIdsHelper::StoreId id, IDispatcher*);
    Store(StoreIdsHelper::StoreId id, IDispatcher*, IStoreProvider*);
    StoreIdsHelper::StoreId getId();

    virtual void notify(const Action &action) = 0;
    virtual ~Store();

    virtual void invalidateStore() {
        for (auto& id : m_sectionValidityStatus.keys())
            m_sectionValidityStatus[id] = false;
    }

    virtual void validateStore() {
        for (auto& id : m_sectionValidityStatus.keys())
            m_sectionValidityStatus[id] = true;
    }

    virtual void invalidateSection(int id) {
        if (!m_sectionValidityStatus.contains(id))
            return;

        m_sectionValidityStatus[id] = false;
    }

    virtual void validateSection(int id) {
        if (!m_sectionValidityStatus.contains(id))
            return;

        m_sectionValidityStatus[id] = true;
    }

    virtual void addSection(int id) {
        m_sectionValidityStatus.insert(id, true);
    }

    virtual bool getSectionValidity(int id) {
        if (m_sectionValidityStatus.contains(id))
            return m_sectionValidityStatus[id];

        return false;
    }

    virtual void clear() = 0; // now every store should implement clearing its content

protected:
    StoreIdsHelper::StoreId m_id;
    IDispatcher* m_dispatcher = Q_NULLPTR;
    IStoreProvider* m_storeProvider = Q_NULLPTR;

    QHash<int, bool> m_sectionValidityStatus;
};

#endif // STORE_H
