import QtQuick 2.12
import QtQuick.VirtualKeyboard 2.4
import QtQuick.VirtualKeyboard.Settings 2.2

Item {
    anchors.fill: parent
    clip: true                             // 建议：裁剪，避免内容溢出 QQuickWidget

    Component.onCompleted: {
        VirtualKeyboardSettings.fullScreenMode = false
        // VirtualKeyboardSettings.keyboardScale = 0.85
    }

    InputPanel {
        id: panel
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: -height      // ☆ 默认隐藏：把它推进父项下方

        states: [
            State {
                name: "shown"
                when: panel.active         // 比 Qt.inputMethod.visible 更贴合
                PropertyChanges { target: panel; anchors.bottomMargin: 0 }
            },
            State {
                name: "hidden"
                when: !panel.active
                PropertyChanges { target: panel; anchors.bottomMargin: -panel.height }
            }
        ]

        transitions: Transition {
            NumberAnimation {
                properties: "anchors.bottomMargin"
                duration: 220
                easing.type: Easing.InOutQuad
            }
        }
    }
}
