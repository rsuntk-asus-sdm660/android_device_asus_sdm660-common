#define LOG_TAG "fingerprint-asus"

#include "BiometricsFingerprint.h"

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <hidl/HidlTransportSupport.h>

using ::aidl::android::hardware::biometrics::fingerprint::BiometricsFingerprint;

int main() {
    LOG(INFO) << "Fingerprint AIDL HAL service (asus) starting";

    // Инициализируем HIDL hwbinder thread pool ДО загрузки блоба
    // fingerprint.sdm660.so при open() вызывает
    // vendor.goodix registerAsService() через hwbinder
    android::hardware::configureRpcThreadpool(2, false);

    // AIDL binder
    ABinderProcess_setThreadPoolMaxThreadCount(1);
    ABinderProcess_startThreadPool();

    auto service = BiometricsFingerprint::create();
    if (!service) {
        LOG(ERROR) << "Failed to create BiometricsFingerprint";
        return EXIT_FAILURE;
    }

    const std::string instance =
        std::string(BiometricsFingerprint::descriptor) + "/default";

    binder_status_t status = AServiceManager_addService(
        service->asBinder().get(), instance.c_str());

    if (status != STATUS_OK) {
        LOG(ERROR) << "Failed to register AIDL service: " << status;
        return EXIT_FAILURE;
    }

    LOG(INFO) << "Service registered as: " << instance;

    ABinderProcess_joinThreadPool();

    return EXIT_FAILURE;
}
