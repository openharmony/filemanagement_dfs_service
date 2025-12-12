/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef CLOUDSYNC_SCREEN_STATUS_MOCK_H
#define CLOUDSYNC_SCREEN_STATUS_MOCK_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace OHOS::FileManagement::CloudSync::Test {
class ScreenStatusMock {
public:
    MOCK_METHOD0(IsScreenOn, bool());
    static inline std::shared_ptr<ScreenStatusMock> proxy_ = nullptr;
};
} // namespace OHOS::FileManagement::CloudSync::Test
#endif // CLOUDSYNC_SCREEN_STATUS_MOCK_H