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
#include <memory>

#include "cloud_disk_common.h"

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing::ext;

namespace {
constexpr uint64_t TEST_DATA_OFFSET = 4;
} // namespace

class CallbackExecuteRequestTest : public testing::Test {
public:
    void SetUp() override
    {
        request_.reqKey = {1, 2, 3, 4, 5, 6, 7, 8};
        request_.syncFolder = "sync";
        request_.filePath = "file.txt";
        request_.data = {9, 10, 11};
        request_.offset = TEST_DATA_OFFSET;
        request_.size = request_.data.size();
        request_.totalSize = request_.offset + request_.size;
        request_.isComplete = true;
    }

    CallbackExecuteRequest request_;
};

/**
 * @tc.name: Marshalling_001
 * @tc.desc: Preserve every Execute field and its wire order through a real Parcel round trip.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CallbackExecuteRequestTest, Marshalling_001, TestSize.Level1)
{
    Parcel parcel;
    ASSERT_TRUE(request_.Marshalling(parcel));
    std::unique_ptr<CallbackExecuteRequest> result(CallbackExecuteRequest::Unmarshalling(parcel));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->reqKey, request_.reqKey);
    EXPECT_EQ(result->syncFolder, request_.syncFolder);
    EXPECT_EQ(result->filePath, request_.filePath);
    EXPECT_EQ(result->callbackType, request_.callbackType);
    EXPECT_EQ(result->offset, request_.offset);
    EXPECT_EQ(result->size, request_.size);
    EXPECT_EQ(result->totalSize, request_.totalSize);
    EXPECT_EQ(result->data, request_.data);
    EXPECT_EQ(result->isComplete, request_.isComplete);
}

/**
 * @tc.name: Marshalling_002
 * @tc.desc: Reject oversize buffers and propagate write failures caused by insufficient parcel capacity.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(CallbackExecuteRequestTest, Marshalling_002, TestSize.Level2)
{
    Parcel complete;
    ASSERT_TRUE(request_.Marshalling(complete));
    for (size_t remaining = 0; remaining < complete.GetDataSize(); remaining += sizeof(int32_t)) {
        Parcel parcel;
        std::vector<uint8_t> filler(parcel.GetMaxCapacity() - remaining);
        ASSERT_TRUE(parcel.WriteBuffer(filler.data(), filler.size()));
        EXPECT_FALSE(request_.Marshalling(parcel));
    }
    Parcel parcel;
    request_.data.resize(MAX_EXECUTE_DATA_SIZE + 1);
    EXPECT_FALSE(request_.Marshalling(parcel));
    request_.data.clear();
    request_.reqKey.clear();
    EXPECT_FALSE(request_.Marshalling(parcel));
}

/**
 * @tc.name: Unmarshalling_001
 * @tc.desc: Reject truncation at every byte boundary instead of accepting partially decoded fields.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(CallbackExecuteRequestTest, Unmarshalling_001, TestSize.Level2)
{
    Parcel complete;
    ASSERT_TRUE(request_.Marshalling(complete));
    for (size_t size = 0; size < complete.GetDataSize(); ++size) {
        Parcel truncated;
        if (size != 0) {
            ASSERT_TRUE(truncated.WriteBuffer(reinterpret_cast<const void *>(complete.GetData()), size));
            ASSERT_TRUE(truncated.SetDataSize(size));
        }
        std::unique_ptr<CallbackExecuteRequest> result(CallbackExecuteRequest::Unmarshalling(truncated));
        EXPECT_EQ(result, nullptr) << "accepted truncated size " << size;
    }
}

/**
 * @tc.name: Unmarshalling_002
 * @tc.desc: Reject negative or excessive vector lengths before allocating their payload.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(CallbackExecuteRequestTest, Unmarshalling_002, TestSize.Level2)
{
    for (int32_t length : {-1, static_cast<int32_t>(MAX_CALLBACK_REQUEST_KEY_SIZE + 1)}) {
        Parcel parcel;
        ASSERT_TRUE(parcel.WriteInt32(length));
        std::unique_ptr<CallbackExecuteRequest> result(CallbackExecuteRequest::Unmarshalling(parcel));
        EXPECT_EQ(result, nullptr);
    }
    Parcel parcel;
    ASSERT_TRUE(parcel.WriteUInt8Vector(request_.reqKey));
    ASSERT_TRUE(parcel.WriteString(request_.syncFolder));
    ASSERT_TRUE(parcel.WriteString(request_.filePath));
    ASSERT_TRUE(parcel.WriteInt32(request_.callbackType));
    ASSERT_TRUE(parcel.WriteUint64(request_.offset));
    ASSERT_TRUE(parcel.WriteUint64(request_.size));
    ASSERT_TRUE(parcel.WriteUint64(request_.totalSize));
    ASSERT_TRUE(parcel.WriteInt32(static_cast<int32_t>(MAX_EXECUTE_DATA_SIZE + 1)));
    std::unique_ptr<CallbackExecuteRequest> result(CallbackExecuteRequest::Unmarshalling(parcel));
    EXPECT_EQ(result, nullptr);
}
} // namespace OHOS::FileManagement::CloudDiskService::Test
