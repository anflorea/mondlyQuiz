import QtQuick 2.0

Item {
    id: lobbyView

    Rectangle {
        id: lobbyHeader

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        height: 70

        color: secondColor

        Rectangle {
            id: leaveLobby

            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.rightMargin: 10

            width: 40
            height: 30

            color: mainColor

            radius: 10

            Text {
                anchors.centerIn: parent

                text: "<-"
                color: whiteColor
            }

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    console.log("leave current lobby")
                    ActionCreator.leaveLobby()
                }
            }
        }

        Text {
            id: gameOwnerText

            anchors {
                top: parent.top
                bottom: parent.bottom
                left: leaveLobby.right
                right: parent.right
            }

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            font.pixelSize: 15

            text: "Owner: " + GeneralStore.currentLobbies.get(GeneralStore.joinedLobby).ownerUsername
            color: whiteColor
        }
    }

    Rectangle {
        id: lobbyContainer

        anchors {
            top: lobbyHeader.bottom
            bottom: lobbyFooter.top
            left: parent.left
            right: parent.right
        }

        ListView {
            id: lobbyPlayers

            anchors.fill: parent
            model: GeneralStore.currentLobbies.get(GeneralStore.joinedLobby).users

            clip: true

            spacing: 10

            delegate: Rectangle {

                anchors.horizontalCenter: parent.horizontalCenter

                width: parent.width * 0.90
                height: 80

                color: thirdColor
                radius: 10

                Text {
                    id: usernameText

                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 20

                    text: username
                }

                Text {
                    id: noPlayers

                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    anchors.rightMargin: 20

                    text: "" + points + " points"
                }
            }
        }

        color: backgroundColor
    }

    Rectangle {
        id: lobbyFooter

        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }

        visible: GeneralStore.currentLobbies.get(GeneralStore.joinedLobby).ownerUsername ===
                        GeneralStore.currentUser

        height: visible ? 40 : 0

        color: secondColor

        Rectangle {
            id: startGame

            anchors.centerIn: parent

            width: 100
            height: 30

            color: mainColor

            radius: 10

            Text {
                anchors.centerIn: parent

                text: "Start Game"
                color: whiteColor
            }

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    console.log("start the game")
                    ActionCreator.startGame()
                }
            }
        }

    }
}
