/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "sync_rule/network_signal_strength_listener.h"

#include <charconv>
#include "common_event_manager.h"
#include "common_event_support.h"
#include "network_status.h"
#include "ffrt_inner.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {

constexpr int32_t SIGNAL_LEVEL_MIN = 0;
constexpr int32_t SIGNAL_LEVEL_MAX = 5;

NetworkSignalStrengthSubscriber::NetworkSignalStrengthSubscriber(
    const EventFwk::CommonEventSubscribeInfo &subscribeInfo, std::shared_ptr<NetworkSignalStrengthListener> listener)
    : EventFwk::CommonEventSubscriber(subscribeInfo), listener_(listener)
{
}

void NetworkSignalStrengthSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    auto action = eventData.GetWant().GetAction();
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_SIGNAL_INFO_CHANGED) {
        if (NetworkStatus::GetNetConnStatus() != NetworkStatus::NetConnStatus::CELLULAR_CONNECT) {
            LOGE("Not cellular connect, no need to do anything");
            return;
        }
        std::vector<std::string> signalInfos = eventData.GetWant().GetStringArrayParam("signalInfos");
        if (signalInfos.size() == 0) {
            LOGE("cellular data size is 0");
            return;
        }
        int32_t signalStrength = listener_->GetCellularSignalLevel(signalInfos[0]);
        LOGI("cellular data signalStrength = %{public}d", signalStrength);
        if (signalStrength < SIGNAL_LEVEL_MIN || signalStrength > SIGNAL_LEVEL_MAX) {
            LOGE("Invaild cellular signal level!!");
            return;
        }
        NetworkStatus::SetCellularSignalStrength(signalStrength);
        if (NetworkStatus::IsAllowSync(SyncTriggerType::NETWORK_AVAIL_TRIGGER)) {
            LOGI("OnReceiveEvent allow sync for cellulardata");
            listener_->OnNetworkSignalAvail();
        }
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_RSSI_VALUE) {
        if (NetworkStatus::GetNetConnStatus() != NetworkStatus::NetConnStatus::WIFI_CONNECT) {
            LOGE("Not wifi connect, no need to do anything");
            return;
        }
        int32_t signalStrength = eventData.GetWant().GetIntParam("wifiSignalLevel", 0);
        LOGI("wifi data signalStrength = %{public}d", signalStrength);
        if (signalStrength < SIGNAL_LEVEL_MIN || signalStrength > SIGNAL_LEVEL_MAX) {
            LOGE("Invaild wifi signal level!!");
            return;
        }
        NetworkStatus::SetWifiSignalStrength(signalStrength);
        if (NetworkStatus::IsAllowSync(SyncTriggerType::NETWORK_AVAIL_TRIGGER)) {
            LOGI("OnReceiveEvent allow sync for for wifidata");
            listener_->OnNetworkSignalAvail();
        }
    } else {
        LOGI("OnReceiveEvent action is invalid");
    }
}

NetworkSignalStrengthListener::NetworkSignalStrengthListener
(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager)
{
    dataSyncManager_ = dataSyncManager;
}

NetworkSignalStrengthListener::~NetworkSignalStrengthListener()
{
    Stop();
}

void NetworkSignalStrengthListener::OnNetworkSignalAvail()
{
    dataSyncManager_->TriggerRecoverySync(SyncTriggerType::NETWORK_AVAIL_TRIGGER);
}

void NetworkSignalStrengthListener::Start()
{
    if (commonEventSubscriber_ != nullptr) {
        LOGW("Already subscriber, skip");
        return;
    }
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_WIFI_RSSI_VALUE);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SIGNAL_INFO_CHANGED);
    EventFwk::CommonEventSubscribeInfo info(matchingSkills);
    commonEventSubscriber_ = std::make_shared<NetworkSignalStrengthSubscriber>(info, shared_from_this());
    auto subRet = EventFwk::CommonEventManager::SubscribeCommonEvent(commonEventSubscriber_);
    LOGI("Subscriber end, SubscribeResult = %{public}d", subRet);
}

void NetworkSignalStrengthListener::Stop()
{
    if (commonEventSubscriber_ != nullptr) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(commonEventSubscriber_);
        commonEventSubscriber_ = nullptr;
    }
}

int32_t NetworkSignalStrengthListener::GetCellularSignalLevel(std::string& data)
{
    size_t pos = data.find("signalLevel:");
    if (pos == std::string::npos) {
        return -1;
    }

    size_t numStart = pos + std::string("signalLevel:").length();
    size_t numEnd = data.find_first_not_of("0123456789", numStart);
    if (numEnd == std::string::npos) {
        numEnd = data.length();
    }
    std::string numStr = data.substr(numStart, numEnd - numStart);
    bool isNum = std::all_of(numStr.begin(), numStr.end(), ::isdigit);
    if (!isNum || numStr.empty()) {
        return -1;
    }
    int32_t level = 0;
    auto [ptr, ec] = std::from_chars(numStr.data(), numStr.data() + numStr.size(), level);
    if (ec != std::errc() || level < SIGNAL_LEVEL_MIN || level > SIGNAL_LEVEL_MAX) {
        return -1;
    }
    return level;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS