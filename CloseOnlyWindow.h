
#ifndef CLOSEONLYWINDOW_H
#define CLOSEONLYWINDOW_H

#include <QDialog>

class CloseOnlyWindow : public QDialog {
    Q_OBJECT
public:
    explicit CloseOnlyWindow(QWidget *parent = nullptr)
      : QDialog(parent)
    {
        // 去掉最大化/最小化，只保留“关闭”按钮
        setWindowFlags(
              Qt::Window
            | Qt::CustomizeWindowHint
            | Qt::WindowTitleHint
            | Qt::WindowCloseButtonHint
        );
    }
};

#endif // CLOSEONLYWINDOW_H
