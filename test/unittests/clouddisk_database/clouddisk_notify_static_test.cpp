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

#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <memory>

#include "dfs_error.h"
#include "clouddisk_rdbstore_mock.cpp"
#include "clouddisk_notify.cpp"

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;

constexpr int32_t USER_ID = 100;
class CloudDiskNotifyStaticTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudDiskNotifyStaticTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudDiskNotifyStaticTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudDiskNotifyStaticTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudDiskNotifyStaticTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: HandleUpdateTest001
 * @tc.desc: Verify the HandleUpdate function
 * @tc.type: FUNC
 * @tc.require: issues2599
 */
HWTEST_F(CloudDiskNotifyStaticTest, HandleUpdateTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleUpdateTest001 Start";
    try {
        NotifyParamService paramService;
        paramService.cloudId = "testCloudId";
        paramService.notifyType = NotifyType::NOTIFY_NONE;
        paramService.oldUri = "oldUri";
        
        ParamServiceOther paramOthers;
        paramOthers.bundleName = "bundleName";
        paramOthers.userId = USER_ID;
        
        auto rdbStore = std::make_shared<CloudDiskRdbStore>(paramOthers.bundleName, paramOthers.userId);
        CacheNode curNode;
        string uri = "";
        int32_t ret = rdbStore->GetCurNode(paramService.cloudId, curNode);
        EXPECT_EQ(ret, E_OK);
        ret = rdbStore->GetNotifyUri(curNode, uri);
        EXPECT_EQ(ret, E_OK);
        HandleUpdate(paramService, paramOthers);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleUpdateTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleUpdateTest001 End";
}

/**
 * @tc.name: HandleUpdateTest002
 * @tc.desc: Verify the HandleUpdate function
 * @tc.type: FUNC
 * @tc.require: issues2599
 */
HWTEST_F(CloudDiskNotifyStaticTest, HandleUpdateTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleUpdateTest002 Start";
    try {
        NotifyParamService paramService;
        paramService.cloudId = "testCloudId";
        paramService.notifyType = NotifyType::NOTIFY_MODIFIED;
        paramService.oldUri = "oldUri";
        
        ParamServiceOther paramOthers;
        paramOthers.bundleName = "bundleName";
        paramOthers.userId = USER_ID;
        
        auto rdbStore = std::make_shared<CloudDiskRdbStore>(paramOthers.bundleName, paramOthers.userId);
        CacheNode curNode;
        string uri = "newUri";
        int32_t ret = rdbStore->GetCurNode(paramService.cloudId, curNode);
        EXPECT_EQ(ret, E_OK);
        ret = rdbStore->GetNotifyUri(curNode, uri);
        EXPECT_EQ(ret, E_OK);
        HandleUpdate(paramService, paramOthers);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleUpdateTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleUpdateTest002 End";
}

/**
 * @tc.name: HandleUpdateTest003
 * @tc.desc: Verify the HandleUpdate function
 * @tc.type: FUNC
 * @tc.require: issues2599
 */
HWTEST_F(CloudDiskNotifyStaticTest, HandleUpdateTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleUpdateTest003 Start";
    try {
        NotifyParamService paramService;
        paramService.cloudId = "testCloudId";
        paramService.notifyType = NotifyType::NOTIFY_NONE;
        paramService.oldUri = "oldUri";
        
        ParamServiceOther paramOthers;
        paramOthers.bundleName = "bundleName";
        paramOthers.userId = USER_ID;
        
        auto rdbStore = std::make_shared<CloudDiskRdbStore>(paramOthers.bundleName, paramOthers.userId);
        CacheNode curNode;
        string uri = "oldUri";
        int32_t ret = rdbStore->GetCurNode(paramService.cloudId, curNode);
        EXPECT_EQ(ret, E_OK);
        ret = rdbStore->GetNotifyUri(curNode, uri);
        EXPECT_EQ(ret, E_OK);
        HandleUpdate(paramService, paramOthers);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleUpdateTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleUpdateTest003 End";
}
}