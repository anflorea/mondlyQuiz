import QtQuick 2.0

Item {
    id: homeView

    Component.onCompleted: {
        console.log("getting lobbies...")
        ActionCreator.getLobbys()
    }

    Rectangle {
        id: homeHeader

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        height: 70

        color: secondColor

        Text {
            id: username

            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.verticalCenter: parent.verticalCenter

            text: GeneralStore.currentUser

            color: whiteColor
            font {
                pixelSize: 20
            }
        }

        Rectangle {
            id: logoutButton

            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 5

            width: 100
            height: 30

            color: mainColor

            radius: 10

            Text {
                anchors.centerIn: parent

                text: "Logout"
                color: whiteColor
            }

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    ActionCreator.logout()
                }
            }
        }
    }

    Rectangle {
        id: homeContainer

        anchors {
            top: homeHeader.bottom
            bottom: homeFooter.top
            left: parent.left
            right: parent.right
        }

        color: backgroundColor

        ListView {
            id: lobbyList

            anchors.fill: parent
            model: GeneralStore.currentLobbies

            clip: true

            spacing: 5

            delegate: Rectangle {

                anchors.horizontalCenter: parent.horizontalCenter

                width: parent.width * 0.95
                height: 70

                color: thirdColor
                radius: 10

                Text {
                    id: lobbyName

                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 10

                    text: "Lobby#" + (index + 1)
                }

                Text {
                    id: noPlayers

                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: joinLobby.left
                    anchors.rightMargin: 10

                    text: "" + playersNo + "/10"
                }

                Rectangle {
                    id: joinLobby

                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    anchors.rightMargin: 10

                    height: 30
                    width: 60

                    radius: 10

                    color: mainColor

                    Text {
                        anchors.centerIn: parent
                        text: "Join"
                        color: whiteColor
                        font {
                            pixelSize: 14
                            bold: true
                        }
                    }

                    MouseArea {
                        anchors.fill: parent

                        onClicked: {
                            console.log("join the lobby with id: " + ref_id)
                            ActionCreator.joinLobby(ref_id)
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        id: homeFooter

        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }

        height: 40

        color: secondColor


        Rectangle {
            id: createLobby

            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: (parent.width - 2 * createLobby.width) / 3

            width: 100
            height: 30

            color: mainColor

            radius: 10

            Text {
                anchors.centerIn: parent

                text: "Create Lobby"
                color: whiteColor
            }

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    console.log("create new lobby")
                    ActionCreator.createLobby()
                }
            }
        }

        Rectangle {
            id: joinRandomLobby

            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: (parent.width - 2 * createLobby.width) / 3

            width: 100
            height: 30

            color: mainColor

            radius: 10

            Text {
                anchors.centerIn: parent

                text: "Join Lobby"
                color: whiteColor
            }

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    console.log("Join random lobby")
                    if (GeneralStore.currentLobbies.size() > 0) {
                        var firstLobbyId = GeneralStore.currentLobbies.get(0).ref_id
                        ActionCreator.joinLobby(firstLobbyId)
                    }
                }
            }
        }
    }
}
