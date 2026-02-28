/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "disk_monitor.h"

#include <chrono>
#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <thread>
#include <unistd.h>

#include "assistant.h"
#include "cloud_disk_sync_folder.h"
#include "securec.h"

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

namespace {
constexpr int32_t INVALID_SYNC_FOLDER = 0;
const string MOUNT_PATH = "/data/service";
const string DATA_SERVICE_EL2 = "/data/service/el2/";
const string HMDFS_DOCS = "/hmdfs/account/files/Docs";
const vector<string> BLACK_DIRS = {"/.Trash", "/.Recent", "/.thumbs", "/HO_DATA_EXT_MISC"};
} // namespace

class DiskMonitorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline shared_ptr<AssistantMock> insMock_;
    int32_t userId = 100;
};

void DiskMonitorTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DiskMonitorTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DiskMonitorTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    insMock_ = make_shared<AssistantMock>();
    Assistant::ins = insMock_;
    insMock_->EnableMock();
}

void DiskMonitorTest::TearDown()
{
    insMock_->DisableMock();
    Assistant::ins = nullptr;
    insMock_ = nullptr;
    GTEST_LOG_(INFO) << "TearDown";
}

/**
@tc.name: GetInstanceTest001
@tc.desc: Verify the GetInstance function
@tc.type: FUNC
*/
HWTEST_F(DiskMonitorTest, GetInstanceTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstanceTest001 Begin";
    try {
        DiskMonitor::GetInstance();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetInstanceTest001 Error";
    }
    GTEST_LOG_(INFO) << "GetInstanceTest001 End";
}

/**
 * @tc.name: StartMonitorTest001
 * @tc.desc: Verify the StartMonitor function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, StartMonitorTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartMonitorTest001 Begin";
    try {
        int32_t userId = 1;
        EXPECT_CALL(*insMock_, fanotify_init(_, _)).WillOnce(Return(-1));
        DiskMonitor::GetInstance().isRunning_ = false;
        bool res = DiskMonitor::GetInstance().StartMonitor(userId);
        DiskMonitor::GetInstance().StopMonitor();
        EXPECT_FALSE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartMonitorTest001 Error";
    }
    GTEST_LOG_(INFO) << "StartMonitorTest001 End";
}

/**
 * @tc.name: StartMonitorTest002
 * @tc.desc: Verify the tartMonitor function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, StartMonitorTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartMonitorTest002 Begin";
    try {
        int32_t userId = 1;
        EXPECT_CALL(*insMock_, fanotify_init(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*insMock_, fanotify_mark(_, _, _, _, _)).WillOnce(Return(0));
        DIR *ch = reinterpret_cast<DIR *>(UINT64_MAX);
        EXPECT_CALL(*insMock_, opendir(_)).WillRepeatedly(Return(ch));
        EXPECT_CALL(*insMock_, dirfd(_)).WillRepeatedly(Return(0));
        bool res = DiskMonitor::GetInstance().StartMonitor(userId);
        DiskMonitor::GetInstance().StopMonitor();
        EXPECT_TRUE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartMonitorTest002 Error";
    }
    GTEST_LOG_(INFO) << "StartMonitorTest002 End";
}

/**
 * @tc.name: StartMonitorTest003
 * @tc.desc: Verify the StartMonitor function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, StartMonitorTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartMonitorTest003 Begin";
    try {
        int32_t userId = 1;
        EXPECT_CALL(*insMock_, fanotify_init(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock_, fanotify_mark(_, _, _, _, _)).WillRepeatedly(Return(0));
        DIR *ch = reinterpret_cast<DIR *>(UINT64_MAX);
        EXPECT_CALL(*insMock_, opendir(_)).WillRepeatedly(Return(ch));
        EXPECT_CALL(*insMock_, dirfd(_)).WillRepeatedly(Return(0));
        DiskMonitor::GetInstance().isRunning_ = false;
        bool res1 = DiskMonitor::GetInstance().StartMonitor(userId);
        ASSERT_TRUE(res1);
        bool res2 = DiskMonitor::GetInstance().StartMonitor(userId);
        EXPECT_FALSE(res2);
        DiskMonitor::GetInstance().StopMonitor();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartMonitorTest003 Error";
    }
    GTEST_LOG_(INFO) << "StartMonitorTest003 End";
}

/**
 * @tc.name: StopMonitorTest001
 * @tc.desc: Verify the StopMonitor function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, StopMonitorTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopMonitorTest001 Begin";
    try {
        DiskMonitor::GetInstance().StopMonitor();
        EXPECT_EQ(DiskMonitor::GetInstance().userId_, -1);
        EXPECT_EQ(DiskMonitor::GetInstance().isRunning_, false);
        EXPECT_TRUE(DiskMonitor::GetInstance().syncFolderPrefix_.empty());
        EXPECT_TRUE(DiskMonitor::GetInstance().blockList_.empty());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StopMonitorTest001 Error";
    }
    GTEST_LOG_(INFO) << "StopMonitorTest001 End";
}

/**
 * @tc.name: InitStatusTest001
 * @tc.desc: Verify the InitStatus function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, InitStatusTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitStatusTest001 Begin";
    try {
        int32_t userId = 1;
        DiskMonitor::GetInstance().InitStatus(userId);
        EXPECT_EQ(DiskMonitor::GetInstance().userId_, userId);
        EXPECT_TRUE(DiskMonitor::GetInstance().isRunning_);
        EXPECT_EQ(DiskMonitor::GetInstance().syncFolderPrefix_, "/data/service/el2/1/hmdfs/account/files/Docs");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitStatusTest001 Error";
    }
    GTEST_LOG_(INFO) << "InitStatusTest001 End";
}

/**
 * @tc.name: InitFanotifyTest001
 * @tc.desc: Verify the InitFanotify function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, InitFanotifyTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitFanotifyTest001 Begin";
    try {
        EXPECT_CALL(*insMock_, fanotify_init(_, _)).WillOnce(Return(-1));
        bool res = DiskMonitor::GetInstance().InitFanotify();
        EXPECT_FALSE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitFanotifyTest001 Error";
    }
    GTEST_LOG_(INFO) << "InitFanotifyTest001 End";
}

/**
 * @tc.name: InitFanotifyTest002
 * @tc.desc: Verify the InitFanotify function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, InitFanotifyTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitFanotifyTest002 Begin";
    try {
        EXPECT_CALL(*insMock_, fanotify_init(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*insMock_, fanotify_mark(_, _, _, _, _)).WillOnce(Return(-1));
        bool res = DiskMonitor::GetInstance().InitFanotify();
        EXPECT_FALSE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitFanotifyTest002 Error";
    }
    GTEST_LOG_(INFO) << "InitFanotifyTest002 End";
}

/**
 * @tc.name: InitFanotifyTest003
 * @tc.desc: Verify the InitFanotify function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, InitFanotifyTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitFanotifyTest003 Begin";
    try {
        EXPECT_CALL(*insMock_, fanotify_init(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*insMock_, fanotify_mark(_, _, _, _, _)).WillOnce(Return(0));
        EXPECT_CALL(*insMock_, opendir(_)).WillOnce(Return(nullptr));
        bool res = DiskMonitor::GetInstance().InitFanotify();
        EXPECT_FALSE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitFanotifyTest003 Error";
    }
    GTEST_LOG_(INFO) << "InitFanotifyTest003 End";
}

/**
 * @tc.name: InitFanotifyTest004
 * @tc.desc: Verify the InitFanotify function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, InitFanotifyTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitFanotifyTest004 Begin";
    try {
        EXPECT_CALL(*insMock_, fanotify_init(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*insMock_, fanotify_mark(_, _, _, _, _)).WillOnce(Return(0));
        DIR *ch = reinterpret_cast<DIR *>(UINT64_MAX);
        EXPECT_CALL(*insMock_, opendir(_)).WillRepeatedly(Return(ch));
        EXPECT_CALL(*insMock_, dirfd(_)).WillRepeatedly(Return(-1));

        bool res = DiskMonitor::GetInstance().InitFanotify();
        EXPECT_FALSE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitFanotifyTest004 Error";
    }
    GTEST_LOG_(INFO) << "InitFanotifyTest004 End";
}

/**
 * @tc.name: InitFanotifyTest005
 * @tc.desc: Verify the InitFanotify function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, InitFanotifyTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitFanotifyTest005 Begin";
    try {
        EXPECT_CALL(*insMock_, fanotify_init(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*insMock_, fanotify_mark(_, _, _, _, _)).WillOnce(Return(0));
        DIR *ch = reinterpret_cast<DIR *>(UINT64_MAX);
        EXPECT_CALL(*insMock_, opendir(_)).WillRepeatedly(Return(ch));
        EXPECT_CALL(*insMock_, dirfd(_)).WillRepeatedly(Return(0));

        bool res = DiskMonitor::GetInstance().InitFanotify();
        EXPECT_TRUE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitFanotifyTest005 Error";
    }
    GTEST_LOG_(INFO) << "InitFanotifyTest005 End";
}

/**
 * @tc.name: CollectEventsTest001
 * @tc.desc: Verify the CollectEvents function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, CollectEventsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CollectEventsTest001 Begin";
    try {
        DiskMonitor::GetInstance().isRunning_ = true;
        DiskMonitor::GetInstance().fanotifyFd_ = -1;
        DiskMonitor::GetInstance().mountFd_ = 0;
        std::thread t([]() { DiskMonitor::GetInstance().CollectEvents(); });
        std::this_thread::sleep_for(std::chrono::microseconds(10));
        DiskMonitor::GetInstance().isRunning_ = false;
        if (t.joinable()) {
            t.join();
        }
    } catch (...) {
        DiskMonitor::GetInstance().isRunning_ = false;
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CollectEventsTest001 Error";
    }
    GTEST_LOG_(INFO) << "CollectEventsTest001 End";
}

/**
 * @tc.name: CollectEventsTest002
 * @tc.desc: Verify the CollectEvents function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, CollectEventsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CollectEventsTest002 Begin";
    try {
        int pipeFd[2];
        if (pipe(pipeFd) != 0) {
            GTEST_SKIP() << "pipe() failed";
        }
        close(pipeFd[1]);
        DiskMonitor::GetInstance().isRunning_ = true;
        DiskMonitor::GetInstance().fanotifyFd_ = pipeFd[0];
        DiskMonitor::GetInstance().mountFd_ = 0;
        std::thread t([]() { DiskMonitor::GetInstance().CollectEvents(); });
        std::this_thread::sleep_for(std::chrono::microseconds(10));
        DiskMonitor::GetInstance().isRunning_ = false;
        if (t.joinable()) {
            t.join();
        }
        close(pipeFd[0]);
    } catch (...) {
        DiskMonitor::GetInstance().isRunning_ = false;
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CollectEventsTest002 Error";
    }
    GTEST_LOG_(INFO) << "CollectEventsTest002 End";
}

/**
 * @tc.name: CollectEventsTest003
 * @tc.desc: Verify the CollectEvents function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, CollectEventsTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CollectEventsTest003 Begin";
    try {
        int pipeFd[2];
        if (pipe(pipeFd) != 0) {
            GTEST_SKIP() << "pipe() failed";
        }
        (void)fcntl(pipeFd[0], F_SETFL, O_NONBLOCK);
        DiskMonitor::GetInstance().isRunning_ = true;
        DiskMonitor::GetInstance().fanotifyFd_ = pipeFd[0];
        DiskMonitor::GetInstance().mountFd_ = 0;
        std::thread t([]() { DiskMonitor::GetInstance().CollectEvents(); });
        std::this_thread::sleep_for(std::chrono::microseconds(10));
        DiskMonitor::GetInstance().isRunning_ = false;
        if (t.joinable()) {
            t.join();
        }
        close(pipeFd[0]);
        close(pipeFd[1]);
    } catch (...) {
        DiskMonitor::GetInstance().isRunning_ = false;
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CollectEventsTest003 Error";
    }
    GTEST_LOG_(INFO) << "CollectEventsTest003 End";
}

/**
 * @tc.name: CollectEventsTest004
 * @tc.desc: Verify the CollectEvents function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, CollectEventsTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CollectEventsTest004 Begin";
    try {
        int pipeFd[2];
        if (pipe(pipeFd) != 0) {
            GTEST_SKIP() << "pipe() failed";
        }
        struct fanotify_event_metadata meta;
        memset_s(&meta, sizeof(meta), 0, sizeof(meta));
        meta.event_len = sizeof(meta) + 32;
        meta.mask = FAN_CREATE;
        (void)write(pipeFd[1], &meta, sizeof(meta));
        (void)write(pipeFd[1], (char*)&meta + sizeof(meta), meta.event_len - sizeof(meta));
        DiskMonitor::GetInstance().isRunning_ = true;
        DiskMonitor::GetInstance().fanotifyFd_ = pipeFd[0];
        DiskMonitor::GetInstance().mountFd_ = -1;
        std::thread t([]() { DiskMonitor::GetInstance().CollectEvents(); });
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        DiskMonitor::GetInstance().isRunning_ = false;
        char unblock = 0;
        (void)write(pipeFd[1], &unblock, 1);
        close(pipeFd[1]);
        if (t.joinable()) {
            t.join();
        }
        close(pipeFd[0]);
    } catch (...) {
        DiskMonitor::GetInstance().isRunning_ = false;
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CollectEventsTest004 Error";
    }
    GTEST_LOG_(INFO) << "CollectEventsTest004 End";
}

/**
 * @tc.name: HandleEventsTest001
 * @tc.desc: Verify the HandleEvents function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, HandleEventsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleEventsTest001 Begin";
    try {
        const size_t bufSize = 256;
        char eventsBuf[bufSize];
        memset_s(eventsBuf, bufSize, 0, bufSize);
        struct fanotify_event_metadata *meta = reinterpret_cast<struct fanotify_event_metadata *>(eventsBuf);
        size_t fidOff = sizeof(struct fanotify_event_metadata);
        meta->event_len = fidOff + sizeof(struct fanotify_event_info_fid);
        meta->mask = FAN_CREATE;
        struct fanotify_event_info_fid *fid = reinterpret_cast<struct fanotify_event_info_fid *>(eventsBuf + fidOff);
        fid->hdr.info_type = FAN_EVENT_INFO_TYPE_FID;
        fid->hdr.len = sizeof(struct fanotify_event_info_fid);
        void **handleSlot = reinterpret_cast<void **>((char *)fid + offsetof(struct fanotify_event_info_fid, handle));
        *handleSlot = nullptr;
        DiskMonitor::GetInstance().mountFd_ = -1;
        DiskMonitor::GetInstance().HandleEvents(-1, eventsBuf, meta->event_len);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleEventsTest001 Error";
    }
    GTEST_LOG_(INFO) << "HandleEventsTest001 End";
}

/**
 * @tc.name: HandleEventsTest002
 * @tc.desc: Verify the HandleEvents function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, HandleEventsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleEventsTest002 Begin";
    try {
        const size_t bufSize = 256;
        char eventsBuf[bufSize];
        memset_s(eventsBuf, bufSize, 0, bufSize);
        struct fanotify_event_metadata *meta = reinterpret_cast<struct fanotify_event_metadata *>(eventsBuf);
        size_t fidOff = sizeof(struct fanotify_event_metadata);
        size_t fhOff = fidOff + sizeof(struct fanotify_event_info_fid);
        meta->event_len = fhOff + 8;
        meta->mask = FAN_CREATE;
        struct fanotify_event_info_fid *fid = reinterpret_cast<struct fanotify_event_info_fid *>(eventsBuf + fidOff);
        fid->hdr.info_type = FAN_EVENT_INFO_TYPE_FID;
        fid->hdr.len = sizeof(struct fanotify_event_info_fid);
        void **handleSlot = reinterpret_cast<void **>((char *)fid + offsetof(struct fanotify_event_info_fid, handle));
        *handleSlot = reinterpret_cast<void *>(eventsBuf + fhOff);
        DiskMonitor::GetInstance().mountFd_ = -1;
        DiskMonitor::GetInstance().HandleEvents(-1, eventsBuf, static_cast<size_t>(meta->event_len));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleEventsTest002 Error";
    }
    GTEST_LOG_(INFO) << "HandleEventsTest002 End";
}

/**
 * @tc.name: HandleEventsTest003
 * @tc.desc: Verify the HandleEvents function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, HandleEventsTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleEventsTest003 Begin";
    try {
        const size_t bufSize = 256;
        char eventsBuf[bufSize];
        memset_s(eventsBuf, bufSize, 0, bufSize);
        struct fanotify_event_metadata *meta = reinterpret_cast<struct fanotify_event_metadata *>(eventsBuf);
        size_t fidOff = sizeof(struct fanotify_event_metadata);
        size_t fhOff = fidOff + sizeof(struct fanotify_event_info_fid);
        meta->event_len = fhOff + 8;
        meta->mask = FAN_CREATE;
        struct fanotify_event_info_fid *fid = reinterpret_cast<struct fanotify_event_info_fid *>(eventsBuf + fidOff);
        fid->hdr.info_type = FAN_EVENT_INFO_TYPE_FID;
        fid->hdr.len = sizeof(struct fanotify_event_info_fid);
        void **handleSlot = reinterpret_cast<void **>((char *)fid + offsetof(struct fanotify_event_info_fid, handle));
        *handleSlot = reinterpret_cast<void *>(eventsBuf + fhOff);
        int pipeFd[2];
        if (pipe(pipeFd) != 0) {
            GTEST_SKIP() << "pipe() failed";
        }
        DiskMonitor::GetInstance().mountFd_ = pipeFd[0];
        DiskMonitor::GetInstance().HandleEvents(pipeFd[0], eventsBuf, static_cast<size_t>(meta->event_len));
        close(pipeFd[0]);
        close(pipeFd[1]);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleEventsTest003 Error";
    }
    GTEST_LOG_(INFO) << "HandleEventsTest003 End";
}

/**
 * @tc.name: EventProcessTest001
 * @tc.desc: Verify EventProcess function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, EventProcessTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventProcessTest001 Begin";
    try {
        fanotify_event_metadata metaData;
        metaData.mask = FAN_CREATE;
        string filePath = "filePath";
        DiskMonitor::GetInstance().EventProcess(&metaData, filePath);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "EventProcessTest001 Error";
    }
    GTEST_LOG_(INFO) << "EventProcessTest001 End";
}

/**
 * @tc.name: EventProcessTest002
 * @tc.desc: Verify EventProcess function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, EventProcessTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventProcessTest002 Begin";
    try {
        fanotify_event_metadata metaData;
        metaData.mask = FAN_DELETE;
        string filePath = "filePath";
        DiskMonitor::GetInstance().EventProcess(&metaData, filePath);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "EventProcessTest002 Error";
    }
    GTEST_LOG_(INFO) << "EventProcessTest002 End";
}

/**
 * @tc.name: EventProcessTest003
 * @tc.desc: Verify EventProcess function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, EventProcessTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventProcessTest003 Begin";
    try {
        fanotify_event_metadata metaData;
        metaData.mask = FAN_MOVED_FROM;
        string filePath = "filePath";
        DiskMonitor::GetInstance().EventProcess(&metaData, filePath);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "EventProcessTest003 Error";
    }
    GTEST_LOG_(INFO) << "EventProcessTest003 End";
}

/**
 * @tc.name: EventProcessTest004
 * @tc.desc: Verify EventProcess function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, EventProcessTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventProcessTest004 Begin";
    try {
        fanotify_event_metadata metaData;
        metaData.mask = FAN_MOVED_TO;
        string filePath = "filePath";
        DiskMonitor::GetInstance().EventProcess(&metaData, filePath);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "EventProcessTest004 Error";
    }
    GTEST_LOG_(INFO) << "EventProcessTest004 End";
}

/**
 * @tc.name: EventProcessTest005
 * @tc.desc: Verify EventProcess function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, EventProcessTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventProcessTest005 Begin";
    try {
        fanotify_event_metadata metaData;
        metaData.mask = FAN_CLOSE_WRITE;
        string filePath = "filePath";
        DiskMonitor::GetInstance().EventProcess(&metaData, filePath);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "EventProcessTest005 Error";
    }
    GTEST_LOG_(INFO) << "EventProcessTest005 End";
}

/**
 * @tc.name: EventProcessTest006
 * @tc.desc: Verify EventProcess function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, EventProcessTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventProcessTest006 Begin";
    try {
        fanotify_event_metadata metaData;
        metaData.mask = FAN_CLOSE_WRITE;
        DiskMonitor::GetInstance().syncFolderPrefix_ = "a";
        string filePath = "abc";
        DiskMonitor::GetInstance().blockList_ = {"b", "c"};
        DiskMonitor::GetInstance().EventProcess(&metaData, filePath);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "EventProcessTest006 Error";
    }
    GTEST_LOG_(INFO) << "EventProcessTest006 End";
}

/**
 * @tc.name: EventProcessTest007
 * @tc.desc: Verify EventProcess function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, EventProcessTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventProcessTest007 Begin";
    try {
        fanotify_event_metadata metaData;
        metaData.mask = FAN_ONDIR;
        DiskMonitor::GetInstance().syncFolderPrefix_ = "a";
        string filePath = "abc";
        DiskMonitor::GetInstance().blockList_ = {"b", "c"};
        DiskMonitor::GetInstance().EventProcess(&metaData, filePath);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "EventProcessTest007 Error";
    }
    GTEST_LOG_(INFO) << "EventProcessTest007 End";
}

/**
 * @tc.name: HandleCreateTest001
 * @tc.desc: Verify HandleCreate function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, HandleCreateTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleCreateTest001 Begin";
    try {
        DiskMonitor::GetInstance().syncFolderPrefix_ = "b";
        string path = "abc";
        DiskMonitor::GetInstance().HandleCreate(path);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleCreateTest001 Error";
    }
    GTEST_LOG_(INFO) << "HandleCreateTest001 End";
}

/**
 * @tc.name: HandleCreateTest002
 * @tc.desc: Verify HandleCreate function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, HandleCreateTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleCreateTest002 Begin";
    try {
        DiskMonitor::GetInstance().syncFolderPrefix_ = "a";
        string path = "abc";
        DiskMonitor::GetInstance().blockList_ = {"b", "c"};
        CloudDiskSyncFolder::GetInstance().syncFolderMap.clear();
        CloudDiskSyncFolder::GetInstance().syncFolderMap.insert({1, {"bundleName", "abc"}});
        DiskMonitor::GetInstance().HandleCreate(path);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleCreateTest002 Error";
    }
    GTEST_LOG_(INFO) << "HandleCreateTest002 End";
}

/**
 * @tc.name: HandleDeleteTest001
 * @tc.desc: Verify HandleDelete function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, HandleDeleteTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleDeleteTest001 Begin";
    try {
        DiskMonitor::GetInstance().syncFolderPrefix_ = "b";
        string path = "abc";
        DiskMonitor::GetInstance().HandleDelete(path);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleDeleteTest001 Error";
    }
    GTEST_LOG_(INFO) << "HandleDeleteTest001 End";
}

/**
 * @tc.name: HandleDeleteTest002
 * @tc.desc: Verify HandleDelete function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, HandleDeleteTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleDeleteTest002 Begin";
    try {
        DiskMonitor::GetInstance().syncFolderPrefix_ = "a";
        string path = "abc";
        DiskMonitor::GetInstance().blockList_ = {"b", "c"};
        CloudDiskSyncFolder::GetInstance().syncFolderMap.clear();
        CloudDiskSyncFolder::GetInstance().syncFolderMap.insert({1, {"bundleName", "abc"}});
        DiskMonitor::GetInstance().HandleDelete(path);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleDeleteTest002 Error";
    }
    GTEST_LOG_(INFO) << "HandleDeleteTest002 End";
}

/**
 * @tc.name: HandleMoveFromTest001
 * @tc.desc: Verify HandleMoveFrom function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, HandleMoveFromTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleMoveFromTest001 Begin";
    try {
        DiskMonitor::GetInstance().syncFolderPrefix_ = "b";
        string path = "abc";
        DiskMonitor::GetInstance().HandleMoveFrom(path);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleMoveFromTest001 Error";
    }
    GTEST_LOG_(INFO) << "HandleMoveFromTest001 End";
}

/**
 * @tc.name: HandleMoveFromTest002
 * @tc.desc: Verify HandleMoveFrom function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, HandleMoveFromTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleMoveFromTest002 Begin";
    try {
        DiskMonitor::GetInstance().syncFolderPrefix_ = "a";
        string path = "abc";
        DiskMonitor::GetInstance().blockList_ = {"b", "c"};
        CloudDiskSyncFolder::GetInstance().syncFolderMap.clear();
        CloudDiskSyncFolder::GetInstance().syncFolderMap.insert({1, {"bundleName", "abc"}});
        DiskMonitor::GetInstance().HandleMoveFrom(path);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleMoveFromTest002 Error";
    }
    GTEST_LOG_(INFO) << "HandleMoveFromTest002 End";
}

/**
 * @tc.name: HandleMoveToTest001
 * @tc.desc: Verify HandleMoveTo function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, HandleMoveToTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleMoveToTest001 Begin";
    try {
        DiskMonitor::GetInstance().syncFolderPrefix_ = "b";
        string path = "abc";
        DiskMonitor::GetInstance().oldEventInfo_.syncFolderIndex = INVALID_SYNC_FOLDER;
        DiskMonitor::GetInstance().HandleMoveTo(path);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleMoveToTest001 Error";
    }
    GTEST_LOG_(INFO) << "HandleMoveToTest001 End";
}

/**
 * @tc.name: HandleMoveToTest002
 * @tc.desc: Verify HandleMoveTo function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, HandleMoveToTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleMoveToTest002 Begin";
    try {
        DiskMonitor::GetInstance().syncFolderPrefix_ = "a";
        string path = "abc";
        DiskMonitor::GetInstance().blockList_ = {"b", "c"};
        CloudDiskSyncFolder::GetInstance().syncFolderMap.clear();
        CloudDiskSyncFolder::GetInstance().syncFolderMap.insert({1, {"bundleName", "abc"}});
        DiskMonitor::GetInstance().oldEventInfo_.syncFolderIndex = INVALID_SYNC_FOLDER;
        DiskMonitor::GetInstance().HandleMoveTo(path);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleMoveToTest002 Error";
    }
    GTEST_LOG_(INFO) << "HandleMoveToTest002 End";
}

/**
 * @tc.name: HandleMoveToTest003
 * @tc.desc: Verify HandleMoveTo function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, HandleMoveToTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleMoveToTest003 Begin";
    try {
        DiskMonitor::GetInstance().syncFolderPrefix_ = "b";
        string path = "abc";
        DiskMonitor::GetInstance().oldEventInfo_.syncFolderIndex = 1;
        DiskMonitor::GetInstance().HandleMoveTo(path);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleMoveToTest003 Error";
    }
    GTEST_LOG_(INFO) << "HandleMoveToTest003 End";
}

/**
 * @tc.name: HandleMoveToTest004
 * @tc.desc: Verify HandleMoveTo function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, HandleMoveToTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleMoveToTest004 Begin";
    try {
        DiskMonitor::GetInstance().syncFolderPrefix_ = "a";
        string path = "abc";
        DiskMonitor::GetInstance().blockList_ = {"b", "c"};
        CloudDiskSyncFolder::GetInstance().syncFolderMap.clear();
        CloudDiskSyncFolder::GetInstance().syncFolderMap.insert({1, {"bundleName", "abc"}});
        DiskMonitor::GetInstance().oldEventInfo_.syncFolderIndex = 1;
        DiskMonitor::GetInstance().HandleMoveTo(path);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleMoveToTest004 Error";
    }
    GTEST_LOG_(INFO) << "HandleMoveToTest004 End";
}

/**
 * @tc.name: HandleMoveToTest005
 * @tc.desc: Verify HandleMoveTo function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, HandleMoveToTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleMoveToTest005 Begin";
    try {
        DiskMonitor::GetInstance().syncFolderPrefix_ = "a";
        string path = "abc";
        DiskMonitor::GetInstance().blockList_ = {"b", "c"};
        CloudDiskSyncFolder::GetInstance().syncFolderMap.clear();
        CloudDiskSyncFolder::GetInstance().syncFolderMap.insert({2, {"bundleName", "abc"}});
        DiskMonitor::GetInstance().oldEventInfo_.syncFolderIndex = 1;
        DiskMonitor::GetInstance().HandleMoveTo(path);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleMoveToTest005 Error";
    }
    GTEST_LOG_(INFO) << "HandleMoveToTest005 End";
}

/**
 * @tc.name: HandleCloseWriteTest001
 * @tc.desc: Verify HandleCloseWrite function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, HandleCloseWriteTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleCloseWriteTest001 Begin";
    try {
        DiskMonitor::GetInstance().syncFolderPrefix_ = "b";
        string path = "abc";
        DiskMonitor::GetInstance().HandleCloseWrite(path);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleCloseWriteTest001 Error";
    }
    GTEST_LOG_(INFO) << "HandleCloseWriteTest001 End";
}

/**
 * @tc.name: HandleCloseWriteTest002
 * @tc.desc: Verify HandleCloseWrite function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, HandleCloseWriteTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleCloseWriteTest002 Begin";
    try {
        DiskMonitor::GetInstance().syncFolderPrefix_ = "a";
        string path = "abc";
        DiskMonitor::GetInstance().blockList_ = {"b", "c"};
        CloudDiskSyncFolder::GetInstance().syncFolderMap.clear();
        CloudDiskSyncFolder::GetInstance().syncFolderMap.insert({1, {"bundleName", "abc"}});
        DiskMonitor::GetInstance().HandleCloseWrite(path);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleCloseWriteTest002 Error";
    }
    GTEST_LOG_(INFO) << "HandleCloseWriteTest002 End";
}

/**
 * @tc.name: IsInBlockListTest001
 * @tc.desc: Verify the IsInBlockList function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, IsInBlockListTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsInBlockListTest001 Begin";
    try {
        DiskMonitor::GetInstance().syncFolderPrefix_ = "b";
        string path = "abc";
        bool res = DiskMonitor::GetInstance().IsInBlockList(path);
        EXPECT_TRUE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsInBlockListTest001 Error";
    }
    GTEST_LOG_(INFO) << "IsInBlockListTest001 End";
}

/**
 * @tc.name: IsInBlockListTest002
 * @tc.desc: Verify the IsInBlockList function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, IsInBlockListTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsInBlockListTest002 Begin";
    try {
        DiskMonitor::GetInstance().syncFolderPrefix_ = "a";
        string path = "abc";
        DiskMonitor::GetInstance().blockList_ = {"a", "b"};
        bool res = DiskMonitor::GetInstance().IsInBlockList(path);
        EXPECT_TRUE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsInBlockListTest002 Error";
    }
    GTEST_LOG_(INFO) << "IsInBlockListTest002 End";
}

/**
 * @tc.name: IsInBlockListTest003
 * @tc.desc: Verify the IsInBlockList function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, IsInBlockListTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsInBlockListTest003 Begin";
    try {
        DiskMonitor::GetInstance().syncFolderPrefix_ = "a";
        string path = "abc";
        DiskMonitor::GetInstance().blockList_ = {"b", "c"};
        bool res = DiskMonitor::GetInstance().IsInBlockList(path);
        EXPECT_FALSE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsInBlockListTest003 Error";
    }
    GTEST_LOG_(INFO) << "IsInBlockListTest003 End";
}

/**
 * @tc.name: GetSyncFolderIndexTest001
 * @tc.desc: Verify the GetSyncFolderIndex function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, GetSyncFolderIndexTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncFolderIndexTest001 Begin";
    try {
        DiskMonitor::GetInstance().syncFolderPrefix_ = "b";
        string path = "abc";
        pair<uint32_t, bool> res = DiskMonitor::GetInstance().GetSyncFolderIndex(path);
        EXPECT_EQ(res.first, INVALID_SYNC_FOLDER);
        EXPECT_FALSE(res.second);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSyncFolderIndexTest001 Error";
    }
    GTEST_LOG_(INFO) << "GetSyncFolderIndexTest001 End";
}

/**
 * @tc.name: GetSyncFolderIndexTest002
 * @tc.desc: Verify the GetSyncFolderIndex function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, GetSyncFolderIndexTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncFolderIndexTest002 Begin";
    try {
        DiskMonitor::GetInstance().syncFolderPrefix_ = "a";
        string path = "abc/d";
        DiskMonitor::GetInstance().blockList_ = {"b", "c"};
        CloudDiskSyncFolder::GetInstance().syncFolderMap.clear();
        CloudDiskSyncFolder::GetInstance().syncFolderMap.insert({1, {"bundleName", "abc"}});
        pair<uint32_t, bool> res = DiskMonitor::GetInstance().GetSyncFolderIndex(path);
        EXPECT_EQ(res.first, 1);
        EXPECT_FALSE(res.second);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSyncFolderIndexTest002 Error";
    }
    GTEST_LOG_(INFO) << "GetSyncFolderIndexTest002 End";
}

/**
 * @tc.name: GetSyncFolderIndexTest003
 * @tc.desc: Verify the GetSyncFolderIndex function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, GetSyncFolderIndexTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncFolderIndexTest003 Begin";
    try {
        DiskMonitor::GetInstance().syncFolderPrefix_ = "a";
        string path = "abc";
        DiskMonitor::GetInstance().blockList_ = {"b", "c"};
        CloudDiskSyncFolder::GetInstance().syncFolderMap.clear();
        CloudDiskSyncFolder::GetInstance().syncFolderMap.insert({1, {"bundleName", "abc"}});
        pair<uint32_t, bool> res = DiskMonitor::GetInstance().GetSyncFolderIndex(path);
        EXPECT_EQ(res.first, 1);
        EXPECT_TRUE(res.second);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSyncFolderIndexTest003 Error";
    }
    GTEST_LOG_(INFO) << "GetSyncFolderIndexTest003 End";
}

/**
 * @tc.name: GetSyncFolderIndexTest004
 * @tc.desc: Verify the GetSyncFolderIndex function
 * @tc.type: FUNC
 */
HWTEST_F(DiskMonitorTest, GetSyncFolderIndexTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncFolderIndexTest004 Begin";
    try {
        DiskMonitor::GetInstance().syncFolderPrefix_ = "a";
        string path = "abc";
        DiskMonitor::GetInstance().blockList_ = {"b", "c"};
        CloudDiskSyncFolder::GetInstance().syncFolderMap.clear();
        CloudDiskSyncFolder::GetInstance().syncFolderMap.insert({1, {"bundleName", "b"}});
        pair<uint32_t, bool> res = DiskMonitor::GetInstance().GetSyncFolderIndex(path);
        EXPECT_EQ(res.first, INVALID_SYNC_FOLDER);
        EXPECT_FALSE(res.second);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSyncFolderIndexTest004 Error";
    }
    GTEST_LOG_(INFO) << "GetSyncFolderIndexTest004 End";
}
} // namespace OHOS::FileManagement::CloudDiskService::Test
