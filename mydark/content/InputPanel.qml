// :/vkbd/content/InputPanel.qml
import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.VirtualKeyboard 2.2
import QtGraphicalEffects 1.12

Item {
    id: panelRoot
    anchors.fill: parent
    visible: keyboard.active

    // ====== 外层气泡容器（实现圆角、阴影、动画） ======
    Rectangle {
        id: bubble
        radius: 12
        color: "#1b2537"
        opacity: 0.97
        border.color: "#2f4763"
        border.width: 1
        width: Math.min(panelRoot.width - 16, Screen.desktopAvailableWidth * 0.6)
        height: keyboard.implicitHeight + 12
        x: Math.round(Math.max(8, Math.min(panelRoot.width - width - 8, targetX)))
        y: Math.round(targetY)

        layer.enabled: true
        layer.effect: DropShadow {
            horizontalOffset: 0
            verticalOffset: 8
            radius: 20
            samples: 32
            color: "#80000000"
        }

        Behavior on x { NumberAnimation { duration: 140; easing.type: Easing.OutCubic } }
        Behavior on y { NumberAnimation { duration: 140; easing.type: Easing.OutCubic } }
        Behavior on width { NumberAnimation { duration: 100; easing.type: Easing.OutCubic } }
    }

    // ====== 真正的键盘控件（官方提供） ======
    Keyboard {
        id: keyboard
        z: 1
        anchors.top: bubble.top
        anchors.topMargin: 6
        anchors.horizontalCenter: bubble.horizontalCenter
        width: bubble.width - 12
        focus: false
    }

    // ====== 输入框矩形（Qt5.12 字段兼容） ======
    readonly property rect inputRect: {
        if (InputContext && InputContext.inputItemRectangle)
            return InputContext.inputItemRectangle
        if (InputContext && InputContext.inputItemGeometry)
            return InputContext.inputItemGeometry
        return Qt.rect(panelRoot.width/2 - 1, panelRoot.height/2 - 1, 2, 2)
    }

    // 锁定的目标位置（由 place() 计算）
    property real targetX: (inputRect.x + inputRect.width/2) - bubble.width/2
    property real targetY: panelRoot.height - bubble.height - 8

    // ====== 智能摆放：尽量在输入框下方；不行则上方；再不行贴底 ======
    function place() {
        var margin = 8
        var w = Math.min(panelRoot.width - 2*margin, Screen.desktopAvailableWidth * 0.6)
        bubble.width = w

        var centerX = inputRect.x + inputRect.width/2
        targetX = centerX - w/2
        // 横向边界
        if (targetX < margin) targetX = margin
        if (targetX + w > panelRoot.width - margin) targetX = panelRoot.width - margin - w

        var h = bubble.height
        var spaceBelow = panelRoot.height - (inputRect.y + inputRect.height) - margin
        if (spaceBelow >= h + 12) {
            // 在下方
            targetY = inputRect.y + inputRect.height + 8
        } else if (inputRect.y - h - 12 >= 0) {
            // 在上方
            targetY = inputRect.y - h - 8
        } else {
            // 贴底部
            targetY = panelRoot.height - h - margin
        }
    }

    // ====== 重算时机（做节流） ======
    Timer {
        id: relayoutDebounce
        interval: 0; running: false; repeat: false
        onTriggered: place()
    }
    function requestRelayout() { relayoutDebounce.restart() }

    // 焦点/几何变化/显示变化 时重算
    Connections {
        target: InputContext
        onInputItemChanged: requestRelayout()
        onInputItemRectangleChanged: requestRelayout()
        onFocusObjectChanged: requestRelayout ? requestRelayout() : null
        onVisibleChanged: requestRelayout()
    }
    onVisibleChanged: requestRelayout()
    onWidthChanged: requestRelayout()
    onHeightChanged: requestRelayout()
    Component.onCompleted: requestRelayout()

    // 点击外面关闭（可选）
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        onClicked: {
            if (!bubble.containsMouse) Qt.inputMethod.hide()
        }
        hoverEnabled: true
    }
}
