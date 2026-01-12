// virtualkeyboardhelper.h
#pragma once
#include <QObject>
#include <QPointer>
#include <QtQuickWidgets/QQuickWidget>

class VirtualKeyboardHelper : public QObject {
    Q_OBJECT
public:
    static VirtualKeyboardHelper* attachTo(QWidget* host,
                                           const char* qmlUrl = "qrc:/KeyboardHost.qml");
    QQuickWidget* widget() const { return m_view; }

protected:
    bool eventFilter(QObject* watched, QEvent* e) override;

private:
    explicit VirtualKeyboardHelper(QWidget* host, const char* qmlUrl);

    QPointer<QWidget>      m_host;
    QPointer<QWidget>      m_overlay;   // 透明覆盖层(不进布局)
    QPointer<QQuickWidget> m_view;      // 承载 QML 的视图
};
