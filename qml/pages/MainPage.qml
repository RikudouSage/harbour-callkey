import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    allowedOrientations: Orientation.All

    property var callers: []
    property var accountModel: []
    property string errorText: ""

    function configureCaller(caller, account) {
        if (!caller || !account) {
            return
        }

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
    }

    function callAccount(index) {
        if (index < 0 || index >= callers.length || !callers[index]) {
            return
        }

        callers[index].placeCall()
    }

    function refreshAccounts() {
        accountModel = []
        callers = []
        errorText = ""
        accountModel = accounts.accounts
    }

    function subtitleFor(account) {
        var target = account.target || ""
        var server = account.sipServer || ""

        if (target && server) {
            return target + " via " + server
        }

        return target || server
    }

    SilicaListView {
        id: actionList

        anchors.fill: parent
        model: page.accountModel

        PullDownMenu {
            MenuItem {
                //% "Actions"
                text: qsTrId("accounts")
                onClicked: pageStack.animatorPush("ConfigPage.qml")
            }
        }

        header: Column {
            width: actionList.width

            PageHeader {
                //: The name of the application, you might not need to translate this, but it's up to you
                //% "CallKey"
                title: qsTrId("app_name")
            }

            Label {
                visible: page.errorText.length > 0
                x: Theme.horizontalPageMargin
                width: parent.width - Theme.horizontalPageMargin * 2
                text: page.errorText
                color: Theme.errorColor
                wrapMode: Text.Wrap
            }
        }

        ViewPlaceholder {
            enabled: actionList.count === 0
            //% "There are no actions configured. Pull down to add one."
            text: qsTrId("main.no_accounts")
        }

        delegate: ListItem {
            id: actionItem

            width: actionList.width
            contentHeight: Math.max(Theme.iconSizeMedium, actionName.implicitHeight
                                    + actionSubtitle.implicitHeight + actionCoverHint.implicitHeight
                                    + Theme.paddingSmall * 2)

            property var caller
            property bool callInProgress: false
            property color resultColor: Theme.primaryColor

            Component.onCompleted: {
                caller = callerFactory.create()
                page.configureCaller(caller, modelData)
                page.callers[index] = caller
                caller.callSucceeded.connect(function() {
                    actionItem.callInProgress = false
                    actionItem.resultColor = Theme.highlightColor
                    successTimer.restart()
                    errorTimer.stop()
                    page.errorText = ""
                })
                caller.callFailed.connect(function(error) {
                    actionItem.callInProgress = false
                    actionItem.resultColor = Theme.errorColor
                    successTimer.stop()
                    errorTimer.restart()
                    page.errorText = error && error.length > 0
                                     ? error : qsTrId("main.action_failed")
                })
            }

            onClicked: {
                if (!callInProgress) {
                    callInProgress = true
                    resultColor = Theme.primaryColor
                    page.errorText = ""
                    page.callAccount(index)
                }
            }

            Timer {
                id: successTimer
                interval: 5000
                onTriggered: actionItem.resultColor = Theme.primaryColor
            }

            Timer {
                id: errorTimer
                interval: 15000
                onTriggered: {
                    actionItem.resultColor = Theme.primaryColor
                    page.errorText = ""
                }
            }

            Item {
                id: actionIconSlot

                width: Theme.iconSizeMedium
                height: Theme.iconSizeMedium
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }

                Icon {
                    id: actionIcon

                    anchors.fill: parent
                    visible: !actionItem.callInProgress && !!modelData.coverIcon
                    source: visible ? "image://theme/" + modelData.coverIcon : ""
                    color: actionItem.resultColor
                }

                BusyIndicator {
                    anchors.fill: parent
                    running: actionItem.callInProgress
                    visible: running
                    size: BusyIndicatorSize.Medium
                    color: actionItem.resultColor
                }
            }

            Label {
                id: actionName

                x: actionIconSlot.x + actionIconSlot.width + Theme.paddingMedium
                y: Theme.paddingMedium
                width: parent.width - x - Theme.horizontalPageMargin
                text: modelData.name || qsTrId("app_name")
                font.pixelSize: Theme.fontSizeLarge
                color: actionItem.resultColor
                truncationMode: TruncationMode.Fade
            }

            Label {
                id: actionSubtitle

                x: actionName.x
                y: actionName.y + actionName.height
                width: actionName.width
                text: page.subtitleFor(modelData)
                color: actionItem.resultColor
                font.pixelSize: Theme.fontSizeSmall
                truncationMode: TruncationMode.Fade
            }

            Label {
                id: actionCoverHint

                visible: modelData.coverAction === true && !!modelData.coverIcon
                x: actionName.x
                y: actionSubtitle.y + actionSubtitle.height
                width: actionName.width
                //% "Available on cover"
                text: qsTrId("main.available_cover")
                color: actionItem.resultColor
                font.pixelSize: Theme.fontSizeExtraSmall
                truncationMode: TruncationMode.Fade
            }
        }

        VerticalScrollDecorator { }
    }

    Component.onCompleted: page.refreshAccounts()
    onStatusChanged: {
        if (status === PageStatus.Active) {
            page.refreshAccounts()
        }
    }

    Connections {
        target: accounts
        onAccountsChanged: page.refreshAccounts()
    }
}
