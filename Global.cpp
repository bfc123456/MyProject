#include "Global.h"
#include "multiuserloginwindow.h"
#include "DeviceAcquisitionWorker.h"
#include "MeasurementDataProcessor.h"
#include "UdpManager.h"

const int MD_PROGRESS_TOTAL_DURATION  = 30000;  // 30s
const int MD_PROGRESS_UPDATE_INTERVAL = 300;    // 300ms
const int MD_PROGRESS_MAX_VALUE       = 100;    // 进度条最大值

MultiUserLoginWindow*  globalLoginWindowPointer = nullptr;

DeviceAcquisitionWorker*  g_DeviceAcquisitionWorker     = nullptr;
QThread*      g_receiverThread  = nullptr;

MeasurementDataProcessor* g_MeasurementDataProcessor   = nullptr;
QThread*       g_processorThread = nullptr;
