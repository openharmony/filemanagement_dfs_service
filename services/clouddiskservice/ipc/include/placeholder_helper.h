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

#ifndef PLACEHOLDER_HELPER_H
#define PLACEHOLDER_HELPER_H

#include <cstdint>
#include <mutex>
#include <string>

namespace OHOS::FileManagement::CloudDiskService {

constexpr const char *CLOUD_DISK_FILE_SYNC_STATE_XATTR = "user.clouddisk.filesyncstate";
constexpr const char *CLOUD_DISK_PLACEHOLDER_COUNT_XATTR = "user.clouddisk.phcount";

constexpr uint8_t PLACEHOLDER_STATE_NONE = 0;
constexpr uint8_t PLACEHOLDER_STATE_UNHYDRATED = 1;
constexpr uint8_t PLACEHOLDER_STATE_PARTIALLY_HYDRATED = 2;
constexpr uint8_t PLACEHOLDER_STATE_FULLY_HYDRATED = 3;
constexpr uint8_t PLACEHOLDER_STATE_HAS_PLACEHOLDER = PLACEHOLDER_STATE_UNHYDRATED;

constexpr uint8_t FILE_SYNC_STATE_SYNC_MASK = 0x1F;
constexpr uint8_t FILE_SYNC_STATE_PLACEHOLDER_MASK = 0xE0;
constexpr uint8_t FILE_SYNC_STATE_PLACEHOLDER_SHIFT = 5;

bool IsPlaceholderState(uint8_t placeholderState);
bool IsValidPlaceholderState(uint8_t placeholderState);
uint8_t GetPlaceholderStateFromFileSyncState(uint8_t state);
uint8_t GetSyncStateFromFileSyncState(uint8_t state);
uint8_t MakeFileSyncState(uint8_t placeholderState, uint8_t syncState);
uint8_t SanitizeDentryPlaceholderState(uint8_t placeholderState);
std::mutex &GetPlaceholderStateMutex();
std::mutex &GetPlaceholderFileMutex(const std::string &path);

int32_t GetFileSyncStateByte(const std::string &path, uint8_t &state, bool missingAsZero);
int32_t GetFilePlaceholderState(const std::string &path, uint8_t &placeholderState);
int32_t GetFilePlaceholderState(int32_t fd, uint8_t &placeholderState);
int32_t SetFilePlaceholderState(const std::string &path, uint8_t newState, uint8_t &oldState);
int32_t SetFilePlaceholderState(int32_t fd, uint8_t newState, uint8_t &oldState);
int32_t SetNewFilePlaceholderState(int32_t fd, uint8_t newState);
int32_t SetFileSyncState(const std::string &path, uint8_t syncState);

int32_t RefreshAncestorPlaceholderCount(const std::string &syncRoot, const std::string &path, int32_t delta);
int32_t RecountPlaceholderState(const std::string &syncRoot,
                                const std::string &subPath,
                                int32_t userId,
                                uint32_t syncFolderIndex);
int32_t UpdateDentryPlaceholderState(int32_t userId,
                                     uint32_t syncFolderIndex,
                                     const std::string &path,
                                     uint8_t placeholderState);

} // namespace OHOS::FileManagement::CloudDiskService

#endif // PLACEHOLDER_HELPER_H
