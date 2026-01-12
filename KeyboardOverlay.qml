import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.VirtualKeyboard 2.2
import QtQuick.VirtualKeyboard.Settings 2.2

Window {
    id: vk
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool | Qt.WindowDoesNotAcceptFocus
    color: "transparent"
    visible: Qt.inputMethod.visible

    // 只缩小内部按键：0.9=缩小10%，外框大小不变
    property real keyScale: 0.7

    width: Screen.width
    height: inputPanel.implicitHeight * keyScale           // 外框保持默认高度
    x: Screen.virtualX
    y: Screen.height - height

    // 仅缩放内容，不改外框尺寸
    Item {
        id: content
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        transformOrigin: Item.Bottom            // 从底部缩放，贴底不动
        scale: keyScale

        InputPanel {
            id: inputPanel
            anchors.fill: parent
            visible: true
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: inputPanel.implicitHeight
        color: "#0F172A"    // 你想要的面板色
        opacity: 0        // 透明度可调
    }

    Component.onCompleted: {
        VirtualKeyboardSettings.styleName = "test";
    }
}

