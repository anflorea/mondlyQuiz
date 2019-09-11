#include "AnswerModel.h"


AnswerModel::AnswerModel(QObject *parent)
    : BaseModel (parent)
{

}

AnswerModel::AnswerModel(Json::Value data, QObject *parent)
    : AnswerModel (parent)
{
    parse(data);
}

void AnswerModel::parse(Json::Value data)
{
    update_ref_id(QString::fromStdString(data.get("id", "").asString()));
    update_answerText(QString::fromStdString(data.get("answer", "").asString()));
    update_questionId(QString::fromStdString(data.get("questionID", "").asString()));
}
