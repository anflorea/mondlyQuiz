#include "QuestionModel.h"

QuestionModel::QuestionModel(QObject *parent)
    : BaseModel (parent)
{
    m_answers = new QQmlObjectListModel<AnswerModel>();
}

QuestionModel::QuestionModel(Json::Value data, QObject *parent)
    : QuestionModel (parent)
{
    parse(data);
}

void QuestionModel::parse(Json::Value data)
{
    qDebug() << "----------------------------";
    qDebug() << "----------------------------";
    qDebug() << QString::fromStdString(data.toStyledString());

    Json::Value json = data.get("question", Json::Value());

    qDebug() << "----------------------------";
    qDebug() << "----------------------------";
    qDebug() << QString::fromStdString(json.toStyledString());

    update_ref_id(QString::fromStdString(json.get("id", "").asString()));
    update_question(QString::fromStdString(json.get("question", "").asString()));

    Json::Value jsonAnswers = data.get("answers", Json::arrayValue);

    if (jsonAnswers.isString()) {
        Json::Reader reader;
        std::string oldData = jsonAnswers.asString();
        reader.parse(oldData, jsonAnswers);
    }

    if (jsonAnswers.isArray()) {
        for (Json::Value one : jsonAnswers) {
            AnswerModel *model = new AnswerModel(one);
            m_answers->append(model);
        }
    }
}

