import QtQuick 2.0
import QtQuick.Controls 2.2

Rectangle {
    id: loginView

    color: backgroundColor

    Text {
        id: title

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: username.top

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        text: "AwsmQuiz"

        color: "#454545"

        font {
            pixelSize: 25
            bold: true
        }
    }

    TextField {
        id: username

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.verticalCenter
        anchors.topMargin: -50

        width: parent.width * 0.8
        height: 30

        placeholderText: "Username"
        text: "testuser"

        horizontalAlignment: Text.AlignHCenter


        background: Item {
            anchors.fill: parent

            Rectangle {
                anchors.bottom: parent.bottom
                color: darkGrey
                width: parent.width
                height: 1

                radius: 3
            }
        }
    }

    TextField {
        id: password

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: username.bottom
        anchors.topMargin: 20

        width: parent.width * 0.8
        height: 30

        text: "testt"

        horizontalAlignment: Text.AlignHCenter

        placeholderText: "Password"
        echoMode: TextInput.Password

        background: Item {
            anchors.fill: parent

            Rectangle {
                anchors.bottom: parent.bottom
                color: darkGrey
                width: parent.width
                height: 1

                radius: 3
            }
        }
    }

    Rectangle {
        id: loginButton

        property bool isEnabled: true

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: password.bottom
        anchors.topMargin: 40

        width: parent.width * 0.5
        height: 35

        radius: 10

        color: loginButton.isEnabled ? mainColor : Qt.darker(mainColor)

        Text {
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            text: "Login"
            font.pixelSize: 15
            font.bold: true

            color: whiteColor
        }

        MouseArea {
            id: loginMouseArea
            anchors.fill: parent

            onClicked: {
                loginButton.isEnabled = false
                ActionCreator.login(username.text, password.text)
                errorText.isShown = false
            }
        }
    }

    Text {
        id: errorText

        anchors.top: loginButton.bottom
        anchors.topMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter

        property bool isShown: false

        visible: isShown

        text: "Some error has occured!"

        color: "red"
    }

    Connections {
        id: loginConnection

        target: GeneralStore
        ignoreUnknownSignals: true

        onIsLoggedInChanged: {
            if (!isLoggedIn) {
                username.clear()
                password.clear()
                loginButton.isEnabled = true
                errorText.isShown = false
            }
        }

        onAuthError: {
            loginButton.isEnabled = true
            errorText.isShown = true
        }
    }
}
