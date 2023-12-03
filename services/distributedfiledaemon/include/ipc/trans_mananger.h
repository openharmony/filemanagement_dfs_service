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
#ifndef TRANS_MANANGER_H
#define TRANS_MANANGER_H

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>

#include "i_file_trans_listener.h"
#include "refbase.h"
namespace OHOS {
namespace Storage {
namespace DistributedFile {
class TransManager {
public:
    static TransManager &GetInstance();

    void AddTransTask(const std::string &sessionName, const sptr<IFileTransListener> &listener);
    void NotifyFileProgress(const std::string &sessionName, uint64_t total, uint64_t processed);
    void NotifyFileFailed(const std::string &sessionName);
    void NotifyFileFinished(const std::string &sessionName);
    void DeleteTransTask(const std::string &sessionName);

private:
    std::mutex sessionTransMapMutex_;
    std::map<std::string, sptr<IFileTransListener>> sessionTransMap_;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // TRANS_MANANGER_H
