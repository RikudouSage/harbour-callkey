import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: content.height

        Column {
            id: content
            width: parent.width

            PageHeader {
                //% "Settings"
                title: qsTrId("settings.title")
            }

            TextSwitch {
                width: parent.width
                checked: appSettings.mprisEnabled
                //% "Media player integration (MPRIS)"
                text: qsTrId("settings.mpris")
                //% "Expose the Primary action to media controls while CallKey is running."
                description: qsTrId("settings.mpris_description")
                onCheckedChanged: appSettings.mprisEnabled = checked
            }
        }

        VerticalScrollDecorator { }
    }
}
