#ifndef QQMLSEARCHMODEL_H
#define QQMLSEARCHMODEL_H

#include <QObject>

#include "QQmlVarPropertyHelpers.h"
#include "QQmlObjectListModel.h"
#include "QQmlProxyObjectModel.h"
#include <QMap>
#include "Trie.h"
#include <QSet>
#include <set>
#include <QtAlgorithms>

/**
 * @author: Cosmin Rusu
 *      - blame me at cosmin@kuende.com
 *
 * @brief The QQmlSearchModel class
 *      - used to search in contacts or other sources based on an inputed prefix
 *      - uses a trie algorithm for fast lookup and native sort :)
 *
 * @example on how to use this class
 *      - first, you need to feed the class with a QList of a certain type T (can be anything
 *          as long as it inherits QObject
 *      - then, you need to provide a lamba function which maps a single object of type
 *          T into a QString object so that string can be used when inputting text
 *
 *      @details suppose you want to create a search for a ChatContactModel
 *          - then, you need to give the class a QList<ChatContactModel> and a lambda
 *              to be used when creating the Trie.
 *          - when the user inputs some text, the algorithm would return a list of all
 *              contacts who mapped to a string that had as prefix that spefici text
 *      @code
 *          /// first, create the model
 *          s = new QQmlSearchModel<ChatContactModel>(this);
 *          /// feed the model with lists
 *          s->setSearchModel(SomeListOfChatContactModel, [](ChatContactModel* item) -> QString {
 *              return item->get_name();
 *          });
 *          /// so suppose you have an object
 *              ChatContactModel with name Cosmin Rusu
 *              ChatContactModel with name Pavel Antohe
 *          /// s->search(QString("an")) would fill its result property with
 *              ChatContactModel with nama Pavel Antohe (splits on spaces)
 *          /// s->search(QString("co")) would fill its results property with
 *              ChatContactModel with name Cosmin Rusu (notice the case insensivity - poor guy)
 */
//#define QQML_SEARCH_MODEL_DEBUG

template <class T>
class QQmlSearchModel : public QQmlObjectListModel <T>
{
public:
    QQmlSearchModel(QObject* parent = 0) : QQmlObjectListModel<T>(parent) {
        root = new Trie<T>();
    }
    void setSearchModel(QQmlObjectListModel<T>* original_model, std::function<QString(T*)> lambda, std::function<bool(T*, T*)> cmp, QList<QString> matchProperty, QList<QString> sortProperty)
    {
#ifdef QQML_SEARCH_MODEL_DEBUG
        qDebug() << Q_FUNC_INFO;
#endif
        m_original_model = original_model;

        if (m_original_model) {
            QObject::connect(m_original_model, &QQmlObjectListModel<T>::added, this, &QQmlSearchModel::onModelAdded);
            QObject::connect(m_original_model, &QQmlObjectListModel<T>::addedItems, this, &QQmlSearchModel::onModelItemsAdded);
            QObject::connect(m_original_model, &QQmlObjectListModel<T>::removed, this, &QQmlSearchModel::onModelRemoved);
            QObject::connect(m_original_model, &QQmlObjectListModel<T>::cleared, this, &QQmlSearchModel::onModelCleared);
            QObject::connect(this, &QQmlSearchModel<T>::itemPropertyChanged, this, &QQmlSearchModel::onMyItemPropertyChanged);
            QObject::connect(m_original_model, &QQmlObjectListModel<T>::itemPropertyChanged, this, &QQmlSearchModel::onModelItemPropertyChanged);
        }

        m_toStringLambda = lambda;
        m_cmp = cmp;
        m_matchProperty = matchProperty;
        m_sortProperty = sortProperty;

        buildTrie();
        this->search(m_last_query);
    }
    QList<T*> resultToQList() {
#ifdef QQML_SEARCH_MODEL_DEBUG
        qDebug() << Q_FUNC_INFO;
#endif
        QList<T*> listResults;
        for(int i = 0; i < this->size(); ++ i)
            listResults.append(this->at(i));
        return listResults;
    }
public:
    void search(QString query) {
#ifdef QQML_SEARCH_MODEL_DEBUG
        qDebug() << Q_FUNC_INFO;
#endif
        QVector<QString> query_words = query.split(QRegExp("\\W+"), QString::SkipEmptyParts).toVector();
        QMap<int, T*> result;
        query = "";
        for(auto word: query_words)
            query = query + word;
        m_last_query = query;
        if (!root)
            return;

        QList<T*> toAdd;

        if (query.isEmpty()) {
            toAdd = m_original_model->toList();
        } else {
            getTrie(root, query.toLower(), &result);
            QSet <T*> _hash;
            for(auto it : result.uniqueKeys()) {
                QList<T*> values = result.values(it);
                qSort(values.begin(), values.end(), m_cmp);
                for(auto el = values.begin(); el != values.end(); ++ el) {
                    if(_hash.find(*el) == _hash.end()) {
                        toAdd.append(*el);
                        _hash.insert(*el);
                    }
                }
            }
        }

        this->clear();
        this->append(toAdd);
    }
    void onModelAdded(QObject* item) {
#ifdef QQML_SEARCH_MODEL_DEBUG
        qDebug() << Q_FUNC_INFO;
#endif
        T* current = static_cast<T*>(item);
        this->addToTrie(current);
        if(matchSearch(current, m_last_query))
            this->insert(getInsertionIndex(current), current);
    }
    void onModelItemsAdded(QList<QObject*> items) {
#ifdef QQML_SEARCH_MODEL_DEBUG
        qDebug() << Q_FUNC_INFO;
#endif
        // first, update the internal Trie
        addToTrie(items);
        // then, see if we need to update the result list
        QHash<int, T*> batches;
        T* item;
        for(auto it = items.rbegin(); it != items.rend() ; ++ it) {
            QObject *act = *it;
            item = static_cast<T*>(act);
            if(!matchSearch(item, m_last_query))
                continue;
            batches.insertMulti(getInsertionIndex(item), item);
        }
        for(int key : batches.uniqueKeys()) {
            QList<T*> values = batches.values(key);
            std::stable_sort(values.begin(), values.end(), m_cmp);
            if(values.size() > 0)
                this->insert(getInsertionIndex(values[0]), values);
        }
    }
    void onModelRemoved(QObject* act) {
#ifdef QQML_SEARCH_MODEL_DEBUG
        qDebug() << Q_FUNC_INFO;
#endif
        T* item = static_cast<T*>(act);
        this->removeFromTrie(static_cast<T*>(item));
        if(this->contains(item))
            this->remove(item);
    }
    void onModelCleared(void) {
#ifdef QQML_SEARCH_MODEL_DEBUG
        qDebug() << Q_FUNC_INFO;
#endif
        if(root != Q_NULLPTR) {
            delete root;
            root = new Trie<T>();
        }
        this->clear();
    }
    void onMyItemPropertyChanged(QObject *obj, int role) {
#ifdef QQML_SEARCH_MODEL_DEBUG
        qDebug() << Q_FUNC_INFO;
#endif
        QString roleName = QString(m_original_model->roleNames().value(role, QByteArrayLiteral("")));
        T* item = static_cast<T*>(obj);
        if(propertyRelevantForMatch(roleName)) {
            this->onModelRemoved(obj);
            this->onModelAdded(obj);
        }
        else {
            if(propertyRelevantForSort(roleName)) {
                if(this->contains(item))
                    this->remove(item);
                this->insert(getInsertionIndex(item), item);
            }
        }
    }
    void onModelItemPropertyChanged(QObject *obj, int role) {
#ifdef QQML_SEARCH_MODEL_DEBUG
        qDebug() << Q_FUNC_INFO;
#endif
        this->onMyItemPropertyChanged(obj, role);
    }
private:
    inline bool propertyRelevantForSort(QString roleName) {
#ifdef QQML_SEARCH_MODEL_DEBUG
        qDebug() << Q_FUNC_INFO;
#endif
        for(auto it : m_matchProperty)
            if(it == roleName)
                return 1;
        return 0;
    }
    inline bool propertyRelevantForMatch(QString roleName) {
#ifdef QQML_SEARCH_MODEL_DEBUG
        qDebug() << Q_FUNC_INFO;
#endif
        for(auto it : m_matchProperty)
            if(it == roleName)
                return 1;
        return 0;
    }

    inline int getInsertionIndex(T* current) {
#ifdef QQML_SEARCH_MODEL_DEBUG
        qDebug() << Q_FUNC_INFO;
#endif
        if(this->m_items.count() == 0)
            return 0;
        int li = 0, ls = this->m_items.count() - 1, res = -1;
        while(li <= ls) {
            int mid = (li + ls) / 2;
            if(m_cmp(this->m_items.at(mid), current)) {
                li = mid + 1;
                res = mid;
            }
            else
                ls = mid - 1;
        }
        return res + 1;
    }
    inline bool matchSearch(T* el, QString query) {
#ifdef QQML_SEARCH_MODEL_DEBUG
        qDebug() << Q_FUNC_INFO;
#endif
        QString actName = m_toStringLambda(el);
        QStringList m_name_list = actName.split(QRegExp("\\W+"), QString::SkipEmptyParts);
        for(int start = 0; start < m_name_list.size(); ++ start) {
            QString prefix = "";
            for(int i = start; i < start + m_name_list.size(); ++ i)
                prefix = prefix + m_name_list.at(i % m_name_list.size()).toLower();
            if(prefix.size() < query.size())
                return 0;
            if(prefix.startsWith(query, Qt::CaseInsensitive))
                return 1;
        }
        return 0;
    }
    void buildTrie() {
#ifdef QQML_SEARCH_MODEL_DEBUG
        qDebug() << Q_FUNC_INFO;
#endif
        for(int i = 0; i < m_original_model->size(); ++ i)
            this->addToTrie(m_original_model->at(i));
    }
    void addToTrie(QObject* item) {
#ifdef QQML_SEARCH_MODEL_DEBUG
        qDebug() << Q_FUNC_INFO;
#endif
        T* act = static_cast<T*>(item);
        QString actName = m_toStringLambda(act);
        QStringList m_name_list = actName.split(QRegExp("\\W+"), QString::SkipEmptyParts);
        for(int start = 0; start < m_name_list.size(); ++ start) {
            QString prefix = "";
            for(int i = start; i < start + m_name_list.size(); ++ i)
                prefix = prefix + m_name_list.at(i % m_name_list.size()).toLower();
            this->root->insert(prefix, act, start);
        }
    }
    void addToTrie(QList<QObject*> items) {
#ifdef QQML_SEARCH_MODEL_DEBUG
        qDebug() << Q_FUNC_INFO;
#endif
        for(int i = 0; i < items.size(); ++ i)
            this->addToTrie(static_cast<T*>(items.at(i)));
    }
    void removeFromTrie(T* act) {
#ifdef QQML_SEARCH_MODEL_DEBUG
        qDebug() << Q_FUNC_INFO;
#endif
        this->root->remove(act);
    }
    void getTrie(Trie<T> *t, QString query, QMap<int, T*>* _hash) {
#ifdef QQML_SEARCH_MODEL_DEBUG
        qDebug() << Q_FUNC_INFO;
#endif
        for(int i = 0; i < query.size(); ++ i) {
            if(!t)
                return ;
            t = t->sons[query.at(i)];
        }
        if(!t)
            return ;
        t->dfs(_hash);
    }

    QString m_last_query = QString();
    QQmlObjectListModel<T>* m_original_model;
    Trie <T> *root = Q_NULLPTR;
    std::function<QString(T*)> m_toStringLambda;
    std::function<bool(T*, T*)> m_cmp;
    QList<QString> m_matchProperty, m_sortProperty;
};

#define QML_SEARCH_MODEL_PROPERTY(type, name, default_value) \
    protected: Q_PROPERTY (QQmlObjectListModelBase * name READ get_##name CONSTANT) \
    protected: QQmlSearchModel<type> * m_##name = default_value; \
    public: QQmlSearchModel<type> * get_##name (void) const { return m_##name; } \
    private:

#endif // QQMLSEARCHMODEL_H
