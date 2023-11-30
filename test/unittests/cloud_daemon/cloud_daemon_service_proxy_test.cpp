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

#include "cloud_daemon_service_mock.h"
#include "cloud_daemon_service_proxy.h"
#include "cloud_file_daemon_interface_code.h"
#include "dfs_error.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "utils_log.h"
#include <fcntl.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/stat.h>

static bool g_isamflag = true;
static bool g_smcflag = true;

namespace OHOS {
class RemoteObject : public IRemoteObject {
public:
    virtual int32_t GetObjectRefCount()
    {
        return 0;
    }
    virtual int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
    {
        return 0;
    }
    virtual bool AddDeathRecipient(const sptr<DeathRecipient> &recipient)
    {
        return 0;
    }
    virtual bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient)
    {
        return false;
    }
    virtual int Dump(int fd, const std::vector<std::u16string> &args)
    {
        return 0;
    }
};

sptr<IRemoteObject> ISystemAbilityManager::CheckSystemAbility(int32_t systemAbilityId)
{
    if (g_isamflag == true) {
        sptr<IRemoteObject> isamObject = sptr(new RemoteObject());
        return isamObject;
    } else {
        return nullptr;
    }
    return nullptr;
}

SystemAbilityManagerClient &SystemAbilityManagerClient::GetInstance()
{
    static auto instance = new SystemAbilityManagerClient();
    return *instance;
}

sptr<ISystemAbilityManager> SystemAbilityManagerClient::GetSystemAbilityManager()
{
    if (g_smcflag == true) {
        sptr<IRemoteObject> smcObject = IPCSkeleton::GetContextObject();
        systemAbilityManager_ = iface_cast<ISystemAbilityManager>(smcObject);
        return systemAbilityManager_;
    } else {
        return nullptr;
    }
}
} // namespace OHOS

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
    shared_ptr<CloudDaemonServiceProxy> proxy_ = nullptr;
    sptr<CloudDaemonServiceMock> mock_ = nullptr;
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
    mock_ = sptr(new CloudDaemonServiceMock());
    proxy_ = make_shared<CloudDaemonServiceProxy>(mock_);
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudDaemonServiceProxyTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: GetInstanceTest001
 * @tc.desc: Verify the GetInstance function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDaemonServiceProxyTest, GetInstanceTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstanceTest001 Start";
    try {
        g_smcflag = false;
        auto CloudDaemonServiceProxy = CloudDaemonServiceProxy::GetInstance();
        EXPECT_EQ(CloudDaemonServiceProxy, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetInstanceTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "GetInstanceTest001 End";
}

/**
 * @tc.name: StartFuseTest001
 * @tc.desc: Verify the StartFuse function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDaemonServiceProxyTest, StartFuseTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartFuseTest001 Start";
    try {
        EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));
        int32_t userId = 100;
        int32_t devFd = open("/dev/fuse", O_RDWR);
        string path = "continue";
        int ret = proxy_->StartFuse(userId, devFd, path);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartFuseTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "StartFuseTest001 End";
}

/**
 * @tc.name: StartFuseTest002
 * @tc.desc: Verify the StartFuse function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDaemonServiceProxyTest, StartFuseTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartFuseTest002 Start";
    try {
        EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(-1));
        int32_t userId = 100;
        int32_t devFd = open("/dev/fuse", O_RDWR);
        string path = "continue";
        int ret = proxy_->StartFuse(userId, devFd, path);
        EXPECT_EQ(ret, E_BROKEN_IPC);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartFuseTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "StartFuseTest002 End";
}
} // namespace OHOS::FileManagement::CloudFile::Test