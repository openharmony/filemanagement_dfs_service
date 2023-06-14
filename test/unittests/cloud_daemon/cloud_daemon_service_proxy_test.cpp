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
#include "cloud_daemon_service_proxy.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudFile::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CloudDaemonServiceProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudDaemonServiceProxyTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudDaemonServiceProxyTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudDaemonServiceProxyTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudDaemonServiceProxyTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: StartFuseTest
 * @tc.desc: Verify the StartFuse function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDaemonServiceProxyTest, StartFuseTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartFuseTest Start";
    try {
        auto CloudDaemonServiceProxy = CloudDaemonServiceProxy::GetInstance();
        EXPECT_NE(CloudDaemonServiceProxy, nullptr);
        MessageParcel data;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudDaemon::GetDescriptor()));
        int32_t devFd = open("/dev/fuse", O_RDWR);
        EXPECT_GE(devFd, 0);
        string path = "/dev/fuse";
        int32_t userId = 100;
        int32_t ret = CloudDaemonServiceProxy->StartFuse(userId, devFd, path);
        EXPECT_NE(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartFuseTest  ERROR";
    }
    GTEST_LOG_(INFO) << "StartFuseTest End";
}

/**
 * @tc.name: GetInstanceTest
 * @tc.desc: Verify the GetInstance function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDaemonServiceProxyTest, GetInstanceTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstanceTest Start";
    try {
        auto CloudDaemonServiceProxy = CloudDaemonServiceProxy::GetInstance();
        EXPECT_NE(CloudDaemonServiceProxy, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetInstanceTest  ERROR";
    }
    GTEST_LOG_(INFO) << "GetInstanceTest End";
}

/**
 * @tc.name: InvaildInstanceTest
 * @tc.desc: Verify the InvaildInstance function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDaemonServiceProxyTest, InvaildInstanceTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InvaildInstanceTest Start";
    try {
        auto CloudDaemonServiceProxy = CloudDaemonServiceProxy::GetInstance();
        EXPECT_NE(CloudDaemonServiceProxy, nullptr);
        CloudDaemonServiceProxy::InvaildInstance();
        EXPECT_EQ(CloudDaemonServiceProxy::serviceProxy_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InvaildInstanceTest  ERROR";
    }
    GTEST_LOG_(INFO) << "InvaildInstanceTest End";
}
} // namespace OHOS::FileManagement::CloudSync::Test