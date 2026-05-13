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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "cloud_optimize_callback_stub.h"
#include "cloud_download_uri_manager.h"
#include "dfs_error.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CloudOptimizeCallbackStubMock : public CloudOptimizeCallbackStub {
public:
    CloudOptimizeCallbackStubMock() {}
    void OnOptimizeProcess(const OptimizeState state, const int32_t progress) override
    {}
};

class CloudOptimizeCallbackStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<CloudOptimizeCallbackStub> cloudOptimizeCallbackStub_;
};

void CloudOptimizeCallbackStubTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudOptimizeCallbackStubTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudOptimizeCallbackStubTest::SetUp(void)
{
    cloudOptimizeCallbackStub_ = std::make_shared<CloudOptimizeCallbackStubMock>();
    std::cout << "SetUp" << std::endl;
}

void CloudOptimizeCallbackStubTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: HandleOnProcessTest001
 * @tc.desc: Verify the HandleOnProcess function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudOptimizeCallbackStubTest, HandleOnProcessTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnProcess Start";
    try {
        MessageParcel data;
        MessageParcel reply;
        int ret = cloudOptimizeCallbackStub_->HandleOnProcess(data, reply);
        EXPECT_EQ(E_INVAL_ARG, ret);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleOnProcess ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnProcess End";
}

/**
 * @tc.name: HandleOnProcessTest002
 * @tc.desc: Verify the HandleOnProcess function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudOptimizeCallbackStubTest, HandleOnProcessTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnProcess Start";
    try {
        MessageParcel data;
        MessageParcel reply;
        int32_t invalidState = -1;
        EXPECT_TRUE(data.WriteInt32(invalidState));
        int32_t ret = cloudOptimizeCallbackStub_->HandleOnProcess(data, reply);
        EXPECT_EQ(E_INVAL_ARG, ret);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleOnProcess ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnProcess End";
}

/**
 * @tc.name: HandleOnProcessTest003
 * @tc.desc: Verify the HandleOnProcess function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudOptimizeCallbackStubTest, HandleOnProcessTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnProcess Start";
    try {
        MessageParcel data;
        MessageParcel reply;
        int32_t state = OPTIMIZE_RUNNING;
        EXPECT_TRUE(data.WriteInt32(state));
        int32_t ret = cloudOptimizeCallbackStub_->HandleOnProcess(data, reply);
        EXPECT_EQ(E_INVAL_ARG, ret);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleOnProcess ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnProcess End";
}

} // namespace Test
} // namespace FileManagement::CloudSync {
} // namespace OHOS