#include "GeneralService.h"

#include <QString>
#include <QDebug>
#include "Request.h"
#include "RequestService.h"

GeneralService::GeneralService(RequestService *requestService)
{
    m_requestService = requestService;
}

void GeneralService::getLobbys(const std::function<void (Json::Value)> &success, const std::function<void (Response *)> &fail)
{
    Request *request = m_requestService->GET("/lobbies");

    request->onSuccess([=](Response* response) {
        Json::Value value = response->bodyAsJson();

        qDebug() << QString::fromStdString(value.toStyledString());

        if (success)
            success(value);
    });

    request->onFail([=](Response* response) {
        if (fail)
            fail(response);
    });

    m_requestService->sendRequest(request);
}

void GeneralService::createLobby(const std::function<void (Json::Value)> &success, const std::function<void (Response *)> &fail)
{
    Request *request = m_requestService->POST("/lobbies/create");

    request->onSuccess([=](Response* response) {
        Json::Value value = response->bodyAsJson();

        if (success)
            success(value);
    });

    request->onFail([=](Response* response) {
        if (fail)
            fail(response);
    });

    m_requestService->sendRequest(request);
}

void GeneralService::joinLobby(QString lobbyId, const std::function<void (QString)> &success, const std::function<void (Response *)> &fail)
{
    Request *request = m_requestService->POST("/lobbies/join/" + lobbyId);

    request->onSuccess([=](Response* response) {
        if (success)
            success(lobbyId);
    });

    request->onFail([=](Response* response) {
        if (fail)
            fail(response);
    });

    m_requestService->sendRequest(request);
}

void GeneralService::leaveLobby(const std::function<void ()> &success, const std::function<void (Response *)> &fail)
{
    Request *request = m_requestService->POST("/lobbies/leave");

    request->onSuccess([=](Response* response) {
        Q_UNUSED(response)

        if (success)
            success();
    });

    request->onFail([=](Response* response) {
        if (fail)
            fail(response);
    });

    m_requestService->sendRequest(request);
}

void GeneralService::startGame(const std::function<void (Json::Value)> &success, const std::function<void (Response *)> &fail)
{
    Request *request = m_requestService->POST("/game/start");

    request->onSuccess([=](Response* response) {
        Json::Value value = response->bodyAsJson();

        qDebug() << "game started " << QString::fromStdString(value.toStyledString());

        if (success)
            success(value);
    });

    request->onFail([=](Response* response) {
        if (fail)
            fail(response);
    });

    m_requestService->sendRequest(request);
}

void GeneralService::answerQuestion(QString questionId, QString answerId, const std::function<void (Json::Value)> &success, const std::function<void (Response *)> &fail)
{
    Request *request = m_requestService->POST("/vote");

    Json::Value data;

    data["questionId"] = questionId.toStdString();
    data["answerId"] = answerId.toStdString();

    request->setPayload(data);

    request->setLogging(true);

    request->onSuccess([=](Response* response) {
        Json::Value value = response->bodyAsJson();

        if (success)
            success(value);
    });

    request->onFail([=](Response* response) {
        if (fail)
            fail(response);
    });

    m_requestService->sendRequest(request);
}
