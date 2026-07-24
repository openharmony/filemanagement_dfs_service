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

#include "decompress_kit.h"

namespace OHOS::FileManagement::Decompress {
DecompressKit *DecompressKit::GetInstance()
{
    return instance_;
}

bool DecompressKit::RegisterDecompressInstance(DecompressKit *instance)
{
    if (instance_ != nullptr) {
        return false;
    }
    instance_ = instance;
    return true;
}

std::string DecompressKit::GetCloudVersionFilePath()
{
    return "";
}

bool DecompressKit::GetParamConfigProperty(std::string &line, std::string &key, std::string &value)
{
    return true;
}

std::string DecompressKit::GetVersionConfigValue(const std::string &path, const std::string &configKey)
{
    return "";
}

bool DecompressKit::IsUpdateVersionCompatible(const std::string &path)
{
    return true;
}

bool DecompressKit::IsNeedCopy(const std::string &srcDir, const std::string &destDir)
{
    return true;
}

bool DecompressKit::CompareVersion(const std::string &srcVersion, const std::string &destVersion)
{
    return true;
}

bool DecompressKit::DoParamDirCopy(const std::string &srcDir, const std::string &destDir)
{
    return true;
}

bool DecompressKit::DoConfigFileCopy(const std::string &srcDir, const std::string &destDir)
{
    return true;
}

bool DecompressKit::DoMkDir(const std::string &dirPath)
{
    return true;
}

bool DecompressKit::DoRmDir(const std::string &path)
{
    return true;
}

bool DecompressKit::DoCopy(const std::string &srcPath, const std::string &destPath)
{
    return true;
}

} // namespace OHOS::FileManagement::Decompress
