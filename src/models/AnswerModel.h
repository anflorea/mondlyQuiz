#ifndef ANSWERMODEL_H
#define ANSWERMODEL_H

#include "BaseModel.h"

class AnswerModel : public BaseModel
{
    Q_OBJECT

    QML_READONLY_VAR_PROPERTY(QString, answerText, QString())
    QML_READONLY_VAR_PROPERTY(QString, questionId, QString())

public:
    AnswerModel(QObject *parent = 0);
    AnswerModel(Json::Value data, QObject *parent = 0);

    void parse(Json::Value data) override;
};

#endif // ANSWERMODEL_H
