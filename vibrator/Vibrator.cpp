/*
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2024, The LineageOS project
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define LOG_TAG "vendor.nubia.hardware.vibrator"

#include <cmath>
#include <inttypes.h>
#include <log/log.h>
#include <string.h>
#include <thread>

#include "include/Vibrator.h"

#define LED_DEVICE "/sys/class/leds/tfa9xxx/"

#define VIB_ENABLE    LED_DEVICE "activate"  // Activates vibrator
                                             // with previously set parameters,
                                             // accepts either 0 or 1.

#define VIB_AMPLITUDE LED_DEVICE "amplitude" // Stores amplitude, 0 - 100.

#define VIB_DURATION  LED_DEVICE "duration"  // Stores duration for further
                                             // activation.

// Helps with making low amplitudes more felt.
// X - amplitude, 0.0 - 1.0
// X * (X < AMPLITUDE_BOOST_CEILING)
//       ? MAX_AMPLITUDE + AMPLITUDE_BOOST
//       : MAX_AMPLITUDE
#define MAX_AMPLITUDE           30
#define AMPLITUDE_BOOST         5
#define AMPLITUDE_BOOST_CEILING 0.4

namespace aidl {
namespace android {
namespace hardware {
namespace vibrator {

int Vibrator::write_value(const char *file, int32_t value) {
    int fd;
    int ret;
    int len;
    char buf[16];

    len = snprintf(buf, sizeof(buf), "%d\n", value);
    buf[len] = '\0';

    fd = TEMP_FAILURE_RETRY(open(file, O_WRONLY));
    if (fd < 0) {
        ALOGE("open %s failed, errno = %d", file, errno);
        return -errno;
    }

    ret = TEMP_FAILURE_RETRY(write(fd, buf, len + 1));
    if (ret == -1) {
        ret = -errno;
    } else if (ret != len + 1) {
        /* even though EAGAIN is an errno value that could be set
        by write() in some cases, none of them apply here.  So, this return
        value can be clearly identified when debugging and suggests the
        caller that it may try to call vibrator_on() again */
        ret = -EAGAIN;
    } else {
        ret = 0;
    }

    errno = 0;
    close(fd);

    return ret;
}

ndk::ScopedAStatus Vibrator::getCapabilities(int32_t* _aidl_return) {
    *_aidl_return = IVibrator::CAP_ON_CALLBACK
                        | IVibrator::CAP_PERFORM_CALLBACK
                        | IVibrator::CAP_AMPLITUDE_CONTROL;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::off() {
    int ret = write_value(VIB_ENABLE, 0);
    if (ret < 0)
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_SERVICE_SPECIFIC));
    
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::on(int32_t timeoutMs,
                                const std::shared_ptr<IVibratorCallback>& callback) {
    int ret;

    ALOGD("Vibrator on for timeoutMs: %d", timeoutMs);

    // Internally, driver can't accept value more than INT32_MAX / 48.
    // Behaviour would be unexpected and unsafe, so I'll just put this check here.
    if (timeoutMs > 89478485)
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));

    ret = write_value(VIB_DURATION, timeoutMs);
    if (ret < 0)
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_SERVICE_SPECIFIC));
    
    ret = write_value(VIB_ENABLE, 1);
    if (ret < 0)
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_SERVICE_SPECIFIC));

    if (callback != nullptr) {
        std::thread([=] {
            ALOGD("Starting on on another thread");
            usleep(timeoutMs * 1000);
            ALOGD("Notifying on complete");
            if (!callback->onComplete().isOk()) {
                ALOGE("Failed to call onComplete");
            }
        }).detach();
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::perform(Effect effect, EffectStrength es,
                                     const std::shared_ptr<IVibratorCallback>& callback,
                                     int32_t* _aidl_return) {
    int32_t durationMs;
    float amplitude;
    int32_t repeatTimeoutMs = 0;

    ALOGD("Vibrator perform effect %d", static_cast<int>(effect));

    // I feel like that. (c)
    switch (effect) {
        case Effect::CLICK:
            switch (es) {
                case EffectStrength::LIGHT:
                    amplitude = 0.25;
                    break;
                case EffectStrength::MEDIUM:
                    amplitude = 0.5;
                    break;
                case EffectStrength::STRONG:
                default:
                    amplitude = 1;
                    break;
            }
            durationMs = 30;
            break;
        case Effect::DOUBLE_CLICK:
            switch (es) {
                case EffectStrength::LIGHT:
                    amplitude = 0.6;
                    break;
                case EffectStrength::MEDIUM:
                    amplitude = 0.8;
                    break;
                case EffectStrength::STRONG:
                default:
                    amplitude = 1;
                    break;
            }
            durationMs = 10;
            repeatTimeoutMs = 35;
            break;
        case Effect::TICK:
            switch (es) {
                case EffectStrength::LIGHT:
                    amplitude = 0.3;
                    break;
                case EffectStrength::MEDIUM:
                    amplitude = 0.65;
                    break;
                case EffectStrength::STRONG:
                default:
                    amplitude = 1;
                    break;
            }
            durationMs = 18;
            break;
        case Effect::TEXTURE_TICK:
            switch (es) {
                case EffectStrength::LIGHT:
                    amplitude = 0.6;
                    break;
                case EffectStrength::MEDIUM:
                case EffectStrength::STRONG:
                default:
                    amplitude = 1;
                    break;
            }
            durationMs = 16;
            break;
        case Effect::HEAVY_CLICK:
        case Effect::THUD:
            switch (es) {
                case EffectStrength::LIGHT:
                    amplitude = 0.7;
                    break;
                case EffectStrength::MEDIUM:
                    amplitude = 0.9;
                    break;
                case EffectStrength::STRONG:
                default:
                    amplitude = 1;
                    break;
            }
            durationMs = 55;
            break;
        case Effect::POP:
            switch (es) {
                case EffectStrength::LIGHT:
                    amplitude = 0.55;
                    break;
                case EffectStrength::MEDIUM:
                    amplitude = 0.85;
                    break;
                case EffectStrength::STRONG:
                default:
                    amplitude = 1;
                    break;
            }
            durationMs = 24;
            break;
        default:
            return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
    }

    setAmplitude(amplitude);
    on(durationMs, nullptr);

    if (callback != nullptr || repeatTimeoutMs > 0) {
        std::thread([=, this] {
            ALOGD("Starting perform on another thread");

            // -> Enable for durationMs
            // -> Wait for repeatTimeoutMs
            // -> Again, enable for durationMs

            if (repeatTimeoutMs) {
                usleep(durationMs * 1000);

                ALOGD("Repeating perform");
                usleep(repeatTimeoutMs * 1000);

                setAmplitude(amplitude);
                on(durationMs, nullptr);
            }
            
            if (callback != nullptr) {
                usleep(durationMs * 1000);
                ALOGD("Notifying perform complete");
                callback->onComplete();
            }
        }).detach();
    }

    *_aidl_return =
        durationMs + repeatTimeoutMs + (repeatTimeoutMs > 0 ? durationMs : 0);
    
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::getSupportedEffects(std::vector<Effect>* _aidl_return) {
    *_aidl_return = {Effect::CLICK, Effect::DOUBLE_CLICK, Effect::TICK,
                     Effect::TEXTURE_TICK, Effect::THUD, Effect::POP,
                     Effect::HEAVY_CLICK};
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::setAmplitude(float amplitude) {
    char buf[16];
    int ret;
    // 0 - 100%, see drivers/leds/nubia_led/tfa9xxx/tfa2_haptic.c L74
    // Follow the HIDL's behaviour here, boost amplitude at low values.
    int32_t percentage = std::lround(
        amplitude *
            ((amplitude < AMPLITUDE_BOOST_CEILING)
                ? MAX_AMPLITUDE + AMPLITUDE_BOOST
                : MAX_AMPLITUDE)
    );

    if (amplitude <= 0 || amplitude > 1)
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));

    ret = write_value(VIB_AMPLITUDE, percentage);
    if (ret < 0)
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_SERVICE_SPECIFIC));
    
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::setExternalControl(bool enabled __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getCompositionDelayMax(int32_t* maxDelayMs  __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getCompositionSizeMax(int32_t* maxSize __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getSupportedPrimitives(std::vector<CompositePrimitive>* supported __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getPrimitiveDuration(CompositePrimitive primitive __unused,
                                                  int32_t* durationMs __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::compose(const std::vector<CompositeEffect>& composite __unused,
                                     const std::shared_ptr<IVibratorCallback>& callback __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::getSupportedAlwaysOnEffects(std::vector<Effect>* _aidl_return __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::alwaysOnEnable(int32_t id __unused, Effect effect __unused,
                                            EffectStrength strength __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

ndk::ScopedAStatus Vibrator::alwaysOnDisable(int32_t id __unused) {
    return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_UNSUPPORTED_OPERATION));
}

}  // namespace vibrator
}  // namespace hardware
}  // namespace android
}  // namespace aidl

