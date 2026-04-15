#pragma once

#include <aidl/android/hardware/biometrics/fingerprint/BnFingerprint.h>
#include <aidl/android/hardware/biometrics/fingerprint/BnSession.h>
#include <aidl/android/hardware/biometrics/fingerprint/ISessionCallback.h>
#include <aidl/android/hardware/biometrics/fingerprint/SensorProps.h>
#include <aidl/android/hardware/biometrics/fingerprint/PointerContext.h>
#include <aidl/android/hardware/biometrics/common/BnCancellationSignal.h>
#include <aidl/android/hardware/biometrics/common/ICancellationSignal.h>
#include <aidl/android/hardware/biometrics/common/OperationContext.h>
#include <aidl/android/hardware/keymaster/HardwareAuthToken.h>

#include <hardware/fingerprint.h>
#include <hardware/hardware.h>

#include <mutex>
#include <memory>
#include <atomic>

namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {

using ::aidl::android::hardware::biometrics::common::ICancellationSignal;
using ::aidl::android::hardware::biometrics::common::OperationContext;
using ::aidl::android::hardware::keymaster::HardwareAuthToken;

class CancellationSignal
    : public ::aidl::android::hardware::biometrics::common::BnCancellationSignal {
public:
    explicit CancellationSignal(fingerprint_device_t* device)
        : mDevice(device) {}

    ndk::ScopedAStatus cancel() override {
        if (mDevice) {
            mDevice->cancel(mDevice);
        }
        return ndk::ScopedAStatus::ok();
    }

private:
    fingerprint_device_t* mDevice;
};

class FingerprintSession : public BnSession {
public:
    FingerprintSession(
        fingerprint_device_t* device,
        int32_t sensorId,
        int32_t userId,
        const std::shared_ptr<ISessionCallback>& cb);

    ~FingerprintSession() override;

    ndk::ScopedAStatus generateChallenge() override;
    ndk::ScopedAStatus revokeChallenge(int64_t challenge) override;

    ndk::ScopedAStatus enroll(
        const HardwareAuthToken& hat,
        std::shared_ptr<ICancellationSignal>* out) override;

    ndk::ScopedAStatus authenticate(
        int64_t operationId,
        std::shared_ptr<ICancellationSignal>* out) override;

    ndk::ScopedAStatus detectInteraction(
        std::shared_ptr<ICancellationSignal>* out) override;

    ndk::ScopedAStatus enumerateEnrollments() override;

    ndk::ScopedAStatus removeEnrollments(
        const std::vector<int32_t>& enrollmentIds) override;

    ndk::ScopedAStatus getAuthenticatorId() override;
    ndk::ScopedAStatus invalidateAuthenticatorId() override;

    ndk::ScopedAStatus resetLockout(
        const HardwareAuthToken& hat) override;

    ndk::ScopedAStatus close() override;

    ndk::ScopedAStatus onPointerDown(
        int32_t pointerId, int32_t x, int32_t y,
        float minor, float major) override;

    ndk::ScopedAStatus onPointerUp(int32_t pointerId) override;

    ndk::ScopedAStatus onUiReady() override;

    // V3 WithContext methods
    ndk::ScopedAStatus authenticateWithContext(
        int64_t operationId,
        const OperationContext& context,
        std::shared_ptr<ICancellationSignal>* out) override;

    ndk::ScopedAStatus enrollWithContext(
        const HardwareAuthToken& hat,
        const OperationContext& context,
        std::shared_ptr<ICancellationSignal>* out) override;

    ndk::ScopedAStatus detectInteractionWithContext(
        const OperationContext& context,
        std::shared_ptr<ICancellationSignal>* out) override;

    ndk::ScopedAStatus onPointerDownWithContext(
        const PointerContext& context) override;

    ndk::ScopedAStatus onPointerUpWithContext(
        const PointerContext& context) override;

    ndk::ScopedAStatus onPointerCancelWithContext(
        const PointerContext& context) override;

    ndk::ScopedAStatus onContextChanged(
        const OperationContext& context) override;

    ndk::ScopedAStatus setIgnoreDisplayTouches(
        bool shouldIgnore) override;

    static void legacyNotify(const fingerprint_msg_t* msg);
    static FingerprintSession* sCurrentSession;

private:
    static Error vendorErrorFilter(int32_t error, int32_t* vendorCode);
    static AcquiredInfo vendorAcquiredFilter(int32_t info, int32_t* vendorCode);

    fingerprint_device_t* mDevice;
    int32_t mUserId;
    std::shared_ptr<ISessionCallback> mCb;
    std::mutex mMutex;
};

class BiometricsFingerprint : public BnFingerprint {
public:
    BiometricsFingerprint();
    ~BiometricsFingerprint() override;

    static std::shared_ptr<BiometricsFingerprint> create();

    ndk::ScopedAStatus getSensorProps(
        std::vector<SensorProps>* props) override;

    ndk::ScopedAStatus createSession(
        int32_t sensorId,
        int32_t userId,
        const std::shared_ptr<ISessionCallback>& cb,
        std::shared_ptr<ISession>* session) override;

private:
    fingerprint_device_t* openHal();
    fingerprint_device_t* mDevice;
    std::mutex mSessionMutex;
};

}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
}  // namespace aidl
