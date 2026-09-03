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

    function configurePasswordCaller(caller, account) {
        caller.sipServer = account.sipServer || ""
        caller.sipUsername = account.sipUsername || ""
        caller.sipServerPort = account.sipServerPort || 0
        caller.target = account.target || ""
    }

    function accountKeyChanged(originalAccount, updatedAccount) {
        return Object.keys(originalAccount).length > 0
                && accountKey(originalAccount) !== accountKey(updatedAccount)
    }

    function savePassword(originalAccount, updatedAccount) {
        var newCaller = callerFactory.create()
        configurePasswordCaller(newCaller, updatedAccount)

        if (accountKeyChanged(originalAccount, updatedAccount)) {
            var originalCaller = callerFactory.create()
            configurePasswordCaller(originalCaller, originalAccount)
            originalCaller.removePassword()
        }

        if (updatedAccount.sipPassword) {
            newCaller.sipPassword = updatedAccount.sipPassword
        } else {
            newCaller.removePassword()
        }
    }

    function coverIconSource(account) {
        return hasCoverIcon(account) ? "image://theme/" + account.coverIcon : ""
    }

    function openAccountDialog(account) {
        var dialog = pageStack.push(Qt.resolvedUrl("AccountEditPage.qml"), {
            account: account || {},
            coverActionCount: coverActionCount(account)
        })
        var accountStore = accounts
        dialog.accepted.connect(function() {
            var originalAccount = dialog.account
            var updatedAccount = dialog.updatedAccount
            var keyChanged = page.accountKeyChanged(originalAccount, updatedAccount)

            page.savePassword(originalAccount, updatedAccount)
            if (keyChanged) {
                accountStore.removeAccount(originalAccount)
            }
            accountStore.storeAccount(updatedAccount)
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
                        var accountStore = accounts
                        remorseDelete(function() {
                            accountStore.removeAccount(account)
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
