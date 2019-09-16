import QtQuick 2.12
import QtQuick.Controls 2.3
import QtGraphicalEffects 1.0
import QtQuick.Controls.Material 2.2
import QtQuick.Layouts 1.3

import "../Common/" as SP

ColumnLayout {
    id: root
    spacing: 20
    state: "hidden"
    clip: true
    anchors.rightMargin: 10
    anchors.leftMargin: 10

    SP.Slider {
        headline: qsTr("Volume")
        onValueChanged: screenPlay.setWallpaperValue(
                            activeMonitorIndex, "volume", value)
        Layout.fillWidth: true
    }
    SP.Slider {
        headline: qsTr("Playback rate")
        onValueChanged: screenPlay.setWallpaperValue(
                            activeMonitorIndex, "playbackRate", value)
        Layout.fillWidth: true
    }
    SP.Slider {
        headline: qsTr("Current Video Time")
        onValueChanged: screenPlay.setWallpaperValue(
                            activeMonitorIndex, "currentTime", value)
        Layout.fillWidth: true
    }
    ColumnLayout {
        height: 50
        Layout.fillWidth: true
        spacing: 5
        
        Text {
            id: txtComboBoxFillMode
            text: qsTr("Fill Mode")
            font.family: "Roboto"
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 10
            color: "#626262"
            wrapMode: Text.WrapAnywhere
            Layout.fillWidth: true
        }
        ComboBox {
            id: settingsComboBox
            Layout.fillWidth: true
            onActivated: {
                screenPlay.setWallpaperValue(
                            activeMonitorIndex, "fillmode",
                            settingsComboBox.currentText)
            }
            
            model: ListModel {
                ListElement {
                    text: "Stretch"
                }
                ListElement {
                    text: "Fill"
                }
                ListElement {
                    text: "Contain"
                }
                ListElement {
                    text: "Cover"
                }
                ListElement {
                    text: "Scale-Down"
                }
            }
        }
    }

    states: [
        State {
            name: "visible"
            PropertyChanges {
                target: root
                opacity: 1
                anchors.topMargin: 60
            }
        },
        State {
            name: "hidden"
            PropertyChanges {
                target: root
                opacity: 0
                anchors.topMargin: -50
            }
        }
    ]

    transitions: [
        Transition {
            from: "visible"
            to: "hidden"
            reversible: true
            PropertyAnimation {
                target: root
                duration: 300
                easing.type: Easing.InOutQuart
                properties: "anchors.topMargin, opacity"
            }
        }
    ]

}