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

#ifndef OHOS_FILEMGMT_DECOMPRESS_KIT_H
#define OHOS_FILEMGMT_DECOMPRESS_KIT_H

#include <cstdint>
#include <memory>
#include <string>

#include "visibility.h"

namespace OHOS::FileManagement::Decompress {
class API_EXPORT DecompressKit {
public:
    API_EXPORT static DecompressKit *GetInstance();
    API_EXPORT static bool RegisterDecompressInstance(DecompressKit *instance);

    virtual ~DecompressKit() = default;

    virtual std::string GetCloudVersionFilePath();
    virtual bool GetParamConfigProperty(std::string &line, std::string &key, std::string &value);
    virtual std::string GetVersionConfigValue(const std::string &path, const std::string &configKey);
    virtual bool IsUpdateVersionCompatible(const std::string &path);
    virtual bool IsNeedCopy(const std::string &srcDir, const std::string &destDir);
    virtual bool CompareVersion(const std::string &srcVersion, const std::string &destVersion);
    virtual bool DoParamDirCopy(const std::string &srcDir, const std::string &destDir);
    virtual bool DoConfigFileCopy(const std::string &srcDir, const std::string &destDir);
    virtual bool DoMkDir(const std::string &dirPath);
    virtual bool DoRmDir(const std::string &path);
    virtual bool DoCopy(const std::string &srcPath, const std::string &destPath);

private:
    static inline DecompressKit *instance_;
};
} // namespace OHOS::FileManagement::Decompress

#endif // OHOS_FILEMGMT_DECOMPRESS_KIT_H
