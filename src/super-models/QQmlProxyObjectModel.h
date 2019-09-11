#ifndef QQMLPROXYOBJECTMODEL_H
#define QQMLPROXYOBJECTMODEL_H

#include <QObject>
#include <QtAlgorithms>
#include "QQmlObjectListModel.h"
#include <QRegularExpression>
#include <algorithm>
#include <QModelIndex>

/*
*  HOW TO USE IT:
*  Suppose you have a simple Data class with a name field, and name() -> methode
* which does what you think it does.
*
*  To make a proxy object based on a data list you can do the following:
*
*  QQmlProxyObjectModel<Data>* m_my_proxy = new QQmlProxyObjectModel<Data>;
*  QQmlObjectListModel<Data> * m_list = new QQmlObjectMode<Data>(this);
*
*
*  to add a list to the proxy use:
*          m_my_proxy -> setModel(m_list);
*
*
*  now whenever the list is modified, the proxy know to handle the modifications
*
*  to add a filter do the following:
*          m_my_proxy->addFilter("filterId", "name", false, [=](const Data*a) -> bool {
*              return QRegularExpression("kuende").match(a->name()).hasMatch();
*          });
*  the first parameter is the name of the filter (its unique id)
*  the second parameter is either a QString, or a QStringList of the fields
*  the third parameter is a boolean that tells the class if it should evaluate conditions
*  which is the list of properties that the filter functions uses (in order to
*  increase the efficiency of property changed)
*
*
*  to add another filter, do the same thing (the view will change itself automatically
* after each filter that has been added
*
*
*  to remove a filter call m_my_proy->removeFilter("filterId");
*
*
*  to add a sort criteria, do the following:
*      m_my_proxy->addSort("sortId", "name", false, [=](const Data* a, const Data* b) -> int {
*          return -1 * a->name().compare(b->name()); /// sorts decending
*      });
*  the lambda function should be treated as follows:
*      for two arguments, a and b, you should return
*          negative if a < b
*          0 otherwise
*          pozitive if a > b
*
*  When having multilpe sorts, the first one gets evaluated, if they are equal, the next are evaluated and so on, be careful when adding sorts :)
*
*  to remove a sort lambda:
*      m_my_proxy.removeSort("sortId");
*
*/

//#define QQML_PROXY_OBJECT_MODEL_DEBUG
template<class ItemType> class QQmlProxyObjectModel : public QQmlObjectListModel<ItemType>
{

public:

    explicit QQmlProxyObjectModel(QObject * parent = Q_NULLPTR) : QQmlObjectListModel<ItemType>(parent) {

    }

    void setModel(QQmlObjectListModel<ItemType>* original_model) {
#ifdef QQML_PROXY_OBJECT_MODEL_DEBUG
        qDebug() << Q_FUNC_INFO;
#endif
        if (m_original_model) {
            QObject::disconnect(m_original_model, &QQmlObjectListModel<ItemType>::added, this, &QQmlProxyObjectModel::onModelAdded);
            QObject::disconnect(m_original_model, &QQmlObjectListModel<ItemType>::addedItems, this, &QQmlProxyObjectModel::onModelItemsAdded);
            QObject::disconnect(m_original_model, &QQmlObjectListModel<ItemType>::removed, this, &QQmlProxyObjectModel::onModelRemoved);
            QObject::disconnect(m_original_model, &QQmlObjectListModel<ItemType>::cleared, this, &QQmlProxyObjectModel::onModelCleared);
            QObject::disconnect(this, &QQmlProxyObjectModel<ItemType>::itemPropertyChanged, this, &QQmlProxyObjectModel::onMyItemPropertyChanged);
            QObject::disconnect(m_original_model, &QQmlObjectListModel<ItemType>::itemPropertyChanged, this, &QQmlProxyObjectModel::onModelItemPropertyChanged);
        }

        m_original_model = original_model;

        this->m_connectedProperties.clear();
        this->m_connectedProperties.append(m_original_model->m_uidRoleName);

        if (m_original_model) {
            QObject::connect(m_original_model, &QQmlObjectListModel<ItemType>::added, this, &QQmlProxyObjectModel::onModelAdded);
            QObject::connect(m_original_model, &QQmlObjectListModel<ItemType>::addedItems, this, &QQmlProxyObjectModel::onModelItemsAdded);
            QObject::connect(m_original_model, &QQmlObjectListModel<ItemType>::removed, this, &QQmlProxyObjectModel::onModelRemoved);
            QObject::connect(m_original_model, &QQmlObjectListModel<ItemType>::cleared, this, &QQmlProxyObjectModel::onModelCleared);
            QObject::connect(this, &QQmlProxyObjectModel<ItemType>::itemPropertyChanged, this, &QQmlProxyObjectModel::onMyItemPropertyChanged);
            QObject::connect(m_original_model, &QQmlObjectListModel<ItemType>::itemPropertyChanged, this, &QQmlProxyObjectModel::onModelItemPropertyChanged);
        }
        evaluate();
    }

    void filter(void) {
        QList<ItemType*> actList;
        for(int i = 0; i < this->m_original_model->count(); ++ i) {
            ItemType* item = m_original_model->at(i);
            if(fulfillsFilters(item)) {
                actList.append(item);
            }
        }
        QQmlObjectListModel<ItemType>::append(actList);
    }

    void addSort(QString sort_name, QString sort_role, bool shouldEvaluate, std::function<int(const ItemType* a, const ItemType * b)> cmp) {
        this->m_connectedProperties.append(sort_role);

        this->m_lambda_sort.append(qMakePair(sort_name, cmp));
        this->m_sort2prop.insertMulti(sort_name, sort_role);
        if(shouldEvaluate)
            evaluate();
    }

    void addSort(QString sort_name, QStringList sort_roles, bool shouldEvaluate, std::function<int(const ItemType* a, const ItemType * b)> cmp) {
        this->m_connectedProperties.append(sort_roles);

        this->m_lambda_sort.append(qMakePair(sort_name, cmp));
        for(auto sort_role : sort_roles)
            this->m_sort2prop.insertMulti(sort_name, sort_role);
        if(shouldEvaluate)
            evaluate();
    }

    std::function<bool(const ItemType*)> addFilter(QString filter_name, QString filter_role, bool shouldEvaluate, std::function<bool(const ItemType*)> filter) {
        this->m_connectedProperties.append(filter_role);

        this->m_filter2prop.insertMulti(filter_name, filter_role);
        auto last_lambda = (this->m_lambda_filters[filter_name] = filter);
        if(shouldEvaluate)
            evaluate();
        return last_lambda;
    }

    std::function<bool(const ItemType*)> addFilter(QString filter_name, QStringList filter_roles, bool shouldEvaluate, std::function<bool(const ItemType*)> filter) {
        this->m_connectedProperties.append(filter_roles);

        for(auto filter_role : filter_roles)
            this->m_filter2prop.insertMulti(filter_name, filter_role);
        auto last_lambda = (this->m_lambda_filters[filter_name] = filter);
        if(shouldEvaluate)
            evaluate();
        return last_lambda;
    }

    int removeFilter(QString filter_name, bool shouldEvaluate) {
        this->m_connectedProperties.removeAll(m_filter2prop[filter_name]);

        int ret = this->m_lambda_filters.remove(filter_name);
        this->m_filter2prop.remove(filter_name);
        if(shouldEvaluate)
            evaluate();
        return ret;
    }

    int removeSort(QString sort_name, bool shouldEvaluate) {
        this->m_connectedProperties.removeAll(m_sort2prop[sort_name]);

        int cnt = 0;
        for(int i = 0; i < this->m_lambda_sort.size(); ++ i) {
            while(i < this->m_lambda_sort.size() && this->m_lambda_sort[i].first == sort_name) {
                ++ cnt;
                this->m_lambda_sort.removeAt(i);
            }
        }
        this->m_sort2prop.remove(sort_name);
        if(shouldEvaluate)
            m_sort();
        return cnt;
    }

    void evaluate() {
        this->reconnectToItemProps();

        this->clear();
        if (m_original_model == Q_NULLPTR)
            return;

        filter();
        m_sort();
    }

    void onModelAdded(QObject* item) {
        if(!fulfillsFilters(static_cast<ItemType*>(item))) {
            return ;
        }
        if(m_lambda_sort.size()) {
            this->insert(getInsertionIndex(item), item);
            return;
        }
        int indexToInsert = m_original_model->indexOf(item);

        this->insert(qMax(0, qMin(indexToInsert, this->count())), item);
    }

    void onModelItemsAdded(QList<QObject*> list) {
        QHash<int, ItemType*> batches;
        ItemType* item;
        for(auto it = list.rbegin(); it != list.rend() ; ++ it) {
            QObject *act = *it;
            item = static_cast<ItemType*>(act);
            if(!fulfillsFilters(item))
                continue;
            if(m_lambda_sort.size())
                batches.insertMulti(getInsertionIndex(item), item);
            else
                batches.insertMulti(this->size(), item);
        }
        for(int key : batches.uniqueKeys()) {
            QList<ItemType*> values = batches.values(key);
            std::stable_sort(values.begin(), values.end(), [=](const ItemType* a, const ItemType* b) -> bool {
                for(auto it : this->m_lambda_sort) {
                    int act = it.second(a, b);
                    if(act < 0)
                        return 1;
                    if(act > 0)
                        return 0;
                }
                return 0;
            });
            if(values.size() > 0) {
#ifdef QQML_PROXY_OBJECT_MODEL_DEBUG
                qDebug() << Q_FUNC_INFO;
                qDebug() << "inserting: " << values;
                if(m_lambda_sort.size() > 0)
                    qDebug() << "at position binary searched: " << getInsertionIndex(values[0]);
                else
                    qDebug() << "at position: " << this->size();
#endif
                if(m_lambda_sort.size())
                    this->insert(getInsertionIndex(values[0]), values);
                else
                    this->insert(this->size(), values);
            }
        }
    }

    void onModelRemoved(QObject* item) {
#ifdef QQML_PROXY_OBJECT_MODEL_DEBUG
        qDebug() << Q_FUNC_INFO;
#endif
        this->remove(item);
    }

    void onModelCleared(void) {
#ifdef QQML_PROXY_OBJECT_MODEL_DEBUG
        qDebug() << Q_FUNC_INFO;
#endif
        this->clear();
    }

    void onMyItemPropertyChanged(QObject *item, int role) {
        Q_UNUSED(role)
#ifdef QQML_PROXY_OBJECT_MODEL_DEBUG
        qDebug() << Q_FUNC_INFO;
#endif
        if (!fulfillsFilters(static_cast<ItemType*>(item))) {
            this->remove(item);
            return;
        }

        if (m_lambda_sort.size()) {
            ///TODO: think about a solution to not remove and add it again if it's already
            /// in a good position
            //int currentPosition = this->indexOf(item);
//            int position = getInsertionIndex(item);
            //if(currentPosition != position) {
                this->remove(item);
                this->insert(this->getInsertionIndex(item), item);
            //}
        }
    }

    // Used for an item changed in original_model that didn't match the filter and now it does
    // TODO: think this again
    void onModelItemPropertyChanged(QObject *obj, int role) {
        ItemType* item = static_cast<ItemType*> (obj);
        QString role_name = QString(m_original_model->roleNames().value(role, QByteArrayLiteral("")));
        ////  don't look at this code...
        if (role_name == "") /// empty property changed???
            return ;
        if (this->contains(item)) {
            if (propertyRelevantForFilter(item, role_name)) {
                if (fulfillsFilters(item)) {
                    if (propertyRelevantForSort(item, role_name)) {
                        //int currentPosition = this->indexOf(item);
//                        int position = getInsertionIndex(item);
                        //if(currentPosition != position) {
                            this->remove(item);
                            this->insert(this->getInsertionIndex(item), item);
                        //}
                    }
                } else {
                    this->remove(item);
                }
            } else {
                if(propertyRelevantForSort(item, role_name)) {
                    //int currentPosition = this->indexOf(item);
//                    int position = getInsertionIndex(item);
                    //if(currentPosition != position) {
                        this->remove(item);
                        this->insert(this->getInsertionIndex(item), item);
                    //}
                }
            }
        }
        else {
            if (propertyRelevantForFilter(item, role_name)) {
                if (fulfillsFilters(item)) {
                    if (propertyRelevantForSort(item, role_name)) {
                        /// move to sorted position
                        this->insert(this->getInsertionIndex(item), item);
                    } else {
                        if(m_lambda_sort.size())
                            this->insert(this->getInsertionIndex(item), item);
                        else
                            this->append(item);
                    }
                }
            }
        }
    }

private:

    QQmlObjectListModel<ItemType>* m_original_model = Q_NULLPTR;

    QHash<QString, QString> m_filter2prop;
    QHash<QString, QString> m_sort2prop;
    QHash<QString, std::function<bool(const ItemType*)>> m_lambda_filters;
    QList<QPair<QString, std::function<int(const ItemType*, const ItemType*)>>> m_lambda_sort;

    QVariant getDataFromRole(QObject *item, int role) {
#ifdef QQML_PROXY_OBJECT_MODEL_DEBUG
        qDebug() << Q_FUNC_INFO;
#endif
        QVariant ret;

        const QByteArray rolename = m_original_model->roleNames().value (role, QByteArrayLiteral(""));
        ret.setValue (item->property (rolename));

        return ret;
    }

    bool fulfillsFilters(ItemType* item) {
#ifdef QQML_PROXY_OBJECT_MODEL_DEBUG
        qDebug() << Q_FUNC_INFO;
#endif
        for(auto filter_fct : m_lambda_filters.values())
            if(!filter_fct(item))
                return false;
        return true;
    }

    /// method checks if there are lambdas that involves role_name for item, and if at least one of them do not pass
    /// then, returns false (need to be removed)
    /// or true if we need to keep itR
    bool propertyRelevantForFilter(ItemType* item, QString role_name) {
        Q_UNUSED(item);
        for(auto filter_fct : m_lambda_filters.keys()) {
            if(!m_filter2prop.values(filter_fct).contains(role_name)) /// if role is not relevant, skyp filter
                continue;
            return true;
        }
        return false;
    }

    bool propertyRelevantForSort(ItemType* item, QString role_name) {
        Q_UNUSED(item);
        for(auto sort_pair : m_lambda_sort) {
            if(!m_sort2prop.values(sort_pair.first).contains(role_name))
                continue;
            return true;
        }
        return false;
    }

    void m_sort(void) {
        if(this->m_lambda_sort.size()) {
            std::stable_sort(this->m_items.begin(), this->m_items.end(), [=](const ItemType* a, const ItemType* b) -> bool {
                for(auto it : this->m_lambda_sort) {
                    int act = it.second(a, b);
                    if(act < 0)
                        return 1;
                    if(act > 0)
                        return 0;
                }
                return 0;
            });
        }
        this->reload();
    }

    int getInsertionIndex(QObject *itemObj) {
        ItemType* item = static_cast<ItemType*>(itemObj);
        if(this->m_items.count() == 0)
            return 0;
        auto lessThanCmp = [=](const ItemType* a, const ItemType* b) -> bool {
            for(auto it : this->m_lambda_sort) {
                int act = it.second(a, b);
                if(act < 0)
                    return 1;
                if(act > 0)
                    return 0;
            }
            return 0;
        };
        int li = 0, ls = this->m_items.count() - 1, res = -1;
        while(li <= ls) {
            int mid = (li + ls) / 2;
            if(lessThanCmp(this->m_items.at(mid), item)) { /// a < b
                li = mid + 1;
                res = mid;
            }
            else
                ls = mid - 1;
        }
        return res + 1;
    }

private:
    void clear(void) override {
        QQmlObjectListModel<ItemType>::clear();
    }
    void append(QObject* item) override {
        QQmlObjectListModel<ItemType>::append(item);
    }
    void prepend(QObject* item) override {
        QQmlObjectListModel<ItemType>::prepend(item);
    }
    void insert(int idx, QObject* item) override {
        QQmlObjectListModel<ItemType>::insert(idx, item);
    }
    void append(const QList <ItemType*> & itemList) override {
        QQmlObjectListModel<ItemType>::append(itemList);
    }
    void prepend(const QList <ItemType*> & itemList) override {
        QQmlObjectListModel<ItemType>::prepend(itemList);
    }
    void insert(int idx, const QList<ItemType*> & itemList) override {
        QQmlObjectListModel<ItemType>::insert(idx, itemList);
    }
    void move(int idx, int pos) override {
        QQmlObjectListModel<ItemType>::move(idx, pos);
    }
    void remove(ItemType* item) override {
        QQmlObjectListModel<ItemType>::remove(item);
    }
    void remove(QObject * item) override {
        QQmlObjectListModel<ItemType>::remove(item);
    }
    void remove(int idx) override {
        QQmlObjectListModel<ItemType>::remove(idx);
    }

};

#define QML_PROXY_OBJMODEL_PROPERTY(type, name, default_value) \
    protected: Q_PROPERTY (QQmlObjectListModelBase * name READ get_##name CONSTANT) \
    protected: QQmlProxyObjectModel<type> * m_##name = default_value; \
    public: QQmlProxyObjectModel<type> * get_##name (void) const { return m_##name; } \
    private:


#endif // QQMLPROXYOBJECTMODEL_H
