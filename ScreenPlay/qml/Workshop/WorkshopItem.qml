import QtQuick 2.9
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.3
import QtQuick.Controls.Material 2.2
import Qt.labs.platform 1.0

import "../Installed"

Item {
    id: workshopItem
    width: 320
    height: 180

    property url imgUrl
    property string name
    property int steamID
    property int itemIndex

    property bool isDownloading: false

    RectangularGlow {
        id: effect
        anchors {
            top: parent.top
            topMargin: 3
        }

        height: parent.height
        width: parent.width
        cached: true
        glowRadius: 3
        spread: 0.2
        color: "black"
        opacity: 0.4
        cornerRadius: 15
    }
    Timer {
        id: timerAnim
        interval: 40 * itemIndex * Math.random()
        running: false
        repeat: false
        onTriggered: showAnim.start()
    }

    transform: [
        Rotation {
            id: rt
            origin.x: width * .5
            origin.y: height * .5
            axis {
                x: -.5
                y: 0
                z: 0
            }
            angle: 0
        },
        Translate {
            id: tr
        },
        Scale {
            id: sc
            origin.x: width * .5
            origin.y: height * .5
        }
    ]
    ParallelAnimation {
        id: showAnim
        running: false
        RotationAnimation {
            target: rt
            from: 90
            to: 0
            duration: 500
            easing.type: Easing.OutQuint
            property: "angle"
        }
        PropertyAnimation {
            target: workshopItem
            from: 0
            to: 1
            duration: 500
            easing.type: Easing.OutQuint
            property: "opacity"
        }
        PropertyAnimation {
            target: tr
            from: 80
            to: 0
            duration: 500
            easing.type: Easing.OutQuint
            property: "y"
        }
        PropertyAnimation {
            target: sc
            from: .8
            to: 1
            duration: 500
            easing.type: Easing.OutQuint
            properties: "xScale,yScale"
        }
    }

    Item {
        id: screenPlay
        anchors.centerIn: parent
        height: 180
        width: 320

        Image {
            id: mask
            source: "qrc:/assets/images/Window.svg"
            sourceSize: Qt.size(screenPlay.width, screenPlay.height)
            visible: false
            smooth: true
            asynchronous: true
            fillMode: Image.PreserveAspectFit
            antialiasing: true
        }

        Item {
            id: itemWrapper
            visible: false
            anchors {
                fill: parent
                margins: 5
            }

            ScreenPlayItemImage {
                id: screenPlayItemImage
                anchors.fill: parent
                sourceImage: workshopItem.imgUrl
            }

            LinearGradient {
                id: shadow
                height: 80
                opacity: 0
                cached: true
                anchors {
                    bottom: parent.bottom
                    right: parent.right
                    left: parent.left
                }
                start: Qt.point(0, 80)
                end: Qt.point(0, 0)
                gradient: Gradient {
                    GradientStop {
                        position: 0.0
                        color: "#CC000000"
                    }
                    GradientStop {
                        position: 1.0
                        color: "#00000000"
                    }
                }
            }
            Text {
                id: txtTitle
                text: workshopItem.name
                renderType: Text.NativeRendering
                opacity: 0
                height: 30
                width: 180
                verticalAlignment: Text.AlignVCenter
                color: "white"
                font.pixelSize: 18
                wrapMode: Text.WordWrap
                font.family: "Roboto"
                anchors {
                    bottom: parent.bottom
                    right: button.left
                    rightMargin: 10
                    left: parent.left
                    leftMargin: 20
                    bottomMargin: -50
                }
            }

            Button {
                id: button
                text: qsTr("Download")
                anchors.right: parent.right
                anchors.rightMargin: 20
                opacity: 0
                Material.background: Material.Orange
                Material.foreground: "white"
                anchors.bottom: parent.bottom
                anchors.bottomMargin: -50
                icon.source: "qrc:/assets/icons/icon_download.svg"
                icon.width: 12
                icon.height: 12
            }

            Item {
                id:openInWorkshop
                height: 20
                width: 20
                z: 99
                opacity: 0
                anchors {
                    margins: 10
                    top: parent.top
                    right: parent.right
                }
                Image {
                    source: "qrc:/assets/icons/icon_open_in_new.svg"
                    sourceSize: Qt.size(parent.width, parent.height)
                    fillMode: Image.PreserveAspectFit

                }
            }
        }

        OpacityMask {
            anchors.fill: itemWrapper
            antialiasing: true
            source: itemWrapper
            maskSource: mask

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onContainsMouseChanged: {
                    if (!isDownloading) {
                        if (containsMouse) {
                            workshopItem.state = "hover"
                        } else {
                            workshopItem.state = ""
                        }
                    }
                }
            }
            MouseArea {
                cursorShape: Qt.PointingHandCursor
                height: 50
                anchors {
                    right: parent.right
                    left: parent.left
                    bottom: parent.bottom
                }

                onClicked: {
                    isDownloading = true
                    workshopItem.state = "downloading"
                    steamWorkshop.subscribeItem(workshopItem.steamID)
                }

                Connections {
                    target: steamWorkshop
                    onWorkshopItemInstalled: {
                        print(appID)
                        if (appID === steamWorkshop.appID) {
                            workshopItem.state = "installed"
                            print("match!")
                        }

                        print(workshopItem.steamID, publishedFile)

                        if (workshopItem.steamID == publishedFile) {

                        }
                    }
                }
            }

            MouseArea {
                height: 20
                width: 20
                cursorShape: Qt.PointingHandCursor
                anchors {
                    margins: 10
                    top: parent.top
                    right: parent.right
                }
                onClicked: {
                    Qt.openUrlExternally("steam://url/CommunityFilePage/" + steamID)
                }
            }
        }
        FastBlur {
            id: effBlur
            anchors.fill: itemWrapper
            source: itemWrapper
            radius: 0
        }

        Item {
            id: itmDownloading
            opacity: 0
            anchors {
                top: parent.top
                topMargin: 50
                right: parent.right
                bottom: parent.bottom
                left: parent.left
            }

            Text {
                id: txtDownloading
                text: qsTr("Downloading Workshop content shortly!")
                color: "white"
                font.pixelSize: 21
                wrapMode: Text.WordWrap
                font.family: "Roboto"
                horizontalAlignment: Qt.AlignHCenter
                anchors {
                    verticalCenter: parent.verticalCenter
                    right: parent.right
                    rightMargin: 20
                    left: parent.left
                    leftMargin: 20
                }
            }

            ProgressBar {
                id: pbDownloading
                indeterminate: true
                anchors {
                    bottom: parent.bottom
                    bottomMargin: 20
                    horizontalCenter: parent.horizontalCenter
                }
            }
        }
    }

    states: [
        State {
            name: "hover"

            PropertyChanges {
                target: button
                opacity: 1
                anchors.bottomMargin: 10
            }


            PropertyChanges {
                target: openInWorkshop
                opacity: .75
            }

            PropertyChanges {
                target: txtTitle
                opacity: 1
                anchors.bottomMargin: 20
            }

            PropertyChanges {
                target: shadow
                opacity: 1
            }
            PropertyChanges {
                target: effBlur
                radius: 0
            }
        },
        State {
            name: "downloading"

            PropertyChanges {
                target: button
                opacity: 0
            }
            PropertyChanges {
                target: openInWorkshop
                opacity: 0
            }

            PropertyChanges {
                target: txtTitle
                opacity: 0
            }

            PropertyChanges {
                target: shadow
                opacity: 0
            }

            PropertyChanges {
                target: effBlur
                radius: 64
            }

            PropertyChanges {
                target: itmDownloading
                opacity: 1
                anchors.topMargin: 0
            }
        },
        State {
            name: "installed"

            PropertyChanges {
                target: button
                opacity: 0
            }

            PropertyChanges {
                target: txtTitle
                opacity: 0
            }

            PropertyChanges {
                target: shadow
                opacity: 0
            }

            PropertyChanges {
                target: effBlur
                radius: 64
            }

            PropertyChanges {
                target: itmDownloading
                opacity: 1
                anchors.topMargin: 0
            }
            PropertyChanges {
                target: txtDownloading
                text: qsTr("Download complete!")
            }
        }
    ]
    transitions: [
        Transition {
            from: ""
            to: "hover"
            reversible: true

            PropertyAnimation {
                target: button
                duration: 100
                properties: "opacity, anchors.bottomMargin"
            }
            PropertyAnimation {
                target: openInWorkshop
                duration: 100
                properties: "opacity"
            }
            PropertyAnimation {
                target: txtTitle
                duration: 100
                properties: "opacity, anchors.bottomMargin"
            }
            PropertyAnimation {
                target: shadow
                duration: 100
                properties: "opacity"
            }
        },
        Transition {
            from: "*"
            to: "downloading"
            reversible: true

            PropertyAnimation {
                target: button
                duration: 100
                properties: "opacity"
            }
            PropertyAnimation {
                target: txtTitle
                duration: 100
                properties: "opacity"
            }
            PropertyAnimation {
                target: shadow
                duration: 100
                properties: "opacity"
            }
            SequentialAnimation {
                PropertyAnimation {
                    target: effBlur
                    duration: 500
                    properties: "radius"
                }
                PropertyAnimation {
                    target: txtTitle
                    duration: 200
                    properties: "opacity, anchors.topMargin"
                }
            }
        }
    ]
}
