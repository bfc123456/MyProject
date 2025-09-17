#ifndef MEASUREMENT_CONFIG_H
#define MEASUREMENT_CONFIG_H

/**
 * @file MeasurementConfig.h
 * @brief 测量过程相关的参数配置
 *
 * 本文件定义了测量过程中使用的全局静态常量，包括：
 * - 测量的总时长
 * - UI 更新的时间间隔
 * - 进度条的最大值
 *
 * 这些参数在测量逻辑、UI 进度显示与状态刷新中起到统一配置的作用，
 * 方便后续维护和修改。
 */

static constexpr int MEASUREMENT_TOTAL_DURATION_MS   = 1000; // 总时长
static constexpr int MEASUREMENT_UPDATE_INTERVAL_MS  = 300;   // 更新间隔
static constexpr int MEASUREMENT_PROGRESS_MAX        = 100;   // 进度条最大值

#endif
