import QtQuick 2.0
import Sailfish.Silica 1.0

CoverBackground {
    id: cover

    property var coverAccounts: []
    property var coverCallers: []
    property var callsInProgress: []
    property string feedbackText: ""
    property color feedbackColor: Theme.primaryColor
    property string feedbackIconSource: "image://theme/icon-m-accept"
    property bool feedbackVisible: false

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
        refreshCoverCallers()
    }

    function refreshCoverCallers() {
        var callers = coverCallers

        for (var i = 0; i < coverAccounts.length; i++) {
            if (!callers[i]) {
                callers[i] = callerFactory.create()
                connectCallerSignals(callers[i])
            }

            configureCaller(callers[i], coverAccounts[i])
        }

        coverCallers = callers
    }

    function connectCallerSignals(caller) {
        caller.callSucceeded.connect(function() {
            cover.setCallInProgress(caller, false)
            cover.showSuccess()
        })
        caller.callFailed.connect(function(error) {
            cover.setCallInProgress(caller, false)
            cover.showError(error)
        })
    }

    function setCallInProgress(caller, inProgress) {
        var index = coverCallers.indexOf(caller)
        if (index < 0) {
            return
        }

        var states = callsInProgress.slice(0)
        states[index] = inProgress
        callsInProgress = states
    }

    function callInProgress(index) {
        return index >= 0 && index < callsInProgress.length
                && callsInProgress[index] === true
    }

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
        caller.ignoreTargetDeclineErrors = account.ignoreTargetDeclineErrors !== false
        caller.earlySuccessResponses = account.earlySuccessResponses === undefined
                                       ? ["2xx", "180"] : account.earlySuccessResponses
    }

    function callAccount(index) {
        if (index < 0 || index >= coverAccounts.length) {
            return
        }

        var caller = coverCallers[index]
        if (!caller) {
            return
        }

        if (callInProgress(index)) {
            return
        }

        setCallInProgress(caller, true)
        caller.placeCall()
    }

    function showSuccess() {
        errorTimer.stop()
        feedbackText = qsTr("Action executed")
        feedbackColor = Theme.highlightColor
        feedbackIconSource = "image://theme/icon-m-accept"
        feedbackVisible = true
        successTimer.restart()
    }

    function showError(error) {
        successTimer.stop()
        feedbackText = error && error.length > 0 ? error : qsTr("Action failed")
        feedbackColor = Theme.errorColor
        feedbackIconSource = "image://theme/icon-splus-error"
        feedbackVisible = true
        errorTimer.restart()
    }

    function clearFeedback() {
        feedbackVisible = false
        feedbackText = ""
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
        if (status === Cover.Active) {
            refreshCoverAccounts()
        }
    }

    Connections {
        target: accounts
        onAccountsChanged: cover.refreshCoverAccounts()
    }

    Timer {
        id: successTimer
        interval: 3000
        onTriggered: cover.clearFeedback()
    }

    Timer {
        id: errorTimer
        interval: 15000
        onTriggered: cover.clearFeedback()
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

    Row {
        anchors {
            left: parent.left
            leftMargin: Theme.paddingMedium
            right: parent.right
            rightMargin: Theme.paddingMedium
            verticalCenter: parent.verticalCenter
        }
        spacing: Theme.paddingSmall
        visible: cover.feedbackVisible

        Icon {
            source: cover.feedbackIconSource
            color: cover.feedbackColor
            anchors.verticalCenter: parent.verticalCenter
        }

        Label {
            width: parent.width - Theme.iconSizeMedium - Theme.paddingSmall
            color: cover.feedbackColor
            text: cover.feedbackText
            font.pixelSize: Theme.fontSizeMedium
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            truncationMode: TruncationMode.Fade
        }
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
        visible: cover.coverAccounts.length > 0 && !cover.feedbackVisible

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

                    Item {
                        width: Theme.iconSizeMedium
                        height: Theme.iconSizeMedium

                        Icon {
                            anchors.fill: parent
                            visible: !cover.callInProgress(index)
                            source: visible && modelData.coverIcon
                                    ? "image://theme/" + modelData.coverIcon : ""
                        }

                        BusyIndicator {
                            id: busyIndicator

                            anchors.fill: parent
                            running: cover.callInProgress(index)
                            visible: running
                            size: BusyIndicatorSize.Medium

                            RotationAnimation on rotation {
                                from: 0
                                to: 360
                                duration: 1000
                                loops: Animation.Infinite
                                running: busyIndicator.running && cover.status === Cover.Active
                            }
                        }
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
            onTriggered: cover.callAccount(0)
        }
    }

    CoverActionList {
        enabled: cover.coverAccounts.length === 2

        CoverAction {
            iconSource: cover.coverIconSource(0)
            onTriggered: cover.callAccount(0)
        }

        CoverAction {
            iconSource: cover.coverIconSource(1)
            onTriggered: cover.callAccount(1)
        }
    }

    CoverActionList {
        enabled: cover.coverAccounts.length === 3

        CoverAction {
            iconSource: cover.coverIconSource(0)
            onTriggered: cover.callAccount(0)
        }

        CoverAction {
            iconSource: cover.coverIconSource(1)
            onTriggered: cover.callAccount(1)
        }

        CoverAction {
            iconSource: cover.coverIconSource(2)
            onTriggered: cover.callAccount(2)
        }
    }

}
