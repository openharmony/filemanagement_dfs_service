/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "cJSON.h"
#include "channel_manager.h"
#include "device_manager.h"
#include "dfs_error.h"
#include "file_ex.h"
#include "image_source.h"
#include "int_wrapper.h"
#include "locale_config.h"
#include "locale_info.h"
#include "notification_helper.h"
#include "system_notifier.h"
#include "utils_log.h"
#include "want_agent_helper.h"
#include "want_agent_info.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;
using namespace OHOS::FileManagement;

namespace {
constexpr const char *CAPSULE_ICON_PATH = "/system/etc/dfs_service/resources/icon/capsule_icon.png";
constexpr const char *NOTIFICATION_ICON_PATH = "/system/etc/dfs_service/resources/icon/notification_icon.png";
constexpr const char *BUTTON_ICON_PATH = "/system/etc/dfs_service/resources/icon/button_icon.png";
constexpr const int DFS_SERVICE_UID = 1009;
constexpr const int DFS_CAPSULE_TYPE_CODE = 50;
constexpr const int MAX_JSON_SIZE = 100;
constexpr int PLACEHOLDER_LENGTH = 2;

constexpr const char *DFS_LANGUAGE_FILEPATH_PREFIX = "/system/etc/dfs_service/resources/i18/";
constexpr const char *DFS_LANGUAGE_FILEPATH_SUFFIX = ".json";
constexpr const char *DFS_DEFAULT_LANGUAGE = "/system/etc/dfs_service/resources/i18/en-Latn-US.json";

constexpr const char *DFS_DEFAULT_BUTTON_NAME = "dfs_default_button_name";
constexpr const char *NOTIFICATION_TITLE = "notification_title";
constexpr const char *NOTIFICATION_TEXT = "notification_text";
constexpr const char *CAPSULE_TITLE = "capsule_title";
constexpr const char *BUTTON_NAME = "button_name";

constexpr const char *KEY_STRING = "key-value";
constexpr const char *KEY_NAME = "key";
constexpr const char *KEY_VALUE = "value";

constexpr const char *SERVICE_NAME = "ohos.storage.distributedfile.daemon";

static std::string g_sysLanguage = "";
static std::string g_sysRegion = "";
static std::mutex g_resourceMapMutex;
static std::map<std::string, std::string> g_resourceMap;

static std::mutex g_notificationMutex;
static int32_t g_notificationId = 0;

class DFSNotificationLocalLiveViewSubscriber : public Notification::NotificationLocalLiveViewSubscriber {
    /**
     * @brief Called back when the subscriber is connected to the Advanced Notification Service (ANS).
     **/
    void OnConnected() {}

    /**
     * @brief Called back when the subscriber is disconnected from the ANS.
     **/
    void OnDisconnected() {}

    /**
     * @brief Called back when the notificationLocalLiveViewButton clicked.
     **/
    void OnResponse(int32_t notificationId, sptr<NotificationButtonOption> buttonOption)
    {
        LOGI("DFSNotificationLocalLiveViewSubscriber OnResponse, notificationId: %{public}d", notificationId);
        SystemNotifier::GetInstance().DestroyNotifyByNotificationId(notificationId);
    }

    /**
     * @brief Called back when connection to the ANS has died.
     **/
    void OnDied()
    {
        LOGE("DFSNotificationLocalLiveViewSubscriber OnDied enter");
    }
};

class DfsNotificationSubscriber : public Notification::NotificationSubscriber {
    void OnConnected() {}
    void OnDisconnected() {}
    void OnUpdate(const std::shared_ptr<Notification::NotificationSortingMap> &sortingMap) {}
    void OnDoNotDisturbDateChange(const std::shared_ptr<Notification::NotificationDoNotDisturbDate> &date) {}
    void OnEnabledNotificationChanged(
        const std::shared_ptr<Notification::EnabledNotificationCallbackData> &callbackData) {}
    void OnDied()
    {
        LOGE("DfsNotificationSubscriber OnDied enter");
    }
    void OnCanceled(const std::shared_ptr<OHOS::Notification::Notification> &request,
                    const std::shared_ptr<Notification::NotificationSortingMap> &sortingMap,
                    int deleteReason)
    {
        int creatorUid = request->GetUid();
        int notificationId = request->GetId();
        LOGI("Oncanceled, creatorUid = %{public}d, notificationId = %{public}d, deleteReason = %{public}d", creatorUid,
             notificationId, deleteReason);
        SystemNotifier::GetInstance().DestroyNotifyByNotificationId(notificationId);
    }
    void OnConsumed(const std::shared_ptr<OHOS::Notification::Notification> &notification,
                    const std::shared_ptr<Notification::NotificationSortingMap> &sortingMap)
    {}
    void OnBadgeChanged(const std::shared_ptr<Notification::BadgeNumberCallbackData> &badgeData) {}
    void OnBadgeEnabledChanged(const sptr<Notification::EnabledNotificationCallbackData> &callbackData) {}
    void OnBatchCanceled(const std::vector<std::shared_ptr<OHOS::Notification::Notification>> &requestList,
                         const std::shared_ptr<Notification::NotificationSortingMap> &sortingMap,
                         int32_t deleteReason)
    {}
};

static std::shared_ptr<DFSNotificationLocalLiveViewSubscriber> g_notificationLocalLiveViewSubscriber =
    std::make_shared<DFSNotificationLocalLiveViewSubscriber>();

static std::shared_ptr<DfsNotificationSubscriber> g_notificationSubscriber =
    std::make_shared<DfsNotificationSubscriber>();
} // namespace

static Notification::NotificationCapsule CreateNotificationCapsule(std::shared_ptr<Media::PixelMap> pixelMap,
                                                                   const string title)
{
    LOGI("CreateNotificationCapsule start");
    auto capsule = Notification::NotificationCapsule();
    capsule.SetIcon(pixelMap);
    capsule.SetTitle(title);
    capsule.SetBackgroundColor("#2E3033");
    return capsule;
}

static Notification::NotificationLocalLiveViewButton CreateNotificationLocalLiveViewButton(
    std::shared_ptr<Media::PixelMap> pixelMap)
{
    LOGI("CreateNotificationLocalLiveViewButton start");
    Notification::NotificationLocalLiveViewButton button;
    button.addSingleButtonName(DFS_DEFAULT_BUTTON_NAME);
    button.addSingleButtonIcon(pixelMap);
    return button;
}

static void SetActionButton(const std::string &buttonName, Notification::NotificationRequest &request)
{
    auto want = std::make_shared<AAFwk::Want>();
    std::vector<std::shared_ptr<AAFwk::Want>> wants;
    wants.push_back(want);
    std::vector<AbilityRuntime::WantAgent::WantAgentConstant::Flags> flags;
    flags.push_back(AbilityRuntime::WantAgent::WantAgentConstant::Flags::CONSTANT_FLAG);
    AbilityRuntime::WantAgent::WantAgentInfo wantAgentInfo(
        0, AbilityRuntime::WantAgent::WantAgentConstant::OperationType::UNKNOWN_TYPE, flags, wants, nullptr);
    auto wantAgentDeal = AbilityRuntime::WantAgent::WantAgentHelper::GetWantAgent(wantAgentInfo);
    std::shared_ptr<Notification::NotificationActionButton> actionButtonDeal =
        Notification::NotificationActionButton::Create(nullptr, buttonName, wantAgentDeal);
    if (actionButtonDeal == nullptr) {
        LOGE("get notification actionButton nullptr");
        return;
    }
    request.AddActionButton(actionButtonDeal);
}

void SystemNotifier::UpdateResourceMap(const std::string &resourcePath)
{
    std::lock_guard<std::mutex> lock(g_resourceMapMutex);
    LOGI("Reading resource string from json config.");
    std::string content;
    if (!std::filesystem::exists(resourcePath)) {
        LOGE("hava not found now language json file, use Eng instead.");
        LoadStringFromFile(DFS_DEFAULT_LANGUAGE, content);
    } else {
        LoadStringFromFile(resourcePath, content);
    }

    cJSON *json = cJSON_Parse(content.c_str());
    if (json == nullptr) {
        LOGE("json nullptr.");
        return;
    }

    cJSON *resJson = cJSON_GetObjectItemCaseSensitive(json, KEY_STRING);
    if (resJson == nullptr || cJSON_GetArraySize(resJson) > MAX_JSON_SIZE) {
        LOGE("fail to parse res json");
        cJSON_Delete(json);
        return;
    }

    g_resourceMap.clear();
    cJSON *resJsonEach = nullptr;
    cJSON_ArrayForEach(resJsonEach, resJson)
    {
        cJSON *key = cJSON_GetObjectItemCaseSensitive(resJsonEach, KEY_NAME);
        if (key == nullptr || !cJSON_IsString(key)) {
            LOGE("json param not string");
            cJSON_Delete(json);
            return;
        }

        cJSON *value = cJSON_GetObjectItemCaseSensitive(resJsonEach, KEY_VALUE);
        if (value == nullptr || !cJSON_IsString(value)) {
            LOGE("json param not string");
            cJSON_Delete(json);
            return;
        }

        g_resourceMap.emplace(std::pair<std::string, std::string>(key->valuestring, value->valuestring));
    }
    cJSON_Delete(json);
}

void SystemNotifier::UpdateResourceMapByLanguage()
{
    std::string curSysLanguage = Global::I18n::LocaleConfig::GetSystemLanguage();
    std::string curSysRegion = Global::I18n::LocaleConfig::GetSystemRegion();
    if (g_sysLanguage == curSysLanguage && curSysRegion == g_sysRegion) {
        LOGW("same language environment[%{public}s], region[%{public}s] ,no need to update resource map.",
             curSysLanguage.c_str(), curSysRegion.c_str());
        return;
    }

    LOGI("current system language[%{public}s], region[%{public}s] changes, should update resource map",
         curSysLanguage.c_str(), curSysRegion.c_str());
    g_sysLanguage = curSysLanguage;
    g_sysRegion = curSysRegion;

    std::string filePath = DFS_LANGUAGE_FILEPATH_PREFIX + g_sysLanguage + DFS_LANGUAGE_FILEPATH_SUFFIX;
    UpdateResourceMap(filePath);
}

template<typename... Args>
std::string SystemNotifier::GetKeyValue(const std::string &key, Args &&...args)
{
    std::lock_guard<std::mutex> lock(g_resourceMapMutex);
    auto it = g_resourceMap.find(key);
    if (it == g_resourceMap.end()) {
        return "";
    }
    std::string value = it->second;

    if constexpr (sizeof...(args) == 0) {
        return value;
    }

    std::vector<std::string> replacements = {std::forward<Args>(args)...};
    size_t argIndex = 0;
    size_t pos = 0;

    while ((pos = value.find("{}", pos)) != std::string::npos) {
        if (argIndex >= replacements.size()) {
            break;
        }

        value.replace(pos, PLACEHOLDER_LENGTH, replacements[argIndex]);
        pos += replacements[argIndex].length();
        argIndex++;
    }

    return value;
}

int32_t SystemNotifier::CreateNotification(const std::string &networkId)
{
    LOGI("CreateNotification enter");
    {
        std::shared_lock<std::shared_mutex> readLock(mutex);
        if (notificationMap_.find(networkId) != notificationMap_.end()) {
            LOGI("Notification %{public}.6s already exists.", networkId.c_str());
            return E_OK;
        }
    }
    UpdateResourceMapByLanguage();
    std::shared_ptr<Notification::NotificationNormalContent> content =
        std::make_shared<Notification::NotificationNormalContent>();
    content->SetTitle(GetKeyValue(NOTIFICATION_TITLE));
    content->SetText(GetKeyValue(NOTIFICATION_TEXT, GetRemoteDeviceName(networkId)));

    std::shared_ptr<Notification::NotificationContent> pcContent =
        std::make_shared<Notification::NotificationContent>(content);

    int32_t notificationId = GenerateNextNotificationId();
    Notification::NotificationRequest request;
    request.SetNotificationId(notificationId);
    request.SetUnremovable(true);
    request.SetCreatorUid(DFS_SERVICE_UID);
    request.SetContent(pcContent);
    request.SetLittleIcon(notificationIconPixelMap_);
    request.SetBadgeIconStyle(Notification::NotificationRequest::BadgeStyle::LITTLE);
    SetActionButton(GetKeyValue(BUTTON_NAME), request);

    auto ret = Notification::NotificationHelper::PublishNotification(request);
    if (ret != E_OK) {
        LOGE("DFS service publish notification result = %{public}d", ret);
        return static_cast<int32_t>(ret);
    }
    std::unique_lock<std::shared_mutex> write_lock(mutex_);
    notificationMap_[networkId] = notificationId;
    LOGI("DFS service publish CreateNotification result = %{public}d", ret);
    return E_OK;
}

int32_t SystemNotifier::CreateLocalLiveView(const std::string &networkId)
{
    LOGI("CreateLocalLiveView enter");
    {
        std::shared_lock<std::shared_mutex> readLock(mutex);
        if (notificationMap_.find(networkId) != notificationMap_.end()) {
            LOGI("Notification %{public}.6s already exists.", networkId.c_str());
            return E_OK;
        }
    }
    UpdateResourceMapByLanguage();
    std::shared_ptr<Notification::NotificationLocalLiveViewContent> localLiveViewContent =
        std::make_shared<Notification::NotificationLocalLiveViewContent>();
    localLiveViewContent->SetTitle(GetKeyValue(NOTIFICATION_TITLE));
    localLiveViewContent->SetText(GetKeyValue(NOTIFICATION_TEXT, GetRemoteDeviceName(networkId)));
    localLiveViewContent->SetType(DFS_CAPSULE_TYPE_CODE);

    auto capsule = CreateNotificationCapsule(capsuleIconPixelMap_, GetKeyValue(CAPSULE_TITLE));
    localLiveViewContent->SetCapsule(capsule);
    localLiveViewContent->addFlag(Notification::NotificationLocalLiveViewContent::LiveViewContentInner::CAPSULE);

    auto button = CreateNotificationLocalLiveViewButton(buttonIconPixelMap_);
    localLiveViewContent->SetButton(button);
    localLiveViewContent->addFlag(Notification::NotificationLocalLiveViewContent::LiveViewContentInner::BUTTON);

    std::shared_ptr<Notification::NotificationContent> content =
        std::make_shared<Notification::NotificationContent>(localLiveViewContent);

    Notification::NotificationRequest request;
    int32_t notificationId = GenerateNextNotificationId();
    request.SetNotificationId(notificationId);
    request.SetSlotType(Notification::NotificationConstant::SlotType::LIVE_VIEW);
    request.SetCreatorUid(DFS_SERVICE_UID);
    request.SetContent(content);
    request.SetLittleIcon(notificationIconPixelMap_);

    auto ret = Notification::NotificationHelper::PublishNotification(request);
    if (ret != E_OK) {
        LOGE("DFS service publish localLiveView result = %{public}d", ret);
        return static_cast<int32_t>(ret);
    }
    std::unique_lock<std::shared_mutex> write_lock(mutex_);
    notificationMap_[networkId] = notificationId;
    LOGI("DFS service publish localLiveView result = %{public}d", ret);
    return E_OK;
}

int32_t SystemNotifier::DestroyNotifyByNotificationId(int32_t notificationId)
{
    LOGI("DestroyNotifyByNotificationId for notificationId: %{public}d", notificationId);
    std::string networkId = "";
    {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        for (auto it = notificationMap_.begin(); it != notificationMap_.end();) {
            if (it->second == notificationId) {
                networkId = it->first;
                LOGI("Removed notification mapping for notificationId: %{public}d (networkId: %{public}.6s)",
                     notificationId, networkId.c_str());
                it = notificationMap_.erase(it);
            } else {
                ++it;
            }
        }
    }

    if (networkId.empty()) {
        LOGE("can not find %{public}.6s in map!", networkId.c_str());
        return ERR_DATA_INVALID;
    }

    int32_t ret = Notification::NotificationHelper::CancelNotification(notificationId);
    LOGI("DestroyNotification (id: %{public}d), result: %{public}d", notificationId, ret);

    ret = DisconnectByNetworkId(networkId);
    LOGI("DisconnectByNetworkId networkId = %{public}.6s , ret = %{public}d", networkId.c_str(), ret);

    return ret;
}

int32_t SystemNotifier::DestroyNotifyByNetworkId(const std::string &networkId)
{
    LOGI("DestroyNotifyByNetworkId for networkId: %{public}.6s", networkId.c_str());
    int32_t notificationId = 0;
    {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        auto it = notificationMap_.find(networkId);
        if (it != notificationMap_.end()) {
            LOGI("Removed notification mapping for networkId: %{public}.6s", networkId.c_str());
            notificationId = it->second;
            notificationMap_.erase(it);
        } else {
            LOGW("No notification mapping found for networkId: %{public}.6s", networkId.c_str());
            return ERR_BAD_VALUE;
        }
    }

    auto ret = Notification::NotificationHelper::CancelNotification(notificationId);
    LOGI("DestroyNotifyByNetworkId (notificationId: %{public}d), result: %{public}d", notificationId, ret);

    ret = DisconnectByNetworkId(networkId);
    LOGI("DisconnectByNetworkId networkId = %{public}.6s , ret = %{public}d", networkId.c_str(), ret);
    return ret;
}

int32_t SystemNotifier::DisconnectByNetworkId(const std::string &networkId)
{
    LOGI("DisconnectByNetworkId enter, networkId is %{public}.6s", networkId.c_str());
    ControlCmd request;
    request.msgType = CMD_ACTIVE_DISCONNECT;
    std::string srcNetworkId;
    auto result = DistributedHardware::DeviceManager::GetInstance().GetLocalDeviceNetWorkId(SERVICE_NAME, srcNetworkId);
    if (result != E_OK || srcNetworkId.empty()) {
        LOGE("Failed to get local device network ID.");
        return ERR_BAD_VALUE;
    }
    request.networkId = srcNetworkId;

    int32_t ret = ChannelManager::GetInstance().NotifyClient(networkId, request);
    LOGI("DisconnectByNetworkId end. networkId = %{public}.6s ,ret = %{public}d", networkId.c_str(), ret);
    return ret;
}

int32_t SystemNotifier::GenerateNextNotificationId()
{
    std::lock_guard<std::mutex> lock(g_notificationMutex);
    if (g_notificationId == INT32_MAX) {
        g_notificationId = 0;
    }
    return ++g_notificationId;
}

bool SystemNotifier::GetPixelMap(const std::string &path, std::shared_ptr<Media::PixelMap> &pixelMap)
{
    if (pixelMap != nullptr) {
        LOGI("dfs icon pixel map already exists.");
        return false;
    }

    if (!std::filesystem::exists(path)) {
        LOGE("dfs icon file path not exists. %{public}s.", path.c_str());
        pixelMap = nullptr;
        return false;
    }

    uint32_t errorCode = 0;
    Media::SourceOptions opts;
    opts.formatHint = "image/png";
    std::unique_ptr<Media::ImageSource> imageSource = Media::ImageSource::CreateImageSource(path, opts, errorCode);
    if (imageSource == nullptr) {
        LOGE("imageSource nullptr");
        pixelMap = nullptr;
        return false;
    }

    Media::DecodeOptions decodeOpts;
    std::unique_ptr<Media::PixelMap> pixelMapTmp = imageSource->CreatePixelMap(decodeOpts, errorCode);
    if (!pixelMapTmp || errorCode != 0) {
        LOGE("CreatePixelMap failed, error: %{public}u", errorCode);
        pixelMap = nullptr;
        return false;
    }

    pixelMap = std::move(pixelMapTmp);
    return true;
}

SystemNotifier::SystemNotifier()
{
    LOGI("start init Notifer");

    GetPixelMap(CAPSULE_ICON_PATH, capsuleIconPixelMap_);
    GetPixelMap(NOTIFICATION_ICON_PATH, notificationIconPixelMap_);
    GetPixelMap(BUTTON_ICON_PATH, buttonIconPixelMap_);

    auto ret = Notification::NotificationHelper::SubscribeNotification(*g_notificationSubscriber);
    LOGI("SubscribeNotification result: %{public}d", ret);

    ret = Notification::NotificationHelper::SubscribeLocalLiveViewNotification(*g_notificationLocalLiveViewSubscriber);
    LOGI("SubscribeLocalLiveViewNotification result: %{public}d", ret);

    UpdateResourceMapByLanguage();

    LOGI("end init SystemNotifier");
}

std::string SystemNotifier::GetRemoteDeviceName(const std::string &networkId)
{
    std::string deviceName;
    int32_t ret = DistributedHardware::DeviceManager::GetInstance().GetDeviceName(SERVICE_NAME, networkId, deviceName);
    if (ret != E_OK) {
        LOGE("GetDeviceName failed. ret = %{public}d", ret);
    }
    LOGI("networkId is %{public}.6s, deviceName is %{public}s", networkId.c_str(), deviceName.c_str());
    return deviceName;
}

} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS