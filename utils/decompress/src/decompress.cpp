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

#include "decompress.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <fcntl.h>
#include <climits>
#include <mutex>
#include <sys/stat.h>
#include <unordered_set>
#include <unistd.h>
#include <securec.h>

#include "fuse_ioctl.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace Decompress {
using namespace CloudFile;

enum FileType {
    SO_FILE = 1,
    RESOURCE_FILE = 2,
};

struct HmfsPdedupExtent {
    uint64_t outerOff; /** offset in outer file */
    uint64_t innerOff; /** offset in inner file */
    uint64_t len; /** map len */
}__attribute__((packed));

struct HmfsPdedupCreate {
    int32_t version; /** pdedup version, default 1*/
    int32_t innerFd; /** fd of inner file*/
    uint64_t fileSize; /**  expected size of outer file*/
    uint32_t nrExtents; /** extents num */
    int32_t mode; /** expected mode of outher file */
    char name[256]; /** expected name of outer file */
    struct HmfsPdedupExtent extents[]; /** map extents */
};

namespace {
    constexpr const char* PDEDUP_FEATURE_NODE_PATH = "/sys/fs/hmfs/userdata/support_pdedup";
    constexpr size_t MAX_PATH_LENGTH = PATH_MAX;
    constexpr const char* CLOUD_SYNC_LIB_PATH = "libcloudsync_kit_inner.z.so";
    constexpr int MAX_RETRY = 2;
    constexpr int RETRY_DELAY_US = 100000;
    constexpr uint32_t MAX_UNCOMPATIBLE_TARGET_VERSION = 20;
    constexpr uint32_t API_VERSION_MOD = 1000;
}

using GetUnsupportedListFunc = int32_t(*)(std::vector<std::string>*);
using GetSystemFeatureFunc = int32_t(*)(bool*);

class CloudSyncLibraryManager {
public:
    static CloudSyncLibraryManager& GetInstance()
    {
        static CloudSyncLibraryManager instance;
        return instance;
    }

    void* GetHandle() const { return handle_; }
    bool IsLoaded() const { return handle_ != nullptr; }

    GetUnsupportedListFunc GetUnsupportedListSymbol()
    {
        return GetSymbolFunc<GetUnsupportedListFunc>("CloudSync_GetDecompressUnsupportedList");
    }

    GetSystemFeatureFunc GetSystemFeatureSymbol()
    {
        return GetSymbolFunc<GetSystemFeatureFunc>("CloudSync_GetDecompressSystemFeature");
    }

private:
    CloudSyncLibraryManager()
    {
        handle_ = dlopen(CLOUD_SYNC_LIB_PATH, RTLD_LAZY | RTLD_LOCAL | RTLD_NODELETE);
        if (handle_ == nullptr) {
            LOGE("dlopen failed for %{public}s: %{public}s", CLOUD_SYNC_LIB_PATH, dlerror());
        }
    }

    ~CloudSyncLibraryManager()
    {
        // No dlclose: RTLD_NODELETE ensures library stays loaded, system auto-cleanup.
    }

    template<typename FuncType>
    FuncType GetSymbolFunc(const char* symbol)
    {
        if (handle_ == nullptr) {
            return nullptr;
        }
        auto func = reinterpret_cast<FuncType>(dlsym(handle_, symbol));
        if (func == nullptr) {
            LOGE("dlsym %{public}s failed: %{public}s", symbol, dlerror());
        }
        return func;
    }

    void* handle_ = nullptr;
};

static bool SplitTargetPath(const std::string &targetPath, std::string &dirPath, std::string &fileName)
{
    size_t lastSlash = targetPath.find_last_of('/');
    if (lastSlash == std::string::npos) {
        LOGE("Target path must be absolute path: %{public}s", targetPath.c_str());
        return false;
    }
    dirPath = targetPath.substr(0, lastSlash);
    fileName = targetPath.substr(lastSlash + 1);
    return true;
}

static bool ValidateSourcePath(const std::string &path)
{
    if (path.empty() || path.length() > MAX_PATH_LENGTH) {
        LOGE("Source path is empty or too long: %{public}zu", path.length());
        return false;
    }

    char resolvedPath[PATH_MAX] = {0};
    if (realpath(path.c_str(), resolvedPath) == nullptr) {
        LOGE("realpath failed for source path: %{public}s, errno: %{public}d",
             path.c_str(), errno);
        return false;
    }

    return true;
}

static bool ValidateTargetPath(const std::string &targetPath)
{
    if (targetPath.empty() || targetPath.length() > MAX_PATH_LENGTH) {
        LOGE("Target path is empty or too long: %{public}zu", targetPath.length());
        return false;
    }

    std::string dirPath;
    std::string fileName;
    if (!SplitTargetPath(targetPath, dirPath, fileName)) {
        return false;
    }

    char resolvedDirPath[PATH_MAX] = {0};
    if (realpath(dirPath.c_str(), resolvedDirPath) == nullptr) {
        LOGE("realpath failed for target directory: %{public}s, errno: %{public}d",
             dirPath.c_str(), errno);
        return false;
    }
    return true;
}

static int OpenSourceFile(const std::string &sourcePath)
{
    int innerFd = open(sourcePath.c_str(), O_RDONLY);
    if (innerFd < 0) {
        LOGE("Failed to open source file: %{public}s, errno: %{public}d", sourcePath.c_str(), errno);
        return -1;
    }
    return innerFd;
}

static struct HmfsPdedupCreate *PreparePdedupCreate(int innerFd,
                                                    const std::string &fileName,
                                                    uint32_t offset, uint32_t size)
{
    size_t createSize = sizeof(struct HmfsPdedupCreate) + sizeof(struct HmfsPdedupExtent);
    struct HmfsPdedupCreate *create = static_cast<struct HmfsPdedupCreate *>(malloc(createSize));
    if (!create) {
        LOGE("Failed to allocate memory for HmfsPdedupCreate");
        return nullptr;
    }

    errno_t ret = memset_s(create, createSize, 0, createSize);
    if (ret != EOK) {
        LOGE("memset_s failed: %{public}d", ret);
        free(create);
        return nullptr;
    }

    create->version = 1;
    create->innerFd = innerFd;
    create->fileSize = size;
    create->nrExtents = 1;
    create->mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    ret = strncpy_s(create->name, sizeof(create->name), fileName.c_str(), fileName.length());
    if (ret != EOK) {
        LOGE("strncpy_s failed: %{public}d", ret);
        free(create);
        return nullptr;
    }

    create->extents[0].outerOff = 0;
    create->extents[0].innerOff = offset;
    create->extents[0].len = size;

    return create;
}

static bool ExecutePdedupCreate(int dirFd, struct HmfsPdedupCreate *create,
                                const std::string &targetPath, uint32_t offset, uint32_t size)
{
    for (int i = 0; i < MAX_RETRY; i++) {
        int ioctlRet = ioctl(dirFd, HMFS_IOC_PDEDLUP_CREATE, create);
        if (ioctlRet >= 0) {
            LOGD("Successfully created pdedup file: %{public}s, offset: %{public}u, size: %{public}u",
                targetPath.c_str(), offset, size);
            return true;
        }

        if (errno == EINTR || errno == EAGAIN || errno == EBUSY || errno == ENOMEM) {
            LOGW("ioctl retry %{public}d/%{public}d for %{public}s, errno: %{public}d",
                 i + 1, MAX_RETRY, targetPath.c_str(), errno);
            usleep(RETRY_DELAY_US);
            continue;
        }
        break;
    }

    LOGE("Failed to create pdedup for %{public}s, offset: %{public}u, errno: %{public}d",
        targetPath.c_str(), offset, errno);
    return false;
}

bool CreateInnerFile(const std::string &sourcePath, const std::string &targetPath,
                     const uint32_t &offset, const uint32_t &size, bool isSystemApp)
{
    LOGD("CreateInnerFile start, sourcePath: %{public}s, targetPath: %{public}s, offset: %{public}u, size: %{public}u",
        sourcePath.c_str(), targetPath.c_str(), offset, size);
    if (!ValidateSourcePath(sourcePath)) {
        LOGE("Invalid source path: %{public}s", sourcePath.c_str());
        return false;
    }
    if (!ValidateTargetPath(targetPath)) {
        LOGE("Invalid target path: %{public}s", targetPath.c_str());
        return false;
    }
    int innerFd = OpenSourceFile(sourcePath);
    if (innerFd < 0) {
        return false;
    }
    std::string dirPath;
    std::string fileName;
    if (!SplitTargetPath(targetPath, dirPath, fileName)) {
        close(innerFd);
        return false;
    }
    LOGD("SplitTargetPath result, dirPath: %{public}s, fileName: %{public}s", dirPath.c_str(), fileName.c_str());
    int dirFd = open(dirPath.c_str(), O_RDONLY | O_DIRECTORY);
    if (dirFd < 0) {
        LOGE("Failed to open directory: %{public}s, errno: %{public}d", dirPath.c_str(), errno);
        close(innerFd);
        return false;
    }
    struct HmfsPdedupCreate *create = PreparePdedupCreate(innerFd, fileName, offset, size);
    if (!create) {
        close(dirFd);
        close(innerFd);
        return false;
    }
    bool success = ExecutePdedupCreate(dirFd, create, targetPath, offset, size);
    LOGD("ExecutePdedupCreate result: %{public}s", success ? "true" : "false");
    free(create);
    close(dirFd);
    close(innerFd);
    return success;
}


bool CheckBundleSupported(const std::string &bundleName, const bool isKeepAlive)
{
    if (isKeepAlive) {
        LOGD("Bundle %{public}s iskeepAlive is true", bundleName.c_str());
        return false;
    }
    static std::unordered_set<std::string> unsupportedCache;
    static std::once_flag initFlag;

    std::call_once(initFlag, []() {
        auto& manager = CloudSyncLibraryManager::GetInstance();
        if (!manager.IsLoaded()) {
            LOGE("Failed to load cloud sync library");
            return;
        }

        auto func = manager.GetUnsupportedListSymbol();
        if (func != nullptr) {
            std::vector<std::string> unsupportedList;
            int32_t ret = func(&unsupportedList);
            if (ret == 0) {
                for (const auto& name : unsupportedList) {
                    unsupportedCache.insert(name);
                }
                LOGD("Cached unsupported list: %{public}zu items", unsupportedCache.size());
            } else {
                LOGE("Get unsupported list failed, ret: %{public}d", ret);
            }
        }
    });

    bool supported = unsupportedCache.find(bundleName) == unsupportedCache.end();
    LOGD("Bundle %{public}s is %{public}s", bundleName.c_str(), supported ? "supported" : "not supported");
    return supported;
}

bool CheckModuleApiVersionSupported(const std::string &moduleName,
                                    const uint32_t targetVersion,
                                    const uint32_t minVersion)
{
    uint32_t apiVersion = targetVersion % API_VERSION_MOD;
    LOGD("Module %{public}s apiVersion %{public}u, targetVersion: %{public}u, minVersion: %{public}u",
        moduleName.c_str(), apiVersion, targetVersion, minVersion);
    if (apiVersion <= MAX_UNCOMPATIBLE_TARGET_VERSION) {
        return false;
    }
    return true;
}

static bool ReadPdedupFeatureNode()
{
    FILE* file = fopen(PDEDUP_FEATURE_NODE_PATH, "r");
    if (file == nullptr) {
        LOGD("Failed to open pdedup feature node file: %{public}s", PDEDUP_FEATURE_NODE_PATH);
        return false;
    }
    char* line = nullptr;
    size_t len = 0;
    ssize_t read = getline(&line, &len, file);
    if (fclose(file) != 0) {
        LOGE("Failed to close pdedup feature node file, errno: %{public}d", errno);
    }
    if (read == -1) {
        LOGD("Failed to read pdedup feature node file");
        return false;
    }
    bool supported = (line != nullptr && line[0] == '1');
    LOGD("Pdedup feature node content: %{public}s, supported: %{public}s",
         line ? line : "", supported ? "true" : "false");
    if (line != nullptr) {
        free(line);
    }
    return supported;
}

bool GetSystemFeature()
{
    static bool cachedFeature = false;
    static std::once_flag initFlag;

    std::call_once(initFlag, []() {
        bool nodeSupported = ReadPdedupFeatureNode();
        if (!nodeSupported) {
            LOGD("Pdedup feature node not supported");
            return;
        }

        auto& manager = CloudSyncLibraryManager::GetInstance();
        if (!manager.IsLoaded()) {
            LOGE("Failed to load cloud sync library");
            return;
        }

        auto func = manager.GetSystemFeatureSymbol();
        if (func != nullptr) {
            bool systemFeature = false;
            int32_t ret = func(&systemFeature);
            if (ret == 0) {
                cachedFeature = systemFeature;
                LOGD("System feature cached: %{public}s", cachedFeature ? "true" : "false");
            } else {
                LOGE("Get system feature via IPC failed, ret: %{public}d", ret);
            }
        }
    });

    return cachedFeature;
}

} // namespace Decompress
} // namespace FileManagement
} // namespace OHOS