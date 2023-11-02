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

#ifndef OHOS_FILEMGMT_XCOLLIE_HELPER_H
#define OHOS_FILEMGMT_XCOLLIE_HELPER_H

#include "xcollie/xcollie.h"

namespace OHOS::FileManagement {
class XCollieHelper {
public:
    static int SetTimer(const std::string &name, uint32_t timeout, XCollieCallback func, void *arg, bool recovery);
    static void CancelTimer(int id);
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_XCOLLIE_HELPER_H