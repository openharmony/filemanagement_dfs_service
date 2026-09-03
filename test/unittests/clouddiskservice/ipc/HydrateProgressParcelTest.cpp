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


#include <gtest/gtest.h>
#include <limits>
#include <memory>

#include "cloud_disk_common.h"

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing::ext;
class HydrateProgressParcelTest : public testing::Test {};
/**
 * @tc.name: RoundTrip_001
 * @tc.desc: Preserve progress wire order and unsigned 64-bit values.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HydrateProgressParcelTest, RoundTrip_001, TestSize.Level2)
{
    HydrateProgress progress;
    progress.filePath = "/storage/Users/currentUser/sync/file";
    progress.state = static_cast<int32_t>(HydrateProgressState::COMPLETED);
    progress.processedSize = std::numeric_limits<uint64_t>::max() - 1;
    progress.totalSize = std::numeric_limits<uint64_t>::max();
    Parcel parcel;
    ASSERT_TRUE(progress.Marshalling(parcel));
    std::unique_ptr<HydrateProgress> copy(HydrateProgress::Unmarshalling(parcel));
    ASSERT_NE(copy, nullptr);
    EXPECT_EQ(copy->filePath, progress.filePath);
    EXPECT_EQ(copy->state, progress.state);
    EXPECT_EQ(copy->processedSize, progress.processedSize);
    EXPECT_EQ(copy->totalSize, progress.totalSize);
}
/**
 * @tc.name: Truncated_001
 * @tc.desc: Reject truncation at every field and byte boundary.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(HydrateProgressParcelTest, Truncated_001, TestSize.Level2)
{
    HydrateProgress progress;
    progress.filePath = "/absolute/path";
    progress.processedSize = 5;
    progress.totalSize = 9;
    Parcel complete;
    ASSERT_TRUE(progress.Marshalling(complete));
    for (size_t size = 0; size < complete.GetDataSize(); ++size) {
        Parcel parcel;
        if (size != 0) {
            ASSERT_TRUE(parcel.WriteBuffer(reinterpret_cast<const void *>(complete.GetData()), size));
            ASSERT_TRUE(parcel.SetDataSize(size));
        }
        std::unique_ptr<HydrateProgress> copy(HydrateProgress::Unmarshalling(parcel));
        EXPECT_EQ(copy, nullptr) << size;
    }
}
/**
 * @tc.name: InvalidState_001
 * @tc.desc: Reject unsupported states including FAILED and propagate capacity errors.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(HydrateProgressParcelTest, InvalidState_001, TestSize.Level2)
{
    for (int32_t state : {-1, 4}) {
        HydrateProgress progress;
        progress.state = state;
        Parcel output;
        EXPECT_FALSE(progress.Marshalling(output));
        Parcel input;
        ASSERT_TRUE(input.WriteString("/path"));
        ASSERT_TRUE(input.WriteInt32(state));
        ASSERT_TRUE(input.WriteUint64(0));
        ASSERT_TRUE(input.WriteUint64(1));
        std::unique_ptr<HydrateProgress> copy(HydrateProgress::Unmarshalling(input));
        EXPECT_EQ(copy, nullptr);
    }
    HydrateProgress progress;
    Parcel full;
    std::vector<uint8_t> filler(full.GetMaxCapacity());
    ASSERT_TRUE(full.WriteBuffer(filler.data(), filler.size()));
    EXPECT_FALSE(progress.Marshalling(full));
}
} // namespace OHOS::FileManagement::CloudDiskService::Test
