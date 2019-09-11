import QtQuick 2.0

Rectangle {
    id: gameView

    anchors.fill: parent

    property bool answered: false
    property bool returned: false
    property bool correct: false

    color: backgroundColor

    property var qObj: GeneralStore.currentQuestion

    onQObjChanged: {
        timerItem.timeLeft = 5
    }

    Timer {
        id: theTimer

        interval: 1000
        running: true
        repeat: true

        onTriggered: {
            if (timerItem.timeLeft > 0)
                timerItem.timeLeft -= 1
        }
    }

    Item {
        id: timerItem

        property int timeLeft: 5

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        height: 100

        Text {
            anchors.centerIn: parent

            font.pixelSize: 30

            text: timerItem.timeLeft
        }
    }

    Text {
        id: question

        anchors.top: timerItem.bottom
        anchors.left: parent.left
        anchors.leftMargin: 50
        anchors.right: parent.right
        anchors.rightMargin: 50

        height: 100

        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter

        wrapMode: Text.Wrap

        text: GeneralStore.currentQuestion.question
    }

    Item {
        id: answersRect

        anchors.top: question.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 20


        Column {

            spacing: 5

            Repeater {
                id: answersRepeater

                model: GeneralStore.currentQuestion.answers

                Rectangle {
                    width: answersRect.width
                    height: 70

                    radius: 10

                    color: {
                        if (thisOne && gameView.returned) {
                            if (gameView.correct) {
                                return "green"
                            } else {
                                return "red"
                            }
                        } else {
                            return thirdColor
                        }
                    }

                    property bool thisOne: false

                    Connections {
                        target: GeneralStore
                        onGotNewQuestion: {
                            thisOne = false
                        }
                    }

                    Text {
                        id: answerTextt

                        anchors.centerIn: parent

                        text: answerText
                    }

                    MouseArea {

                        enabled: !gameView.answered

                        anchors.fill: parent

                        onClicked: {
                            gameView.answered = true
                            thisOne = true
                            console.log("Clicked answer: " + (index + 1))
                            ActionCreator.answerQuestion(questionId, ref_id)
                        }
                    }
                }
            }
        }
    }

    Connections {
        id: answerConnections

        target: GeneralStore

        onAnswerCorrect: {
            gameView.correct = true
            gameView.returned = true
        }

        onAnswerWrong: {
            gameView.correct = false
            gameView.returned = true
        }

        onGotNewQuestion: {
            gameView.answered = false
            gameView.returned = false
            gameView.correct = false
        }
    }
}
