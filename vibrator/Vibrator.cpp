/*
 * Copyright (C) 2023 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "VibratorService"

#include <log/log.h>

#include <hardware/hardware.h>
#include <hardware/vibrator.h>

#include "Vibrator.h"

#include <cinttypes>
#include <cmath>
#include <iostream>
#include <fstream>

static constexpr char DURATION_PATH[] = "/sys/class/leds/tfa9xxx/duration";
namespace android {
namespace hardware {
namespace vibrator {
namespace V1_0 {
namespace implementation {

//static constexpr int MAX_VOLTAGE = 40;
//static constexpr int MIN_VOLTAGE = 10;

//static constexpr uint32_t CLICK_TIMING_MS = 20;

/*
 * Write value to path and close file.
 */
template <typename T>
static void set(const std::string& path, const T& value) {
    std::ofstream file(path);

    if (!file.is_open()) {
//        LOG(ERROR) << "Unable to open: " << path << " (" <<  strerror(errno) << ")";
        return;
    }

    file << value;
}

Vibrator::Vibrator(std::ofstream&& enable, std::ofstream&& amplitude) :
        mEnable(std::move(enable)),
        mAmplitude(std::move(amplitude)) {}

// Methods from ::android::hardware::vibrator::V1_0::IVibrator follow.
Return<Status> Vibrator::on(uint32_t timeout_ms) {
    mEnable << timeout_ms << std::endl;
    if (!mEnable) {
        ALOGE("Failed to turn vibrator on (%d): %s", errno, strerror(errno));
        return Status::UNKNOWN_ERROR;
    }

    uint32_t val;
    if ((timeout_ms == 400) || (timeout_ms == 640)) {
        val = 200;
    } else {
        val = timeout_ms;
    }
    set(DURATION_PATH, val);
    return Status::OK;
}

Return<Status> Vibrator::off()  {
    mEnable << 0 << std::endl;
    if (!mEnable) {
        ALOGE("Failed to turn vibrator off (%d): %s", errno, strerror(errno));
        return Status::UNKNOWN_ERROR;
    }
    return Status::OK;
}

Return<bool> Vibrator::supportsAmplitudeControl()  {
    return true;
}

Return<Status> Vibrator::setAmplitude(uint8_t amplitude) {
    long voltage;
    if (amplitude == 0) {
        return Status::BAD_VALUE;
    }

    if (amplitude < 100) {
        voltage = std::lround(amplitude * 60 / 255);
    }

    if (amplitude <= 255) {
        voltage = std::lround(amplitude * 50 / 255);
    }

    ALOGI("Setting amplitude  to: %ld", voltage);
    mAmplitude << voltage << std::endl;
    if (!mAmplitude) {
        ALOGE("Failed to set amplitude (%d): %s", errno, strerror(errno));
        return Status::UNKNOWN_ERROR;
    }
    return Status::OK;
}

Return<void> Vibrator::perform(Effect effect, EffectStrength strength, perform_cb _hidl_cb) {
    if (effect == Effect::CLICK) {
        uint8_t amplitude;
        switch (strength) {
        case EffectStrength::LIGHT:
            amplitude = 64; // min -- 20
            break;
        case EffectStrength::MEDIUM:
            amplitude = 128; // medium -- 30
            break;
        case EffectStrength::STRONG:
            amplitude = 255; // max -- 40
            break;
        default:
            _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);
            return Void();
        }
        on(30);
        setAmplitude(amplitude);
        _hidl_cb(Status::OK, 30);
    } else {
        _hidl_cb(Status::UNSUPPORTED_OPERATION, 0);
    }
    return Void();
}

} // namespace implementation
}  // namespace V1_0
}  // namespace vibrator
}  // namespace hardware
}  // namespace android
