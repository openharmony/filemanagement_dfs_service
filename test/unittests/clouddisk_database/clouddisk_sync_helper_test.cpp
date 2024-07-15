/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "clouddisk_sync_helper.h"
#include "utils_log.h"
#include "dfs_error.h"
#include "common_timer_errors.h"

namespace OHOS {
namespace FileManagement::CloudDisk {
namespace Test {
using namespace testing::ext;
using namespace std;

constexpr int32_t MIN_USER_ID = 100;

class CloudDiskSyncHelperTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudDiskSyncHelperTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudDiskSyncHelperTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudDiskSyncHelperTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void CloudDiskSyncHelperTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: RegisterTriggerSyncTest001
 * @tc.desc: Verify the RegisterTriggerSync function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskSyncHelperTest, RegisterTriggerSyncTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterTriggerSync Start";
    string bundleName = "";
    int32_t userId = 0;
    CloudDiskSyncHelper::GetInstance().RegisterTriggerSync(bundleName, userId);
    GTEST_LOG_(INFO) << "RegisterTriggerSync End";
}

/**
 * @tc.name: RegisterTriggerSyncTest002
 * @tc.desc: Verify the RegisterTriggerSync function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskSyncHelperTest, RegisterTriggerSyncTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterTriggerSync Start";
    string bundleName = "com.ohos.photos";
    int32_t userId = 100;
    CloudDiskSyncHelper::GetInstance().RegisterTriggerSync(bundleName, userId);
    GTEST_LOG_(INFO) << "RegisterTriggerSync End";
}

/**
 * @tc.name: UnregisterRepeatingTriggerSyncTest001
 * @tc.desc: Verify the UnregisterRepeatingTriggerSync function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskSyncHelperTest, UnregisterRepeatingTriggerSyncTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterRepeatingTriggerSync Start";
    string bundleName = "";
    int32_t userId = 0;
    CloudDiskSyncHelper::GetInstance().UnregisterRepeatingTriggerSync(bundleName, userId);
    GTEST_LOG_(INFO) << "UnregisterRepeatingTriggerSync End";
}

/**
 * @tc.name: UnregisterRepeatingTriggerSyncTest002
 * @tc.desc: Verify the UnregisterRepeatingTriggerSync function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskSyncHelperTest, UnregisterRepeatingTriggerSyncTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterRepeatingTriggerSync Start";
    string bundleName = "com.ohos.photos";
    int32_t userId = 100;
    CloudDiskSyncHelper::GetInstance().UnregisterRepeatingTriggerSync(bundleName, userId);
    GTEST_LOG_(INFO) << "UnregisterRepeatingTriggerSync End";
}

/**
 * @tc.name: OnTriggerSyncCallbackTest001
 * @tc.desc: Verify the OnTriggerSyncCallback function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskSyncHelperTest, OnTriggerSyncCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnTriggerSyncCallback Start";
    string bundleName = "";
    int32_t userId = 0;
    CloudDiskSyncHelper::GetInstance().OnTriggerSyncCallback(bundleName, userId);
    GTEST_LOG_(INFO) << "OnTriggerSyncCallback End";
}

/**
 * @tc.name: OnTriggerSyncCallbackTest002
 * @tc.desc: Verify the OnTriggerSyncCallback function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskSyncHelperTest, OnTriggerSyncCallbackTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnTriggerSyncCallback Start";
    string bundleName = "com.ohos.photos";
    int32_t userId = 100;
    CloudDiskSyncHelper::GetInstance().OnTriggerSyncCallback(bundleName, userId);
    GTEST_LOG_(INFO) << "OnTriggerSyncCallback End";
}
} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS