#ifndef STOREIDS_H
#define STOREIDS_H

#include <QObject>

class StoreIdsHelper : public QObject
{
    Q_OBJECT

public:
    enum StoreId {
        NONE_STORE = 0,
        GENERAL_STORE,
    };
    Q_ENUM(StoreId)
};
Q_DECLARE_METATYPE(StoreIdsHelper::StoreId)

#endif // STOREIDS_H
