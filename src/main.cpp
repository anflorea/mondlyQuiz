#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "ActionTypes.h"
#include "StoreIds.h"
#include "IDispatcher.h"
#include "Dispatcher.h"
#include "StoreProvider.h"
#include "ServiceProvider.h"
#include "ActionCreator.h"

#include "QuestionModel.h"

#include "GeneralStore.h"

void registerTypes();
void registerMetaTypes();

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    registerMetaTypes();
    registerTypes();

    IDispatcher* dispatcher = new Dispatcher();

    ServiceProvider* serviceProvider = new ServiceProvider();
    StoreProvider* storeProvider = new StoreProvider(dispatcher);

    ActionCreator* actionCreator = new ActionCreator(dispatcher, serviceProvider);

    engine.rootContext()->setContextProperty("StoreProvider", storeProvider);
    engine.rootContext()->setContextProperty("ActionCreator", actionCreator);


    engine.rootContext()->setContextProperty("GeneralStore", static_cast<GeneralStore*>(storeProvider->getStoreById(StoreIdsHelper::GENERAL_STORE)));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}

void registerTypes()
{
    qmlRegisterType<StoreIdsHelper>("Flux", 1, 0, "StoreIdsHelper");
}

void registerMetaTypes()
{
    qRegisterMetaType<Json::Value>("Json::Value");
    qRegisterMetaType<QuestionModel*>("QuestionModel*");
}
