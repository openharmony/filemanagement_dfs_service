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

#include "network/softbus/softbus_permission_check.h"

#include "device_manager.h"
#include "dfs_error.h"
#include "dfs_radar.h"
#include "radar_report.h"
#include "dm_device_info.h"
#include "ipc_skeleton.h"
#include "ohos_account_kits.h"
#include "os_account_manager.h"
#include "transport/socket.h"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
constexpr const char *ACCOUNT_ID = "accountId";
static inline const std::string SERVICE_NAME { "ohos.storage.distributedfile.daemon" };
#ifdef SUPPORT_SAME_ACCOUNT
constexpr const char* PARAM_KEY_OS_TYPE = "OS_TYPE";
constexpr int32_t DEVICE_OS_TYPE_OH = 10;
constexpr uint32_t MAX_ONLINE_DEVICE_SIZE = 10000;
#endif
using namespace DistributedHardware;
using namespace FileManagement;
bool SoftBusPermissionCheck::CheckSrcPermission(const std::string &sinkNetworkId, int32_t userId)
{
    LOGI("Begin");
    AccountInfo localAccountInfo;
    if (!GetLocalAccountInfo(localAccountInfo, userId)) {
        LOGE("Get os account data failed");
        return false;
    }
#ifdef SUPPORT_SAME_ACCOUNT
    if (!CheckSrcIsSameAccount(sinkNetworkId, localAccountInfo)) {
        LOGE("Check src same account failed");
        return false;
    }
#endif
    return true;
}

bool SoftBusPermissionCheck::CheckSinkPermission(const AccountInfo &callerAccountInfo)
{
    LOGI("Begin");
    AccountInfo localAccountInfo;
    if (!GetLocalAccountInfo(localAccountInfo)) {
        LOGE("Get local account info failed");
        return false;
    }
#ifdef SUPPORT_SAME_ACCOUNT
    if (!CheckSinkIsSameAccount(callerAccountInfo, localAccountInfo)) {
        LOGE("Check sink same account failed");
        return false;
    }
#endif
    return true;
}

bool SoftBusPermissionCheck::GetLocalAccountInfo(AccountInfo &localAccountInfo, int32_t userId)
{
    if (userId == INVALID_USER_ID) {
        userId = GetCurrentUserId();
        if (userId == INVALID_USER_ID) {
            LOGE("Get current userid failed");
            return false;
        }
    }
    localAccountInfo.userId_ = userId;
#ifdef SUPPORT_SAME_ACCOUNT
    AccountSA::OhosAccountInfo osAccountInfo;
    int ret = AccountSA::OhosAccountKits::GetInstance().GetOhosAccountInfo(osAccountInfo);
    if (ret != 0 || osAccountInfo.uid_ == "") {
        LOGE("Get accountId from Ohos account info failed, ret: %{public}d.", ret);
        RadarParaInfo info = {"GetLocalAccountInfo", ReportLevel::INNER, DfxBizStage::DEFAULT,
            "account", "", ret, "from Ohos account info failed"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return false;
    }
    localAccountInfo.accountId_ = osAccountInfo.uid_;
#endif
    localAccountInfo.tokenId_ = IPCSkeleton::GetSelfTokenID();

    if (!GetLocalNetworkId(localAccountInfo.networkId_)) {
        LOGE("Get local networkid failed");
        return false;
    }
    LOGI("Get local accountinfo success");
    return true;
}

bool SoftBusPermissionCheck::CheckSrcIsSameAccount(const std::string &sinkNetworkId,
    const AccountInfo &localAccountInfo)
{
#ifdef SUPPORT_SAME_ACCOUNT
    DmAccessCaller caller = {
        .accountId = localAccountInfo.accountId_,
        .networkId = localAccountInfo.networkId_,
        .userId = localAccountInfo.userId_,
    };
    DmAccessCallee callee = {
        .networkId = sinkNetworkId
    };
    if (!DeviceManager::GetInstance().CheckSrcIsSameAccount(caller, callee)) {
        LOGE("Check src same account failed");
        RadarParaInfo info = {"CheckSrcIsSameAccount", ReportLevel::INNER, DfxBizStage::DEFAULT,
            "DM", sinkNetworkId, DEFAULT_ERR, "Check src same account fail"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return false;
    }
#endif
    return true;
}

bool SoftBusPermissionCheck::CheckSinkIsSameAccount(const AccountInfo &callerAccountInfo,
    const AccountInfo &calleeAccountInfo)
{
#ifdef SUPPORT_SAME_ACCOUNT
    DmAccessCaller caller = {
        .accountId = callerAccountInfo.accountId_,
        .networkId = callerAccountInfo.networkId_,
        .userId = callerAccountInfo.userId_,
    };
    DmAccessCallee callee = {
        .accountId = calleeAccountInfo.accountId_,
        .networkId = calleeAccountInfo.networkId_,
        .userId = calleeAccountInfo.userId_,
    };
    if (!DeviceManager::GetInstance().CheckSinkIsSameAccount(caller, callee)) {
        LOGE("Check sink same account failed");
        RadarParaInfo info = {"CheckSinkIsSameAccount", ReportLevel::INNER, DfxBizStage::DEFAULT,
            "DM", calleeAccountInfo.networkId_, DEFAULT_ERR, "Check sink same account fail"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return false;
    }
#endif
    return true;
}

bool SoftBusPermissionCheck::CheckSrcAccessControl(const std::string &sinkNetworkId,
    const AccountInfo &localAccountInfo)
{
    DmAccessCaller caller = {
        .accountId = localAccountInfo.accountId_,
        .pkgName = SERVICE_NAME,
        .networkId = localAccountInfo.networkId_,
        .userId = localAccountInfo.userId_,
        .tokenId = localAccountInfo.tokenId_,
    };
    DmAccessCallee callee = {
        .networkId = sinkNetworkId,
    };
    if (!DeviceManager::GetInstance().CheckSrcAccessControl(caller, callee)) {
        LOGE("Check src acl failed");
        RadarParaInfo info = {"CheckSrcAccessControl", ReportLevel::INNER, DfxBizStage::DEFAULT,
            "DM", sinkNetworkId, DEFAULT_ERR, "Check src acl failed"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return false;
    }
    return true;
}

bool SoftBusPermissionCheck::CheckSinkAccessControl(const AccountInfo &callerAccountInfo,
    const AccountInfo &calleeAccountInfo)
{
    DmAccessCaller caller = {
        .accountId = callerAccountInfo.accountId_,
        .pkgName = SERVICE_NAME,
        .networkId = callerAccountInfo.networkId_,
        .userId = callerAccountInfo.userId_,
        .tokenId = callerAccountInfo.tokenId_,
    };
    DmAccessCallee callee = {
        .accountId = calleeAccountInfo.accountId_,
        .networkId = calleeAccountInfo.networkId_,
        .pkgName = SERVICE_NAME,
        .userId = calleeAccountInfo.userId_,
        .tokenId = calleeAccountInfo.tokenId_,
    };
    if (!DeviceManager::GetInstance().CheckSinkAccessControl(caller, callee)) {
        LOGE("Check sink acl failed");
        RadarParaInfo info = {"CheckSinkAccessControl", ReportLevel::INNER, DfxBizStage::DEFAULT,
            "DM", calleeAccountInfo.networkId_, DEFAULT_ERR, "Check sink acl failed"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return false;
    }
    return true;
}

int32_t SoftBusPermissionCheck::GetCurrentUserId()
{
    std::vector<int32_t> userIds{};
    auto ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(userIds);
    if (ret != NO_ERROR || userIds.empty()) {
        LOGE("Query active os account id failed, ret = %{public}d", ret);
        RadarParaInfo info = {"GetCurrentUserId", ReportLevel::INNER, DfxBizStage::DEFAULT,
            "account", "", ret, "GetCurrentUserId failed"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return INVALID_USER_ID;
    }
    return userIds[0];
}

bool SoftBusPermissionCheck::GetLocalNetworkId(std::string &networkId)
{
    DistributedHardware::DmDeviceInfo localDeviceInfo{};
    int errCode = DeviceManager::GetInstance().GetLocalDeviceInfo(SERVICE_NAME, localDeviceInfo);
    if (errCode != 0) {
        LOGE("Get localdeviceInfo failed, ret = %{public}d", errCode);
        RadarParaInfo info = {"GetLocalNetworkId", ReportLevel::INNER, DfxBizStage::DEFAULT,
            "DM", "", errCode, "Get localdeviceInfo failed"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return false;
    }
    networkId = localDeviceInfo.networkId;
    return true;
}

bool SoftBusPermissionCheck::SetAccessInfoToSocket(const int32_t sessionId, int32_t userId)
{
#ifdef SUPPORT_SAME_ACCOUNT
    AccountInfo accountInfo;
    if (!GetLocalAccountInfo(accountInfo, userId)) {
        LOGE("GetOsAccountData failed.");
        return false;
    }
    nlohmann::json jsonObj;
    jsonObj[ACCOUNT_ID] = accountInfo.accountId_;

    SocketAccessInfo accessInfo;
    accessInfo.userId = accountInfo.userId_;
    std::string jsonStr = jsonObj.dump();
    size_t len = jsonStr.size();
    auto charArr = std::make_unique<char[]>(len + 1);
    if (strcpy_s(charArr.get(), len + 1, jsonStr.c_str()) != 0) {
        LOGE("strcpy_s failed, errno = %{public}d", errno);
        return false;
    }
    accessInfo.extraAccessInfo = charArr.get();
    accessInfo.localTokenId = accountInfo.tokenId_;
    if (SetAccessInfo(sessionId, accessInfo) != 0) {
        LOGE("set access info failed");
        RadarParaInfo info = {"SetAccessInfoToSocket", ReportLevel::INNER, DfxBizStage::DEFAULT,
            "softbus", "", DEFAULT_ERR, "set info fail"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return false;
    }
#endif
    return true;
}

bool SoftBusPermissionCheck::TransCallerInfo(SocketAccessInfo *callerInfo,
    AccountInfo &callerAccountInfo, const std::string &networkId)
{
    if (callerInfo == nullptr || callerInfo->extraAccessInfo == nullptr) {
        LOGE("callerInfo or extraAccessInfo is nullptr.");
        return true;
    }
    nlohmann::json jsonObj = nlohmann::json::parse(std::string(callerInfo->extraAccessInfo), nullptr, false);
    if (jsonObj.is_discarded()) {
        LOGE("jsonObj parse failed.");
        RadarParaInfo info = {"TransCallerInfo", ReportLevel::INNER, DfxBizStage::DEFAULT,
            DEFAULT_PKGNAME, networkId, DEFAULT_ERR, "get ext jsonObj parse failed"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return false;
    }
    if (!jsonObj.contains(ACCOUNT_ID) || !jsonObj[ACCOUNT_ID].is_string()) {
        LOGE("get ext jsonObj parse failed.");
        RadarParaInfo info = {"TransCallerInfo", ReportLevel::INNER, DfxBizStage::DEFAULT,
            DEFAULT_PKGNAME, networkId, DEFAULT_ERR, "get ext jsonObj parse failed"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return false;
    }
    callerAccountInfo.accountId_ = jsonObj[ACCOUNT_ID].get<std::string>();
    callerAccountInfo.userId_ = callerInfo->userId;
    callerAccountInfo.networkId_ = networkId;
    LOGI("TransCallerInfo success");
    return true;
}

bool SoftBusPermissionCheck::FillLocalInfo(SocketAccessInfo *localInfo)
{
    if (localInfo == nullptr) {
        LOGE("localInfo is nullptr.");
        return false;
    }
    int32_t userId = GetCurrentUserId();
    if (userId == INVALID_USER_ID) {
        LOGE("get current user id falied");
        return false;
    }
    localInfo->userId = userId;
    localInfo->localTokenId = IPCSkeleton::GetSelfTokenID();
    return true;
}

bool SoftBusPermissionCheck::IsSameAccount(const std::string &networkId)
{
#ifdef SUPPORT_SAME_ACCOUNT
    LOGI("SoftBusPermissionCheck::IsSameAccount called");
    std::vector<DistributedHardware::DmDeviceInfo> deviceList;
    DistributedHardware::DeviceManager::GetInstance().GetTrustedDeviceList(SERVICE_NAME, "", deviceList);
    if (deviceList.size() == 0 || deviceList.size() > MAX_ONLINE_DEVICE_SIZE) {
        LOGE("Trust device list size is invalid, size=%zu", deviceList.size());
        RadarParaInfo info = {"IsSameAccount", ReportLevel::INNER, DfxBizStage::DEFAULT,
            "DM", networkId, DEFAULT_ERR, "list size err"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return false;
    }
    DistributedHardware::DmDeviceInfo deviceInfoTemp;
    for (const auto &deviceInfo : deviceList) {
        if (std::string(deviceInfo.networkId) == networkId) {
            deviceInfoTemp = deviceInfo;
            break;
        }
    }
    if (deviceInfoTemp.authForm != DistributedHardware::DmAuthForm::IDENTICAL_ACCOUNT) {
        LOGE("The source and sink device is not same account, not support.");
        return false;
    }
    if (deviceInfoTemp.extraData.empty()) {
        LOGE("Device extraData is empty");
        return false;
    }
    nlohmann::json entraDataJson = nlohmann::json::parse(deviceInfoTemp.extraData, nullptr, false);
    if (entraDataJson.is_discarded()) {
        LOGE("Device entraDataJson parse failed.");
        RadarParaInfo info = {"IsSameAccount", ReportLevel::INNER, DfxBizStage::DEFAULT,
            DEFAULT_PKGNAME, networkId, DEFAULT_ERR, "entraDataJson parse failed"};
        RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info);
        return false;
    }
    if (!Utils::IsInt32(entraDataJson, PARAM_KEY_OS_TYPE)) {
        LOGE("Device error json int32_t param.");
        return false;
    }
    int32_t osType = entraDataJson[PARAM_KEY_OS_TYPE].get<int32_t>();
    if (osType != DEVICE_OS_TYPE_OH) {
        LOGE("Device os type = %{public}d is not openharmony.", osType);
        return false;
    }
    LOGI("SoftBusPermissionCheck::IsSameAccount end");
#endif
    return true;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS