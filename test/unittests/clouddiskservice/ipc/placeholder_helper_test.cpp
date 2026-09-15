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

#include "placeholder_helper.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cerrno>
#include <climits>
#include <iterator>
#include <memory>

#include "assistant.h"
#include "cloud_disk_common.h"
#include "cloud_disk_service_error.h"
#include "cloud_disk_service_metafile.h"

extern "C" {
int removexattr(const char *path, const char *name)
{
    using OHOS::FileManagement::CloudDiskService::Assistant;
    return Assistant::ins == nullptr ? 0 : Assistant::ins->removexattr(path, name);
}
}

namespace OHOS::FileManagement::CloudDiskService {
void ResetPlaceholderMetaFileMock();
void SetPlaceholderMetaFileUpdateResult(int32_t result);
uint32_t GetPlaceholderMetaFileUpdateCount();
uint8_t GetLastPlaceholderMetaFileUpdateState();
std::string GetLastPlaceholderMetaFileUpdateName();
} // namespace OHOS::FileManagement::CloudDiskService

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;

namespace {
constexpr char DOT_ENTRY_NAME[] = ".";
constexpr char DOT_DOT_ENTRY_NAME[] = "..";
constexpr char FAILED_ENTRY_NAME[] = "failed";
constexpr char PLACEHOLDER_ENTRY_NAME[] = "placeholder";
constexpr ino_t TEST_DIRECTORY_INODE = 123;
constexpr int32_t DIRECTORY_STAT_CALL_COUNT = 2;
constexpr int32_t READ_DIRECTORY_CALL_COUNT = 5;

void EncodeTestPlaceholderCount(void *value, uint32_t count)
{
    auto *bytes = static_cast<uint8_t *>(value);
    for (size_t index = 0; index < sizeof(count); ++index) {
        bytes[index] = static_cast<uint8_t>(count >> (index * CHAR_BIT));
    }
}

uint32_t DecodeTestPlaceholderCount(const void *value)
{
    const auto *bytes = static_cast<const uint8_t *>(value);
    uint32_t count = 0;
    for (size_t index = 0; index < sizeof(count); ++index) {
        count |= static_cast<uint32_t>(bytes[index]) << (index * CHAR_BIT);
    }
    return count;
}

struct RecountChildEntries {
    struct dirent dotEntry {};
    struct dirent dotDotEntry {};
    struct dirent failedEntry {};
    struct dirent placeholderEntry {};
    struct stat directoryStat {};
    struct stat fileStat {};

    RecountChildEntries()
    {
        std::copy(std::begin(DOT_ENTRY_NAME), std::end(DOT_ENTRY_NAME), dotEntry.d_name);
        std::copy(std::begin(DOT_DOT_ENTRY_NAME), std::end(DOT_DOT_ENTRY_NAME), dotDotEntry.d_name);
        std::copy(std::begin(FAILED_ENTRY_NAME), std::end(FAILED_ENTRY_NAME), failedEntry.d_name);
        std::copy(std::begin(PLACEHOLDER_ENTRY_NAME), std::end(PLACEHOLDER_ENTRY_NAME), placeholderEntry.d_name);
        directoryStat.st_mode = S_IFDIR;
        directoryStat.st_ino = TEST_DIRECTORY_INODE;
        fileStat.st_mode = S_IFREG;
    }

    DIR *GetDir()
    {
        return reinterpret_cast<DIR *>(&dotEntry);
    }
};

void ExpectRecountChildTraversal(const std::shared_ptr<AssistantMock> &mock,
                                 const std::string &root,
                                 const std::string &failedChild,
                                 const std::string &placeholderChild,
                                 RecountChildEntries &entries)
{
    DIR *dir = entries.GetDir();
    EXPECT_CALL(*mock, MockStat(StrEq(root), _))
        .Times(DIRECTORY_STAT_CALL_COUNT)
        .WillRepeatedly(DoAll(SetArgPointee<1>(entries.directoryStat), Return(0)));
    EXPECT_CALL(*mock, MockStat(StrEq(failedChild), _)).WillOnce(Invoke([](const char *, struct stat *) {
        errno = EIO;
        return -1;
    }));
    EXPECT_CALL(*mock, MockStat(StrEq(placeholderChild), _))
        .WillOnce(DoAll(SetArgPointee<1>(entries.fileStat), Return(0)));
    EXPECT_CALL(*mock, opendir(StrEq(root))).WillOnce(Return(dir));
    EXPECT_CALL(*mock, readdir(dir))
        .Times(READ_DIRECTORY_CALL_COUNT)
        .WillOnce(Return(&entries.dotEntry))
        .WillOnce(Return(&entries.dotDotEntry))
        .WillOnce(Return(&entries.failedEntry))
        .WillOnce(Return(&entries.placeholderEntry))
        .WillOnce(Return(nullptr));
    EXPECT_CALL(*mock, CloseDir(dir)).WillOnce(Return(0));
}

void ExpectRecountStateUpdates(const std::shared_ptr<AssistantMock> &mock,
                               const std::string &root,
                               const std::string &placeholderChild)
{
    EXPECT_CALL(*mock, getxattr(StrEq(placeholderChild), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t)))
        .WillOnce(Invoke([](const char *, const char *, void *value, size_t size) {
            *static_cast<uint8_t *>(value) = MakeFileSyncState(PLACEHOLDER_STATE_UNHYDRATED, 0);
            return static_cast<ssize_t>(size);
        }));
    EXPECT_CALL(*mock, setxattr(StrEq(root), StrEq(CLOUD_DISK_PLACEHOLDER_COUNT_XATTR), _, sizeof(uint32_t), 0))
        .WillOnce(Invoke([](const char *, const char *, const void *value, size_t, int) {
            EXPECT_EQ(DecodeTestPlaceholderCount(value), 1U);
            return 0;
        }));
    EXPECT_CALL(*mock, getxattr(StrEq(root), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t)))
        .WillOnce(Invoke([](const char *, const char *, void *, size_t) {
            errno = ENODATA;
            return static_cast<ssize_t>(-1);
        }));
    EXPECT_CALL(*mock, setxattr(StrEq(root), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t), 0))
        .WillOnce(Return(0));
}
} // namespace

class PlaceholderHelperTest : public testing::Test {
public:
    void SetUp() override
    {
        mock_ = std::make_shared<AssistantMock>();
        Assistant::ins = mock_;
        Assistant::mockFdApi = false;
        Assistant::mockLstatApi = false;
        ResetPlaceholderMetaFileMock();
    }

    void TearDown() override
    {
        Mock::VerifyAndClearExpectations(mock_.get());
        Assistant::mockFdApi = false;
        Assistant::mockLstatApi = false;
        ResetPlaceholderMetaFileMock();
        Assistant::ins = nullptr;
        mock_ = nullptr;
    }

    std::shared_ptr<AssistantMock> mock_;
};

/**
 * @tc.name: PlaceholderStatePredicateTest001
 * @tc.desc: Verify all defined placeholder substates are counted and NONE is not counted.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderHelperTest, PlaceholderStatePredicateTest001, TestSize.Level1)
{
    EXPECT_FALSE(IsPlaceholderState(PLACEHOLDER_STATE_NONE));
    EXPECT_TRUE(IsPlaceholderState(PLACEHOLDER_STATE_UNHYDRATED));
    EXPECT_TRUE(IsPlaceholderState(PLACEHOLDER_STATE_PARTIALLY_HYDRATED));
    EXPECT_TRUE(IsPlaceholderState(PLACEHOLDER_STATE_FULLY_HYDRATED));
}

/**
 * @tc.name: FileSyncStatePackingTest001
 * @tc.desc: Verify high and low filesyncstate bits remain independent.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderHelperTest, FileSyncStatePackingTest001, TestSize.Level1)
{
    constexpr uint8_t syncState = 5;
    uint8_t value = MakeFileSyncState(PLACEHOLDER_STATE_PARTIALLY_HYDRATED, syncState);

    EXPECT_EQ(GetPlaceholderStateFromFileSyncState(value), PLACEHOLDER_STATE_PARTIALLY_HYDRATED);
    EXPECT_EQ(GetSyncStateFromFileSyncState(value), syncState);
}

/**
 * @tc.name: PlaceholderDeltaTest001
 * @tc.desc: Verify only normal-to-placeholder transitions change the aggregate count.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderHelperTest, PlaceholderDeltaTest001, TestSize.Level1)
{
    auto delta = [](uint8_t oldState, uint8_t newState) {
        return static_cast<int32_t>(IsPlaceholderState(newState)) - static_cast<int32_t>(IsPlaceholderState(oldState));
    };

    EXPECT_EQ(delta(PLACEHOLDER_STATE_NONE, PLACEHOLDER_STATE_UNHYDRATED), 1);
    EXPECT_EQ(delta(PLACEHOLDER_STATE_UNHYDRATED, PLACEHOLDER_STATE_NONE), -1);
    EXPECT_EQ(delta(PLACEHOLDER_STATE_UNHYDRATED, PLACEHOLDER_STATE_PARTIALLY_HYDRATED), 0);
    EXPECT_EQ(delta(PLACEHOLDER_STATE_PARTIALLY_HYDRATED, PLACEHOLDER_STATE_FULLY_HYDRATED), 0);
}

/**
 * @tc.name: DentryPlaceholderStateTest001
 * @tc.desc: Verify the reserved dentry byte is sanitized without changing the dentry layout.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderHelperTest, DentryPlaceholderStateTest001, TestSize.Level1)
{
    CloudDiskServiceDentry dentry{};
    SetDentryPlaceholderState(dentry, PLACEHOLDER_STATE_FULLY_HYDRATED);
    EXPECT_EQ(GetDentryPlaceholderState(dentry), PLACEHOLDER_STATE_FULLY_HYDRATED);

    dentry.reserved[0] = 7;
    EXPECT_EQ(GetDentryPlaceholderState(dentry), PLACEHOLDER_STATE_NONE);
    EXPECT_EQ(sizeof(CloudDiskServiceDentryGroup), DENTRYGROUP_SIZE);
}

/**
 * @tc.name: PlaceholderStateValidationTest001
 * @tc.desc: Verify the strict public-state validator rejects every reserved encoding sampled at both boundaries.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(PlaceholderHelperTest, PlaceholderStateValidationTest001, TestSize.Level2)
{
    EXPECT_TRUE(IsValidPlaceholderState(PLACEHOLDER_STATE_NONE));
    EXPECT_TRUE(IsValidPlaceholderState(PLACEHOLDER_STATE_UNHYDRATED));
    EXPECT_TRUE(IsValidPlaceholderState(PLACEHOLDER_STATE_PARTIALLY_HYDRATED));
    EXPECT_TRUE(IsValidPlaceholderState(PLACEHOLDER_STATE_FULLY_HYDRATED));
    constexpr uint8_t FIRST_RESERVED_STATE = PLACEHOLDER_STATE_FULLY_HYDRATED + 1;
    EXPECT_FALSE(IsValidPlaceholderState(FIRST_RESERVED_STATE));
    EXPECT_FALSE(IsValidPlaceholderState(UINT8_MAX));
    EXPECT_EQ(SanitizeDentryPlaceholderState(FIRST_RESERVED_STATE), PLACEHOLDER_STATE_NONE);
}

/**
 * @tc.name: GetFilePlaceholderStateTest001
 * @tc.desc: Verify missing, malformed, failed and successful xattr reads have distinct results.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderHelperTest, GetFilePlaceholderStateTest001, TestSize.Level2)
{
    const std::string path = "/placeholder/file";
    EXPECT_CALL(*mock_, getxattr(StrEq(path), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t)))
        .WillOnce(Invoke([](const char *, const char *, void *, size_t) {
            errno = ENODATA;
            return static_cast<ssize_t>(-1);
        }))
        .WillOnce(Return(0))
        .WillOnce(Invoke([](const char *, const char *, void *, size_t) {
            errno = EIO;
            return static_cast<ssize_t>(-1);
        }))
        .WillOnce(Invoke([](const char *, const char *, void *value, size_t size) {
            *static_cast<uint8_t *>(value) =
                MakeFileSyncState(PLACEHOLDER_STATE_PARTIALLY_HYDRATED, static_cast<uint8_t>(SyncState::SYNCING));
            return static_cast<ssize_t>(size);
        }));

    uint8_t state = PLACEHOLDER_STATE_FULLY_HYDRATED;
    EXPECT_EQ(GetFilePlaceholderState(path, state), E_OK);
    EXPECT_EQ(state, PLACEHOLDER_STATE_NONE);
    EXPECT_EQ(GetFilePlaceholderState(path, state), EINVAL);
    EXPECT_EQ(GetFilePlaceholderState(path, state), EIO);
    EXPECT_EQ(GetFilePlaceholderState(path, state), E_OK);
    EXPECT_EQ(state, PLACEHOLDER_STATE_PARTIALLY_HYDRATED);
}

/**
 * @tc.name: SetFilePlaceholderStateTest001
 * @tc.desc: Verify placeholder updates preserve sync bits and propagate read, write and validation failures.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderHelperTest, SetFilePlaceholderStateTest001, TestSize.Level2)
{
    const std::string path = "/placeholder/file";
    constexpr uint8_t SYNC_STATE = static_cast<uint8_t>(SyncState::SYNCING);
    constexpr uint8_t INVALID_STATE = PLACEHOLDER_STATE_FULLY_HYDRATED + 1;
    uint8_t oldState = PLACEHOLDER_STATE_NONE;
    EXPECT_EQ(SetFilePlaceholderState(path, INVALID_STATE, oldState), EINVAL);

    EXPECT_CALL(*mock_, getxattr(StrEq(path), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t)))
        .WillOnce(Invoke([syncState = SYNC_STATE](const char *, const char *, void *value, size_t size) {
            *static_cast<uint8_t *>(value) = MakeFileSyncState(PLACEHOLDER_STATE_UNHYDRATED, syncState);
            return static_cast<ssize_t>(size);
        }));
    EXPECT_CALL(*mock_, setxattr(StrEq(path), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t), 0))
        .WillOnce(Invoke([syncState = SYNC_STATE](const char *, const char *, const void *value, size_t, int) {
            uint8_t stored = *static_cast<const uint8_t *>(value);
            EXPECT_EQ(GetPlaceholderStateFromFileSyncState(stored), PLACEHOLDER_STATE_FULLY_HYDRATED);
            EXPECT_EQ(GetSyncStateFromFileSyncState(stored), syncState);
            return 0;
        }));
    EXPECT_EQ(SetFilePlaceholderState(path, PLACEHOLDER_STATE_FULLY_HYDRATED, oldState), E_OK);
    EXPECT_EQ(oldState, PLACEHOLDER_STATE_UNHYDRATED);

    EXPECT_CALL(*mock_, getxattr(_, _, _, _)).WillOnce(Invoke([](const char *, const char *, void *, size_t) {
        errno = EIO;
        return static_cast<ssize_t>(-1);
    }));
    EXPECT_EQ(SetFilePlaceholderState(path, PLACEHOLDER_STATE_UNHYDRATED, oldState), EIO);

    EXPECT_CALL(*mock_, getxattr(_, _, _, _)).WillOnce(Invoke([](const char *, const char *, void *value, size_t size) {
        *static_cast<uint8_t *>(value) = 0;
        return static_cast<ssize_t>(size);
    }));
    EXPECT_CALL(*mock_, setxattr(_, _, _, _, _))
        .WillOnce(Invoke([](const char *, const char *, const void *, size_t, int) {
            errno = ENOSPC;
            return -1;
        }));
    EXPECT_EQ(SetFilePlaceholderState(path, PLACEHOLDER_STATE_UNHYDRATED, oldState), ENOSPC);
}

/**
 * @tc.name: FileSyncStateUpdateTest001
 * @tc.desc: Verify sync-state updates preserve placeholder bits and reject values outside the sync mask.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderHelperTest, FileSyncStateUpdateTest001, TestSize.Level1)
{
    const std::string path = "/placeholder/file";
    constexpr uint8_t NEW_SYNC_STATE = static_cast<uint8_t>(SyncState::SYNC_SUCCESSED);
    constexpr uint8_t INVALID_SYNC_STATE = FILE_SYNC_STATE_SYNC_MASK + 1;
    EXPECT_EQ(SetFileSyncState(path, INVALID_SYNC_STATE), EINVAL);

    EXPECT_CALL(*mock_, getxattr(StrEq(path), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t)))
        .WillOnce(Invoke([](const char *, const char *, void *value, size_t size) {
            *static_cast<uint8_t *>(value) = MakeFileSyncState(PLACEHOLDER_STATE_PARTIALLY_HYDRATED, 0);
            return static_cast<ssize_t>(size);
        }));
    EXPECT_CALL(*mock_, setxattr(StrEq(path), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t), 0))
        .WillOnce(Invoke([newSyncState = NEW_SYNC_STATE](const char *, const char *, const void *value, size_t, int) {
            uint8_t stored = *static_cast<const uint8_t *>(value);
            EXPECT_EQ(GetPlaceholderStateFromFileSyncState(stored), PLACEHOLDER_STATE_PARTIALLY_HYDRATED);
            EXPECT_EQ(GetSyncStateFromFileSyncState(stored), newSyncState);
            return 0;
        }));
    EXPECT_EQ(SetFileSyncState(path, NEW_SYNC_STATE), E_OK);
}

/**
 * @tc.name: PlaceholderPathGuardTest001
 * @tc.desc: Verify no-op propagation and invalid recount or dentry paths exit before filesystem access.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(PlaceholderHelperTest, PlaceholderPathGuardTest001, TestSize.Level2)
{
    EXPECT_EQ(RefreshAncestorPlaceholderCount("/root", "/root/file", 0), E_OK);
    EXPECT_EQ(RefreshAncestorPlaceholderCount("", "/root/file", 1), E_OK);
    EXPECT_EQ(RefreshAncestorPlaceholderCount("/root", "", 1), E_OK);
    EXPECT_EQ(RecountPlaceholderState("/root", "/outside/file", 100, 200), EINVAL);
    EXPECT_EQ(UpdateDentryPlaceholderState(100, 200, "name", PLACEHOLDER_STATE_UNHYDRATED), EINVAL);
    EXPECT_EQ(UpdateDentryPlaceholderState(100, 200, "/", PLACEHOLDER_STATE_UNHYDRATED), EINVAL);
}

/**
 * @tc.name: GetFileSyncStateByte_001
 * @tc.desc: Verify callers can choose whether a missing path xattr is reported or normalized to zero.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderHelperTest, GetFileSyncStateByte_001, TestSize.Level1)
{
    const std::string path = "/placeholder/file";
    EXPECT_CALL(*mock_, getxattr(StrEq(path), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t)))
        .Times(2)
        .WillRepeatedly(Invoke([](const char *, const char *, void *, size_t) {
            errno = ENODATA;
            return static_cast<ssize_t>(-1);
        }));

    uint8_t state = UINT8_MAX;
    EXPECT_EQ(GetFileSyncStateByte(path, state, false), ENODATA);
    EXPECT_EQ(state, UINT8_MAX);
    EXPECT_EQ(GetFileSyncStateByte(path, state, true), E_OK);
    EXPECT_EQ(state, 0);
}

/**
 * @tc.name: GetFilePlaceholderStateFd_001
 * @tc.desc: Cover successful, missing, malformed and failed fd xattr reads.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderHelperTest, GetFilePlaceholderStateFd_001, TestSize.Level2)
{
    constexpr int32_t TEST_FD = 42;
    EXPECT_CALL(*mock_, fgetxattr(TEST_FD, StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t)))
        .WillOnce(Invoke([](int, const char *, void *, size_t) {
            errno = ENODATA;
            return static_cast<ssize_t>(-1);
        }))
        .WillOnce(Return(0))
        .WillOnce(Invoke([](int, const char *, void *, size_t) {
            errno = EIO;
            return static_cast<ssize_t>(-1);
        }))
        .WillOnce(Invoke([](int, const char *, void *value, size_t size) {
            *static_cast<uint8_t *>(value) = MakeFileSyncState(PLACEHOLDER_STATE_FULLY_HYDRATED, 1);
            return static_cast<ssize_t>(size);
        }));

    uint8_t state = PLACEHOLDER_STATE_FULLY_HYDRATED;
    EXPECT_EQ(GetFilePlaceholderState(TEST_FD, state), E_OK);
    EXPECT_EQ(state, PLACEHOLDER_STATE_NONE);
    EXPECT_EQ(GetFilePlaceholderState(TEST_FD, state), EINVAL);
    EXPECT_EQ(GetFilePlaceholderState(TEST_FD, state), EIO);
    EXPECT_EQ(GetFilePlaceholderState(TEST_FD, state), E_OK);
    EXPECT_EQ(state, PLACEHOLDER_STATE_FULLY_HYDRATED);
}

/**
 * @tc.name: SetFilePlaceholderStateFd_001
 * @tc.desc: Verify fd placeholder updates preserve sync bits and propagate validation, read and write failures.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderHelperTest, SetFilePlaceholderStateFd_001, TestSize.Level2)
{
    constexpr int32_t TEST_FD = 43;
    constexpr uint8_t SYNC_STATE = static_cast<uint8_t>(SyncState::SYNCING);
    constexpr uint8_t INVALID_STATE = PLACEHOLDER_STATE_FULLY_HYDRATED + 1;
    uint8_t oldState = PLACEHOLDER_STATE_NONE;
    EXPECT_EQ(SetFilePlaceholderState(TEST_FD, INVALID_STATE, oldState), EINVAL);

    EXPECT_CALL(*mock_, fgetxattr(TEST_FD, StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t)))
        .WillOnce(Invoke([SYNC_STATE](int, const char *, void *value, size_t size) {
            *static_cast<uint8_t *>(value) = MakeFileSyncState(PLACEHOLDER_STATE_UNHYDRATED, SYNC_STATE);
            return static_cast<ssize_t>(size);
        }));
    EXPECT_CALL(*mock_, fsetxattr(TEST_FD, StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t), 0))
        .WillOnce(Invoke([SYNC_STATE](int, const char *, const void *value, size_t, int) {
            uint8_t stored = *static_cast<const uint8_t *>(value);
            EXPECT_EQ(GetPlaceholderStateFromFileSyncState(stored), PLACEHOLDER_STATE_FULLY_HYDRATED);
            EXPECT_EQ(GetSyncStateFromFileSyncState(stored), SYNC_STATE);
            return 0;
        }));
    EXPECT_EQ(SetFilePlaceholderState(TEST_FD, PLACEHOLDER_STATE_FULLY_HYDRATED, oldState), E_OK);
    EXPECT_EQ(oldState, PLACEHOLDER_STATE_UNHYDRATED);

    EXPECT_CALL(*mock_, fgetxattr(TEST_FD, _, _, _)).WillOnce(Invoke([](int, const char *, void *, size_t) {
        errno = EIO;
        return static_cast<ssize_t>(-1);
    }));
    EXPECT_EQ(SetFilePlaceholderState(TEST_FD, PLACEHOLDER_STATE_UNHYDRATED, oldState), EIO);

    EXPECT_CALL(*mock_, fgetxattr(TEST_FD, _, _, _)).WillOnce(Invoke([](int, const char *, void *value, size_t size) {
        *static_cast<uint8_t *>(value) = 0;
        return static_cast<ssize_t>(size);
    }));
    EXPECT_CALL(*mock_, fsetxattr(TEST_FD, _, _, _, _))
        .WillOnce(Invoke([](int, const char *, const void *, size_t, int) {
            errno = ENOSPC;
            return -1;
        }));
    EXPECT_EQ(SetFilePlaceholderState(TEST_FD, PLACEHOLDER_STATE_UNHYDRATED, oldState), ENOSPC);
}

/**
 * @tc.name: SetNewFilePlaceholderState_001
 * @tc.desc: Verify new-file state validation, encoding and xattr failure handling.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderHelperTest, SetNewFilePlaceholderState_001, TestSize.Level2)
{
    constexpr int32_t TEST_FD = 44;
    constexpr uint8_t INVALID_STATE = PLACEHOLDER_STATE_FULLY_HYDRATED + 1;
    EXPECT_EQ(SetNewFilePlaceholderState(TEST_FD, INVALID_STATE), EINVAL);

    EXPECT_CALL(*mock_, fsetxattr(TEST_FD, StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t), 0))
        .WillOnce(Invoke([](int, const char *, const void *value, size_t, int) {
            EXPECT_EQ(*static_cast<const uint8_t *>(value),
                      MakeFileSyncState(PLACEHOLDER_STATE_UNHYDRATED, PLACEHOLDER_STATE_NONE));
            return 0;
        }))
        .WillOnce(Invoke([](int, const char *, const void *, size_t, int) {
            errno = EIO;
            return -1;
        }));
    EXPECT_EQ(SetNewFilePlaceholderState(TEST_FD, PLACEHOLDER_STATE_UNHYDRATED), E_OK);
    EXPECT_EQ(SetNewFilePlaceholderState(TEST_FD, PLACEHOLDER_STATE_UNHYDRATED), EIO);
}

/**
 * @tc.name: SetFileSyncState_001
 * @tc.desc: Verify path sync-state updates propagate xattr read and write failures.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderHelperTest, SetFileSyncState_001, TestSize.Level2)
{
    const std::string path = "/placeholder/file";
    constexpr uint8_t SYNC_STATE = static_cast<uint8_t>(SyncState::SYNCING);
    EXPECT_CALL(*mock_, getxattr(StrEq(path), _, _, _)).WillOnce(Invoke([](const char *, const char *, void *, size_t) {
        errno = EIO;
        return static_cast<ssize_t>(-1);
    }));
    EXPECT_EQ(SetFileSyncState(path, SYNC_STATE), EIO);

    EXPECT_CALL(*mock_, getxattr(StrEq(path), _, _, _))
        .WillOnce(Invoke([](const char *, const char *, void *value, size_t size) {
            *static_cast<uint8_t *>(value) = MakeFileSyncState(PLACEHOLDER_STATE_UNHYDRATED, 0);
            return static_cast<ssize_t>(size);
        }));
    EXPECT_CALL(*mock_, setxattr(StrEq(path), _, _, _, _))
        .WillOnce(Invoke([](const char *, const char *, const void *, size_t, int) {
            errno = ENOSPC;
            return -1;
        }));
    EXPECT_EQ(SetFileSyncState(path, SYNC_STATE), ENOSPC);
}

/**
 * @tc.name: RefreshAncestorPlaceholderCount_001
 * @tc.desc: Cover invalid delta, counter overflow, first-placeholder propagation and storage failures.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderHelperTest, RefreshAncestorPlaceholderCount_001, TestSize.Level2)
{
    const std::string root = "/placeholder/root";
    const std::string file = root + "/file";
    EXPECT_EQ(RefreshAncestorPlaceholderCount(root, file, 2), EINVAL);

    EXPECT_CALL(*mock_, getxattr(StrEq(root), StrEq(CLOUD_DISK_PLACEHOLDER_COUNT_XATTR), _, sizeof(uint32_t)))
        .WillOnce(Invoke([](const char *, const char *, void *value, size_t size) {
            std::fill_n(static_cast<uint8_t *>(value), size, UCHAR_MAX);
            return static_cast<ssize_t>(size);
        }));
    EXPECT_EQ(RefreshAncestorPlaceholderCount(root, file, 1), EOVERFLOW);

    EXPECT_CALL(*mock_, getxattr(StrEq(root), StrEq(CLOUD_DISK_PLACEHOLDER_COUNT_XATTR), _, sizeof(uint32_t)))
        .WillOnce(Invoke([](const char *, const char *, void *, size_t) {
            errno = ENODATA;
            return static_cast<ssize_t>(-1);
        }));
    EXPECT_CALL(*mock_, setxattr(StrEq(root), StrEq(CLOUD_DISK_PLACEHOLDER_COUNT_XATTR), _, sizeof(uint32_t), 0))
        .WillOnce(Return(0));
    EXPECT_CALL(*mock_, getxattr(StrEq(root), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t)))
        .WillOnce(Invoke([](const char *, const char *, void *value, size_t size) {
            *static_cast<uint8_t *>(value) = MakeFileSyncState(PLACEHOLDER_STATE_NONE, 1);
            return static_cast<ssize_t>(size);
        }));
    EXPECT_CALL(*mock_, setxattr(StrEq(root), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t), 0))
        .WillOnce(Invoke([](const char *, const char *, const void *value, size_t, int) {
            uint8_t stored = *static_cast<const uint8_t *>(value);
            EXPECT_EQ(GetPlaceholderStateFromFileSyncState(stored), PLACEHOLDER_STATE_HAS_PLACEHOLDER);
            EXPECT_EQ(GetSyncStateFromFileSyncState(stored), 1);
            return 0;
        }));
    EXPECT_EQ(RefreshAncestorPlaceholderCount(root, file, 1), E_OK);

    EXPECT_CALL(*mock_, getxattr(StrEq(root), StrEq(CLOUD_DISK_PLACEHOLDER_COUNT_XATTR), _, sizeof(uint32_t)))
        .WillOnce(Invoke([](const char *, const char *, void *value, size_t size) {
            std::fill_n(static_cast<uint8_t *>(value), size, 0);
            static_cast<uint8_t *>(value)[0] = 1;
            return static_cast<ssize_t>(size);
        }));
    EXPECT_CALL(*mock_, setxattr(StrEq(root), StrEq(CLOUD_DISK_PLACEHOLDER_COUNT_XATTR), _, sizeof(uint32_t), 0))
        .WillOnce(Invoke([](const char *, const char *, const void *, size_t, int) {
            errno = ENOSPC;
            return -1;
        }));
    EXPECT_EQ(RefreshAncestorPlaceholderCount(root, file, 1), ENOSPC);
}

/**
 * @tc.name: RefreshAncestorPlaceholderCountDecrease_001
 * @tc.desc: Verify the last placeholder removal clears the count and directory placeholder bit.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderHelperTest, RefreshAncestorPlaceholderCountDecrease_001, TestSize.Level1)
{
    const std::string root = "/placeholder/root";
    const std::string file = root + "/file";
    constexpr uint8_t SYNC_STATE = static_cast<uint8_t>(SyncState::SYNCING);
    InSequence sequence;
    EXPECT_CALL(*mock_, getxattr(StrEq(root), StrEq(CLOUD_DISK_PLACEHOLDER_COUNT_XATTR), _, sizeof(uint32_t)))
        .WillOnce(Invoke([](const char *, const char *, void *value, size_t size) {
            EncodeTestPlaceholderCount(value, 1);
            return static_cast<ssize_t>(size);
        }));
    EXPECT_CALL(*mock_, removexattr(StrEq(root), StrEq(CLOUD_DISK_PLACEHOLDER_COUNT_XATTR))).WillOnce(Return(0));
    EXPECT_CALL(*mock_, getxattr(StrEq(root), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t)))
        .WillOnce(Invoke([SYNC_STATE](const char *, const char *, void *value, size_t size) {
            *static_cast<uint8_t *>(value) = MakeFileSyncState(PLACEHOLDER_STATE_HAS_PLACEHOLDER, SYNC_STATE);
            return static_cast<ssize_t>(size);
        }));
    EXPECT_CALL(*mock_, setxattr(StrEq(root), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t), 0))
        .WillOnce(Invoke([SYNC_STATE](const char *, const char *, const void *value, size_t, int) {
            uint8_t stored = *static_cast<const uint8_t *>(value);
            EXPECT_EQ(GetPlaceholderStateFromFileSyncState(stored), PLACEHOLDER_STATE_NONE);
            EXPECT_EQ(GetSyncStateFromFileSyncState(stored), SYNC_STATE);
            return 0;
        }));

    EXPECT_EQ(RefreshAncestorPlaceholderCount(root, file, -1), E_OK);
}

/**
 * @tc.name: RefreshAncestorPlaceholderCountRemoveMissing_001
 * @tc.desc: Verify a concurrently removed zero-count xattr is treated as a successful decrement.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderHelperTest, RefreshAncestorPlaceholderCountRemoveMissing_001, TestSize.Level2)
{
    const std::string root = "/placeholder/root";
    const std::string file = root + "/file";
    InSequence sequence;
    EXPECT_CALL(*mock_, getxattr(StrEq(root), StrEq(CLOUD_DISK_PLACEHOLDER_COUNT_XATTR), _, sizeof(uint32_t)))
        .WillOnce(Invoke([](const char *, const char *, void *value, size_t size) {
            EncodeTestPlaceholderCount(value, 1);
            return static_cast<ssize_t>(size);
        }));
    EXPECT_CALL(*mock_, removexattr(StrEq(root), StrEq(CLOUD_DISK_PLACEHOLDER_COUNT_XATTR)))
        .WillOnce(Invoke([](const char *, const char *) {
            errno = ENODATA;
            return -1;
        }));
    EXPECT_CALL(*mock_, getxattr(StrEq(root), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t)))
        .WillOnce(Invoke([](const char *, const char *, void *, size_t) {
            errno = ENODATA;
            return static_cast<ssize_t>(-1);
        }));
    EXPECT_CALL(*mock_, setxattr(StrEq(root), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t), 0))
        .WillOnce(Invoke([](const char *, const char *, const void *value, size_t, int) {
            EXPECT_EQ(GetPlaceholderStateFromFileSyncState(*static_cast<const uint8_t *>(value)),
                      PLACEHOLDER_STATE_NONE);
            return 0;
        }));

    EXPECT_EQ(RefreshAncestorPlaceholderCount(root, file, -1), E_OK);
}

/**
 * @tc.name: RefreshAncestorPlaceholderCountRemoveFailure_001
 * @tc.desc: Verify a count-xattr removal error is returned without modifying directory state.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderHelperTest, RefreshAncestorPlaceholderCountRemoveFailure_001, TestSize.Level2)
{
    const std::string root = "/placeholder/root";
    const std::string file = root + "/file";
    EXPECT_CALL(*mock_, getxattr(StrEq(root), StrEq(CLOUD_DISK_PLACEHOLDER_COUNT_XATTR), _, sizeof(uint32_t)))
        .WillOnce(Invoke([](const char *, const char *, void *value, size_t size) {
            EncodeTestPlaceholderCount(value, 1);
            return static_cast<ssize_t>(size);
        }));
    EXPECT_CALL(*mock_, removexattr(StrEq(root), StrEq(CLOUD_DISK_PLACEHOLDER_COUNT_XATTR)))
        .WillOnce(Invoke([](const char *, const char *) {
            errno = EACCES;
            return -1;
        }));
    EXPECT_CALL(*mock_, getxattr(StrEq(root), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, _)).Times(0);
    EXPECT_CALL(*mock_, setxattr(StrEq(root), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, _, _)).Times(0);

    EXPECT_EQ(RefreshAncestorPlaceholderCount(root, file, -1), EACCES);
}

/**
 * @tc.name: RefreshAncestorPlaceholderCountPropagation_001
 * @tc.desc: Verify a first child placeholder propagates once and stops at an already non-empty parent.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderHelperTest, RefreshAncestorPlaceholderCountPropagation_001, TestSize.Level1)
{
    const std::string root = "/placeholder/root";
    const std::string child = root + "/child";
    const std::string file = child + "/file";
    constexpr uint8_t SYNC_STATE = static_cast<uint8_t>(SyncState::SYNCING);
    {
        InSequence sequence;
        EXPECT_CALL(*mock_, getxattr(StrEq(child), StrEq(CLOUD_DISK_PLACEHOLDER_COUNT_XATTR), _, sizeof(uint32_t)))
            .WillOnce(Invoke([](const char *, const char *, void *, size_t) {
                errno = ENODATA;
                return static_cast<ssize_t>(-1);
            }));
        EXPECT_CALL(*mock_, setxattr(StrEq(child), StrEq(CLOUD_DISK_PLACEHOLDER_COUNT_XATTR), _, sizeof(uint32_t), 0))
            .WillOnce(Invoke([](const char *, const char *, const void *value, size_t, int) {
                EXPECT_EQ(DecodeTestPlaceholderCount(value), 1U);
                return 0;
            }));
        EXPECT_CALL(*mock_, getxattr(StrEq(child), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t)))
            .WillOnce(Invoke([SYNC_STATE](const char *, const char *, void *value, size_t size) {
                *static_cast<uint8_t *>(value) = MakeFileSyncState(PLACEHOLDER_STATE_NONE, SYNC_STATE);
                return static_cast<ssize_t>(size);
            }));
        EXPECT_CALL(*mock_, setxattr(StrEq(child), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t), 0))
            .WillOnce(Invoke([SYNC_STATE](const char *, const char *, const void *value, size_t, int) {
                uint8_t stored = *static_cast<const uint8_t *>(value);
                EXPECT_EQ(GetPlaceholderStateFromFileSyncState(stored), PLACEHOLDER_STATE_HAS_PLACEHOLDER);
                EXPECT_EQ(GetSyncStateFromFileSyncState(stored), SYNC_STATE);
                return 0;
            }));
        EXPECT_CALL(*mock_, getxattr(StrEq(root), StrEq(CLOUD_DISK_PLACEHOLDER_COUNT_XATTR), _, sizeof(uint32_t)))
            .WillOnce(Invoke([](const char *, const char *, void *value, size_t size) {
                EncodeTestPlaceholderCount(value, 1);
                return static_cast<ssize_t>(size);
            }));
        EXPECT_CALL(*mock_, setxattr(StrEq(root), StrEq(CLOUD_DISK_PLACEHOLDER_COUNT_XATTR), _, sizeof(uint32_t), 0))
            .WillOnce(Invoke([](const char *, const char *, const void *value, size_t, int) {
                EXPECT_EQ(DecodeTestPlaceholderCount(value), 2U);
                return 0;
            }));
    }
    EXPECT_CALL(*mock_, getxattr(StrEq(root), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, _)).Times(0);

    EXPECT_EQ(RefreshAncestorPlaceholderCount(root, file, 1), E_OK);
}

/**
 * @tc.name: RecountPlaceholderState_001
 * @tc.desc: Verify recount reports missing nodes and directory-open failures.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderHelperTest, RecountPlaceholderState_001, TestSize.Level2)
{
    EXPECT_EQ(RecountPlaceholderState("/placeholder/root", "/placeholder/other", 100, 200), EINVAL);
    EXPECT_EQ(RecountPlaceholderState("/", "/__dfs_placeholder_missing__/node", 100, 200), ENOENT);
    EXPECT_CALL(*mock_, opendir(StrEq("/"))).WillOnce(Invoke([](const char *) {
        errno = EACCES;
        return static_cast<DIR *>(nullptr);
    }));
    EXPECT_EQ(RecountPlaceholderState("/", "/", 100, 200), EACCES);
}

/**
 * @tc.name: RecountPlaceholderStateFileReadFailure_001
 * @tc.desc: Treat an unreadable file xattr as non-placeholder and still refresh its dentry.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderHelperTest, RecountPlaceholderStateFileReadFailure_001, TestSize.Level2)
{
    const std::string root = "/placeholder/root";
    const std::string file = root + "/file";
    struct stat fileStat {};
    fileStat.st_mode = S_IFREG;
    struct stat parentStat {};
    parentStat.st_mode = S_IFDIR;
    parentStat.st_ino = 123;
    Assistant::mockFdApi = true;
    Assistant::mockLstatApi = true;

    EXPECT_CALL(*mock_, MockStat(StrEq(file), _)).WillOnce(DoAll(SetArgPointee<1>(fileStat), Return(0)));
    EXPECT_CALL(*mock_, getxattr(StrEq(file), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t)))
        .WillOnce(Invoke([](const char *, const char *, void *, size_t) {
            errno = EIO;
            return static_cast<ssize_t>(-1);
        }));
    EXPECT_CALL(*mock_, MockStat(StrEq(root), _)).WillOnce(DoAll(SetArgPointee<1>(parentStat), Return(0)));

    EXPECT_EQ(RecountPlaceholderState(root, file, 100, 200), E_OK);
    EXPECT_EQ(GetPlaceholderMetaFileUpdateCount(), 1U);
    EXPECT_EQ(GetLastPlaceholderMetaFileUpdateName(), "file");
    EXPECT_EQ(GetLastPlaceholderMetaFileUpdateState(), PLACEHOLDER_STATE_NONE);
}

/**
 * @tc.name: RecountPlaceholderStateContinuesAfterChildFailure_001
 * @tc.desc: Skip dot entries and a failed child while retaining a later placeholder child.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderHelperTest, RecountPlaceholderStateContinuesAfterChildFailure_001, TestSize.Level2)
{
    const std::string root = "/placeholder/root";
    const std::string failedChild = root + "/failed";
    const std::string placeholderChild = root + "/placeholder";
    RecountChildEntries entries;
    Assistant::mockFdApi = true;
    Assistant::mockLstatApi = true;
    ExpectRecountChildTraversal(mock_, root, failedChild, placeholderChild, entries);
    ExpectRecountStateUpdates(mock_, root, placeholderChild);

    EXPECT_EQ(RecountPlaceholderState(root, root, 100, 200), E_OK);
    EXPECT_EQ(GetPlaceholderMetaFileUpdateCount(), 1U);
    EXPECT_EQ(GetLastPlaceholderMetaFileUpdateName(), "placeholder");
    EXPECT_EQ(GetLastPlaceholderMetaFileUpdateState(), PLACEHOLDER_STATE_UNHYDRATED);
}

/**
 * @tc.name: RecountPlaceholderStateDirectoryUpdateFailures_001
 * @tc.desc: Keep recount successful when persisting derived directory metadata fails.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderHelperTest, RecountPlaceholderStateDirectoryUpdateFailures_001, TestSize.Level2)
{
    const std::string root = "/placeholder/root";
    struct dirent marker {};
    auto *dir = reinterpret_cast<DIR *>(&marker);
    struct stat directoryStat {};
    directoryStat.st_mode = S_IFDIR;
    Assistant::mockLstatApi = true;

    EXPECT_CALL(*mock_, MockStat(StrEq(root), _)).WillOnce(DoAll(SetArgPointee<1>(directoryStat), Return(0)));
    EXPECT_CALL(*mock_, opendir(StrEq(root))).WillOnce(Return(dir));
    EXPECT_CALL(*mock_, readdir(dir)).WillOnce(Return(nullptr));
    EXPECT_CALL(*mock_, CloseDir(dir)).WillOnce(Return(0));
    EXPECT_CALL(*mock_, removexattr(StrEq(root), StrEq(CLOUD_DISK_PLACEHOLDER_COUNT_XATTR)))
        .WillOnce(Invoke([](const char *, const char *) {
            errno = EACCES;
            return -1;
        }));
    EXPECT_CALL(*mock_, getxattr(StrEq(root), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t)))
        .WillOnce(Invoke([](const char *, const char *, void *, size_t) {
            errno = EIO;
            return static_cast<ssize_t>(-1);
        }));
    EXPECT_CALL(*mock_, setxattr(StrEq(root), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, _, _)).Times(0);
    EXPECT_EQ(RecountPlaceholderState(root, root, 100, 200), E_OK);

    Mock::VerifyAndClearExpectations(mock_.get());
    EXPECT_CALL(*mock_, MockStat(StrEq(root), _)).WillOnce(DoAll(SetArgPointee<1>(directoryStat), Return(0)));
    EXPECT_CALL(*mock_, opendir(StrEq(root))).WillOnce(Return(dir));
    EXPECT_CALL(*mock_, readdir(dir)).WillOnce(Return(nullptr));
    EXPECT_CALL(*mock_, CloseDir(dir)).WillOnce(Return(0));
    EXPECT_CALL(*mock_, removexattr(StrEq(root), StrEq(CLOUD_DISK_PLACEHOLDER_COUNT_XATTR))).WillOnce(Return(0));
    EXPECT_CALL(*mock_, getxattr(StrEq(root), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t)))
        .WillOnce(Invoke([](const char *, const char *, void *value, size_t size) {
            *static_cast<uint8_t *>(value) = MakeFileSyncState(PLACEHOLDER_STATE_UNHYDRATED, 1);
            return static_cast<ssize_t>(size);
        }));
    EXPECT_CALL(*mock_, setxattr(StrEq(root), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t), 0))
        .WillOnce(Invoke([](const char *, const char *, const void *value, size_t, int) {
            EXPECT_EQ(GetPlaceholderStateFromFileSyncState(*static_cast<const uint8_t *>(value)),
                      PLACEHOLDER_STATE_NONE);
            errno = ENOSPC;
            return -1;
        }));
    EXPECT_EQ(RecountPlaceholderState(root, root, 100, 200), E_OK);
}

/**
 * @tc.name: RefreshAncestorPlaceholderCountContinues_001
 * @tc.desc: Continue at the sync root after a child-directory update fails and return the first error.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderHelperTest, RefreshAncestorPlaceholderCountContinues_001, TestSize.Level2)
{
    const std::string root = "/placeholder/root";
    const std::string child = root + "/child";
    const std::string file = child + "/file";
    EXPECT_EQ(RefreshAncestorPlaceholderCount(root, file, 0), E_OK);
    EXPECT_EQ(RefreshAncestorPlaceholderCount("", file, 1), E_OK);
    EXPECT_EQ(RefreshAncestorPlaceholderCount(root, "", 1), E_OK);

    EXPECT_CALL(*mock_, getxattr(StrEq(child), StrEq(CLOUD_DISK_PLACEHOLDER_COUNT_XATTR), _, sizeof(uint32_t)))
        .WillOnce(Invoke([](const char *, const char *, void *, size_t) {
            errno = EIO;
            return static_cast<ssize_t>(-1);
        }));
    EXPECT_CALL(*mock_, getxattr(StrEq(root), StrEq(CLOUD_DISK_PLACEHOLDER_COUNT_XATTR), _, sizeof(uint32_t)))
        .WillOnce(Invoke([](const char *, const char *, void *, size_t) {
            errno = ENODATA;
            return static_cast<ssize_t>(-1);
        }));
    EXPECT_CALL(*mock_, setxattr(StrEq(root), StrEq(CLOUD_DISK_PLACEHOLDER_COUNT_XATTR), _, sizeof(uint32_t), 0))
        .WillOnce(Return(0));
    EXPECT_CALL(*mock_, getxattr(StrEq(root), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t)))
        .WillOnce(Invoke([](const char *, const char *, void *, size_t) {
            errno = ENODATA;
            return static_cast<ssize_t>(-1);
        }));
    EXPECT_CALL(*mock_, setxattr(StrEq(root), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t), 0))
        .WillOnce(Return(0));

    EXPECT_EQ(RefreshAncestorPlaceholderCount(root, file, 1), EIO);
}

/**
 * @tc.name: RecountPlaceholderStateSuccess_001
 * @tc.desc: Recount a directory containing one placeholder and persist its aggregate state.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderHelperTest, RecountPlaceholderStateSuccess_001, TestSize.Level1)
{
    const std::string root = "/placeholder/root";
    const std::string child = root + "/file";
    constexpr uint8_t SYNC_STATE = static_cast<uint8_t>(SyncState::SYNCING);
    struct dirent childEntry {};
    constexpr char CHILD_NAME[] = "file";
    std::copy_n(CHILD_NAME, sizeof(CHILD_NAME), childEntry.d_name);
    auto *dir = reinterpret_cast<DIR *>(&childEntry);
    struct stat directoryStat {};
    directoryStat.st_mode = S_IFDIR;
    directoryStat.st_ino = 123;
    struct stat fileStat {};
    fileStat.st_mode = S_IFREG;

    Assistant::mockLstatApi = true;
    EXPECT_CALL(*mock_, MockStat(StrEq(root), _))
        .WillOnce(DoAll(SetArgPointee<1>(directoryStat), Return(0)));
    EXPECT_CALL(*mock_, MockStat(StrEq(child), _)).WillOnce(DoAll(SetArgPointee<1>(fileStat), Return(0)));
    EXPECT_CALL(*mock_, opendir(StrEq(root))).WillOnce(Return(dir));
    EXPECT_CALL(*mock_, readdir(dir)).Times(2).WillOnce(Return(&childEntry)).WillOnce(Return(nullptr));
    EXPECT_CALL(*mock_, CloseDir(dir)).WillOnce(Return(0));
    EXPECT_CALL(*mock_, getxattr(StrEq(child), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t)))
        .WillOnce(Invoke([](const char *, const char *, void *value, size_t size) {
            *static_cast<uint8_t *>(value) = MakeFileSyncState(PLACEHOLDER_STATE_UNHYDRATED, 0);
            return static_cast<ssize_t>(size);
        }));
    EXPECT_CALL(*mock_, setxattr(StrEq(root), StrEq(CLOUD_DISK_PLACEHOLDER_COUNT_XATTR), _, sizeof(uint32_t), 0))
        .WillOnce(Invoke([](const char *, const char *, const void *value, size_t, int) {
            EXPECT_EQ(DecodeTestPlaceholderCount(value), 1U);
            return 0;
        }));
    EXPECT_CALL(*mock_, getxattr(StrEq(root), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t)))
        .WillOnce(Invoke([SYNC_STATE](const char *, const char *, void *value, size_t size) {
            *static_cast<uint8_t *>(value) = MakeFileSyncState(PLACEHOLDER_STATE_NONE, SYNC_STATE);
            return static_cast<ssize_t>(size);
        }));
    EXPECT_CALL(*mock_, setxattr(StrEq(root), StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t), 0))
        .WillOnce(Invoke([SYNC_STATE](const char *, const char *, const void *value, size_t, int) {
            uint8_t state = *static_cast<const uint8_t *>(value);
            EXPECT_EQ(GetPlaceholderStateFromFileSyncState(state), PLACEHOLDER_STATE_HAS_PLACEHOLDER);
            EXPECT_EQ(GetSyncStateFromFileSyncState(state), SYNC_STATE);
            return 0;
        }));

    EXPECT_EQ(RecountPlaceholderState(root, root, 100, 200), E_OK);
}

/**
 * @tc.name: UpdateDentryPlaceholderState_001
 * @tc.desc: Verify parent metadata lookup errors are returned and valid parent metadata reaches the meta file.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderHelperTest, UpdateDentryPlaceholderState_001, TestSize.Level2)
{
    const std::string parent = "/placeholder";
    const std::string path = parent + "/file";
    Assistant::mockFdApi = true;
    EXPECT_CALL(*mock_, MockStat(StrEq(parent), _)).WillOnce(Invoke([](const char *, struct stat *) {
        errno = EACCES;
        return -1;
    }));
    EXPECT_EQ(UpdateDentryPlaceholderState(100, 200, path, PLACEHOLDER_STATE_UNHYDRATED), EACCES);

    struct stat parentStat = {};
    parentStat.st_ino = 123;
    EXPECT_CALL(*mock_, MockStat(StrEq(parent), _)).WillOnce(DoAll(SetArgPointee<1>(parentStat), Return(0)));
    SetPlaceholderMetaFileUpdateResult(EIO);
    EXPECT_EQ(UpdateDentryPlaceholderState(100, 200, path, PLACEHOLDER_STATE_UNHYDRATED), EIO);
    EXPECT_EQ(GetPlaceholderMetaFileUpdateCount(), 1U);
    EXPECT_EQ(GetLastPlaceholderMetaFileUpdateName(), "file");
    EXPECT_EQ(GetLastPlaceholderMetaFileUpdateState(), PLACEHOLDER_STATE_UNHYDRATED);

    EXPECT_CALL(*mock_, MockStat(StrEq(parent), _)).WillOnce(DoAll(SetArgPointee<1>(parentStat), Return(0)));
    SetPlaceholderMetaFileUpdateResult(E_OK);
    EXPECT_EQ(UpdateDentryPlaceholderState(100, 200, path, PLACEHOLDER_STATE_UNHYDRATED), E_OK);
    EXPECT_EQ(GetPlaceholderMetaFileUpdateCount(), 2U);
}

} // namespace OHOS::FileManagement::CloudDiskService::Test
