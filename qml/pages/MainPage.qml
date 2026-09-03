import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        PullDownMenu {
            MenuItem {
                //% "Settings"
                text: qsTrId("settings")
                onClicked: pageStack.animatorPush("ConfigPage.qml")
            }
        }

        Column {
            id: column

            width: page.width
            spacing: Theme.paddingLarge

            PageHeader {
                //: The name of the application, you might not need to translate this, but it's up to you
                //% "CallKey"
                title: qsTrId("app_name")
            }

            Label {
                visible: accounts.accounts.length === 0
                x: Theme.horizontalPageMargin
                width: parent.width - Theme.horizontalPageMargin * 2
                //% "There are no actions configured. Please use the pull down menu to configure actions."
                text: qsTrId("main.no_accounts")
                wrapMode: Text.Wrap
            }
        }
    }
}
