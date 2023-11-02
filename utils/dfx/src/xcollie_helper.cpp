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

#include "xcollie_helper.h"

#include "xcollie/xcollie_define.h"
#include "utils_log.h"
#include "dfs_error.h"

namespace OHOS::FileManagement {
using namespace std;
int XCollieHelper::SetTimer(const std::string &name, uint32_t timeout, XCollieCallback func, void *arg, bool recovery)
{
#ifdef HICOLLIE_ENABLE
    unsigned int flag = HiviewDFX::XCOLLIE_FLAG_LOG | HiviewDFX::XCOLLIE_FLAG_NOOP;
    if (recovery) {
        flag |= HiviewDFX::XCOLLIE_FLAG_RECOVERY;
    }
    return HiviewDFX::XCollie::GetInstance().SetTimer(name, timeout, func, arg, flag);
#else
    LOGI("XCollie not supported");
    return HiviewDFX::INVALID_ID;
#endif
}
void XCollieHelper::CancelTimer(int id)
{
#ifdef HICOLLIE_ENABLE
    HiviewDFX::XCollie::GetInstance().CancelTimer(id);
#endif
}
} // namespace OHOS::FileManagement::CloudSync