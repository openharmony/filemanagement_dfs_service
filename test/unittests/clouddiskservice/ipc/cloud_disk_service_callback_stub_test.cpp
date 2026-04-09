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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "cloud_disk_service_callback_stub.h"
#include "cloud_disk_service_error.h"
#include "dfs_error.h"
#include "message_parcel_mock.h"

namespace OHOS {
namespace FileManagement::CloudDiskService {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace OHOS::Storage::DistributedFile;

class MockCloudDiskServiceCallback : public CloudDiskServiceCallbackStub {
public:
    MOCK_METHOD2(OnChangeData, void(const std::string &sandboxPath, const std::vector<ChangeData> &changeData));
};

class CloudDiskServiceCallbackStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<MockCloudDiskServiceCallback> callback_;
    std::shared_ptr<MessageParcelMock> messageParcelMock_;
};

void CloudDiskServiceCallbackStubTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudDiskServiceCallbackStubTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudDiskServiceCallbackStubTest::SetUp(void)
{
    callback_ = std::make_shared<MockCloudDiskServiceCallback>();
    messageParcelMock_ = std::make_shared<MessageParcelMock>();
    DfsMessageParcel::messageParcel = messageParcelMock_;
    std::cout << "SetUp" << std::endl;
}

void CloudDiskServiceCallbackStubTest::TearDown(void)
{
    DfsMessageParcel::messageParcel = nullptr;
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: HandleOnChangeDataTest001
 * @tc.desc: Verify HandleOnChangeData function - ReadString(sandboxPath) failed
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskServiceCallbackStubTest, HandleOnChangeDataTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnChangeDataTest001 Start";
    try {

        EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(false));
        
        MessageParcel data;
        MessageParcel reply;
        int ret = callback_->HandleOnChangeData(data, reply);
        EXPECT_EQ(ret, E_INVALID_ARG);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleOnChangeDataTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnChangeDataTest001 End";
}

/**
 * @tc.name: HandleOnChangeDataTest002
 * @tc.desc: Verify HandleOnChangeData function - ReadInt32(size) failed
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskServiceCallbackStubTest, HandleOnChangeDataTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnChangeDataTest002 Start";
    try {

        EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(DoAll(SetArgReferee<0>("path"), Return(true)));
        EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(false));
        
        MessageParcel data;
        MessageParcel reply;
        int ret = callback_->HandleOnChangeData(data, reply);
        EXPECT_EQ(ret, E_INVALID_ARG);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleOnChangeDataTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnChangeDataTest002 End";
}

/**
 * @tc.name: HandleOnChangeDataTest003
 * @tc.desc: Verify HandleOnChangeData function - all reads succeed
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskServiceCallbackStubTest, HandleOnChangeDataTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnChangeDataTest003 Start";
    try {
        EXPECT_CALL(*callback_, OnChangeData(_, _)).WillOnce(Return());
        
        EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(DoAll(SetArgReferee<0>("path"), Return(true)));
        EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(0), Return(true)));
        
        MessageParcel data;
        MessageParcel reply;
        int ret = callback_->HandleOnChangeData(data, reply);
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleOnChangeDataTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnChangeDataTest003 End";
}

} // namespace Test
} // namespace FileManagement::CloudDiskService
} // namespace OHOS