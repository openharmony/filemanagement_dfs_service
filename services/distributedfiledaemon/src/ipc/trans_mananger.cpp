/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "trans_mananger.h"

#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
TransManager &TransManager::GetInstance()
{
    static TransManager instance;
    return instance;
}

void TransManager::NotifyFileProgress(const std::string &sessionName, uint64_t total, uint64_t processed)
{
    std::lock_guard<std::mutex> lock(sessionTransMapMutex_);
    auto iter = sessionTransMap_.find(sessionName);
    if (iter == sessionTransMap_.end()) {
        LOGE("sessionName not registered!");
        return;
    }
    if (iter->second == nullptr) {
        LOGE("IFileTransListener is empty!");
        return;
    }
    iter->second->OnFileReceive(total, processed);
}

void TransManager::NotifyFileFailed(const std::string &sessionName)
{
    std::lock_guard<std::mutex> lock(sessionTransMapMutex_);
    auto iter = sessionTransMap_.find(sessionName);
    if (iter == sessionTransMap_.end()) {
        LOGE("sessionName not registered!");
        return;
    }
    if (iter->second == nullptr) {
        LOGE("IFileTransListener is empty!");
        return;
    }
    iter->second->OnFailed(sessionName);
}

void TransManager::NotifyFileFinished(const std::string &sessionName)
{
    std::lock_guard<std::mutex> lock(sessionTransMapMutex_);
    auto iter = sessionTransMap_.find(sessionName);
    if (iter == sessionTransMap_.end()) {
        LOGE("sessionName not registered!");
        return;
    }
    if (iter->second == nullptr) {
        LOGE("IFileTransListener is empty!");
        return;
    }
    iter->second->OnFinished(sessionName);
}

void TransManager::AddTransTask(const std::string &sessionName, const sptr<IFileTransListener> &listener)
{
    std::lock_guard<std::mutex> lock(sessionTransMapMutex_);
    auto iter = sessionTransMap_.find(sessionName);
    if (iter != sessionTransMap_.end()) {
        LOGE("sessionName registered!");
        return;
    }
    sessionTransMap_.insert(std::make_pair(sessionName, listener));
}

void TransManager::DeleteTransTask(const std::string &sessionName)
{
    std::lock_guard<std::mutex> lock(sessionTransMapMutex_);
    auto iter = sessionTransMap_.find(sessionName);
    if (iter == sessionTransMap_.end()) {
        LOGE("sessionName not registered!");
        return;
    }
    sessionTransMap_.erase(iter);
}

} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS