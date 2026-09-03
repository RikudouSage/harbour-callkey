import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {
    id: page

    allowedOrientations: Orientation.All
    property var account: ({})
    property bool existingAccount: false
    property var firstAdvancedField: null
    property int coverActionCount: 0

    Component.onCompleted: {
        existingAccount = Object.keys(account).length > 0
        if (!existingAccount) {
            setAccountValue("sipServerPort", 5060)
            setAccountValue("sipTransport", "udp")
            setAccountValue("nat", true)
        }
    }

    function setAccountValue(name, value) {
        var updatedAccount = account || {}
        updatedAccount[name] = value
        account = updatedAccount
    }

    function advancedExpanded() {
        return advancedSections.currentIndex >= 0
    }

    function acceptOrFocusAdvanced() {
        if (advancedExpanded() && firstAdvancedField) {
            firstAdvancedField.focus = true
            return
        }

        page.accept()
    }

    function coverIcon() {
        return account.coverIcon || ""
    }

    function hasCoverIcon() {
        return coverIcon().length > 0
    }

    function coverIconSource() {
        return hasCoverIcon() ? "image://theme/" + coverIcon() : ""
    }

    function openIconSelector() {
        var selector = pageStack.push(Qt.resolvedUrl("SelectIconPage.qml"), {
            selectedIcon: coverIcon()
        })
        selector.iconSelected.connect(function(icon) {
            page.setAccountValue("coverIcon", icon)
        })
    }

    canAccept: name.text.length > 0
               && sipUsername.text.length > 0
               && sipServer.text.length > 0
               && sipServerPort.text.length > 0
               && target.text.length > 0
               && (!account.coverAction || hasCoverIcon())

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column

            width: page.width
            spacing: Theme.paddingLarge

            DialogHeader {
                acceptText: existingAccount
                    //% "Save action"
                    ? qsTrId("account.save")
                    //% "Add action"
                    : qsTrId("account.add")
                //% "Cancel"
                cancelText: qsTrId("cancel")
            }

            TextField {
                id: name
                width: parent.width
                text: page.account.name || ""
                //% "Name"
                label: qsTrId("account.name")
                //% "Used only to identify the action in the app."
                description: qsTrId("account.name_description")
                placeholderText: label
                inputMethodHints: Qt.ImhNoPredictiveText
                onTextChanged: page.setAccountValue("name", text)
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: sipUsername.focus = true
            }

            TextField {
                id: sipUsername
                width: parent.width
                text: page.account.sipUsername || ""
                //% "Username"
                label: qsTrId("account.username")
                placeholderText: label
                inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
                onTextChanged: page.setAccountValue("sipUsername", text)
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: sipPassword.focus = true
            }

            TextField {
                property bool passwordVisible: false

                id: sipPassword
                width: parent.width
                text: page.account.sipPassword || ""
                //% "Password"
                label: qsTrId("account.password")
                placeholderText: label
                echoMode: passwordVisible ? TextInput.Normal : TextInput.Password
                inputMethodHints: Qt.ImhSensitiveData | Qt.ImhNoPredictiveText
                rightItem: IconButton {
                    icon.source: !sipPassword.passwordVisible
                                 ? "image://theme/icon-splus-hide-password"
                                 : "image://theme/icon-splus-show-password"
                    onClicked: sipPassword.passwordVisible = !sipPassword.passwordVisible
                }
                onTextChanged: page.setAccountValue("sipPassword", text)
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: sipServer.focus = true
            }

            TextField {
                id: sipServer
                width: parent.width
                text: page.account.sipServer || ""
                //% "Server"
                label: qsTrId("account.server")
                placeholderText: label
                inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
                onTextChanged: page.setAccountValue("sipServer", text)
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: sipServerPort.focus = true
            }

            TextField {
                id: sipServerPort
                width: parent.width
                text: page.account.sipServerPort === undefined
                      ? "5060"
                      : page.account.sipServerPort > 0 ? page.account.sipServerPort.toString() : ""
                //% "Port"
                label: qsTrId("account.port")
                //% "Defaults to 5060."
                description: qsTrId("account.port_description")
                placeholderText: label
                inputMethodHints: Qt.ImhDigitsOnly
                validator: IntValidator {
                    bottom: 1
                    top: 65535
                }
                onTextChanged: page.setAccountValue("sipServerPort", text.length > 0 ? parseInt(text, 10) : 0)
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: target.focus = true
            }

            ComboBox {
                id: sipTransport
                width: parent.width
                //% "Transport"
                label: qsTrId("account.transport")
                currentIndex: {
                    switch (page.account.sipTransport) {
                    case "tcp":
                        return 1
                    case "tls":
                        return 2
                    default:
                        return 0
                    }
                }
                menu: ContextMenu {
                    MenuItem {
                        text: "UDP"
                    }
                    MenuItem {
                        text: "TCP"
                    }
                    MenuItem {
                        text: "TLS"
                    }
                }
                //% "Defaults to UDP."
                description: qsTrId("account.transport_description")
                onCurrentIndexChanged: page.setAccountValue("sipTransport", ["udp", "tcp", "tls"][currentIndex])
            }

            TextField {
                id: target
                width: parent.width
                text: page.account.target || ""
                //% "Target"
                label: qsTrId("account.target")
                placeholderText: label
                inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
                onTextChanged: page.setAccountValue("target", text)
                EnterKey.iconSource: page.advancedExpanded()
                                     ? "image://theme/icon-m-enter-next"
                                     : "image://theme/icon-m-enter-accept"
                EnterKey.onClicked: page.acceptOrFocusAdvanced()
            }

            ExpandingSectionGroup {
                id: advancedSections

                width: parent.width
                currentIndex: page.account.coverAction ? 0 : -1

                ExpandingSection {
                    //% "Cover settings"
                    title: qsTrId("account.cover_settings")

                    content.sourceComponent: Column {
                        TextSwitch {
                            id: coverAction
                            width: parent.width
                            checked: page.account.coverAction || false
                            enabled: checked || page.coverActionCount < 3
                            //% "Show as cover action"
                            text: qsTrId("account.cover_action")
                            description: enabled
                                //% "Makes this action available from the app cover."
                                ? qsTrId("account.cover_action_description")
                                //% "Disable another cover action first."
                                : qsTrId("account.cover_action_limit_description")
                            onCheckedChanged: {
                                page.setAccountValue("coverAction", checked)
                                if (!checked) {
                                    page.setAccountValue("coverIcon", "")
                                }
                            }
                        }

                        BackgroundItem {
                            id: coverIconItem

                            width: parent.width
                            height: Theme.itemSizeMedium
                            enabled: coverAction.enabled && coverAction.checked
                            opacity: enabled ? 1.0 : Theme.opacityLow
                            onClicked: page.openIconSelector()

                            Icon {
                                id: coverIconPreview

                                visible: page.hasCoverIcon()
                                source: page.coverIconSource()
                                anchors.left: parent.left
                                anchors.leftMargin: Theme.horizontalPageMargin
                                anchors.verticalCenter: parent.verticalCenter
                                width: Theme.iconSizeMedium
                                height: Theme.iconSizeMedium
                            }

                            Column {
                                anchors.left: coverIconPreview.visible ? coverIconPreview.right : parent.left
                                anchors.leftMargin: coverIconPreview.visible ? Theme.paddingMedium : Theme.horizontalPageMargin
                                anchors.right: parent.right
                                anchors.rightMargin: Theme.horizontalPageMargin
                                anchors.verticalCenter: parent.verticalCenter

                                Label {
                                    width: parent.width
                                    //% "Cover icon"
                                    text: qsTrId("account.cover_icon")
                                    color: coverIconItem.highlighted ? Theme.highlightColor : Theme.primaryColor
                                    truncationMode: TruncationMode.Fade
                                }

                                Label {
                                    width: parent.width
                                    text: page.hasCoverIcon()
                                          ? page.coverIcon()
                                          //% "No icon selected"
                                          : qsTrId("account.cover_icon_none")
                                    color: coverIconItem.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                                    font.pixelSize: Theme.fontSizeSmall
                                    truncationMode: TruncationMode.Fade
                                }
                            }
                        }
                    }
                }

                ExpandingSection {
                    //% "Advanced"
                    title: qsTrId("account.advanced")

                    content.sourceComponent: Column {
                        TextField {
                            id: advertisedHost
                            width: parent.width
                            text: page.account.advertisedHost || ""
                            //% "Advertised host"
                            label: qsTrId("account.advertised_host")
                            //% "Detected automatically from the SIP server and port when empty."
                            description: qsTrId("account.advertised_host_description")
                            placeholderText: label
                            inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
                            onTextChanged: page.setAccountValue("advertisedHost", text)
                            Component.onCompleted: page.firstAdvancedField = advertisedHost
                            EnterKey.iconSource: "image://theme/icon-m-enter-next"
                            EnterKey.onClicked: localAddress.focus = true
                        }

                        TextField {
                            id: localAddress
                            width: parent.width
                            text: page.account.localAddress || ""
                            //% "Local address"
                            label: qsTrId("account.local_address")
                            placeholderText: label
                            inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
                            onTextChanged: page.setAccountValue("localAddress", text)
                            EnterKey.iconSource: "image://theme/icon-m-enter-next"
                            EnterKey.onClicked: localPort.focus = true
                        }

                        TextField {
                            id: localPort
                            width: parent.width
                            text: page.account.localPort ? page.account.localPort.toString() : ""
                            //% "Local port"
                            label: qsTrId("account.local_port")
                            placeholderText: label
                            inputMethodHints: Qt.ImhDigitsOnly
                            validator: IntValidator {
                                bottom: 0
                                top: 65535
                            }
                            onTextChanged: page.setAccountValue("localPort", text.length > 0 ? parseInt(text, 10) : 0)
                            EnterKey.iconSource: "image://theme/icon-m-enter-next"
                            EnterKey.onClicked: timeoutMs.focus = true
                        }

                        TextSwitch {
                            id: nat
                            width: parent.width
                            checked: page.account.nat === undefined ? true : page.account.nat
                            //% "NAT"
                            text: qsTrId("account.nat")
                            //% "Enabled by default."
                            description: qsTrId("account.nat_description")
                            onCheckedChanged: page.setAccountValue("nat", checked)
                        }

                        TextField {
                            id: timeoutMs
                            width: parent.width
                            text: page.account.timeoutMs ? page.account.timeoutMs.toString() : ""
                            //% "Timeout"
                            label: qsTrId("account.timeout")
                            //% "Defaults to 5000 milliseconds."
                            description: qsTrId("account.timeout_description")
                            placeholderText: label
                            inputMethodHints: Qt.ImhDigitsOnly
                            validator: IntValidator {
                                bottom: 1
                            }
                            onTextChanged: page.setAccountValue("timeoutMs", text.length > 0 ? parseInt(text, 10) : 0)
                            EnterKey.iconSource: "image://theme/icon-m-enter-accept"
                            EnterKey.onClicked: page.accept()
                        }
                    }
                }
            }
        }
    }
}
