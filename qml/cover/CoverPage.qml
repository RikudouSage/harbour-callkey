import QtQuick 2.0
import Sailfish.Silica 1.0

CoverBackground {
    id: cover

    property var coverAccounts: []

    function refreshCoverAccounts() {
        var accountList = accounts.accounts
        var enabledAccounts = []

        for (var i = 0; i < accountList.length && enabledAccounts.length < 3; i++) {
            if (accountList[i].coverAction === true
                    && accountList[i].coverIcon
                    && accountList[i].coverIcon.length > 0) {
                enabledAccounts.push(accountList[i])
            }
        }

        coverAccounts = enabledAccounts
    }

    function callAccount(account) {
        if (!account) {
            return
        }

        var caller = callerFactory.create()
        caller.sipServer = account.sipServer || ""
        caller.sipUsername = account.sipUsername || ""
        caller.sipServerPort = account.sipServerPort || 0
        caller.sipTransport = account.sipTransport || "udp"
        caller.advertisedHost = account.advertisedHost || ""
        caller.localAddress = account.localAddress || ""
        caller.localPort = account.localPort || 0
        caller.nat = account.nat !== false
        caller.target = account.target || ""
        caller.timeoutMs = account.timeoutMs || 0
        caller.placeCall()
    }

    function accountAt(index) {
        return index >= 0 && index < coverAccounts.length ? coverAccounts[index] : null
    }

    function coverIconSource(index) {
        var account = accountAt(index)
        return account && account.coverIcon ? "image://theme/" + account.coverIcon : ""
    }

    Component.onCompleted: refreshCoverAccounts()
    onStatusChanged: {
        if (status === PageStatus.Active) {
            refreshCoverAccounts()
        }
    }

    Connections {
        target: accounts
        onAccountsChanged: cover.refreshCoverAccounts()
    }

    Label {
        anchors.centerIn: parent
        width: parent.width - Theme.paddingLarge * 2
        horizontalAlignment: Text.AlignHCenter
        visible: cover.coverAccounts.length === 0
        //: The name of the application, you might not need to translate this, but it's up to you
        //% "CallKey"
        text: qsTrId("app_name")
        font.pixelSize: Theme.fontSizeLarge
        truncationMode: TruncationMode.Fade
    }

    Column {
        anchors {
            left: parent.left
            leftMargin: Theme.paddingMedium
            right: parent.right
            rightMargin: Theme.paddingMedium
            verticalCenter: parent.verticalCenter
        }
        spacing: Theme.paddingSmall
        visible: cover.coverAccounts.length > 0

        Repeater {
            model: cover.coverAccounts

            Item {
                width: parent.width
                height: Theme.iconSizeMedium

                Row {
                    anchors.centerIn: parent
                    width: Math.min(parent.width, Theme.iconSizeMedium + Theme.paddingSmall + legendLabel.implicitWidth)
                    height: Theme.iconSizeMedium
                    spacing: Theme.paddingSmall

                    Icon {
                        source: modelData.coverIcon ? "image://theme/" + modelData.coverIcon : ""
                        width: Theme.iconSizeMedium
                        height: Theme.iconSizeMedium
                    }

                    Label {
                        id: legendLabel

                        width: Math.min(implicitWidth, parent.parent.width - Theme.iconSizeMedium - Theme.paddingSmall)
                        anchors.verticalCenter: parent.verticalCenter
                        text: "= " + modelData.name
                        font.pixelSize: Theme.fontSizeMedium
                        truncationMode: TruncationMode.Fade
                    }
                }
            }
        }
    }

    CoverActionList {
        enabled: cover.coverAccounts.length === 1

        CoverAction {
            iconSource: cover.coverIconSource(0)
            onTriggered: cover.callAccount(cover.accountAt(0))
        }
    }

    CoverActionList {
        enabled: cover.coverAccounts.length === 2

        CoverAction {
            iconSource: cover.coverIconSource(0)
            onTriggered: cover.callAccount(cover.accountAt(0))
        }

        CoverAction {
            iconSource: cover.coverIconSource(1)
            onTriggered: cover.callAccount(cover.accountAt(1))
        }
    }

    CoverActionList {
        enabled: cover.coverAccounts.length === 3

        CoverAction {
            iconSource: cover.coverIconSource(0)
            onTriggered: cover.callAccount(cover.accountAt(0))
        }

        CoverAction {
            iconSource: cover.coverIconSource(1)
            onTriggered: cover.callAccount(cover.accountAt(1))
        }

        CoverAction {
            iconSource: cover.coverIconSource(2)
            onTriggered: cover.callAccount(cover.accountAt(2))
        }
    }

}
