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

#include "system_load.h"
#include "dfs_error.h"
#include "utils_log.h"
#include "res_sched_client.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class SytemLoadTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    shared_ptr<SystemLoadStatus> SystemLoadStatus_ = nullptr;
    shared_ptr<SystemLoadListener> SystemLoadListener_ = nullptr;
};

void SytemLoadTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void SytemLoadTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void SytemLoadTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    SystemLoadStatus_ = make_shared<SystemLoadStatus>();
    SystemLoadListener_ = make_shared<SystemLoadListener>();
}

void SytemLoadTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    SystemLoadStatus_ = nullptr;
    SystemLoadListener_ = nullptr;
}

/**
 * @tc.name: RegisterSystemloadCallbackTest
 * @tc.desc: Verify the RegisterSystemloadCallback function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SytemLoadTest, RegisterSystemloadCallbackTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSystemloadCallbackTest Start";
    try {
        shared_ptr<CloudFile::DataSyncManager> dataSyncManager = make_shared<CloudFile::DataSyncManager>();
        SystemLoadStatus_->RegisterSystemloadCallback(dataSyncManager);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterSystemloadCallbackTest FAILED";
    }
    GTEST_LOG_(INFO) << "RegisterSystemloadCallbackTest End";
}

/**
 * @tc.name: OnSystemloadLevelTest001
 * @tc.desc: Verify the OnSystemloadLevel function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SytemLoadTest, OnSystemloadLevelTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnSystemloadLevelTest001 Start";
    try {
        int32_t level = 10;
        SystemLoadListener_->OnSystemloadLevel(level);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnSystemloadLevelTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "OnSystemloadLevelTest001 End";
}

/**
 * @tc.name: OnSystemloadLevelTest002
 * @tc.desc: Verify the OnSystemloadLevel function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SytemLoadTest, OnSystemloadLevelTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnSystemloadLevelTest002 Start";
    try {
        int32_t level = 1;
        SystemLoadListener_->OnSystemloadLevel(level);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnSystemloadLevelTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "OnSystemloadLevelTest002 End";
}

/**
 * @tc.name: IsLoadStatusOkayTest001
 * @tc.desc: Verify the IsLoadStatusOkay function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SytemLoadTest, IsLoadStatusOkayTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsLoadStatusOkayTest001 Start";
    try {
        SystemLoadStatus_->Setload(10);
        bool ret = SystemLoadStatus_->IsLoadStatusUnderHeat();
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsLoadStatusOkayTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "IsLoadStatusOkayTest001 End";
}

/**
 * @tc.name: IsLoadStatusOkayTest002
 * @tc.desc: Verify the IsLoadStatusOkay function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SytemLoadTest, IsLoadStatusOkayTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsLoadStatusOkayTest002 Start";
    try {
        SystemLoadStatus_->Setload(1);
        bool ret = SystemLoadStatus_->IsLoadStatusUnderHeat();
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsLoadStatusOkayTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "IsLoadStatusOkayTest002 End";
}
} // namespace OHOS::FileManagement::CloudSync::Test