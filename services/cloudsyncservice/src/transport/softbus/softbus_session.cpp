/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "softbus_session.h"

#include <mutex>
#include <thread>

#include "dfs_error.h"
#include "softbus_adapter.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;
const std::string GROUP_TYPE_P2P = "CloudSyncService_P2PGroup";
constexpr int OPEN_SESSION_RETRY_TIMES = 10 * 30;
constexpr int OPEN_SESSION_RETRY_INTERVAL_MS = 100;
constexpr int LOG_SPAN = 10;
constexpr int32_t HEAD_SIZE = 3;
constexpr int32_t END_SIZE = 3;
constexpr int32_t MIN_SIZE = HEAD_SIZE + END_SIZE + 3;
constexpr const char *REPLACE_CHAIN = "***";
constexpr const char *DEFAULT_ANONYMOUS = "******";

SoftbusSession::SoftbusSession(const std::string &peerDeviceId, const std::string &sessionName, DataType type)
    : peerDeviceId_(peerDeviceId), sessionName_(sessionName), type_(type)
{
}

int32_t SoftbusSession::Start()
{
    CancelReleaseSessionIfNeeded();
    if (sessionId_ != INVALID_SESSION_ID) {
        LOGI("session is exist, no need open again");
        return E_OK;
    }

    std::unique_lock<mutex> lock(sessionMutex_);
    if (sessionId_ == INVALID_SESSION_ID) {
        LOGI("open session with device: %{public}s", ToBeAnonymous(peerDeviceId_).c_str());
        int session = SoftbusAdapter::GetInstance().OpenSessionByP2P(
            sessionName_.c_str(), peerDeviceId_.c_str(), GROUP_TYPE_P2P.c_str(), (type_ == DataType::TYPE_FILE));
        if (session < 0) {
            LOGE("open session failed");
            return E_OPEN_SESSION;
        }

        auto ret = WaitSessionOpened(session);
        if (ret != E_OK) {
            return ret;
        }
        sessionId_ = session;
    }

    return E_OK;
}

int32_t SoftbusSession::Stop()
{
    LOGD("stop connection");
    std::unique_lock<mutex> lock(sessionMutex_);
    SoftbusAdapter::GetInstance().CloseSession(sessionId_);
    sessionId_ = INVALID_SESSION_ID;
    return E_OK;
}

int32_t SoftbusSession::SendData(const void *data, uint32_t dataLen)
{
    return SoftbusAdapter::GetInstance().SendBytes(sessionId_, data, dataLen);
}

int32_t SoftbusSession::SendFile(const std::vector<std::string> &sFileList, const std::vector<std::string> &dFileList)
{
    return SoftbusAdapter::GetInstance().SendFile(sessionId_, sFileList, dFileList);
}

int32_t SoftbusSession::GetSessionId()
{
    return sessionId_;
}

SoftbusSession::DataType SoftbusSession::GetDataType()
{
    return type_;
}

std::string SoftbusSession::GetPeerDeviceId()
{
    return peerDeviceId_;
}

void SoftbusSession::CancelReleaseSessionIfNeeded() {}

void SoftbusSession::CancelDelayReleaseSessionTask() {}

int32_t SoftbusSession::WaitSessionOpened(int sessionId)
{
    int retryTimes = 0;
    int logSpan = LOG_SPAN;
    while (retryTimes++ < OPEN_SESSION_RETRY_TIMES) {
        if (!SoftbusAdapter::GetInstance().IsSessionOpened(sessionId)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(OPEN_SESSION_RETRY_INTERVAL_MS));
            if (retryTimes % logSpan == 0) {
                LOGI("openSession, waiting for:%{public}d ms", retryTimes * OPEN_SESSION_RETRY_INTERVAL_MS);
            }
            continue;
        }
        return E_OK;
    }
    LOGE("wait session opened timeout");
    return E_WAIT_SESSION_OPENED;
}

std::string SoftbusSession::ToBeAnonymous(const std::string &name)
{
    if (name.length() <= HEAD_SIZE) {
        return DEFAULT_ANONYMOUS;
    }

    if (name.length() < MIN_SIZE) {
        return (name.substr(0, HEAD_SIZE) + REPLACE_CHAIN);
    }

    return (name.substr(0, HEAD_SIZE) + REPLACE_CHAIN + name.substr(name.length() - END_SIZE, END_SIZE));
}
} // namespace OHOS::FileManagement::CloudSync
