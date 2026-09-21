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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "file_dfs_listener_mock.h"
#include "file_dfs_listener_stub.h"
#include "file_dfs_listener_interface_code.h"
#include "message_parcel_mock.h"
#include "utils_log.h"
#include "dfs_error.h"

namespace OHOS::Storage::DistributedFile::Test {
using namespace testing;
using namespace testing::ext;
using namespace OHOS::FileManagement;
using namespace FileManagement::ModuleFileIO;

namespace {
    constexpr int NO_ERROR = 0;
    constexpr int E_INVAL_ARG = 1;
}

class MockFileDfsListenerStub final : public FileDfsListenerStub {
public:
    MOCK_METHOD4(OnStatus, void(const std::string &networkId, int32_t status, const std::string &path, int32_t type));
};

class FileDfsListenerStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
public:
    static inline std::shared_ptr<MockFileDfsListenerStub> mockStub_ = nullptr;
    static inline std::shared_ptr<MessageParcelMock> messageParcelMock_ = nullptr;
};

void FileDfsListenerStubTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    mockStub_ = std::make_shared<MockFileDfsListenerStub>();
    messageParcelMock_ = std::make_shared<MessageParcelMock>();
    MessageParcelMock::messageParcel = messageParcelMock_;
}

void FileDfsListenerStubTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    mockStub_ = nullptr;
    MessageParcelMock::messageParcel = nullptr;
    messageParcelMock_ = nullptr;
}

void FileDfsListenerStubTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FileDfsListenerStubTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FileDfsListenerStub_HandleOnStatus_0100
 * @tc.desc: Verify the HandleOnStatus function with various failure scenarios
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(FileDfsListenerStubTest, FileDfsListenerStub_HandleOnStatus_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileDfsListenerStub_HandleOnStatus_0100 Start";
    MessageParcel data;
    MessageParcel reply;

    // Test case: Read networkId fails
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(false));
    auto ret = mockStub_->HandleOnStatus(data, reply);
    EXPECT_EQ(ret, E_INVAL_ARG);

    // Test case: Read status fails
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(false));
    ret = mockStub_->HandleOnStatus(data, reply);
    EXPECT_EQ(ret, E_INVAL_ARG);

    // Test case: Read path fails
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(true));
    ret = mockStub_->HandleOnStatus(data, reply);
    EXPECT_EQ(ret, E_INVAL_ARG);

    // Test case: Read type fails
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(true)).WillOnce(Return(false));
    ret = mockStub_->HandleOnStatus(data, reply);
    EXPECT_EQ(ret, E_INVAL_ARG);

    // Test case: Empty networkId
    std::string networkId = "aaa";
    std::string path = "testPath";
    int32_t status = 0;
    int32_t type = 1;
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(DoAll(SetArgReferee<0>(networkId), Return(true)))
                                               .WillOnce(DoAll(SetArgReferee<0>(path), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(status), Return(true)))
                                                .WillOnce(DoAll(SetArgReferee<0>(type), Return(true)));
    ret = mockStub_->HandleOnStatus(data, reply);
    EXPECT_EQ(ret, NO_ERROR);

    GTEST_LOG_(INFO) << "FileDfsListenerStub_HandleOnStatus_0100 End";
}

} // namespace OHOS::Storage::DistributedFile::Test