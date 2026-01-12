import QtQuick 2.12
import QtQuick.VirtualKeyboard 2.1
import QtQuick.VirtualKeyboard.Styles 2.1

KeyboardStyle  {
    key: KeyStyle {
        // 让文字/图标整体更“沉”
        contentMargins: Qt.rect(6, 6, 6, 10)   // 左 上 右 下
        label: Text {
            anchors.centerIn: parent
            anchors.verticalCenterOffset: 2    // 下移 1~3 像素按需调
            text: control.displayText
            color: KeyboardStyle.keyTextColor
            font.pixelSize: Math.round(parent.height * 0.45)
            visible: control.displayText.length > 0
        }
        icon: Image {
            anchors.centerIn: parent
            anchors.verticalCenterOffset: 2
            source: control.iconSource
            fillMode: Image.PreserveAspectFit
            visible: control.iconSource !== ""
        }
    }
}
