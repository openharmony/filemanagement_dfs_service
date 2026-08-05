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

#ifndef OHOS_FILEMGMT_DECOMPRESS_KIT_IMPL_H
#define OHOS_FILEMGMT_DECOMPRESS_KIT_IMPL_H

#include "decompress_kit.h"

namespace OHOS::FileManagement::Decompress {
class DecompressKitImpl final : public DecompressKit {
public:
    std::string GetCloudVersionFilePath() override;
    bool GetParamConfigProperty(std::string &line, std::string &key, std::string &value) override;
    std::string GetVersionConfigValue(const std::string &path, const std::string &configKey) override;
    bool IsUpdateVersionCompatible(const std::string &path) override;
    bool IsNeedCopy(const std::string &srcDir, const std::string &destDir) override;
    bool CompareVersion(const std::string &srcVersion, const std::string &destVersion) override;
    bool DoParamDirCopy(const std::string &srcDir, const std::string &destDir) override;
    bool DoConfigFileCopy(const std::string &srcDir, const std::string &destDir) override;
    bool DoMkDir(const std::string &dirPath) override;
    bool DoRmDir(const std::string &path) override;
    bool DoCopy(const std::string &srcPath, const std::string &destPath) override;
};
} // namespace OHOS::FileManagement::Decompress

#endif // OHOS_FILEMGMT_DECOMPRESS_KIT_IMPL_H
