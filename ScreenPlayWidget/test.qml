import QtQuick 2.12

Rectangle {
    anchors.fill: parent
    color: "#80000000"

    Text {
        id: name
        text: qsTr("This is a empty test widget. You can change the source in test.qml")
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap
        anchors.fill: parent
        anchors.margins: 10
        color: "white"
    }

}



/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
 ##^##*/
