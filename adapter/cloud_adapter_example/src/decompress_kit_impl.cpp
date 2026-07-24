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

#include "decompress_kit_impl.h"

namespace OHOS::FileManagement::Decompress {

__attribute__((used)) static bool g_isInit =
    DecompressKit::RegisterDecompressInstance(new (std::nothrow) DecompressKitImpl());

std::string DecompressKitImpl::GetCloudVersionFilePath()
{
    return "";
}

bool DecompressKitImpl::GetParamConfigProperty(std::string &line, std::string &key, std::string &value)
{
    return true;
}

std::string DecompressKitImpl::GetVersionConfigValue(const std::string &path, const std::string &configKey)
{
    return "";
}

bool DecompressKitImpl::IsUpdateVersionCompatible(const std::string &path)
{
    return true;
}

bool DecompressKitImpl::IsNeedCopy(const std::string &srcDir, const std::string &destDir)
{
    return true;
}

bool DecompressKitImpl::CompareVersion(const std::string &srcVersion, const std::string &destVersion)
{
    return true;
}

bool DecompressKitImpl::DoParamDirCopy(const std::string &srcDir, const std::string &destDir)
{
    return true;
}

bool DecompressKitImpl::DoConfigFileCopy(const std::string &srcDir, const std::string &destDir)
{
    return true;
}

bool DecompressKitImpl::DoMkDir(const std::string &dirPath)
{
    return true;
}

bool DecompressKitImpl::DoRmDir(const std::string &path)
{
    return true;
}

bool DecompressKitImpl::DoCopy(const std::string &srcPath, const std::string &destPath)
{
    return true;
}

} // namespace OHOS::FileManagement::Decompress
