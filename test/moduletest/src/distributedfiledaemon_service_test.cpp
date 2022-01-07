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

static const string srcHead = "/data/misc_ce/";
static const string dstHead = "/mnt/hmdfs/";
static const string cacheHead = "/data/misc_ce/";
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
        OHOS::Storage::DistributedFile::Utils::MountArgumentDescriptors::Alpha(9527));

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
 * @tc.name: distributedFileDaemon_service_test_001_mount
 * @tc.desc: Verify the mount/umount function.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DistributedFileDaemonServiceTest, distributedFileDaemon_service_test_001_mount, TestSize.Level1)
{
    const int userId = 3333;

    auto mountArgument = OHOS::Storage::DistributedFile::Utils::MountArgumentDescriptors::Alpha(userId);
    auto mp = make_unique<OHOS::Storage::DistributedFile::MountPoint>(mountArgument);

    shared_ptr<OHOS::Storage::DistributedFile::MountPoint> smp = move(mp);

    try {
        smp->Mount();

        const string src = mountArgument.GetFullSrc();
        const char *srcPath = src.data();
        int ret = access(srcPath, F_OK);
        EXPECT_EQ(0, ret);

        const string dst = mountArgument.GetFullDst();
        const char *dstPath = dst.data();
        ret = access(dstPath, F_OK);
        EXPECT_EQ(0, ret);

        const string cache = mountArgument.GetCachePath();
        const char *cachePath = cache.data();
        ret = access(cachePath, F_OK);
        EXPECT_EQ(0, ret);

        stringstream srcTest;
        srcTest << src << "/test";
        Utils::ForceCreateDirectory(srcTest.str(), S_IRWXU | S_IRWXG | S_IXOTH);

        stringstream dstTest;
        dstTest << dst << "/device_view/local"
                << "/test";
        const string pathStr = dstTest.str();
        const char *dstPathTest = pathStr.data();
        ret = access(dstPathTest, F_OK);
        EXPECT_EQ(0, ret);

        Utils::ForceRemoveDirectory(dstPathTest);

        const string strTestStr = srcTest.str();
        ret = access(strTestStr.data(), F_OK);
        EXPECT_EQ(-1, ret);

        smp->Umount();

        stringstream srcStream;
        srcStream << srcHead << userId;
        Utils::ForceRemoveDirectory(srcStream.str());

        ret = access(srcStream.str().data(), F_OK);
        EXPECT_EQ(-1, ret);

        LOGE("testcase distributedFileDaemon_service_test_001_mount run end");
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        EXPECT_EQ(0, 1);
    }
    EXPECT_EQ(0, 0);
}

/**
 * @tc.name: distributedFileDaemon_service_test_002_multiple_mount
 * @tc.desc: Verify the mount/umount function.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DistributedFileDaemonServiceTest, distributedFileDaemon_service_test_002_multiple_mount, TestSize.Level1)
{
    const int userId[2] = {3335, 3336};
    Utils::MountArgument mountArgument[2];
    shared_ptr<OHOS::Storage::DistributedFile::MountPoint> smpArr[2];

    const int len = sizeof(userId) / sizeof(userId[0]);
    for (int i = 0; i < len; i++) {
        mountArgument[i] = OHOS::Storage::DistributedFile::Utils::MountArgumentDescriptors::Alpha(userId[i]);
        auto mp = make_unique<OHOS::Storage::DistributedFile::MountPoint>(mountArgument[i]);
        smpArr[i] = move(mp);
    }

    try {
        for (int j = 0; j < len; j++) {
            smpArr[j]->Mount();
        }

        for (int k = 0; k < len; k++) {
            const string src = mountArgument[k].GetFullSrc();
            const char *srcPath = src.data();
            int ret = access(srcPath, F_OK);
            EXPECT_EQ(0, ret);

            const string dst = mountArgument[k].GetFullDst();
            const char *dstPath = dst.data();
            ret = access(dstPath, F_OK);
            EXPECT_EQ(0, ret);

            const string cache = mountArgument[k].GetCachePath();
            const char *cachePath = cache.data();
            ret = access(cachePath, F_OK);
            EXPECT_EQ(0, ret);

            stringstream srcTest;
            srcTest << src << "/test";
            Utils::ForceCreateDirectory(srcTest.str(), S_IRWXU | S_IRWXG | S_IXOTH);

            stringstream dstTest;
            dstTest << dst << "/device_view/local"
                    << "/test";
            const string pathStr = dstTest.str();
            const char *dstPathTest = pathStr.data();
            ret = access(dstPathTest, F_OK);
            EXPECT_EQ(0, ret);

            Utils::ForceRemoveDirectory(dstPathTest);

            const string strTestStr = srcTest.str();
            ret = access(strTestStr.data(), F_OK);
            EXPECT_EQ(-1, ret);

            smpArr[k]->Umount();

            stringstream srcStream;
            srcStream << srcHead << userId[k];
            Utils::ForceRemoveDirectory(srcStream.str());

            ret = access(srcStream.str().data(), F_OK);
            EXPECT_EQ(-1, ret);
        }

        LOGE("testcase distributedFileDaemon_service_test_002_mount run end");
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        EXPECT_EQ(0, 1);
    }
    EXPECT_EQ(0, 0);
}

/**
 * @tc.name: distributedFileDaemon_service_test_003_unmount
 * @tc.desc: Verify the mount/umount function.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DistributedFileDaemonServiceTest, distributedFileDaemon_service_test_003_unmount, TestSize.Level1)
{
    const int userId = 3337;

    auto mountArgument = OHOS::Storage::DistributedFile::Utils::MountArgumentDescriptors::Alpha(userId);
    auto mp = make_unique<OHOS::Storage::DistributedFile::MountPoint>(mountArgument);

    shared_ptr<OHOS::Storage::DistributedFile::MountPoint> smp = move(mp);

    try {
        smp->Mount();

        const string dst = mountArgument.GetFullDst();
        const char *dstPath = dst.data();
        stringstream dstTest1, dstTest2;
        dstTest1 << dst << "/device_view/local"
                 << "/test1";
        dstTest2 << dst << "/device_view/local"
                 << "/test2";
        Utils::ForceCreateDirectory(dstTest1.str(), S_IRWXU | S_IRWXG | S_IXOTH);
        Utils::ForceCreateDirectory(dstTest2.str(), S_IRWXU | S_IRWXG | S_IXOTH);

        const string src = mountArgument.GetFullSrc();
        const string srcTest1 = src + "/test1";
        int ret = access(srcTest1.data(), F_OK);
        EXPECT_EQ(0, ret);

        Utils::ForceRemoveDirectory(srcTest1);

        ret = access(dstTest1.str().data(), F_OK);
        EXPECT_EQ(-1, ret);

        smp->Umount();

        ret = access(dstPath, F_OK);
        EXPECT_EQ(-1, ret);

        const string srcTest2 = src + "/test2";
        const char *srcPathTest2 = srcTest2.data();
        ret = access(srcTest2.data(), F_OK);
        EXPECT_EQ(0, ret);

        Utils::ForceRemoveDirectory(srcPathTest2);

        stringstream srcStream;
        srcStream << srcHead << userId;
        Utils::ForceRemoveDirectory(srcStream.str());

        ret = access(srcStream.str().data(), F_OK);
        EXPECT_EQ(-1, ret);

        LOGE("testcase distributedFileDaemon_service_test_003_unmount run end");
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        EXPECT_EQ(0, 1);
    }
    EXPECT_EQ(0, 0);
}

/**
 * @tc.name: distributedFileDaemon_service_test_004_repeats_mount
 * @tc.desc: Verify the mount/umount function.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DistributedFileDaemonServiceTest, distributedFileDaemon_service_test_004_repeats_mount, TestSize.Level1)
{
    const int userId = 3338;

    auto mountArgument = OHOS::Storage::DistributedFile::Utils::MountArgumentDescriptors::Alpha(userId);
    auto mp = make_unique<OHOS::Storage::DistributedFile::MountPoint>(mountArgument);

    shared_ptr<OHOS::Storage::DistributedFile::MountPoint> smp = move(mp);

    try {
        smp->Mount();
        smp->Mount();

        const string src = mountArgument.GetFullSrc();
        const char *srcPath = src.data();
        int ret = access(srcPath, F_OK);
        EXPECT_EQ(0, ret);

        const string dst = mountArgument.GetFullDst();
        const char *dstPath = dst.data();
        ret = access(dstPath, F_OK);
        EXPECT_EQ(0, ret);

        const string cache = mountArgument.GetCachePath();
        const char *cachePath = cache.data();
        ret = access(cachePath, F_OK);
        EXPECT_EQ(0, ret);

        stringstream srcTest;
        srcTest << src << "/test";
        Utils::ForceCreateDirectory(srcTest.str(), S_IRWXU | S_IRWXG | S_IXOTH);

        stringstream dstTest;
        dstTest << dst << "/device_view/local"
                << "/test";
        const string pathStr = dstTest.str();
        const char *dstPathTest = pathStr.data();
        ret = access(dstPathTest, F_OK);
        EXPECT_EQ(0, ret);

        Utils::ForceRemoveDirectory(dstPathTest);

        const string strTestStr = srcTest.str();
        ret = access(strTestStr.data(), F_OK);
        EXPECT_EQ(-1, ret);

        smp->Umount();

        stringstream srcStream;
        srcStream << srcHead << userId;
        Utils::ForceRemoveDirectory(srcStream.str());

        ret = access(srcStream.str().data(), F_OK);
        EXPECT_EQ(-1, ret);

        LOGE("testcase distributedFileDaemon_service_test_004_repeats_mount run end");
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        EXPECT_EQ(0, 1);
    }
    EXPECT_EQ(0, 0);
}

/**
 * @tc.name: distributedFileDaemon_service_test_005_repeats_unmount
 * @tc.desc: Verify the mount/umount function.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DistributedFileDaemonServiceTest, distributedFileDaemon_service_test_005_repeats_unmount, TestSize.Level1)
{
    const int userId = 3339;

    auto mountArgument = OHOS::Storage::DistributedFile::Utils::MountArgumentDescriptors::Alpha(userId);
    auto mp = make_unique<OHOS::Storage::DistributedFile::MountPoint>(mountArgument);

    shared_ptr<OHOS::Storage::DistributedFile::MountPoint> smp = move(mp);

    try {
        smp->Mount();

        const string dst = mountArgument.GetFullDst();
        const char *dstPath = dst.data();
        stringstream dstTest1, dstTest2;
        dstTest1 << dst << "/device_view/local"
                 << "/test1";
        dstTest2 << dst << "/device_view/local"
                 << "/test2";
        Utils::ForceCreateDirectory(dstTest1.str(), S_IRWXU | S_IRWXG | S_IXOTH);
        Utils::ForceCreateDirectory(dstTest2.str(), S_IRWXU | S_IRWXG | S_IXOTH);

        const string src = mountArgument.GetFullSrc();
        const string srcTest1 = src + "/test1";
        const char *srcPathTest1 = srcTest1.data();
        int ret = access(srcPathTest1, F_OK);
        EXPECT_EQ(0, ret);

        Utils::ForceRemoveDirectory(srcPathTest1);

        ret = access(dstTest1.str().data(), F_OK);
        EXPECT_EQ(-1, ret);

        smp->Umount();
        try {
            smp->Umount();
        } catch (const exception &e) {
            string err = e.what();
            string out = "No such file or directory";

            if (err.find(out) == string::npos) {
                LOGE("005-Error:%{public}s", e.what());
                EXPECT_EQ(0, 1);
            }
        }

        ret = access(dstPath, F_OK);
        EXPECT_EQ(-1, ret);

        const string srcTest2 = src + "/test2";
        const char *srcPathTest2 = srcTest2.data();
        printf("005ck--7--info:%s\n", srcPathTest2);
        ret = access(srcPathTest2, F_OK);
        EXPECT_EQ(0, ret);

        Utils::ForceRemoveDirectory(srcPathTest2);

        stringstream srcStream;
        srcStream << srcHead << userId;
        Utils::ForceRemoveDirectory(srcStream.str());

        ret = access(srcStream.str().data(), F_OK);
        EXPECT_EQ(-1, ret);

        LOGE("testcase distributedFileDaemon_service_test_005_unmount run end");
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
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
    auto mountArgument = OHOS::Storage::DistributedFile::Utils::MountArgumentDescriptors::Alpha(userId);
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
    auto mountArgument = OHOS::Storage::DistributedFile::Utils::MountArgumentDescriptors::Alpha(userId);
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
    auto mountArgument1 = OHOS::Storage::DistributedFile::Utils::MountArgumentDescriptors::Alpha(userId1);
    auto mountArgument2 = OHOS::Storage::DistributedFile::Utils::MountArgumentDescriptors::Alpha(userId2);
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
    auto mountArgument = OHOS::Storage::DistributedFile::Utils::MountArgumentDescriptors::Alpha(userId);
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

/**
 * @tc.name: distributedFileDaemon_service_test_015_kernel_notify_deal
 * @tc.desc: Verify the JoinGroup/QuitGroup function.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DistributedFileDaemonServiceTest, distributedFileDaemon_service_test_015_kernel_notify_deal, TestSize.Level1)
{
    const int userId = 4500;
    auto mountArgument = OHOS::Storage::DistributedFile::Utils::MountArgumentDescriptors::Alpha(userId);
    auto mp = make_unique<OHOS::Storage::DistributedFile::MountPoint>(mountArgument);
    shared_ptr<OHOS::Storage::DistributedFile::MountPoint> smp = move(mp);

    KernelTalker kt(
        smp,
        [&](NotifyParam &param) {
            EXPECT_EQ(param.notify, 2);
        },
        [&](const std::string &cid) {
            string str = "remoteCid-1";
            EXPECT_TRUE(strcmp(cid.data(), str.data()) == 0);
        });

    for (int i = 0; i < 7; i++) {
        std::string str = "remoteCid-" + to_string(i);
        NotifyParam param = {i, 72, 16, 8, 336, 326};
        strcpy_s(param.remoteCid, CID_MAX_LEN, str.data());
        kt.NotifyHandler(param);
    }

    EXPECT_EQ(0, 0);
}

} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS