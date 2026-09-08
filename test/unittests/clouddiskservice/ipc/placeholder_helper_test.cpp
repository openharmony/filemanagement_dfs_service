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

#include <cerrno>
#include <memory>

#include "assistant.h"
#include "cloud_disk_common.h"
#include "cloud_disk_service_error.h"
#include "cloud_disk_service_metafile.h"

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;

class PlaceholderHelperTest : public testing::Test {
public:
    void SetUp() override
    {
        mock_ = std::make_shared<AssistantMock>();
        Assistant::ins = mock_;
    }

    void TearDown() override
    {
        Mock::VerifyAndClearExpectations(mock_.get());
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

} // namespace OHOS::FileManagement::CloudDiskService::Test
