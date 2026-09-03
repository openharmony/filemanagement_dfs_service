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

#include "cloud_disk_service_metafile.h"

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;

class PlaceholderHelperTest : public testing::Test {};

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

} // namespace OHOS::FileManagement::CloudDiskService::Test
