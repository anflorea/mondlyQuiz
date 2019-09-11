#include "GeneralStore.h"

GeneralStore::GeneralStore(StoreIdsHelper::StoreId id, IDispatcher *dispatcher) : Store(id, dispatcher)
{
    m_currentLobbies = new QQmlObjectListModel<LobbyModel>(this, "ref_id");
    m_lastGameStats = new QQmlObjectListModel<UserModel>(this);
}

void GeneralStore::notify(const Action &action)
{
    switch (action.getType()) {
        case ActionType::AuthSuccess:
        {
            update_isLoggedIn(true);
            break;
        }
        case ActionType::AuthError:
        {
            authError();
            break;
        }
        case ActionType::GotUsername:
        {
            update_currentUser(action.getPayload<QString>());
            break;
        }
        case ActionType::LogoutSuccess:
        {
            update_isLoggedIn(false);
            m_currentLobbies->clear();
            m_lastGameStats->clear();
            update_joinedLobby(-1);
            break;
        }
        case ActionType::GotEvent:
        {
            Json::Value event = action.getPayload<Json::Value>();
            handleEvent(event);
            break;
        }
        case ActionType::GetLobbysSuccess:
        {
            Json::Value lobbies = action.getPayload<Json::Value>();
            bool joinLobby = false;
            if (lobbies.isArray()) {
                for (Json::Value lobby : lobbies) {
                    LobbyModel *lobbyModel = new LobbyModel(lobby);
                    if (lobbyModel->get_ownerUsername() == m_currentUser) {
                        update_joinedLobby(m_currentLobbies->size());
                        joinLobby = true;
                    }
                    else {
                        for (UserModel *user : lobbyModel->get_users()->toList()) {
                            if (user->get_username() == m_currentUser) {
                                update_joinedLobby(m_currentLobbies->size());
                                joinLobby = true;
                            }
                        }
                    }
                    m_currentLobbies->append(lobbyModel);
                }
            }
            qDebug() << "Got lobbies: " << m_currentLobbies->size();
            if (joinLobby)
                joinedLobby();
            break;
        }
        case ActionType::CreateLobbySuccess:
        {
            Json::Value lobbyJson = action.getPayload<Json::Value>();
            LobbyModel *lobby = new LobbyModel(lobbyJson);
            m_currentLobbies->prepend(lobby);
            update_joinedLobby(0);
            joinedLobby();
            break;
        }
        case ActionType::LeaveLobbySuccess:
        {
            leftLobby();
            break;
        }
        case ActionType::JoinLobbySuccess:
        {
            QString lobbyId = action.getPayload<QString>();
            qDebug() << "joined lobby: " << lobbyId;
            for (int i = 0; i < m_currentLobbies->size(); i++) {
                LobbyModel *lobby = static_cast<LobbyModel*>(m_currentLobbies->get(i));
                if (lobby->get_ref_id() == lobbyId) {
                    update_joinedLobby(i);
                }
            }
            joinedLobby();
            break;
        }
        case ActionType::ReturnToLobby:
        {
            m_lastGameStats->clear();
            returnToLobby();
            break;
        }
        case ActionType::AnswerQuestionSuccess:
        {
            answerCorrect();
            break;
        }
        case ActionType::AnswerQuestionFail:
        {
            answerWrong();
            break;
        }
        default:
            break;
    }
}

void GeneralStore::clear()
{

}

void GeneralStore::handleEvent(Json::Value event)
{
    qDebug() << "Got event in store: " << QString::fromStdString(event.toStyledString());
    QString eventType = QString::fromStdString(event.get("eventType", "").asString());
    Json::Value data = event.get("event", Json::Value());

    if (data.isString()) {
        Json::Reader reader;
        std::string oldData = data.asString();
        reader.parse(oldData, data);
    }

    if (eventType == "LobbyChanged") {
        LobbyModel *newLobby = new LobbyModel(data);
        if (!m_currentLobbies->getByUid(newLobby->get_ref_id())) {
            if (newLobby->get_ownerUsername() != m_currentUser)
                m_currentLobbies->append(newLobby);
        } else {
            for (LobbyModel *lobby : m_currentLobbies->toList()) {
                if (lobby->get_ref_id() == newLobby->get_ref_id()) {
                    lobby->update(newLobby);
                }
            }
        }
    } else if (eventType == "LobbyDeleted") {
        QString lobbyId = QString::fromStdString(data.get("id", "").asString());
        if (!lobbyId.isEmpty()) {
            LobbyModel *lobby = m_currentLobbies->getByUid(lobbyId);
            if (lobby) {
                for (UserModel *user : lobby->get_users()->toList()) {
                    if (user->get_username() == m_currentUser && lobby->get_ownerUsername() != m_currentUser)
                        leftLobby();
                }
                m_currentLobbies->remove(lobby);
            }
        }
    } else if (eventType == "NewQuestion") {

        Json::Value players = data.get("players", Json::arrayValue);

        bool ok = false;
        if (players.isArray()) {
            for (Json::Value player : players) {
                UserModel *user = new UserModel(player);
                if (user->get_username() == m_currentUser)
                    ok = true;
            }
        }

        if (!ok)
            return;

        bool gameStart = false;
        if (m_currentQuestion == Q_NULLPTR) {
            gameStart = true;
        }

        QuestionModel *question = new QuestionModel(data);
        update_currentQuestion(question);

        if (gameStart)
            gameStarted();
        else
            gotNewQuestion();
    } else if (eventType == "GameEnded") {
        m_currentQuestion = Q_NULLPTR;

        if (data.isArray()) {
            for (Json::Value player : data) {
                UserModel *user = new UserModel(player);
                m_lastGameStats->append(user);
            }
        }
        gameEnded();
    }
}
