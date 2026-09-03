import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    allowedOrientations: Orientation.All

    function refreshAccounts() {
        list.model = null
        list.model = accounts.accounts
    }

    function accountKey(account) {
        return account.sipUsername + "@" + account.sipServer + ":" + account.sipServerPort + "|" + account.target
    }

    function coverActionCount(excludedAccount) {
        var accountList = accounts.accounts
        var excludedKey = excludedAccount ? accountKey(excludedAccount) : ""
        var count = 0

        for (var i = 0; i < accountList.length; i++) {
            if (accountList[i].coverAction === true && accountKey(accountList[i]) !== excludedKey) {
                count++
            }
        }

        return count
    }

    function hasCoverIcon(account) {
        return account.coverIcon ? account.coverIcon.length > 0 : false
    }

    function coverIconSource(account) {
        return hasCoverIcon(account) ? "image://theme/" + account.coverIcon : ""
    }

    function openAccountDialog(account) {
        var dialog = pageStack.push(Qt.resolvedUrl("AccountEditPage.qml"), {
            account: account || {},
            coverActionCount: coverActionCount(account)
        })
        dialog.accepted.connect(function() {
            accounts.storeAccount(dialog.account)
            page.refreshAccounts()
        })
    }

    SilicaListView {
        id: list

        anchors.fill: parent
        model: accounts.accounts

        header: Column {
            width: list.width

            PageHeader {
                //% "Actions"
                title: qsTrId("accounts")
            }

            Label {
                visible: list.count === 0
                x: Theme.horizontalPageMargin
                width: parent.width - Theme.horizontalPageMargin * 2
                //% "There are no actions configured. Pull down to add one."
                text: qsTrId("config.no_accounts")
                wrapMode: Text.Wrap
            }
        }

        PullDownMenu {
            MenuItem {
                //% "Add action"
                text: qsTrId("config.add_account")
                onClicked: page.openAccountDialog()
            }
        }

        delegate: ListItem {
            id: listItem

            width: list.width
            contentHeight: Theme.itemSizeMedium
            menu: ContextMenu {
                IconMenuItem {
                    //% "Delete action"
                    text: qsTrId("config.remove_account")
                    icon.source: "image://theme/icon-m-remove"
                    onClicked: {
                        var account = modelData
                        remorseDelete(function() {
                            accounts.removeAccount(account)
                            page.refreshAccounts()
                        })
                    }
                }
            }
            onClicked: page.openAccountDialog(modelData)

            Label {
                anchors.left: iconPreview.visible ? iconPreview.right : parent.left
                anchors.leftMargin: iconPreview.visible ? Theme.paddingMedium : Theme.horizontalPageMargin
                anchors.right: parent.right
                anchors.rightMargin: Theme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter
                text: modelData.name
                truncationMode: TruncationMode.Fade
                color: parent.highlighted ? Theme.highlightColor : Theme.primaryColor
            }

            Icon {
                id: iconPreview

                visible: page.hasCoverIcon(modelData)
                source: page.coverIconSource(modelData)
                anchors.left: parent.left
                anchors.leftMargin: Theme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter
                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium
            }
        }
    }
}
