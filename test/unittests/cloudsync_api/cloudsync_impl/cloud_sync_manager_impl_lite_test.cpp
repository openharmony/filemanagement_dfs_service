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

#include <gtest/gtest.h>

#include "cloud_sync_manager_impl_lite.h"
#include "dfs_error.h"
#include "service_proxy_mock.h"

namespace OHOS {
namespace FileManagement::CloudSync {
using namespace testing::ext;
using namespace testing;
using namespace std;

class CloudSyncManagerImplLiteTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<MockServiceProxy> proxy_ = nullptr;
    static inline sptr<CloudSyncServiceMock> serviceProxy_ = nullptr;
};

void CloudSyncManagerImplLiteTest::SetUpTestCase(void)
{
    proxy_ = std::make_shared<MockServiceProxy>();
    IserviceProxy::proxy_ = proxy_;
    serviceProxy_ = sptr(new CloudSyncServiceMock());
    CloudSyncManagerImplLite::GetInstance().isFirstCall_.test_and_set();
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudSyncManagerImplLiteTest::TearDownTestCase(void)
{
    IserviceProxy::proxy_ = nullptr;
    proxy_ = nullptr;
    serviceProxy_ = nullptr;
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudSyncManagerImplLiteTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudSyncManagerImplLiteTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: RemovedCleanTest001
 * @tc.desc: Verify the RemovedClean function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudSyncManagerImplLiteTest, RemovedCleanTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemovedCleanTest001 Start";
    try {
        string bundleName = "";
        int32_t userId = 1;
        auto res = CloudSyncManagerImplLite::GetInstance().RemovedClean(bundleName, userId);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemovedCleanTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "RemovedCleanTest001 End";
}

/**
 * @tc.name: RemovedCleanTest002
 * @tc.desc: Verify the RemovedClean function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudSyncManagerImplLiteTest, RemovedCleanTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemovedCleanTest002 Start";
    try {
        string bundleName = "test.bundle.name";
        int32_t userId = 1;
        auto res = CloudSyncManagerImplLite::GetInstance().RemovedClean(bundleName, userId);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemovedCleanTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "RemovedCleanTest002 End";
}

/**
 * @tc.name: RemovedCleanTest003
 * @tc.desc: Verify the RemovedClean function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudSyncManagerImplLiteTest, RemovedCleanTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemovedCleanTest003 Start";
    try {
        string bundleName = "test.bundle.name";
        int32_t userId = 101;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(nullptr));
        auto res = CloudSyncManagerImplLite::GetInstance().RemovedClean(bundleName, userId);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemovedCleanTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "RemovedCleanTest003 End";
}

/**
 * @tc.name: RemovedCleanTest004
 * @tc.desc: Verify the RemovedClean function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudSyncManagerImplLiteTest, RemovedCleanTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemovedCleanTest004 Start";
    try {
        string bundleName = "test.bundle.name";
        int32_t userId = 101;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        CloudSyncManagerImplLite::GetInstance().RemovedClean(bundleName, userId);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemovedCleanTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "RemovedCleanTest004 End";
}
} // namespace FileManagement::CloudSync
} // namespace OHOS