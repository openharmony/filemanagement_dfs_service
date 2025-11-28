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
#include "dfs_radar.h"

#include "device_manager.h"
#include "dfsu_access_token_helper.h"
#include "dfs_error.h"
#include "hisysevent.h"
#include "ipc_skeleton.h"
#include "utils_log.h"
#include "os_account_manager.h"
#include "softbus_bus_center.h"

#include <iomanip>

namespace OHOS {
namespace FileManagement {
using namespace std;
constexpr char DFS_SERVICE_DOMAIN[] = "FILEMANAGEMENT";
constexpr const char *DFS_SERVICE_BEHAVIOR  = "DFS_SERVICE_BEHAVIOR";
constexpr const char *DFS_SERVICE_STATISTIC = "DFS_SERVICE_STATISTIC";
constexpr const char *DFS_SERVICE_NAME = "ohos.storage.distributedfile.daemon";
const int32_t DEFAULT_SIZE = -1;

void DfsRadar::ReportLinkConnection(const RadarParaInfo &info)
{
    auto callingUid = IPCSkeleton::GetCallingUid();
    RadarParameter param = {
        .orgPkg = DEFAULT_PKGNAME,
        .hostPkg = to_string(callingUid),
        .funcName = info.funcName,
        .faultLevel = info.faultLevel,
        .bizScene = BizScene::LINK_CONNECTION,
        .bizStage = info.bizStage,
        .toCallPkg = info.toCallPkg,
        .fileSize = DEFAULT_SIZE,
        .fileCount = DEFAULT_SIZE,
        .operateTime = DEFAULT_SIZE,
        .resultCode = info.resultCode,
        .errorInfo = info.errorInfo
    };
    GetLocalNetIdAndUdid(param);
    GetPeerUdid(param, info.peerNetId);
    RecordFunctionResult(param);
}

void DfsRadar::ReportLinkConnectionEx(const RadarParaInfo &info)
{
    string bundleName;
    DfsuAccessTokenHelper::GetCallerBundleName(bundleName);
    RadarParameter param = {
        .orgPkg = DEFAULT_PKGNAME,
        .hostPkg = bundleName,
        .funcName = info.funcName,
        .faultLevel = info.faultLevel,
        .bizScene = BizScene::LINK_CONNECTION,
        .bizStage = info.bizStage,
        .toCallPkg = info.toCallPkg,
        .fileSize = DEFAULT_SIZE,
        .fileCount = DEFAULT_SIZE,
        .operateTime = DEFAULT_SIZE,
        .resultCode = info.resultCode,
        .errorInfo = info.errorInfo
    };
    GetLocalNetIdAndUdid(param);
    GetPeerUdid(param, info.peerNetId);
    RecordFunctionResult(param);
}

void DfsRadar::ReportGenerateDisUri(const RadarParaInfo &info)
{
    auto callingUid = IPCSkeleton::GetCallingUid();
    RadarParameter param = {
        .orgPkg = DEFAULT_PKGNAME,
        .hostPkg = to_string(callingUid),
        .funcName = info.funcName,
        .faultLevel = info.faultLevel,
        .bizScene = BizScene::GENERATE_DIS_URI,
        .bizStage = info.bizStage,
        .toCallPkg = info.toCallPkg,
        .fileSize = DEFAULT_SIZE,
        .fileCount = DEFAULT_SIZE,
        .operateTime = DEFAULT_SIZE,
        .resultCode = info.resultCode,
        .errorInfo = info.errorInfo
    };
    GetLocalNetIdAndUdid(param);
    GetPeerUdid(param, info.peerNetId);
    RecordFunctionResult(param);
}

void DfsRadar::ReportFileAccess(const RadarParaInfo &info)
{
    auto callingUid = IPCSkeleton::GetCallingUid();
    RadarParameter param = {
        .orgPkg = DEFAULT_PKGNAME,
        .hostPkg = to_string(callingUid),
        .funcName = info.funcName,
        .faultLevel = info.faultLevel,
        .bizScene = BizScene::FILE_ACCESS,
        .bizStage = info.bizStage,
        .toCallPkg = info.toCallPkg,
        .fileSize = DEFAULT_SIZE,
        .fileCount = DEFAULT_SIZE,
        .operateTime = DEFAULT_SIZE,
        .resultCode = info.resultCode,
        .errorInfo = info.errorInfo
    };
    GetLocalNetIdAndUdid(param);
    GetPeerUdid(param, info.peerNetId);
    RecordFunctionResult(param);
}

bool DfsRadar::RecordFunctionResult(const RadarParameter &parRes)
{
    int32_t res = E_OK;
    if (parRes.resultCode != E_OK) {
        res = HiSysEventWrite(
            DFS_SERVICE_DOMAIN,
            DFS_SERVICE_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", parRes.orgPkg,
            "HOST_PKG", parRes.hostPkg,
            "USER_ID", GetCurrentUserId(),
            "FUNC", parRes.funcName,
            "FAULT_LEVEL", static_cast<int32_t>(parRes.faultLevel),
            "BIZ_SCENE", static_cast<int32_t>(parRes.bizScene),
            "BIZ_STAGE", static_cast<int32_t>(parRes.bizStage),
            "TO_CALL_PKG", parRes.toCallPkg,
            "LINK_STATUS", static_cast<int32_t>(LinkStatus::CONNECTED),
            "FILE_SIZE", parRes.fileSize,
            "FILE_COUNT", parRes.fileCount,
            "OPERATE_TIME", parRes.operateTime,
            "LOCAL_UDID", parRes.localUdid,
            "LOCAL_NET_ID", parRes.localNetId,
            "PEER_UDID", parRes.peerUdid,
            "PEER_NET_ID", parRes.peerNetId,
            "IS_TRUST", static_cast<int32_t>(TrustType::SAME_TRUST),
            "STAGE_RES", static_cast<int32_t>(StageRes::FAIL),
            "ERROR_CODE", parRes.resultCode,
            "ERROR_INFO", parRes.errorInfo,
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END));
    }
    if (res != E_OK) {
        LOGE("DfsRadar Error, res :%{public}d", res);
        return false;
    }
    return true;
}

void DfsRadar::ReportStatistics(const RadarStatisticInfo radarInfo)
{
    int32_t res = HiSysEventWrite(
        DFS_SERVICE_DOMAIN,
        DFS_SERVICE_STATISTIC,
        HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "ORG_PKG", DEFAULT_PKGNAME,
        "USER_ID", GetCurrentUserId(),
        "BIZ_SCENE", static_cast<int32_t>(BizScene::DEFAULT),
        "BIZ_STAGE", static_cast<int32_t>(DfxBizStage::DEFAULT),
        "STAGE_RES", static_cast<int32_t>(StageRes::SUCC),
        "CONNECT_DFS_SUCC_CNT", radarInfo.connectSuccCount,
        "CONNECT_DFS_FAIL_CNT", radarInfo.connectFailCount,
        "GENERATE_DIS_URI_SUCC_CNT", radarInfo.generateUriSuccCount,
        "GENERATE_DIS_URI_FAIL_CNT", radarInfo.generateUriFailCount,
        "FILE_ACCESS_SUCC_CNT", radarInfo.fileAccessSuccCount,
        "FILE_ACCESS_FAIL_CNT", radarInfo.fileAccessFailCount);
    if (res != E_OK) {
        LOGE("DfsRadar Error, res :%{public}d", res);
    }
}

int32_t DfsRadar::GetCurrentUserId()
{
    std::vector<int32_t> userIds{};
    auto ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(userIds);
    if (ret != NO_ERROR || userIds.empty()) {
        LOGE("Query active os account id failed, ret = %{public}d", ret);
        return DEFAULT_ERR;
    }
    return userIds[0];
}

void DfsRadar::GetLocalNetIdAndUdid(RadarParameter &parameterRes)
{
    std::string localNetworkId = "";
    std::string localUdid = "";
    if (DistributedHardware::DeviceManager::GetInstance()
        .GetLocalDeviceNetWorkId(DFS_SERVICE_NAME, localNetworkId) != 0) {
        LOGE("GetLocalDeviceNetWorkId failed");
        return;
    }

    uint8_t udid[65] = { 0 };
    int32_t ret = GetNodeKeyInfo(DFS_SERVICE_NAME, localNetworkId.c_str(),
        NodeDeviceInfoKey::NODE_KEY_UDID, udid, sizeof(udid));
    if (ret != E_OK) {
        LOGE("GetNodeKeyInfo failed");
    }
    localUdid = reinterpret_cast<char *>(udid);
    parameterRes.localNetId = GetAnonymStr(localNetworkId);
    parameterRes.localUdid = GetAnonymStr(localUdid);
}

void DfsRadar::GetPeerUdid(RadarParameter &parameterRes, const std::string &networkId)
{
    if (networkId.empty()) {
        parameterRes.peerUdid = "";
        parameterRes.peerNetId = "";
        return;
    }
    uint8_t udid[65] = { 0 };
    int32_t ret = GetNodeKeyInfo(DFS_SERVICE_NAME, networkId.c_str(),
        NodeDeviceInfoKey::NODE_KEY_UDID, udid, sizeof(udid));
    if (ret != E_OK) {
        LOGE("GetNodeKeyInfo failed");
        return;
    }
    std::string peerUdid = reinterpret_cast<char *>(udid);
    parameterRes.peerNetId = GetAnonymStr(networkId);
    parameterRes.peerUdid = GetAnonymStr(peerUdid);
}

std::string DfsRadar::GetAnonymStr(const std::string &value)
{
    const size_t minLength = 10;
    if (value.empty() || value.length() < minLength) {
        return "";
    }
    const size_t prefixLength = 5;
    const size_t suffixLength = 5;
    return value.substr(0, prefixLength) + "**" + value.substr(value.length() - suffixLength);
}
} // namespace FileManagement
} // namespace OHOS
