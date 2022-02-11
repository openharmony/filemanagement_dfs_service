/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "service_proxy.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {

int32_t ServiceProxy::SendFile(const std::string &cid,
                               const std::vector<std::string> &sourceFileList,
                               const std::vector<std::string> &destinationFileList,
                               const uint32_t fileCount)
{
    return 0;
}

int32_t ServiceProxy::sendTest()
{
    LOGE("sendTest enter");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int ret = Remote()->SendRequest(TEST_CODE, data, reply, option);
    LOGE("sendTest sendrequest done %{public}d", ret);
    return ret;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS