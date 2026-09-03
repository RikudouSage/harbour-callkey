import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {
    id: page

    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column

            width: page.width
            spacing: Theme.paddingLarge

            DialogHeader {
                //% "Settings"
                acceptText: qsTrId("settings")
                //% "Cancel"
                cancelText: qsTrId("cancel")
            }
        }
    }
}
