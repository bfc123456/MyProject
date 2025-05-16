
#include <QDateEdit>
#include <QCalendarWidget>
#include <QEvent>

class CustomDateEdit : public QDateEdit {
    Q_OBJECT
public:
    explicit CustomDateEdit(QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};
