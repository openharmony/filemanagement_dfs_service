/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "copy/ipc_wrapper.h"
#include <gtest/gtest.h>
#include <securec.h>

#include "dfs_error.h"
#include "i_daemon_mock.h"
#include "message_parcel_mock.h"

#include "directory_ex.h"

namespace OHOS::Storage::DistributedFile::Test {
using namespace OHOS::FileManagement;
using namespace testing;
using namespace testing::ext;
using namespace std;

constexpr size_t MAX_IPC_RAW_DATA_SIZE = 128 * 1024 * 1024;
class IpcWrapperTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    static inline shared_ptr<MessageParcelMock> messageParcelMock_ = nullptr;
};

void IpcWrapperTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    messageParcelMock_ = make_shared<MessageParcelMock>();
    MessageParcelMock::messageParcel = messageParcelMock_;
}

void IpcWrapperTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    messageParcelMock_ = nullptr;
    MessageParcelMock::messageParcel = nullptr;
}

void IpcWrapperTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void IpcWrapperTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
* @tc.name: WriteUriByRawData
* @tc.desc: The execution of the GetSize failed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(IpcWrapperTest, WriteUriByRawData, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "WriteUriByRawData Start";
    MessageParcel data;
    std::vector<std::string> uriVec = {"test_uri1", "test_uri2"};
    // Test case 1: WriteStringVector fails
    EXPECT_CALL(*messageParcelMock_, WriteStringVector(uriVec)).WillOnce(Return(false));
    bool result = IpcWrapper::WriteUriByRawData(data, uriVec);
    EXPECT_FALSE(result);

    // Test case 2: WriteInt32 fails (data size)
    EXPECT_CALL(*messageParcelMock_, WriteStringVector(uriVec)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    result = IpcWrapper::WriteUriByRawData(data, uriVec);
    EXPECT_FALSE(result);

    // Test case 3: WriteRawData fails
    EXPECT_CALL(*messageParcelMock_, WriteStringVector(uriVec)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRawData(_, _)).WillOnce(Return(false));
    result = IpcWrapper::WriteUriByRawData(data, uriVec);
    EXPECT_FALSE(result);

    // Test case 4: Successful write
    EXPECT_CALL(*messageParcelMock_, WriteStringVector(uriVec)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRawData(_, _)).WillOnce(Return(true));
    result = IpcWrapper::WriteUriByRawData(data, uriVec);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "WriteUriByRawData End";
}

/**
* @tc.name: WriteBatchUris
* @tc.desc: The execution of the GetSize failed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(IpcWrapperTest, WriteBatchUris, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "WriteBatchUris Start";
    MessageParcel data;
    std::vector<std::string> uriVec = {"test_uri1", "test_uri2"};

    // Test case 1: WriteUint32 fails
    EXPECT_CALL(*messageParcelMock_, WriteUint32(_)).WillOnce(Return(false));
    bool result = IpcWrapper::WriteBatchUris(data, uriVec);
    EXPECT_FALSE(result);

    // Test case 2: WriteUriByRawData fails
    EXPECT_CALL(*messageParcelMock_, WriteUint32(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteStringVector(_)).WillOnce(Return(false));
    result = IpcWrapper::WriteBatchUris(data, uriVec);
    EXPECT_FALSE(result);

    // Test case 3: Successful write
    EXPECT_CALL(*messageParcelMock_, WriteUint32(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteStringVector(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteRawData(_, _)).WillOnce(Return(true));
    result = IpcWrapper::WriteBatchUris(data, uriVec);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "WriteBatchUris End";
}

/**
* @tc.name: GetData
* @tc.desc: Test GetData function under different conditions.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(IpcWrapperTest, GetData, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetData Start";

    // Test case 1: Null input data
    void* buffer = nullptr;
    bool result = IpcWrapper::GetData(buffer, 1, nullptr);
    EXPECT_FALSE(result);
    EXPECT_EQ(buffer, nullptr);

    // Test case 2: Zero size
    const char* validData = "test";
    result = IpcWrapper::GetData(buffer, 0, validData);
    EXPECT_FALSE(result);
    EXPECT_EQ(buffer, nullptr);

    // Test case 3: Size exceeds maximum limit
    result = IpcWrapper::GetData(buffer, MAX_IPC_RAW_DATA_SIZE + 1, validData);
    EXPECT_FALSE(result);
    EXPECT_EQ(buffer, nullptr);

    // Test case 4: Successful memory copy
    size_t validSize = 5; // 5: "test" + null terminator
    result = IpcWrapper::GetData(buffer, validSize, validData);
    EXPECT_TRUE(result);
    ASSERT_NE(buffer, nullptr);
    EXPECT_EQ(memcmp(buffer, validData, validSize), 0);
    if (buffer != nullptr) {
        free(buffer); // Cleanup allocated memory
        buffer = nullptr;
    }
    GTEST_LOG_(INFO) << "GetData End";
}

/**
* @tc.name: ReadBatchUriByRawData
* @tc.desc: Test ReadBatchUriByRawData function under different conditions.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(IpcWrapperTest, ReadBatchUriByRawData, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ReadBatchUriByRawData Start";
    MessageParcel data;
    std::vector<std::string> uriVec;

    // Test case 1: ReadInt32 returns zero data size
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(0));
    bool result = IpcWrapper::ReadBatchUriByRawData(data, uriVec);
    EXPECT_FALSE(result);

    // Test case 2: GetData fails (null raw data)
    size_t dataSize = 5; // 5: test size
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(dataSize));
    EXPECT_CALL(*messageParcelMock_, ReadRawData(_)).WillOnce(Return(nullptr));
    result = IpcWrapper::ReadBatchUriByRawData(data, uriVec);
    EXPECT_FALSE(result);

    // Test case 3: Successful
    const char* validData = "test";
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(dataSize));
    EXPECT_CALL(*messageParcelMock_, ReadRawData(_)).WillOnce(Return(validData));
    EXPECT_CALL(*messageParcelMock_, ReadStringVector(_)).WillOnce(Return(true));
    result = IpcWrapper::ReadBatchUriByRawData(data, uriVec);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "ReadBatchUriByRawData End";
}

/**
* @tc.name: ReadBatchUris
* @tc.desc: Test ReadBatchUris function under different conditions.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(IpcWrapperTest, ReadBatchUris, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ReadBatchUris Start";
    MessageParcel data;
    std::vector<std::string> uriVec;

    // Test case 1: ReadUint32 returns zero
    EXPECT_CALL(*messageParcelMock_, ReadUint32()).WillOnce(Return(0));
    int32_t result = IpcWrapper::ReadBatchUris(data, uriVec);
    EXPECT_EQ(result, OHOS::FileManagement::E_INVAL_ARG);

    // Test case 2: ReadBatchUriByRawData fails
    EXPECT_CALL(*messageParcelMock_, ReadUint32()).WillOnce(Return(1)); // 1: test size
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(0)); // Causes failure
    result = IpcWrapper::ReadBatchUris(data, uriVec);
    EXPECT_EQ(result, OHOS::FileManagement::E_IPC_READ_FAILED);

    // Test case 3: Successful read
    const char* validData = "test";
    size_t dataSize = 5; // 5: test size
    std::vector<std::string> testVec;
    testVec.emplace_back("test");
    testVec.emplace_back("test");
    EXPECT_CALL(*messageParcelMock_, ReadUint32()).WillOnce(Return(1)); // 1: test size
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(dataSize));
    EXPECT_CALL(*messageParcelMock_, ReadRawData(_)).WillOnce(Return(validData));
    EXPECT_CALL(*messageParcelMock_, ReadStringVector(_)).WillOnce(Return(true));
    result = IpcWrapper::ReadBatchUris(data, uriVec);
    EXPECT_EQ(result, OHOS::FileManagement::E_IPC_READ_FAILED);
    GTEST_LOG_(INFO) << "ReadBatchUris End";
}
}
