import QtQuick 2.12

Item {
    id: workshopLoader

    property bool available: true

    Loader {
        id: wsLoader
        source: "qrc:/qml/Workshop/WorkshopAvailableTest.qml"
        onStatusChanged: {
            if (status == Loader.Ready) {
                available = true
            } else if (status == Loader.Error) {
                available = false
            }
        }
    }
}
