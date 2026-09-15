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

#include "placeholder_helper.h"

#include <array>
#include <cerrno>
#include <climits>
#include <cstring>
#include <dirent.h>
#include <functional>
#include <mutex>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <unistd.h>

#include "cloud_disk_service_error.h"
#include "cloud_disk_service_metafile.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDiskService {
std::mutex &GetPlaceholderStateMutex()
{
    static std::mutex mutex;
    return mutex;
}

std::mutex &GetPlaceholderFileMutex(const std::string &path)
{
    constexpr size_t FILE_LOCK_BUCKET_COUNT = 64;
    static std::array<std::mutex, FILE_LOCK_BUCKET_COUNT> mutexes;
    return mutexes[std::hash<std::string>{}(path) % mutexes.size()];
}

namespace {
constexpr size_t PLACEHOLDER_COUNT_SIZE = sizeof(uint32_t);
constexpr uint32_t BYTE_MASK = UCHAR_MAX;
constexpr size_t BITS_PER_BYTE = CHAR_BIT;
constexpr uint32_t DENTRY_PLACEHOLDER_RESERVED_INDEX = 0;
constexpr int32_t PLACEHOLDER_DELTA_INCREASE = 1;
constexpr int32_t PLACEHOLDER_DELTA_DECREASE = -1;

bool IsSameOrUnderPath(const std::string &root, const std::string &path)
{
    if (root.empty() || path.empty()) {
        return false;
    }
    if (path == root) {
        return true;
    }
    std::string prefix = root;
    if (prefix.back() != '/') {
        prefix += "/";
    }
    return path.size() > prefix.size() && path.compare(0, prefix.size(), prefix) == 0;
}

std::string GetParentPath(const std::string &path)
{
    size_t pos = path.find_last_of('/');
    if (pos == std::string::npos) {
        return "";
    }
    if (pos == 0) {
        return "/";
    }
    return path.substr(0, pos);
}

std::string GetFileName(const std::string &path)
{
    size_t pos = path.find_last_of('/');
    if (pos == std::string::npos) {
        return path;
    }
    return path.substr(pos + 1);
}

int32_t ReadFileSyncStateByteNoLock(const std::string &path, uint8_t &state, bool missingAsZero)
{
    ssize_t size = getxattr(path.c_str(), CLOUD_DISK_FILE_SYNC_STATE_XATTR, &state, sizeof(state));
    if (size == static_cast<ssize_t>(sizeof(state))) {
        return E_OK;
    }
    if (size < 0) {
        int32_t err = errno;
        if (err == ENODATA && missingAsZero) {
            state = 0;
            return E_OK;
        }
        return err;
    }
    return EINVAL;
}

int32_t ReadFileSyncStateByteNoLock(int32_t fd, uint8_t &state, bool missingAsZero)
{
    ssize_t size = fgetxattr(fd, CLOUD_DISK_FILE_SYNC_STATE_XATTR, &state, sizeof(state));
    if (size == static_cast<ssize_t>(sizeof(state))) {
        return E_OK;
    }
    if (size < 0) {
        int32_t err = errno;
        if (err == ENODATA && missingAsZero) {
            state = 0;
            return E_OK;
        }
        return err;
    }
    return EINVAL;
}

int32_t WriteFileSyncStateByteNoLock(const std::string &path, uint8_t state)
{
    if (setxattr(path.c_str(), CLOUD_DISK_FILE_SYNC_STATE_XATTR, &state, sizeof(state), 0) != 0) {
        return errno;
    }
    return E_OK;
}

int32_t WriteFileSyncStateByteNoLock(int32_t fd, uint8_t state)
{
    if (fsetxattr(fd, CLOUD_DISK_FILE_SYNC_STATE_XATTR, &state, sizeof(state), 0) != 0) {
        return errno;
    }
    return E_OK;
}

std::array<uint8_t, PLACEHOLDER_COUNT_SIZE> EncodePlaceholderCount(uint32_t count)
{
    std::array<uint8_t, PLACEHOLDER_COUNT_SIZE> data{};
    for (size_t index = 0; index < data.size(); ++index) {
        data[index] = static_cast<uint8_t>((count >> (index * BITS_PER_BYTE)) & BYTE_MASK);
    }
    return data;
}

uint32_t DecodePlaceholderCount(const std::array<uint8_t, PLACEHOLDER_COUNT_SIZE> &data)
{
    uint32_t count = 0;
    for (size_t index = 0; index < data.size(); ++index) {
        count |= static_cast<uint32_t>(data[index]) << (index * BITS_PER_BYTE);
    }
    return count;
}

int32_t ReadPlaceholderCountNoLock(const std::string &path, uint32_t &count)
{
    std::array<uint8_t, PLACEHOLDER_COUNT_SIZE> data{};
    ssize_t size = getxattr(path.c_str(), CLOUD_DISK_PLACEHOLDER_COUNT_XATTR, data.data(), data.size());
    if (size == static_cast<ssize_t>(data.size())) {
        count = DecodePlaceholderCount(data);
        return E_OK;
    }
    if (size < 0) {
        int32_t err = errno;
        if (err == ENODATA) {
            count = 0;
            return E_OK;
        }
        return err;
    }
    return EINVAL;
}

int32_t WritePlaceholderCountNoLock(const std::string &path, uint32_t count)
{
    if (count == 0) {
        if (removexattr(path.c_str(), CLOUD_DISK_PLACEHOLDER_COUNT_XATTR) != 0 && errno != ENODATA) {
            return errno;
        }
        return E_OK;
    }
    auto data = EncodePlaceholderCount(count);
    if (setxattr(path.c_str(), CLOUD_DISK_PLACEHOLDER_COUNT_XATTR, data.data(), data.size(), 0) != 0) {
        return errno;
    }
    return E_OK;
}

int32_t UpdateDirectoryCountAndState(const std::string &path, int32_t delta, bool &shouldPropagate)
{
    shouldPropagate = false;
    if (delta != PLACEHOLDER_DELTA_INCREASE && delta != PLACEHOLDER_DELTA_DECREASE) {
        return EINVAL;
    }

    std::lock_guard<std::mutex> lock(GetPlaceholderStateMutex());
    uint32_t oldCount = 0;
    int32_t ret = ReadPlaceholderCountNoLock(path, oldCount);
    if (ret != E_OK) {
        return ret;
    }

    uint32_t newCount = oldCount;
    if (delta == PLACEHOLDER_DELTA_INCREASE) {
        if (oldCount == UINT32_MAX) {
            return EOVERFLOW;
        }
        newCount = oldCount + 1;
    } else if (oldCount > 0) {
        newCount = oldCount - 1;
    }

    ret = WritePlaceholderCountNoLock(path, newCount);
    if (ret != E_OK) {
        return ret;
    }

    bool oldHasPlaceholder = oldCount > 0;
    bool newHasPlaceholder = newCount > 0;
    shouldPropagate = oldHasPlaceholder != newHasPlaceholder;
    if (!shouldPropagate) {
        return E_OK;
    }

    uint8_t oldState = 0;
    ret = ReadFileSyncStateByteNoLock(path, oldState, true);
    if (ret != E_OK) {
        return ret;
    }
    uint8_t newPlaceholderState = newHasPlaceholder ? PLACEHOLDER_STATE_HAS_PLACEHOLDER : PLACEHOLDER_STATE_NONE;
    uint8_t newState = MakeFileSyncState(newPlaceholderState, GetSyncStateFromFileSyncState(oldState));
    return WriteFileSyncStateByteNoLock(path, newState);
}

int32_t RecountNode(const std::string &syncRoot,
                    const std::string &path,
                    int32_t userId,
                    uint32_t syncFolderIndex,
                    bool &isPlaceholder);

int32_t RecountFileNode(const std::string &path, int32_t userId, uint32_t syncFolderIndex, bool &isPlaceholder)
{
    isPlaceholder = false;
    uint8_t placeholderState = PLACEHOLDER_STATE_NONE;
    int32_t ret = GetFilePlaceholderState(path, placeholderState);
    if (ret != E_OK) {
        LOGE("recount get file placeholder failed, errno:%{public}d", ret);
        placeholderState = PLACEHOLDER_STATE_NONE;
    }
    isPlaceholder = IsPlaceholderState(placeholderState);
    (void)UpdateDentryPlaceholderState(userId, syncFolderIndex, path, placeholderState);
    return E_OK;
}

int32_t RecountDirectoryChildren(const std::string &syncRoot,
                                 const std::string &path,
                                 int32_t userId,
                                 uint32_t syncFolderIndex,
                                 uint32_t &count)
{
    count = 0;
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) {
        LOGE("Open directory for placeholder recount failed, errno:%{public}d", errno);
        return errno;
    }

    struct dirent *entry = nullptr;
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        bool childPlaceholder = false;
        std::string childPath = path + "/" + entry->d_name;
        int32_t ret = RecountNode(syncRoot, childPath, userId, syncFolderIndex, childPlaceholder);
        if (ret != E_OK) {
            LOGE("recount child failed, errno:%{public}d", ret);
            continue;
        }
        if (childPlaceholder && count < UINT32_MAX) {
            ++count;
        }
    }
    closedir(dir);
    return E_OK;
}

void UpdateDirectoryRecountState(const std::string &path, uint32_t count)
{
    std::lock_guard<std::mutex> lock(GetPlaceholderStateMutex());
    int32_t ret = WritePlaceholderCountNoLock(path, count);
    if (ret != E_OK) {
        LOGE("Write recounted placeholder count failed, errno:%{public}d", ret);
    }
    uint8_t oldState = 0;
    ret = ReadFileSyncStateByteNoLock(path, oldState, true);
    if (ret != E_OK) {
        LOGE("Read directory state after placeholder recount failed, errno:%{public}d", ret);
        return;
    }
    uint8_t dirState = count > 0 ? PLACEHOLDER_STATE_HAS_PLACEHOLDER : PLACEHOLDER_STATE_NONE;
    uint8_t newState = MakeFileSyncState(dirState, GetSyncStateFromFileSyncState(oldState));
    ret = WriteFileSyncStateByteNoLock(path, newState);
    if (ret != E_OK) {
        LOGE("Write directory state after placeholder recount failed, errno:%{public}d", ret);
    }
}

int32_t RecountNode(const std::string &syncRoot,
                    const std::string &path,
                    int32_t userId,
                    uint32_t syncFolderIndex,
                    bool &isPlaceholder)
{
    isPlaceholder = false;
    struct stat statInfo {};
    if (lstat(path.c_str(), &statInfo) != 0) {
        LOGE("Read node metadata for placeholder recount failed, errno:%{public}d", errno);
        return errno;
    }

    if (!S_ISDIR(statInfo.st_mode)) {
        return RecountFileNode(path, userId, syncFolderIndex, isPlaceholder);
    }

    uint32_t count = 0;
    int32_t ret = RecountDirectoryChildren(syncRoot, path, userId, syncFolderIndex, count);
    if (ret != E_OK) {
        return ret;
    }
    UpdateDirectoryRecountState(path, count);
    isPlaceholder = count > 0;
    if (path != syncRoot) {
        (void)UpdateDentryPlaceholderState(userId, syncFolderIndex, path,
                                           isPlaceholder ? PLACEHOLDER_STATE_HAS_PLACEHOLDER : PLACEHOLDER_STATE_NONE);
    }
    return E_OK;
}
} // namespace

bool IsPlaceholderState(uint8_t placeholderState)
{
    return placeholderState != PLACEHOLDER_STATE_NONE;
}

bool IsValidPlaceholderState(uint8_t placeholderState)
{
    return placeholderState <= PLACEHOLDER_STATE_FULLY_HYDRATED;
}

uint8_t GetPlaceholderStateFromFileSyncState(uint8_t state)
{
    return static_cast<uint8_t>((state & FILE_SYNC_STATE_PLACEHOLDER_MASK) >> FILE_SYNC_STATE_PLACEHOLDER_SHIFT);
}

uint8_t GetSyncStateFromFileSyncState(uint8_t state)
{
    return static_cast<uint8_t>(state & FILE_SYNC_STATE_SYNC_MASK);
}

uint8_t MakeFileSyncState(uint8_t placeholderState, uint8_t syncState)
{
    return static_cast<uint8_t>(
        ((placeholderState << FILE_SYNC_STATE_PLACEHOLDER_SHIFT) & FILE_SYNC_STATE_PLACEHOLDER_MASK) |
        (syncState & FILE_SYNC_STATE_SYNC_MASK));
}

uint8_t SanitizeDentryPlaceholderState(uint8_t placeholderState)
{
    return IsValidPlaceholderState(placeholderState) ? placeholderState : PLACEHOLDER_STATE_NONE;
}

uint8_t GetDentryPlaceholderState(const CloudDiskServiceDentry &dentry)
{
    return SanitizeDentryPlaceholderState(dentry.reserved[DENTRY_PLACEHOLDER_RESERVED_INDEX]);
}

void SetDentryPlaceholderState(CloudDiskServiceDentry &dentry, uint8_t placeholderState)
{
    dentry.reserved[DENTRY_PLACEHOLDER_RESERVED_INDEX] = SanitizeDentryPlaceholderState(placeholderState);
}

int32_t GetFileSyncStateByte(const std::string &path, uint8_t &state, bool missingAsZero)
{
    std::lock_guard<std::mutex> lock(GetPlaceholderStateMutex());
    return ReadFileSyncStateByteNoLock(path, state, missingAsZero);
}

int32_t GetFilePlaceholderState(const std::string &path, uint8_t &placeholderState)
{
    uint8_t state = 0;
    int32_t ret = GetFileSyncStateByte(path, state, true);
    if (ret != E_OK) {
        return ret;
    }
    placeholderState = GetPlaceholderStateFromFileSyncState(state);
    return E_OK;
}

int32_t GetFilePlaceholderState(int32_t fd, uint8_t &placeholderState)
{
    std::lock_guard<std::mutex> lock(GetPlaceholderStateMutex());
    uint8_t state = 0;
    int32_t ret = ReadFileSyncStateByteNoLock(fd, state, true);
    if (ret != E_OK) {
        return ret;
    }
    placeholderState = GetPlaceholderStateFromFileSyncState(state);
    return E_OK;
}

int32_t SetFilePlaceholderState(const std::string &path, uint8_t newState, uint8_t &oldState)
{
    if (!IsValidPlaceholderState(newState)) {
        return EINVAL;
    }
    std::lock_guard<std::mutex> lock(GetPlaceholderStateMutex());
    uint8_t state = 0;
    int32_t ret = ReadFileSyncStateByteNoLock(path, state, true);
    if (ret != E_OK) {
        return ret;
    }
    oldState = GetPlaceholderStateFromFileSyncState(state);
    uint8_t newFileSyncState = MakeFileSyncState(newState, GetSyncStateFromFileSyncState(state));
    return WriteFileSyncStateByteNoLock(path, newFileSyncState);
}

int32_t SetFilePlaceholderState(int32_t fd, uint8_t newState, uint8_t &oldState)
{
    if (!IsValidPlaceholderState(newState)) {
        return EINVAL;
    }
    std::lock_guard<std::mutex> lock(GetPlaceholderStateMutex());
    uint8_t state = 0;
    int32_t ret = ReadFileSyncStateByteNoLock(fd, state, true);
    if (ret != E_OK) {
        return ret;
    }
    oldState = GetPlaceholderStateFromFileSyncState(state);
    uint8_t newFileSyncState = MakeFileSyncState(newState, GetSyncStateFromFileSyncState(state));
    return WriteFileSyncStateByteNoLock(fd, newFileSyncState);
}

int32_t SetNewFilePlaceholderState(int32_t fd, uint8_t newState)
{
    if (!IsValidPlaceholderState(newState)) {
        return EINVAL;
    }
    std::lock_guard<std::mutex> lock(GetPlaceholderStateMutex());
    uint8_t newFileSyncState = MakeFileSyncState(newState, 0);
    return WriteFileSyncStateByteNoLock(fd, newFileSyncState);
}

int32_t SetFileSyncState(const std::string &path, uint8_t syncState)
{
    if (syncState > FILE_SYNC_STATE_SYNC_MASK) {
        return EINVAL;
    }
    std::lock_guard<std::mutex> lock(GetPlaceholderStateMutex());
    uint8_t state = 0;
    int32_t ret = ReadFileSyncStateByteNoLock(path, state, true);
    if (ret != E_OK) {
        return ret;
    }
    uint8_t newState = MakeFileSyncState(GetPlaceholderStateFromFileSyncState(state), syncState);
    return WriteFileSyncStateByteNoLock(path, newState);
}

int32_t RefreshAncestorPlaceholderCount(const std::string &syncRoot, const std::string &path, int32_t delta)
{
    if (delta == 0 || syncRoot.empty() || path.empty()) {
        return E_OK;
    }

    std::string curPath = GetParentPath(path);
    int32_t lastError = E_OK;
    while (IsSameOrUnderPath(syncRoot, curPath)) {
        bool shouldPropagate = false;
        int32_t ret = UpdateDirectoryCountAndState(curPath, delta, shouldPropagate);
        if (ret != E_OK) {
            LOGE("refresh placeholder count failed, path:%{public}s, errno:%{public}d",
                 GetAnonyStringStrictly(curPath).c_str(), ret);
            lastError = ret;
            if (curPath == syncRoot) {
                break;
            }
            curPath = GetParentPath(curPath);
            continue;
        }
        if (!shouldPropagate || curPath == syncRoot) {
            break;
        }
        curPath = GetParentPath(curPath);
    }
    return lastError;
}

int32_t RecountPlaceholderState(const std::string &syncRoot,
                                const std::string &subPath,
                                int32_t userId,
                                uint32_t syncFolderIndex)
{
    if (!IsSameOrUnderPath(syncRoot, subPath)) {
        return EINVAL;
    }
    bool isPlaceholder = false;
    return RecountNode(syncRoot, subPath, userId, syncFolderIndex, isPlaceholder);
}

int32_t UpdateDentryPlaceholderState(int32_t userId,
                                     uint32_t syncFolderIndex,
                                     const std::string &path,
                                     uint8_t placeholderState)
{
    std::string parentPath = GetParentPath(path);
    std::string name = GetFileName(path);
    if (parentPath.empty() || name.empty()) {
        return EINVAL;
    }

    struct stat parentStat {};
    if (stat(parentPath.c_str(), &parentStat) != 0) {
        return errno;
    }

    auto parentMetaFile =
        MetaFileMgr::GetInstance().GetCloudDiskServiceMetaFile(userId, syncFolderIndex, parentStat.st_ino);
    MetaBase base(name);
    return parentMetaFile->DoUpdatePlaceholderState(base, placeholderState);
}
} // namespace OHOS::FileManagement::CloudDiskService
