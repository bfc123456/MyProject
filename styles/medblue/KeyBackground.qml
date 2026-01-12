import QtQuick 2.12
import QtQuick.VirtualKeyboard 2.3

KeyboardStyle {
    // —— 你的配色（自己改成喜欢的十六进制颜色）——
    readonly property color panelColor       : "#0F1F3A"   // 面板底色
    readonly property color keyColor         : "#2E3440"   // 普通键
    readonly property color keyColorPressed  : "#3B4252"   // 按下
    readonly property color keyTextColor     : "#E6EDF3"   // 字/图标
    readonly property color keyBorderColor   : "#00000000" // 键边框

    keyboardBackground: Component { KeyboardBackground {} }
    keyBackground:      Component { KeyBackground {} }

    // 键文字（同时影响图标的着色）
    keyTextStyle: Text {
        color: keyTextColor
        font.pixelSize: 22
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        renderType: Text.NativeRendering
        elide: Text.ElideRight
    }
}
