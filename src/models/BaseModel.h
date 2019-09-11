#ifndef BASEMODEL_H
#define BASEMODEL_H

#include "QQmlVarPropertyHelpers.h"
#include "json/json.h"


//#define PARSABLE_MODEL(class_name) \
//    explicit class_name(QObject* parent = nullptr); \
//    class_name(Json::Value data, QObject* parent = nullptr); \
//    void parse(Json::Value data);

//#define PRINTABLE_MODEL(class_name) \
//    Json::Value	toJson() const; \
//    friend QDebug operator<< (QDebug dbg, const class_name &info) { \
//    dbg.space().noquote() << info.metaObject()->className() << qPrintable(QString::fromStdString((&info)->toJson().toStyledString())); \
//        return dbg.maybeSpace(); \
//    }

//#define PARSABLE_AND_PRINTABLE_MODEL(class_name) \
//    PARSABLE_MODEL(class_name) \
//    PRINTABLE_MODEL(class_name)

class BaseModel : public QObject
{
    Q_OBJECT
    QML_READONLY_VAR_PROPERTY(QString, ref_id, QString())
public:

    explicit BaseModel(QObject* parent) : QObject(parent) { }

    virtual void parse(Json::Value data) = 0;
    virtual Json::Value toJson() const { return Json::nullValue; }

    bool operator ==(const BaseModel &other) const;

};

#endif // BASEMODEL_H
