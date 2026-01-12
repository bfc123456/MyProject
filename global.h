
#ifndef GLOBAL_H
#define GLOBAL_H

/**
 * @file Global.h
 * @brief 应用程序全局变量声明
 *
 * 本文件集中声明了应用中的全局常量、全局对象和跨线程指针，
 * 方便在不同模块之间共享，而不必重复定义。
 *
 * 功能与内容：
 * - 定义测量流程相关的常量（进度条时长、更新间隔、最大值）
 * - 声明全局 UI 指针（如登录窗口）
 * - 声明采集线程与数据处理线程对象，以及对应的工作对象指针
 *
 * 使用场景：
 * - 在 main.cpp 中初始化这些全局变量
 * - 在业务模块中直接引用这些全局对象（例如通过 g_DeviceAcquisitionWorker 控制采集）
 *
 * @note
 * - 这里仅做 **extern 声明**，具体定义在对应的 .cpp 文件中（通常是 Global.cpp 或 main.cpp）
 * - 使用全局变量需谨慎，避免线程安全问题；
 *   推荐在退出时正确清理（quit + wait + delete）
 */

// 前置声明
class ImplantMonitor;               // 前置声明
class DeviceAcquisitionWorker;      // 前置声明
class MeasurementDataProcessor;     // 前置声明

// 常量声明
extern const int MD_PROGRESS_TOTAL_DURATION;    // 总时长（毫秒）
extern const int MD_PROGRESS_UPDATE_INTERVAL;   // UI 更新间隔（毫秒）
extern const int MD_PROGRESS_MAX_VALUE;         // 进度条最大值

//1)Project Headers
#include "implantmonitor.h"

//2)Qt Headers
#include <QThread>

// 全局 UI 指针
extern ImplantMonitor* g_pGlobalImplantMonitorPtr; // 声明一个全局指针

// UDP 接收线程及对象
extern DeviceAcquisitionWorker* g_pDeviceAcquisitionWorkerPtr; // 声明全局指针，指向采集工作对象
extern QThread* g_pReceiverThreadPtr; // 声明全局指针，指向接收线程

// 数据处理线程及对象
extern MeasurementDataProcessor* g_pMeasurementDataProcessorPtr; // 声明全局指针，指向数据处理工作对象
extern QThread* g_pProcessorThreadPtr; // 声明全局指针，指向处理线程

#endif // APP_GLOBAL_H
