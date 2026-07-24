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

#ifndef OHOS_FILEMGMT_DECOMPRESS_KIT_MOCK_H
#define OHOS_FILEMGMT_DECOMPRESS_KIT_MOCK_H

#include <gmock/gmock.h>
#include <string>

#include "decompress_kit.h"

namespace OHOS::FileManagement::Decompress {
class DecompressKitMock : public DecompressKit {
public:
    MOCK_METHOD0(GetCloudVersionFilePath, std::string());
    MOCK_METHOD3(GetParamConfigProperty, bool(std::string &line, std::string &key, std::string &value));
    MOCK_METHOD2(GetVersionConfigValue, std::string(const std::string &path, const std::string &configKey));
    MOCK_METHOD1(IsUpdateVersionCompatible, bool(const std::string &path));
    MOCK_METHOD2(IsNeedCopy, bool(const std::string &srcDir, const std::string &destDir));
    MOCK_METHOD2(CompareVersion, bool(const std::string &srcVersion, const std::string &destVersion));
    MOCK_METHOD2(DoParamDirCopy, bool(const std::string &srcDir, const std::string &destDir));
    MOCK_METHOD2(DoConfigFileCopy, bool(const std::string &srcDir, const std::string &destDir));
    MOCK_METHOD1(DoMkDir, bool(const std::string &dirPath));
    MOCK_METHOD1(DoRmDir, bool(const std::string &path));
    MOCK_METHOD2(DoCopy, bool(const std::string &srcPath, const std::string &destPath));
};
} // namespace OHOS::FileManagement::Decompress

#endif // OHOS_FILEMGMT_DECOMPRESS_KIT_MOCK_H
