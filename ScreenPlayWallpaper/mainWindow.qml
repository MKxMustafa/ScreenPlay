import QtQuick 2.12
import ScreenPlay.Wallpaper 1.0

Rectangle {
    id: root
    anchors.fill: parent
    color: {
        if (desktopProperties.color === null) {
            return "black"
        } else {
            return desktopProperties.color
        }
    }

    property bool canFadeByWallpaperFillMode: true

    Connections {
        target: window

        onQmlExit: {
            if (canFadeByWallpaperFillMode && window.canFade) {
                imgCover.state = "outExit"
            } else {
                window.terminate()
            }
        }

        onQmlSceneValueReceived: {
            var obj2 = 'import QtQuick 2.0; Item {Component.onCompleted: loader.item.'
                    + key + ' = ' + value + '; }'
            print(key, value)
            var newObject = Qt.createQmlObject(obj2.toString(), root, "err")
            newObject.destroy(10000)
        }
    }

    Component.onCompleted: {
        switch (window.type) {
        case Wallpaper.WallpaperType.Video:
            loader.source = "qrc:/WebView.qml"

            break
        case Wallpaper.WallpaperType.Html:
            loader.source = "qrc:/WebView.qml"
            loader.webViewUrl = Qt.resolvedUrl(window.fullContentPath)
            break
        case Wallpaper.WallpaperType.ThreeJSScene:
            loader.source = "qrc:/WebView.qml"
            loader.webViewUrl = Qt.resolvedUrl(window.fullContentPath)
            break
        case Wallpaper.WallpaperType.Qml:
            loader.source = Qt.resolvedUrl(window.fullContentPath)
            imgCover.state = "out"
            break
        }
    }

    function fadeIn() {
        print("fadeIn()")
        window.setVisible(true)
        print("setVisible()")
        if (canFadeByWallpaperFillMode && window.canFade) {
            print("fadein")
            imgCover.state = "out"
        } else {
            print("imgCover.opacity = 0")
            imgCover.opacity = 0
        }
    }

    Loader {
        id: loader
        anchors.fill: parent
        enabled: false
        property string webViewUrl
        onStatusChanged: {
            if (loader.status === Loader.Ready) {
                if (window.type === Wallpaper.WallpaperType.Video) {
                    loader.item.url = loader.webViewUrl
                }
            }
        }

        Connections {
            ignoreUnknownSignals: true
            target: loader.item
            onRequestFadeIn:fadeIn()
        }
    }

    Image {
        id: imgCover
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        state: "in"

        sourceSize.width: window.width
        sourceSize.height: window.height
        source: Qt.resolvedUrl("file:///" + desktopProperties.wallpaperPath)

        states: [
            State {
                name: "in"
                PropertyChanges {
                    target: imgCover
                    opacity: 1
                }
            },
            State {
                name: "out"
                PropertyChanges {
                    target: imgCover
                    opacity: 0
                }
            },
            State {
                name: "outExit"
                PropertyChanges {
                    target: imgCover
                    opacity: 1
                }
            }
        ]
        transitions: [
            Transition {
                from: "out"
                to: "in"
                reversible: true
                PropertyAnimation {
                    target: imgCover
                    duration: 600
                    property: "opacity"
                }
            },
            Transition {
                from: "out"
                to: "outExit"
                reversible: true
                SequentialAnimation {
                    PropertyAnimation {
                        target: imgCover
                        duration: 600
                        property: "opacity"
                    }
                    ScriptAction {
                        script: window.terminate()
                    }
                }
            }
        ]

        Component.onCompleted: {

            switch (desktopProperties.wallpaperStyle) {
            case 10:
                imgCover.fillMode = Image.PreserveAspectCrop
                break
            case 6:
                imgCover.fillMode = Image.PreserveAspectFit
                break
            case 2:
                break
            case 0:
                if (desktopProperties.isTiled) {
                    // Tiled
                    imgCover.fillMode = Image.Tile
                } else {
                    // Center
                    imgCover.fillMode = Image.PreserveAspectFit
                    imgCover.anchors.centerIn = parent
                    imgCover.width = sourceSize.width
                    imgCover.height = sourceSize.height
                }
                break
            case 22:
                canFadeByWallpaperFillMode = false
                break
            }
        }
    }
}
