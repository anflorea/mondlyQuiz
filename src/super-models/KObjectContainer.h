#ifndef KOBJECTCONTAINER_H
#define KOBJECTCONTAINER_H

#include <QObject>
#ifdef QT_DEBUG
#include <QMetaProperty>
#include <QDebug>
#endif

template <class ItemType>
class KObjectContainer : public QObject
{
public:
    explicit KObjectContainer() {}

    void setUidRoleName(const QString uidRoleName) {
        Q_ASSERT_X(m_uidRoleName.isEmpty(), getClassName(), "uid role already set!");
        Q_ASSERT_X(!uidRoleName.isEmpty(), getClassName(), "uid role cannot be empty!");

#ifdef QT_DEBUG
        bool isRoleName = false;
        const int len = ItemType::staticMetaObject.propertyCount ();
        for (int propertyIdx = 0, role = ( Qt::UserRole + 1); propertyIdx < len; propertyIdx++, role++) {
            QMetaProperty metaProp = ItemType::staticMetaObject.property (propertyIdx);
            const QByteArray propName = QByteArray (metaProp.name ());
            if (propName == uidRoleName) {
                isRoleName = true;
                break;
            }
        }
        Q_ASSERT_X(isRoleName, getClassName(), QByteArray(uidRoleName.toUtf8() + " is not a valid rolename!").toStdString().c_str());
#endif
        m_uidRoleName = uidRoleName;
    }

    void insert(ItemType* item) {
        if (item != Q_NULLPTR) {
            if (item->parent () == Q_NULLPTR) {
                item->setParent (this);
            }
            const QString roleValue = item->property (m_uidRoleName.toUtf8()).toString();
            Q_ASSERT_X(!roleValue.isEmpty(), getClassName(), "Id of append item is empty!");
            m_internalHash.insert(roleValue, item);
        }
    }

    ItemType *getById(const QString uid) {
        return m_internalHash.value(uid, Q_NULLPTR);
    }

    void removeById(const QString uid) {
        ItemType *item = getById(uid);
        m_internalHash.remove(uid);
        dereferenceItem(item);

    }

    void clear() {
        QList<ItemType*> items = m_internalHash.values();
        m_internalHash.clear();

        foreach (ItemType *item, items) {
            dereferenceItem(item);
        }
    }

private:

    QByteArray getClassName() {
        return (QByteArrayLiteral ("KObjectContainer<") % ItemType::staticMetaObject.className () % '>');
    }

    void dereferenceItem (ItemType *item) {
        if (item) {
            if (item->parent() == this) {
                item->deleteLater ();
            }
        }
    }

    QString m_uidRoleName = QString();

    QHash<QString, ItemType*> m_internalHash;
};

#endif // KOBJECTCONTAINER_H
