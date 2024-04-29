/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef SOFTBUS_AGENT_H
#define SOFTBUS_AGENT_H

#include <map>
#include <mutex>

#include "inner_socket.h"
#include "network/network_agent_template.h"
#include "transport/socket.h"
#include "transport/trans_type.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class SoftbusAgent final : public NetworkAgentTemplate, public std::enable_shared_from_this<SoftbusAgent> {
public:
    explicit SoftbusAgent(std::weak_ptr<MountPoint> mountPoint);
    ~SoftbusAgent() = default;
    void OnSessionOpened(const int32_t sessionId, PeerSocketInfo info);
    void OnSessionClosed(int32_t sessionId, const std::string peerDeviceId);
    static bool IsSameAccount(const std::string &networkId);
protected:
    void JoinDomain() override;
    void QuitDomain() override;
    void StopTopHalf() override;
    void StopBottomHalf() override;
    void OpenSession(const DeviceInfo &info, const uint8_t &linkType) override;
    void OpenApSession(const DeviceInfo &info, const uint8_t &linkType) override;
    void CloseSession(std::shared_ptr<BaseSession> session) override;

private:
    std::mutex serverIdMapMutex_;
    std::map<std::string, int32_t> serverIdMap_;
    bool IsContinueRetry(const std::string &cid);
    std::map<std::string, int32_t> OpenSessionRetriedTimesMap_;

    std::string sessionName_;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // SOFTBUS_AGENT_H