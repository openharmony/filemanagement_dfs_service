/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include <iostream>
#include <sstream>
#include <string>
#include <sys/mount.h>
#include <system_error>
#include <unistd.h>

#include "device/device_info.h"
#include "device/device_manager_agent.h"
#include "mountpoint/mount_point.h"
#include "network/kernel_talker.h"
#include "network/session_pool.h"
#include "network/softbus/softbus_session.h"
#include "securec.h"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing::ext;
using namespace std;

static const string srcHead = "/data/service/el2/";
static const string dstHead = "/mnt/hmdfs/";
static const string cacheHead = "/data/service/el2/";
static const string SAME_ACCOUNT = "account";
std::mutex cmdMutex_;

const int KEY_MAX_LEN = 32;
const int CID_MAX_LEN = 64;
struct UpdateSocketParam {
    int32_t cmd;
    int32_t newfd;
    uint8_t status;
    uint8_t protocol;
    uint16_t udpPort;
    uint8_t deviceType;
    uint8_t masterKey[KEY_MAX_LEN];
    char cid[CID_MAX_LEN];
    int32_t linkType;
    int32_t binderFd;
} __attribute__((packed));

class DistributedFileDaemonServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedFileDaemonServiceTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void DistributedFileDaemonServiceTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void DistributedFileDaemonServiceTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void DistributedFileDaemonServiceTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}


/**
 * @tc.name: mount_test_001
 * @tc.desc: Verify the mount/umount function.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DistributedFileDaemonServiceTest, mount_umount_test_001, TestSize.Level1)
{
    auto mp = make_unique<OHOS::Storage::DistributedFile::MountPoint>(
        Utils::DfsuMountArgumentDescriptors::Alpha(100, SAME_ACCOUNT));

    shared_ptr<OHOS::Storage::DistributedFile::MountPoint> smp = move(mp);

    try {
        smp->Mount();
        smp->Umount();
        LOGE("testcase run OK");
    } catch (const exception &e) {
        LOGE("%{public}s", e.what());
        EXPECT_EQ(0, 1);
    }
    EXPECT_EQ(0, 0);
}

/**
 * @tc.name: distributedFileDaemon_service_test_006_repeats_init_register
 * @tc.desc: Verify the Start/Stop function.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DistributedFileDaemonServiceTest,
         distributedFileDaemon_service_test_006_repeats_init_register,
         TestSize.Level1)
{
    auto dm = DeviceManagerAgent::GetInstance();
    shared_ptr<OHOS::Storage::DistributedFile::DeviceManagerAgent> sdm = move(dm);

    try {
        sdm->Start();
        sdm->Stop();
        LOGE("testcase distributedFileDaemon_service_test_006_repeats_init_register run end");
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        EXPECT_EQ(0, 1);
    }
    EXPECT_EQ(0, 0);
}
/**
 * @tc.name: distributedFileDaemon_service_test_007_repeats_create_net_instance
 * @tc.desc: Verify the JoinGroup/QuitGroup function.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DistributedFileDaemonServiceTest,
         distributedFileDaemon_service_test_007_repeats_create_net_instance,
         TestSize.Level1)
{
    const int userId = 4200;
    auto mountArgument = Utils::DfsuMountArgumentDescriptors::Alpha(userId, SAME_ACCOUNT);
    auto mp = make_unique<OHOS::Storage::DistributedFile::MountPoint>(mountArgument);
    shared_ptr<OHOS::Storage::DistributedFile::MountPoint> smp = move(mp);

    try {
        auto sdm = DeviceManagerAgent::GetInstance();

        smp->Mount();
        sdm->Start();
        sdm->JoinGroup(smp);
        smp->Umount();
        sdm->QuitGroup(smp);
        sdm->Stop();
        LOGE("testcase distributedFileDaemon_service_test_007_repeats_create_net_instance run end");
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        EXPECT_EQ(0, 1);
    }
    EXPECT_EQ(0, 0);
}
/**
 * @tc.name: distributedFileDaemon_service_test_008_repeats_repeat_create_net_instance
 * @tc.desc: Verify the JoinGroup/QuitGroup function.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DistributedFileDaemonServiceTest,
         distributedFileDaemon_service_test_008_repeats_repeat_create_net_instance,
         TestSize.Level1)
{
    const int userId = 4201;
    auto mountArgument = Utils::DfsuMountArgumentDescriptors::Alpha(userId, SAME_ACCOUNT);
    auto mp = make_unique<OHOS::Storage::DistributedFile::MountPoint>(mountArgument);
    shared_ptr<OHOS::Storage::DistributedFile::MountPoint> smp = move(mp);

    try {
        auto sdm = DeviceManagerAgent::GetInstance();

        smp->Mount();
        sdm->Start();
        sdm->JoinGroup(smp);
        try {
            sdm->JoinGroup(smp);
        } catch (const exception &e) {
            string err = e.what();
            string out = "Mountpoint existed";

            if (err.find(out) == string::npos) {
                LOGE("008-Error:%{public}s", e.what());
                EXPECT_EQ(0, 1);
            }
        }
        smp->Umount();
        sdm->QuitGroup(smp);
        sdm->Stop();
        LOGE("testcase distributedFileDaemon_service_test_008_repeats_repeat_create_net_instance run end");
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        EXPECT_EQ(0, 1);
    }
    EXPECT_EQ(0, 0);
}
/**
 * @tc.name: distributedFileDaemon_service_test_009_repeats_create_multipe_net_instance
 * @tc.desc: Verify the JoinGroup/QuitGroup function.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DistributedFileDaemonServiceTest,
         distributedFileDaemon_service_test_009_repeats_create_multipe_net_instance,
         TestSize.Level1)
{
    const int userId1 = 4202;
    const int userId2 = 4203;
    auto mountArgument1 = Utils::DfsuMountArgumentDescriptors::Alpha(userId1, SAME_ACCOUNT);
    auto mountArgument2 = Utils::DfsuMountArgumentDescriptors::Alpha(userId2, SAME_ACCOUNT);
    auto mp1 = make_unique<OHOS::Storage::DistributedFile::MountPoint>(mountArgument1);
    auto mp2 = make_unique<OHOS::Storage::DistributedFile::MountPoint>(mountArgument2);
    shared_ptr<OHOS::Storage::DistributedFile::MountPoint> smp1 = move(mp1);
    shared_ptr<OHOS::Storage::DistributedFile::MountPoint> smp2 = move(mp2);

    try {
        auto sdm = DeviceManagerAgent::GetInstance();

        smp1->Mount();
        smp2->Mount();
        sdm->Start();
        sdm->JoinGroup(smp1);
        sdm->JoinGroup(smp2);
        smp1->Umount();
        smp2->Umount();
        sdm->QuitGroup(smp1);
        sdm->QuitGroup(smp2);
        sdm->Stop();
        LOGE("testcase distributedFileDaemon_service_test_009_repeats_create_multipe_net_instance run end");
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        EXPECT_EQ(0, 1);
    }
    EXPECT_EQ(0, 0);
}
/**
 * @tc.name: distributedFileDaemon_service_test_010_repeats_repeat_del_net_instance
 * @tc.desc: Verify the JoinGroup/QuitGroup function.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DistributedFileDaemonServiceTest,
         distributedFileDaemon_service_test_010_repeats_repeat_del_net_instance,
         TestSize.Level1)
{
    const int userId = 4204;
    auto mountArgument = Utils::DfsuMountArgumentDescriptors::Alpha(userId, SAME_ACCOUNT);
    auto mp = make_unique<OHOS::Storage::DistributedFile::MountPoint>(mountArgument);
    shared_ptr<OHOS::Storage::DistributedFile::MountPoint> smp = move(mp);

    try {
        auto sdm = DeviceManagerAgent::GetInstance();

        smp->Mount();
        sdm->Start();
        sdm->JoinGroup(smp);
        smp->Umount();
        sdm->QuitGroup(smp);
        try {
            sdm->QuitGroup(smp);
        } catch (const exception &e) {
            string err = e.what();
            string out = "Mountpoint didn't exist";

            if (err.find(out) == string::npos) {
                LOGE("010-Error:%{public}s", e.what());
                EXPECT_EQ(0, 1);
            }
        }
        sdm->Stop();
        LOGE("testcase distributedFileDaemon_service_test_010_repeats_repeat_del_net_instance run end");
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        EXPECT_EQ(0, 1);
    }
    EXPECT_EQ(0, 0);
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
