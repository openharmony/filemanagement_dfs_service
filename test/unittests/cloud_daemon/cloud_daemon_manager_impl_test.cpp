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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "cloud_daemon_manager_impl.h"
#include "cloud_daemon_service_proxy.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudFile::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CloudDaemonManagerImplTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<CloudDaemonManagerImpl> cloudDaemonManagerImpl_;
};

void CloudDaemonManagerImplTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudDaemonManagerImplTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudDaemonManagerImplTest::SetUp(void)
{
    cloudDaemonManagerImpl_ = std::make_shared<CloudDaemonManagerImpl>();
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudDaemonManagerImplTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: GetInstanceTest
 * @tc.desc: Verify the GetInstance function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDaemonManagerImplTest, GetInstanceTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstanceTest Start";
    try {
        CloudDaemonManagerImpl::GetInstance();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetInstanceTest  ERROR";
    }
    GTEST_LOG_(INFO) << "GetInstanceTest End";
}

/**
 * @tc.name: StartFuseTest
 * @tc.desc: Verify the StartFuse function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDaemonManagerImplTest, StartFuseTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartFuseTest Start";
    try {
        int32_t devFd = -1;
        string path = "path";
        int32_t userId = 100;
        auto res = CloudDaemonManagerImpl::GetInstance().StartFuse(userId, devFd, path);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartFuse  ERROR";
    }
    GTEST_LOG_(INFO) << "StartFuse End";
}

/**
 * @tc.name: SetDeathRecipientTest
 * @tc.desc: Verify the SetDeathRecipient function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDaemonManagerImplTest, SetDeathRecipientTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetDeathRecipientTest Start";
    try {
        auto CloudDaemonServiceProxy = CloudDaemonServiceProxy::GetInstance();
        EXPECT_NE(CloudDaemonServiceProxy, nullptr);
        auto remoteObject = CloudDaemonServiceProxy->AsObject();
        EXPECT_NE(remoteObject, nullptr);
        cloudDaemonManagerImpl_->SetDeathRecipient(remoteObject);
        EXPECT_EQ(cloudDaemonManagerImpl_->isFirstCall_.test_and_set(), false);
        EXPECT_NE(cloudDaemonManagerImpl_->deathRecipient_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetDeathRecipientTest  ERROR";
    }
    GTEST_LOG_(INFO) << "SetDeathRecipientTest End";
}
} // namespace OHOS::FileManagement::CloudSync::Test