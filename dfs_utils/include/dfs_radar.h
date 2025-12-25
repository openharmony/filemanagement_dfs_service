/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef DFS_RADAR_H
#define DFS_RADAR_H

#include <string>

namespace OHOS {
namespace FileManagement {
constexpr const char *DEFAULT_PKGNAME = "DFS_SERVICE";
const int32_t DEFAULT_ERR = -1;
const int32_t DEFAULT_VAL = -1;

enum class ReportLevel : int32_t {
    DEFAULT = 0,
    INTERFACE = 1,
    INNER = 2,
};

enum class BizScene : int32_t {
    DEFAULT = 0,
    LINK_CONNECTION,
    GENERATE_DIS_URI,
    FILE_ACCESS,
};

enum class DfxBizStage : int32_t {
    DEFAULT = 0,

    SOFTBUS_OPENP2P = 11,
    KERNEL_NEG = 12,
    MOUNT_DOCS = 13,
    SOFTBUS_CLOSEP2P = 14,
    UNMOUNT_DOCS = 15,

    GENERATE_DIS_URI = 21,
    GENERATE_DIS_MOUNT = 22,

    HMDFS_COPY = 31,
    SOFTBUS_COPY = 32,
    PUSH_ASSERT = 33,
    RECV_ASSERT = 34,
};

enum class LinkStatus : int32_t {
    DISCONNECTED = 0,
    CONNECTED = 1,
};

enum class BizState : int32_t {
    BIZ_STATE_START = 1,
    BIZ_STATE_END = 2,
};

enum class TrustType : int32_t {
    UNKNOWN = 0,
    SAME_TRUST = 1,
    UNTRUST = 2,
    DIFF_TRUST = 3,
};

enum class StageRes : int32_t {
    SUCC = 1,
    FAIL = 2,
};

struct RadarParameter {
    std::string orgPkg;
    std::string hostPkg;
    int32_t userId = DEFAULT_VAL;
    std::string funcName;
    enum ReportLevel faultLevel = ReportLevel::DEFAULT;
    enum BizScene bizScene = BizScene::DEFAULT;
    enum DfxBizStage bizStage = DfxBizStage::DEFAULT;
    std::string toCallPkg;
    enum LinkStatus linkStatus = LinkStatus::DISCONNECTED;
    int32_t fileSize = DEFAULT_VAL;
    int32_t fileCount = DEFAULT_VAL;
    int32_t operateTime = DEFAULT_VAL;
    std::string localUdid;
    std::string localNetId;
    std::string peerUdid;
    std::string peerNetId;
    enum TrustType isTrust = TrustType::UNKNOWN;
    enum StageRes stageRes = StageRes::FAIL;
    int32_t resultCode = DEFAULT_VAL;
    std::string errorInfo;
};

enum RadarStatisticInfoType {
    CONNECT_DFS_SUCC_CNT,
    CONNECT_DFS_FAIL_CNT,
    GENERATE_DIS_URI_SUCC_CNT,
    GENERATE_DIS_URI_FAIL_CNT,
    FILE_ACCESS_SUCC_CNT,
    FILE_ACCESS_FAIL_CNT,
};

struct RadarStatisticInfo {
    uint32_t connectSuccCount;
    uint32_t connectFailCount;
    uint32_t generateUriSuccCount;
    uint32_t generateUriFailCount;
    uint32_t fileAccessSuccCount;
    uint32_t fileAccessFailCount;
    bool empty() const
    {
        return connectSuccCount == 0 && connectFailCount == 0 && generateUriSuccCount == 0 &&
               generateUriFailCount == 0 && fileAccessSuccCount == 0 && fileAccessFailCount == 0;
    }
    void clear()
    {
        connectSuccCount = 0;
        connectFailCount = 0;
        generateUriSuccCount = 0;
        generateUriFailCount = 0;
        fileAccessSuccCount = 0;
        fileAccessFailCount = 0;
    }
};

struct RadarParaInfo {
    std::string funcName;
    enum ReportLevel faultLevel;
    enum DfxBizStage bizStage;
    std::string toCallPkg;
    std::string peerNetId;
    int32_t resultCode;
    std::string errorInfo;
};

struct RadarIDInfo {
    std::string localDeviceNetId;
    std::string localDeviceUdid;
    std::string peerDeviceNetId;
    std::string peerDeviceUdid;
};

class DfsRadar {
public:
    static DfsRadar &GetInstance()
    {
        static DfsRadar instance;
        return instance;
    }

public:
    void ReportLinkConnection(const RadarParaInfo &info, const RadarIDInfo &radarIdInfo);
    void ReportGenerateDisUri(const RadarParaInfo &info, const RadarIDInfo &radarIdInfo);
    void ReportFileAccess(const RadarParaInfo &info, const RadarIDInfo &radarIdInfo);
    void ReportStatistics(const RadarStatisticInfo radarInfo);

private:
    bool RecordFunctionResult(const RadarParameter &parameterRes);
    int32_t GetCurrentUserId();
    int32_t RecordBehavior(const RadarParameter &parRes);

private:
    DfsRadar() = default;
    ~DfsRadar() = default;
    DfsRadar(const DfsRadar &) = delete;
    DfsRadar &operator=(const DfsRadar &) = delete;
    DfsRadar(DfsRadar &&) = delete;
    DfsRadar &operator=(DfsRadar &&) = delete;
};
} // namespace FileManagement
} // namespace OHOS
#endif // DFS_RADAR_H
