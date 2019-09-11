#ifndef QQMLOBJECTLISTMODEL_H
#define QQMLOBJECTLISTMODEL_H

#include <QObject>
#include <QDebug>
#include <QAbstractListModel>
#include <QByteArray>
#include <QChar>
#include <QMetaMethod>
#include <QMetaObject>
#include <QMetaProperty>
#include <QVariant>
#include <QVector>
#include <QQmlEngine>
#include <QSet>
#include <QList>
#include <QStringBuilder>

template<typename T> QList<T> qListFromVariant (const QVariantList & list) {
    QList<T> ret;
    ret.reserve (list.size ());
    for (QVariantList::const_iterator it = list.constBegin (); it != list.constEnd (); it++) {
        const QVariant & var = static_cast<QVariant>(* it);
        ret.append (var.value<T> ());
    }
    return ret;
}

template<typename T> QVariantList qListToVariant (const QList<T> & list) {
    QVariantList ret;
    ret.reserve (list.size ());
    for (typename QList<T>::const_iterator it = list.constBegin (); it != list.constEnd (); it++) {
        const T & val = static_cast<T>(* it);
        ret.append (QVariant::fromValue (val));
    }
    return ret;
}

// custom foreach for QList, which uses no copy and check pointer non-null
#define FOREACH_PTR_IN_QLIST(_type_, _var_, _list_) \
    for (typename QList<_type_ *>::const_iterator it = _list_.begin (); it != _list_.end (); it++) \
    for (_type_ * _var_ = static_cast<_type_ *> (* it); _var_ != Q_NULLPTR; _var_ = Q_NULLPTR)

class QQmlObjectListModelBase : public QAbstractListModel { // abstract Qt base class
    Q_OBJECT
    Q_PROPERTY (int count READ count NOTIFY countChanged)
    Q_PROPERTY (int length READ count NOTIFY countChanged) // compatibility with js array length property

public:
    explicit QQmlObjectListModelBase (QObject * parent = Q_NULLPTR) : QAbstractListModel (parent) { }

public slots: // virtual methods API for QML
    virtual int size (void) const = 0;
    virtual int count (void) const = 0;
    virtual bool isEmpty (void) const = 0;
    virtual bool contains (QObject * item) const = 0;
    virtual int indexOf (QObject * item) const = 0;
    virtual int roleForName (const QByteArray & name) const = 0;
    virtual void clear (void) = 0;
    virtual void append (QObject * item) = 0;
    virtual void prepend (QObject * item) = 0;
    virtual void insert (int idx, QObject * item) = 0;
    virtual void move (int idx, int pos) = 0;
    virtual void remove (QObject * item) = 0;
    virtual void remove (int idx) = 0;
    virtual void reload() = 0; ///function to reload the whole list
    virtual QObject * get (int idx) const = 0;
    virtual QObject * get (const QString & uid) const = 0;
    virtual QObject * getFirst (void) const = 0;
    virtual QObject * getLast (void) const = 0;
    virtual QVariantList toVarArray (void) const = 0;
    virtual QVariant qtObjectAt (int index) const = 0;

protected slots: // internal callback
    virtual void onItemPropertyChanged (void) = 0;

signals: // notifier
    void countChanged (int);
    void added(QObject* item);
    void addedItems(QList<QObject*> itemList);
    void removed(QObject* item);
    void removedItems(QList<QObject*> itemList);
    void cleared();
    void itemPropertyChanged(QObject* item, int role);
};

template<class ItemType> class QQmlObjectListModel : public QQmlObjectListModelBase {
public:
    explicit QQmlObjectListModel (QObject *          parent      = Q_NULLPTR,
                                  const QByteArray & uidRole     = QByteArray ())
        : QQmlObjectListModelBase (parent)
        , m_uidRoleName (uidRole)
        , m_metaObj (ItemType::staticMetaObject)
    {
        static QSet<QByteArray> roleNamesBlacklist;
        if (roleNamesBlacklist.isEmpty ()) {
            roleNamesBlacklist << QByteArrayLiteral ("id")
                               << QByteArrayLiteral ("index")
                               << QByteArrayLiteral ("class")
                               << QByteArrayLiteral ("model")
                               << QByteArrayLiteral ("modelData");
        }
        static const char * HANDLER = "onItemPropertyChanged()";
        m_handler = metaObject ()->method (metaObject ()->indexOfMethod (HANDLER));

        m_roles.insert (baseRole (), QByteArrayLiteral ("qtObject"));
        const int len = m_metaObj.propertyCount ();
        for (int propertyIdx = 0, role = (baseRole () +1); propertyIdx < len; propertyIdx++, role++) {
            QMetaProperty metaProp = m_metaObj.property (propertyIdx);
            const QByteArray propName = QByteArray (metaProp.name ());
            if (!roleNamesBlacklist.contains (propName)) {
                m_roles.insert (role, propName);
                if (metaProp.hasNotifySignal ()) {
                    m_signalIdxToRole.insert (metaProp.notifySignalIndex (), role);
                    m_connectedProperties.append(propName);
                }
            }
            else {
                static const QByteArray CLASS_NAME = (QByteArrayLiteral ("QQmlObjectListModel<") % m_metaObj.className () % '>');
                qWarning () << "Can't have" << propName << "as a role name in" << qPrintable (CLASS_NAME);
            }
        }
    }

    Qt::ItemFlags flags(const QModelIndex &index) const override {
        Qt::ItemFlags flags = QAbstractItemModel::flags(index);
        flags |= Qt::ItemIsEditable;
        return flags;
    }

    bool setData (const QModelIndex & index, const QVariant & value, int role) override {
        bool ret = false;
        ItemType * item = at (index.row ());
        const QByteArray rolename = m_roles.value (role, emptyBA ());
        if (item != Q_NULLPTR && role != baseRole () && !rolename.isEmpty ()) {
            ret = item->setProperty (rolename, value);
        }
        return ret;
    }
    QVariant data (const QModelIndex & index, int role) const override {
        QVariant ret;
        ItemType * item = at (index.row ());
        const QByteArray rolename = m_roles.value (role, emptyBA ());
        if (item != Q_NULLPTR && !rolename.isEmpty ()) {
            ret.setValue (role != baseRole () ? item->property (rolename) : QVariant::fromValue (static_cast<QObject *> (item)));
        }
        return ret;
    }
    QHash<int, QByteArray> roleNames (void) const override{
        return m_roles;
    }
    typedef typename QList<ItemType *>::const_iterator const_iterator;
    const_iterator begin (void) const {
        return m_items.begin ();
    }
    const_iterator end (void) const {
        return m_items.end ();
    }
    const_iterator constBegin (void) const {
        return m_items.constBegin ();
    }
    const_iterator constEnd (void) const {
        return m_items.constEnd ();
    }

public: // C++ API
    virtual ItemType * at (int idx) const {
        ItemType * ret = Q_NULLPTR;
        if (idx >= 0 && idx < m_items.size ()) {
            ret = m_items.value (idx);
        }
        return ret;
    }
    virtual ItemType * getByUid (const QString & uid) const {
        return m_indexByUid.value (uid, Q_NULLPTR);
    }
    int roleForName (const QByteArray & name) const override {
        return m_roles.key (name, -1);
    }
    Q_INVOKABLE int count (void) const override {
        return m_count;
    }
    Q_INVOKABLE int size (void) const override {
        return m_count;
    }
    bool isEmpty (void) const override {
        return m_items.isEmpty ();
    }
    virtual bool contains (ItemType * item) const {
        return m_items.contains (item);
    }
    virtual int indexOf (ItemType * item) const {
        return m_items.indexOf (item);
    }
    void clear (void) override {
        if (!m_items.isEmpty ()) {
            beginRemoveRows (noParent (), 0, m_items.count () -1);
            FOREACH_PTR_IN_QLIST (ItemType, item, m_items) {
                dereferenceItem (item);
            }
            m_items.clear ();
            endRemoveRows ();
            updateCounter ();
            emit cleared();
        }
    }
    virtual void append (ItemType * item) {
        if (item != Q_NULLPTR) {
            const int pos = m_items.count ();
            QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
            beginInsertRows (noParent (), pos, pos);
            m_items.append (item);
            referenceItem (item);
            endInsertRows ();
            updateCounter ();
            emit added(item);
        }
    }
    virtual void prepend (ItemType * item) {
        if (item != Q_NULLPTR) {
            beginInsertRows (noParent (), 0, 0);
            QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
            m_items.prepend (item);
            referenceItem (item);
            endInsertRows ();
            updateCounter ();
            emit added(item);
        }
    }
    virtual void insert (int idx, ItemType * item) {
        if (item != Q_NULLPTR) {
            beginInsertRows (noParent (), idx, idx);
            QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
            m_items.insert (idx, item);
            referenceItem (item);
            endInsertRows ();
            updateCounter ();
            emit added(item);
        }
    }
    virtual void append (const QList<ItemType *> & itemList) {
        if (!itemList.isEmpty ()) {
            QList<QObject*> addedList;
            const int pos = m_items.count ();
            beginInsertRows (noParent (), pos, pos + itemList.count () -1);
            m_items.reserve (m_items.count () + itemList.count ());
            m_items.append (itemList);
            FOREACH_PTR_IN_QLIST (ItemType, item, itemList) {
                referenceItem (item);
                addedList.append(item);
            }
            endInsertRows ();
            updateCounter ();
            emit addedItems(addedList);
        }
    }
    virtual void prepend (const QList<ItemType *> & itemList) {
        if (!itemList.isEmpty ()) {
            QList<QObject*> addedList;
            beginInsertRows (noParent (), 0, itemList.count () -1);
            m_items.reserve (m_items.count () + itemList.count ());
            int offset = 0;
            FOREACH_PTR_IN_QLIST (ItemType, item, itemList) {
                m_items.insert (offset, item);
                referenceItem (item);
                offset++;
                addedList.append(item);
            }
            endInsertRows ();
            updateCounter ();
            emit addedItems(addedList);
        }
    }
    virtual void insert (int idx, const QList<ItemType *> & itemList) {
        if (!itemList.isEmpty ()) {
            QList<QObject*> addedList;
            beginInsertRows (noParent (), idx, idx + itemList.count () -1);
            m_items.reserve (m_items.count () + itemList.count ());
            int offset = 0;
            FOREACH_PTR_IN_QLIST (ItemType, item, itemList) {
                m_items.insert (idx + offset, item);
                referenceItem (item);
                offset++;
                addedList.append(item);
            }
            endInsertRows ();
            updateCounter ();
            emit addedItems(addedList);
        }
    }
    void move (int idx, int pos) override {
        if (idx != pos) {
            if (idx > pos)
                beginMoveRows (noParent (), idx, idx, noParent (), pos);
            else
                beginMoveRows (noParent (), idx, idx, noParent (), pos + 1);
            m_items.move (idx, pos);
            endMoveRows ();
        }
    }
    virtual void remove (ItemType * item) {
        if (item != Q_NULLPTR) {
            const int idx = m_items.indexOf (item);
            remove (idx);
            emit removed(item);
        }
    }
    void remove (int idx) override {
        if (idx >= 0 && idx < m_items.size ()) {
            beginRemoveRows (noParent (), idx, idx);
            ItemType * item = m_items.takeAt (idx);
            dereferenceItem (item);
            endRemoveRows ();
            updateCounter ();
            emit removed(item);
        }
    }
    void reload(void) override {
        emit dataChanged(QAbstractListModel::index(0, 0, noParent()), QAbstractListModel::index (this->count() - 1, 0, noParent ()));
    }
    virtual ItemType * first (void) const {
        return m_items.first ();
    }
    virtual ItemType * last (void) const {
        return m_items.last ();
    }
    virtual const QList<ItemType *> & toList (void) const {
        return m_items;
    }

public: // QML slots implementation
    void append (QObject * item) override {
        append (qobject_cast<ItemType *> (item));
    }
    void prepend (QObject * item) override {
        prepend (qobject_cast<ItemType *> (item));
    }
    void insert (int idx, QObject * item) override {
        insert (idx, qobject_cast<ItemType *> (item));
    }
    void remove (QObject * item) override {
        remove (qobject_cast<ItemType *> (item));
    }
    bool contains (QObject * item) const override {
        return contains (qobject_cast<ItemType *> (item));
    }
    int indexOf (QObject * item) const override {
        return indexOf (qobject_cast<ItemType *> (item));
    }
    int indexOf (const QString & uid) const {
        return indexOf (get (uid));
    }
    QObject * get (int idx) const override {
        return static_cast<QObject *> (at (idx));
    }
    QObject * get (const QString & uid) const override {
        return static_cast<QObject *> (getByUid (uid));
    }
    QObject * getFirst (void) const override {
        return static_cast<QObject *> (first ());
    }
    QObject * getLast (void) const override {
        return static_cast<QObject *> (last ());
    }
    Q_INVOKABLE QVariantList toVarArray (void) const override {
        return qListToVariant<ItemType *> (m_items);
    }

    Q_INVOKABLE QVariant qtObjectAt(int index) const override {
        ItemType *item = at(index);
        return QVariant::fromValue(item);
    }

protected: // internal stuff
    static const QString & emptyStr (void) {
        static const QString ret = QStringLiteral ("");
        return ret;
    }
    static const QByteArray & emptyBA (void) {
        static const QByteArray ret = QByteArrayLiteral ("");
        return ret;
    }
    static const QModelIndex & noParent (void) {
        static const QModelIndex ret = QModelIndex ();
        return ret;
    }
    static const int & baseRole (void) {
        static const int ret = Qt::UserRole;
        return ret;
    }
    int rowCount (const QModelIndex & parent = QModelIndex ()) const override {
        Q_UNUSED (parent);
        return m_items.count ();
    }
    void referenceItem (ItemType * item) {
        if (item != Q_NULLPTR) {
            if (item->parent () == Q_NULLPTR) {
                item->setParent (this);
            }

            connectToItemProps (item);

            if (!m_uidRoleName.isEmpty ()) {
                const QString key = m_indexByUid.key (item, emptyStr ());
                if (!key.isEmpty ()) {
                    m_indexByUid.remove (key);
                }
                const QString value = item->property (m_uidRoleName).toString ();
                if (!value.isEmpty ()) {
                    m_indexByUid.insert (value, item);
                }
            }
        }
    }

    void reconnectToItemProps() {
        foreach(ItemType *item, m_items) {
            disconnect (this, Q_NULLPTR, item, Q_NULLPTR);
            disconnect (item, Q_NULLPTR, this, Q_NULLPTR);
            connectToItemProps (item);
        }
    }

    void connectToItemProps(QObject *item) {
        const QList<int> signalsIdxList = m_signalIdxToRole.keys ();
        for (QList<int>::const_iterator it = signalsIdxList.constBegin (); it != signalsIdxList.constEnd (); it++) {
            const int signalIdx = static_cast<int> (* it);
            if (m_connectedProperties.contains(m_roles[m_signalIdxToRole[signalIdx]])) {
                QMetaMethod notifier = item->metaObject ()->method (signalIdx);
                connect (item, notifier, this, m_handler, Qt::UniqueConnection);
            }
        }
    }

    void dereferenceItem (ItemType * item) {
        if (item != Q_NULLPTR) {
            disconnect (this, Q_NULLPTR, item, Q_NULLPTR);
            disconnect (item, Q_NULLPTR, this, Q_NULLPTR);
            if (!m_uidRoleName.isEmpty ()) {
                const QString key = m_indexByUid.key (item, emptyStr ());
                if (!key.isEmpty ()) {
                    m_indexByUid.remove (key);
                }
            }
            if (item->parent () == this) {
                item->deleteLater ();
            }
        }
    }
    void onItemPropertyChanged (void) override {
        ItemType * item = qobject_cast<ItemType *> (sender ());

        const int row = m_items.indexOf (item);
        const int sig = senderSignalIndex ();
        const int role = m_signalIdxToRole.value (sig, -1);
        if (row >= 0 && role >= 0) {
            QModelIndex index = QAbstractListModel::index (row, 0, noParent ());
            QVector<int> rolesList;
            rolesList.append (role);
            emit dataChanged (index, index, rolesList);
        }
        if (!m_uidRoleName.isEmpty ()) {
            const QByteArray roleName = m_roles.value (role, emptyBA ());
            if (!roleName.isEmpty () && roleName == m_uidRoleName) {
                const QString key = m_indexByUid.key (item, emptyStr ());
                if (!key.isEmpty ()) {
                    m_indexByUid.remove (key);
                }
                const QString value = item->property (m_uidRoleName).toString ();
                if (!value.isEmpty ()) {
                    m_indexByUid.insert (value, item);
                }
            }
        }
        emit itemPropertyChanged(item, role);
    }

    inline void updateCounter (void) {
        if (m_count != m_items.count ()) {
            m_count = m_items.count ();
            emit countChanged (m_count);
        }
    }
public:
    // data members
    QList<ItemType *>          m_items;
    QList<QString>             m_connectedProperties;
    QByteArray                 m_uidRoleName;
private:
    int                        m_count = 0;
    QMetaObject                m_metaObj;
    QMetaMethod                m_handler;
    QHash<int, QByteArray>     m_roles;
    QHash<int, int>            m_signalIdxToRole;
    QHash<QString, ItemType *> m_indexByUid;

};

#define QML_OBJMODEL_PROPERTY(type, name, default_value) \
    protected: Q_PROPERTY (QQmlObjectListModelBase * name READ get_##name CONSTANT) \
    protected: QQmlObjectListModel<type> * m_##name = default_value; \
    public: QQmlObjectListModel<type> * get_##name (void) const { return m_##name; } \
    private:

#endif // QQMLOBJECTLISTMODEL_H
