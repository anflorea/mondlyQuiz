import QtQuick 2.11
import QtQuick.Controls 2.4

ApplicationWindow {
    id: rootWindow
    visible: true
    x: ((Qt.platform.os == "osx") ? 1000 : 0)
    y: ((Qt.platform.os == "osx") ? 100 : 0)
    width: ((Qt.platform.os == "osx") ? 320 : Screen.width)
    height: ((Qt.platform.os == "osx") ? 568 : Screen.height)

    property color mainColor: "#FDD835"
    property color secondColor: "#00BCD4"
    property color thirdColor: "#4DD0E1"
    property color backgroundColor: "#DCDCDC"
    property color darkGrey: "#222222"
    property color whiteColor: "#FFFFFF"

    StackView {
        id: stackView
        initialItem: "Login.qml"
        anchors.fill: parent
    }

    Connections {
        id: loginConnection

        target: GeneralStore
        ignoreUnknownSignals: true

        onIsLoggedInChanged: {
            if (isLoggedIn) {
                stackView.push("HomeView.qml")
            }
            else {
                stackView.pop(null)
            }
        }
    }

    Connections {
        id: lobbyConnection

        target: GeneralStore

        onJoinedLobby: {
            stackView.push("LobbyView.qml")
        }

        onLeftLobby: {
            stackView.pop()
        }

        onGameStarted: {
            stackView.push("GameView.qml")
        }

        onGameEnded: {
            stackView.replace("GameStatsView.qml")
        }

        onReturnToLobby: {
            stackView.pop({item: stackView.get(1)})
        }
    }
}
