import QtQuick 2.0

Rectangle {
    id: gameStatsView

    color: backgroundColor

    Rectangle {
        id: gameStatsHeader

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
                    ActionCreator.returnToLobby()
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

            text: "Game stats"
            color: whiteColor
        }
    }

    ListView {
        anchors.top: gameStatsHeader.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        model: GeneralStore.lastGameStats

        clip: true

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
}
