import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3

import "Components"

ApplicationWindow {
    id: window
    color: "#eeeeee"
    visible: true
    width: 1380
    minimumHeight: 768
    minimumWidth: 1050


    Component.onCompleted: {
        setX(Screen.width / 2 - width / 2);
        setY(Screen.height / 2 - height / 2);
    }

    Loader {
        id: pageLoader
        source : "qrc:/qml/Components/Installed.qml"
        anchors {
            top: nav.bottom
            right: parent.right
            bottom: parent.bottom
            left: parent.left
        }
        onStateChanged: {
            if(pageLoader.state === Loader.Loading){
                loaderText.visible = true
            } else if(pageLoader.state === Loader.Ready){
                loaderText.visible = false
            }
        }

        Text {
            id: loaderText
            visible: true
            text: qsTr("Loading...")
            anchors.centerIn: parent
            font.pixelSize: 32

            font.family: font_LibreBaskerville_Italic.name
            font.pointSize: 32
            font.italic: true
            color: "#818181"
            FontLoader{
                id: font_LibreBaskerville_Italic
                source: "qrc:/assets/fonts/LibreBaskerville-Italic.ttf"
            }
        }



//        onSourceChanged: pageLoaderAnim.running = true

//        SequentialAnimation {
//            id:pageLoaderAnim
//            running: true
//            NumberAnimation { target: pageLoader.item; property: "opacity"; from:0; to: 1; duration: 500 }
//            NumberAnimation { target: pageLoader.item; property: "y"; from: -100; to: 0; duration: 300 }
//        }

        Connections{
            target: pageLoader.item
            ignoreUnknownSignals: true
            onSetSidebaractiveItem: {
                if( sidebar.activeScreen == screenId && sidebar.state ==  "active"){
                    sidebar.state =  "inactive"
                } else {
                    sidebar.state =  "active"
                }

                sidebar.activeScreen = screenId

            }
        }
    }

    Sidebar {
        id: sidebar
        width:400
        anchors {
            top:nav.bottom
            right:parent.right
            bottom:parent.bottom
        }
    }

    Navigation {
        id: nav
        anchors {
            top: parent.top
            right: parent.right
            left: parent.left
        }
        onChangePage: {
            pageLoader.setSource("qrc:/qml/Components/"+name+".qml")
            sidebar.state = "inactive"
        }

        onToggleMonitors: {
            monitors.state = monitors.state == "active" ? "inactive" : "active"
        }

    }

    Monitors {
        id: monitors
        state: "inactive"
        anchors.fill: pageLoader
        z:98
    }

    FileDropper {
        anchors.fill: parent
        z:99
    }

}
