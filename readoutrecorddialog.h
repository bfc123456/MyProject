
#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include "measurementdata.h"
#include "closeonlywindow.h"

/**
 * @class ReadoutRecordDialog
 * @brief 测量记录查看与管理对话框
 *
 * 该类用于展示测量记录列表（QTableWidget），并提供刷新、
 * 删除等操作。用户可通过按钮交互更新表格内容或删除指定行，
 * 同时通过信号通知上层业务逻辑。
 *
 * 功能：
 * - populateData()：将测量记录列表加载到表格中。
 * - onDeleteButtonClicked()：删除选中行并发射 rowDeleted() 信号。
 * - 支持刷新按钮，触发 onRefreshButtonClicked() 信号。
 * - returnImplantationsite()：返回植入点界面（业务相关）。
 *
 * 信号：
 * - rowDeleted(int row)：删除某行后发射，参数为行号。
 * - returnImplantationsite()：请求返回植入点界面。
 * - onRefreshButtonClicked()：请求刷新表格数据。
 *
 * 内部成员：
 * - QTableWidget *table：用于展示测量记录的表格。
 * - QPushButton *btnRefresh / *btnDelete：刷新、删除按钮。
 * - float scaleX, scaleY：界面缩放比例（适配不同分辨率）。
 */

class ReadoutRecordDialog : public CloseOnlyWindow {
    Q_OBJECT

public:
    explicit ReadoutRecordDialog(QWidget *parent = nullptr);
    ~ ReadoutRecordDialog();

public slots:
    void populateData(const QList<MeasurementData> &list);

private slots:
    void onDeleteButtonClicked();//删除动作函数

signals:
    void rowDeleted(int row);
    void returnImplantationsite();
    void onRefreshButtonClicked();

private:
    void changeEvent(QEvent *event);

    QLabel *titleLabel;
    QTableWidget *table;
    QPushButton *btnRefresh;
    QPushButton *btnDelete;
    float scaleX;
    float scaleY;
};
