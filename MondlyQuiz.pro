QT += quick websockets
CONFIG += c++14

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

android {
        QT += androidextras
}

INCLUDEPATH += \
    src/third-party/jsoncpp \
    src/actions \
    src/common \
    src/communication \
    src/dispatcher \
    src/enums \
    src/models \
    src/providers \
    src/services \
    src/super-models \
    src/stores \

SOURCES += \
    src/main.cpp \
    src/third-party/jsoncpp/jsoncpp.cpp \
    src/actions/Action.cpp \
    src/dispatcher/Dispatcher.cpp \
    src/providers/StoreProvider.cpp \
    src/stores/Store.cpp \
    src/super-models/QQmlHelpers.cpp \
    src/super-models/QQmlObjectListModel.cpp \
    src/super-models/QQmlVariantListModel.cpp \
    src/providers/ServiceProvider.cpp \
    src/services/RequestService.cpp \
    src/communication/Request.cpp \
    src/services/NetworkManager.cpp \
    src/actions/ActionCreator.cpp \
    src/services/AuthService.cpp \
    src/stores/GeneralStore.cpp \
    src/services/GeneralService.cpp \
    src/models/BaseModel.cpp \
    src/models/LobbyModel.cpp \
    src/models/UserModel.cpp \
    src/models/QuestionModel.cpp \
    src/models/AnswerModel.cpp

HEADERS += \
    src/third-party/jsoncpp/json/json-forwards.h \
    src/third-party/jsoncpp/json/json.h \
    src/actions/Action.h \
    src/enums/ActionTypes.h \
    src/dispatcher/Dispatcher.h \
    src/dispatcher/IDispatcher.h \
    src/enums/StoreIds.h \
    src/providers/IStoreProvider.h \
    src/providers/StoreProvider.h \
    src/stores/Store.h \
    src/super-models/KObjectContainer.h \
    src/super-models/QQmlAutoPropertyHelpers.h \
    src/super-models/QQmlConstRefPropertyHelpers.h \
    src/super-models/QQmlEnumClassHelper.h \
    src/super-models/QQmlHelpersCommon.h \
    src/super-models/QQmlObjectListModel.h \
    src/super-models/QQmlProxyObjectModel.h \
    src/super-models/QQmlPtrPropertyHelpers.h \
    src/super-models/QQmlSearchModel.h \
    src/super-models/QQmlVariantListModel.h \
    src/super-models/QQmlVarPropertyHelpers.h \
    src/providers/ServiceProvider.h \
    src/providers/IServiceProvider.h \
    src/services/RequestService.h \
    src/communication/Request.h \
    src/communication/Response.h \
    src/services/INetworkManager.h \
    src/services/NetworkManager.h \
    src/common/Utils.h \
    src/actions/ActionCreator.h \
    src/common/Defines.h \
    src/services/AuthService.h \
    src/services/IAuthService.h \
    src/stores/GeneralStore.h \
    src/services/GeneralService.h \
    src/models/BaseModel.h \
    src/models/LobbyModel.h \
    src/models/UserModel.h \
    src/models/QuestionModel.h \
    src/models/AnswerModel.h

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
