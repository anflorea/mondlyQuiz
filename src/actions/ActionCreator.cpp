#include "ActionCreator.h"
#include <QQmlContext>
#include "ServiceProvider.h"

#include "Response.h"
#include "RequestService.h"

ActionCreator::ActionCreator(IDispatcher *dispatcher,
                             IServiceProvider *serviceProvider,
                             QObject *parent)
    : QObject(parent)
{
    m_dispatcher = dispatcher;

    m_requestService = serviceProvider->getRequestService();
    m_authService = serviceProvider->getAuthService();
    m_generalService = serviceProvider->getGeneralService();
}

void ActionCreator::login(QString username, QString password)
{
    qDebug() << Q_FUNC_INFO << username << " -- " << password;
    m_dispatcher->dispatch(Action(ActionType::AuthStarted));

    auto success = [&](QString token, QString username) {
        m_requestService->setAccessToken(token);
        m_dispatcher->dispatch(Action(ActionType::GotUsername, username));
        m_dispatcher->dispatch(Action(ActionType::AuthSuccess, token));
        connectToEventStream();
    };

    auto fail = [&](Response* response) {
        m_dispatcher->dispatch(Action(ActionType::AuthError));
    };

    m_authService->signIn(username, password, success, fail);
}

void ActionCreator::logout()
{
    qDebug() << Q_FUNC_INFO;
    m_dispatcher->dispatch(Action(ActionType::LogoutSuccess));
    m_requestService->setAccessToken("");
}

void ActionCreator::getLobbys()
{
    qDebug() << Q_FUNC_INFO;
    m_dispatcher->dispatch(Action(ActionType::GetLobbysStarted));

    auto success = [&](Json::Value response) {
        m_dispatcher->dispatch(Action(ActionType::GetLobbysSuccess, response));
    };

    auto fail = [&](Response* response) {
        m_dispatcher->dispatch(Action(ActionType::GetLobbysError, response->bodyAsJson()));
    };

    m_generalService->getLobbys(success, fail);
}

void ActionCreator::connectToEventStream()
{
    qDebug() << Q_FUNC_INFO;
    auto success = [&]() {
    };

    auto fail = [&]() {
    };

    auto gotEvent = [&](Json::Value event) {
        m_dispatcher->dispatch(Action(ActionType::GotEvent, event));
    };

    m_authService->connectToEventStream(success, gotEvent, fail);
}

void ActionCreator::createLobby()
{
    qDebug() << Q_FUNC_INFO;

    auto success = [&](Json::Value lobby) {
        m_dispatcher->dispatch(Action(ActionType::CreateLobbySuccess, lobby));
    };

    auto fail = [&](Response* response) {
        Q_UNUSED(response)
        m_dispatcher->dispatch(Action(ActionType::CreateLobbyFail));
    };

    m_generalService->createLobby(success, fail);
}

void ActionCreator::joinLobby(QString lobbyId)
{
    qDebug() << Q_FUNC_INFO;

    auto success = [&](QString id) {
        m_dispatcher->dispatch(Action(ActionType::JoinLobbySuccess, id));
    };

    auto fail = [&](Response* response) {
        Q_UNUSED(response)
        m_dispatcher->dispatch(Action(ActionType::JoinLobbyFail));
    };

    m_generalService->joinLobby(lobbyId, success, fail);
}

void ActionCreator::leaveLobby()
{
    qDebug() << Q_FUNC_INFO;

    auto success = [&]() {
        m_dispatcher->dispatch(Action(ActionType::LeaveLobbySuccess));
    };

    auto fail = [&](Response* response) {
        Q_UNUSED(response)
        m_dispatcher->dispatch(Action(ActionType::LeaveLobbyFail));
    };

    m_generalService->leaveLobby(success, fail);
}

void ActionCreator::startGame()
{
    auto success = [&](Json::Value) {

    };

    auto fail = [&](Response*) {

    };

    m_generalService->startGame(success, fail);
}

void ActionCreator::answerQuestion(QString questionId, QString answerId)
{
    qDebug() << "Answer question: " << questionId << "  " << answerId;

    auto success = [&](Json::Value data) {
        bool ok = data.get("ok", false).asBool();
        if (ok) {
            m_dispatcher->dispatch(Action(ActionType::AnswerQuestionSuccess));
        } else {
            m_dispatcher->dispatch(Action(ActionType::AnswerQuestionFail));
        }
    };

    auto fail = [&](Response*) {

    };

    m_generalService->answerQuestion(questionId, answerId, success, fail);
}

void ActionCreator::returnToLobby()
{
    m_dispatcher->dispatch(Action(ActionType::ReturnToLobby));
}
