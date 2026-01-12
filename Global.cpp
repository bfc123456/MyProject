#include "global.h"
#include "deviceacquisitionworker.h"
#include "measurementdataprocessor.h"
#include "udpmanager.h"

const int MD_PROGRESS_TOTAL_DURATION  = 30000;  // 30s
const int MD_PROGRESS_UPDATE_INTERVAL = 300;    // 300ms
const int MD_PROGRESS_MAX_VALUE       = 100;    // 进度条最大值

ImplantMonitor*  g_pGlobalImplantMonitorPtr = nullptr;

DeviceAcquisitionWorker*  g_pDeviceAcquisitionWorkerPtr     = nullptr;
QThread*      g_pReceiverThreadPtr  = nullptr;

MeasurementDataProcessor* g_pMeasurementDataProcessorPtr   = nullptr;
QThread*       g_pProcessorThreadPtr = nullptr;
