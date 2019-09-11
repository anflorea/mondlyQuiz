#ifndef ACTION_H
#define ACTION_H

#include <QObject>
#include <QVariant>
#include "ActionTypes.h"
#include "json/json.h"

class Action : public QObject
{
    Q_OBJECT
public:
    Action() {}
    Action(const Action& a);
    Action(ActionType::Type type);
    Action(ActionType::Type type, int number);
    Action(ActionType::Type type, Json::Value payload);
    Action(ActionType::Type type, QVariant variant);
    Action(ActionType::Type type, QString string);
    Action(ActionType::Type type, const QStringList& strings);

    ActionType::Type getType() const;

    template<typename T>
    T getPayload() const {
        return m_payload.value<T>();
    }

    void operator=(const Action& a);

protected:
    ActionType::Type    m_type;
    QVariant            m_payload;
};

Q_DECLARE_METATYPE(Json::Value)

#endif // ACTION_H
