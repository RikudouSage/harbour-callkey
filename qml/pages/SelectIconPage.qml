import QtQuick 2.0
import Sailfish.Silica 1.0

import "../js/themeIcons.js" as ThemeIconFallback

Page {
    id: page

    allowedOrientations: Orientation.All
    property var icons: themeIcons.icons.length > 0 ? themeIcons.icons : ThemeIconFallback.icons
    property string filterText
    property string selectedIcon
    signal iconSelected(string icon)

    function iconMatches(icon) {
        return filterText.length === 0 || icon.toLowerCase().indexOf(filterText.toLowerCase()) !== -1
    }

    SilicaListView {
        id: list

        anchors.fill: parent
        model: page.icons

        header: Column {
            width: list.width

            PageHeader {
                //% "Choose icon"
                title: qsTrId("icon_select.title")
            }

            SearchField {
                width: parent.width
                //% "Search icons"
                placeholderText: qsTrId("icon_select.search")
                onTextChanged: page.filterText = text
            }
        }

        VerticalScrollDecorator {}

        delegate: ListItem {
            visible: page.iconMatches(modelData)
            width: list.width
            contentHeight: visible ? Theme.itemSizeMedium : 0

            onClicked: {
                page.iconSelected(modelData)
                pageStack.pop()
            }

            Icon {
                id: iconPreview

                source: "image://theme/" + modelData
                anchors.left: parent.left
                anchors.leftMargin: Theme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter
                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium
            }

            Label {
                anchors.left: iconPreview.right
                anchors.leftMargin: Theme.paddingMedium
                anchors.right: selectedIndicator.left
                anchors.rightMargin: Theme.paddingMedium
                anchors.verticalCenter: parent.verticalCenter
                text: modelData
                truncationMode: TruncationMode.Fade
                color: parent.highlighted ? Theme.highlightColor : Theme.primaryColor
            }

            Icon {
                id: selectedIndicator

                visible: modelData === page.selectedIcon
                source: "image://theme/icon-m-accept"
                anchors.right: parent.right
                anchors.rightMargin: Theme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter
                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium
            }
        }
    }
}
