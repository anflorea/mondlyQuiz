#ifndef QUESTIONMODEL_H
#define QUESTIONMODEL_H

#include "BaseModel.h"
#include "AnswerModel.h"

class QuestionModel : public BaseModel
{
    Q_OBJECT

    QML_READONLY_VAR_PROPERTY(QString, question, QString())
    QML_OBJMODEL_PROPERTY(AnswerModel, answers, Q_NULLPTR)

public:
    QuestionModel(QObject *parent = 0);
    QuestionModel(Json::Value data, QObject *parent = 0);

    void parse(Json::Value data) override;

    void parseAnswers(Json::Value data);
};

#endif // QUESTIONMODEL_H
