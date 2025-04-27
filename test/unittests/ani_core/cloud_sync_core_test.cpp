/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cloud_sync_core.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/types.h>
#include <sys/xattr.h>

#include "cloud_sync_manager.h"
#include "dfs_error.h"
#include "uri.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace OHOS::FileManagement::CloudSync;

const int32_t E_INVAL = 13900020;

class CloudSyncCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudSyncCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudSyncCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudSyncCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudSyncCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: Constructor
 * @tc.desc: Verify the CloudSyncCore::Constructor function
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncCoreTest, ConstructorTest1, TestSize.Level1)
{
    auto data = CloudSyncCore::Constructor();
    EXPECT_TRUE(data.IsSuccess());
}

/**
 * @tc.name: GetBundleName
 * @tc.desc: Verify the CloudSyncCore::GetBundleName function
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncCoreTest, GetBundleNameTest1, TestSize.Level1)
{
    const CloudSyncCore *cloudSync = CloudSyncCore::Constructor().GetData().value();
    string ret = cloudSync->GetBundleName();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.name: DoGetFileSyncState
 * @tc.desc: Verify the CloudSyncCore::DoGetFileSyncState function
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncCoreTest, DoGetFileSyncStateTest1, TestSize.Level1)
{
    string filePath = "/test/test.txt";
    auto ret = CloudSyncCore::DoGetFileSyncState(filePath);
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, E_INVAL);
}
} // namespace OHOS::FileManagement::CloudDisk::Test