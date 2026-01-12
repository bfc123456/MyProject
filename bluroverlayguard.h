/********************************************************************************/
/* 文件名    : BlurOverlayGuard.h                                               */
/* 功能      : 背景遮罩 + 模糊效果的临时管理类（RAII 风格）                      */
/* 版本      : 1.0                                                              */
/* 作者      :                                                         */
/* 日期      : 2025-12-26                                                       */
/* 说明      : 该文件定义了一个背景遮罩和模糊效果的管理类，用于在弹出对话框、  */
/*            二级窗口时，为当前界面加上一层半透明模糊效果，突出弹窗。            */
/********************************************************************************/

#ifndef BLUROVERLAYGUARD_H
#define BLUROVERLAYGUARD_H

#include <QWidget>

class QGraphicsBlurEffect;

/**
 * @brief  背景遮罩 + 模糊效果的临时管理类（RAII 风格）。
 *
 * 使用场景：
 * - 在弹出对话框、二级窗口时，希望对当前界面加一层
 *   「半透明变暗 + 模糊」的效果，以突出弹窗。
 *
 * 设计思路：
 * - 在构造函数中，对指定的 host 窗口：
 *     1. 创建一个覆盖整个 host 的半透明遮罩 QWidget；
 *     2. 给 host 设置 QGraphicsBlurEffect。
 * - 在析构函数中，自动还原：
 *     1. 移除模糊效果；
 *     2. 释放遮罩层。
 */
class BlurOverlayGuard
{
public:
    explicit BlurOverlayGuard(QWidget *pHost,
                              int iBlurRadius = 18,
                              int iAlpha      = 100);

    /**
     * @brief 析构函数：自动移除模糊效果并销毁遮罩层。
     *
     * 在 guard 所在的作用域结束时自动调用，确保界面状态被还原。
     */
    ~BlurOverlayGuard();

    // 禁止拷贝，防止重复管理同一套资源
    BlurOverlayGuard(const BlurOverlayGuard &)            = delete;
    BlurOverlayGuard &operator=(const BlurOverlayGuard &) = delete;

private:
    QWidget *m_pHostWidget;        ///< 宿主窗口指针（不负责销毁）
    QWidget *m_pOverlayWidget;     ///< 遮罩层窗口指针（由本类负责销毁）
    QGraphicsBlurEffect *m_pBlurEffect; ///< 模糊效果对象指针（由本类负责销毁）
};

#endif // BLUROVERLAYGUARD_H
