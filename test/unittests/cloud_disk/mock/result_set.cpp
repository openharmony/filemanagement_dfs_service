/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "result_set.h"

#include "dfs_error.h"

namespace OHOS::NativeRdb {
using namespace OHOS::FileManagement;

int32_t ResultSet::GetRowCount(int32_t &count)
{
    if (ResultSetMock::proxy_ != nullptr) {
        int32_t ret = ResultSetMock::proxy_->GetRowCount(count);
        count = ResultSetMock::count_;
        return ret;
    }

    return E_OK;
}
} // namespace OHOS::NativeRdb