#include "Action.h"

Action::Action(const Action &a)
{
    m_type = a.m_type;
    m_payload = a.m_payload;
}

Action::Action(ActionType::Type type)
{
    m_type = type;
}

Action::Action(ActionType::Type type, int number)
    : Action(type)
{
    m_payload = QVariant::fromValue(number);
}

Action::Action(ActionType::Type type, Json::Value payload)
    : Action(type)
{
    m_payload = QVariant::fromValue(payload);
}

Action::Action(ActionType::Type type, QVariant variant)
    : Action(type)
{
    m_payload = variant;
}

Action::Action(ActionType::Type type, QString string)
    : Action(type)
{
    m_payload = QVariant(string);
}

Action::Action(ActionType::Type type, const QStringList &strings)
    : Action(type)
{
    m_payload = QVariant(strings);
}

void Action::operator=(const Action &a)
{
    m_type = a.m_type;
    m_payload = a.m_payload;
}

ActionType::Type Action::getType() const
{
    return m_type;
}
