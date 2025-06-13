/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "cloud_sync_common.h"

namespace OHOS {
namespace FileManagement::CloudSync {
using namespace testing::ext;
using namespace testing;
using namespace std;
constexpr uint32_t MAX_MAP_SIZE = 1024;
class CloudSyncCommonTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<ParcelMock> parcel_{nullptr};
};

void CloudSyncCommonTest::SetUpTestCase(void)
{
    parcel_ = std::make_shared<ParcelMock>();
    IParcel::parcel_ = parcel_;
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudSyncCommonTest::TearDownTestCase(void)
{
    parcel_ = nullptr;
    IParcel::parcel_ = nullptr;
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudSyncCommonTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void CloudSyncCommonTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/*
 * @tc.name: Marshalling
 * @tc.desc: Verify the Marshalling function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, Marshalling, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Marshalling Start";
    try {
        SwitchDataObj switchDataObj;
        Parcel parcel;
        EXPECT_CALL(*parcel_, WriteUint32(_)).WillOnce(Return(false));
        auto res = switchDataObj.Marshalling(parcel);
        EXPECT_TRUE(!res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Marshalling FAILED";
    }
    GTEST_LOG_(INFO) << "Marshalling End";
}

/*
 * @tc.name: Marshalling001
 * @tc.desc: Verify the Marshalling001 function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, Marshalling001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Marshalling001 Start";
    try {
        SwitchDataObj switchDataObj;
        switchDataObj.switchData.insert(std::make_pair("1", false));
        Parcel parcel;
        EXPECT_CALL(*parcel_, WriteUint32(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteString(_)).WillOnce(Return(false));
        auto res = switchDataObj.Marshalling(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, WriteUint32(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteString(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteBool(_)).WillOnce(Return(true));
        res = switchDataObj.Marshalling(parcel);
        EXPECT_TRUE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Marshalling001 FAILED";
    }
    GTEST_LOG_(INFO) << "Marshalling001 End";
}

/*
 * @tc.name: Marshalling002
 * @tc.desc: Verify the Marshalling002 function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, Marshalling002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Marshalling002 Start";
    try {
        SwitchDataObj switchDataObj;
        switchDataObj.switchData.insert(std::make_pair("1", false));
        Parcel parcel;

        EXPECT_CALL(*parcel_, WriteUint32(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteString(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteBool(_)).WillOnce(Return(false));
        auto res = switchDataObj.Marshalling(parcel);
        EXPECT_TRUE(!res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Marshalling002 FAILED";
    }
    GTEST_LOG_(INFO) << "Marshalling002 End";
}

/*
 * @tc.name: Marshalling003
 * @tc.desc: Verify the Marshalling003 function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, Marshalling003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Marshalling003 Start";
    try {
        DownloadProgressObj downloadProgressObj;
        Parcel parcel;
        EXPECT_CALL(*parcel_, WriteString(_)).WillOnce(Return(false));
        auto res = downloadProgressObj.Marshalling(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, WriteString(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(false));
        res = downloadProgressObj.Marshalling(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, WriteString(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt64(_)).WillOnce(Return(false));
        res = downloadProgressObj.Marshalling(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, WriteString(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt64(_)).WillOnce(Return(true)).WillOnce(Return(false));
        res = downloadProgressObj.Marshalling(parcel);
        EXPECT_TRUE(!res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Marshalling003 FAILED";
    }
    GTEST_LOG_(INFO) << "Marshalling003 End";
}

/*
 * @tc.name: Marshalling004
 * @tc.desc: Verify the Marshalling004 function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, Marshalling004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Marshalling004 Start";
    try {
        CleanOptions cleanOptions;
        cleanOptions.appActionsData.insert(std::make_pair("1", 0));
        Parcel parcel;
        EXPECT_CALL(*parcel_, WriteUint32(_)).WillOnce(Return(false));
        auto res = cleanOptions.Marshalling(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, WriteUint32(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteString(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(true));
        res = cleanOptions.Marshalling(parcel);
        EXPECT_TRUE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Marshalling004 FAILED";
    }
    GTEST_LOG_(INFO) << "Marshalling004 End";
}

/*
 * @tc.name: Marshalling005
 * @tc.desc: Verify the Marshalling005 function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, Marshalling005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Marshalling005 Start";
    try {
        CleanOptions cleanOptions;
        cleanOptions.appActionsData.insert(std::make_pair("1", 0));
        Parcel parcel;
        EXPECT_CALL(*parcel_, WriteUint32(_)).WillOnce(Return(false));
        auto res = cleanOptions.Marshalling(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, WriteUint32(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteString(_)).WillOnce(Return(false));
        res = cleanOptions.Marshalling(parcel);
        EXPECT_TRUE(!res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Marshalling005 FAILED";
    }
    GTEST_LOG_(INFO) << "Marshalling005 End";
}

/*
 * @tc.name: Marshalling006
 * @tc.desc: Verify the Marshalling006 function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, Marshalling006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Marshalling006 Start";
    try {
        CleanOptions cleanOptions;
        cleanOptions.appActionsData.insert(std::make_pair("1", 0));
        Parcel parcel;
        EXPECT_CALL(*parcel_, WriteUint32(_)).WillOnce(Return(false));
        auto res = cleanOptions.Marshalling(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, WriteUint32(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteString(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteString(_)).WillOnce(Return(false));
        res = cleanOptions.Marshalling(parcel);
        EXPECT_TRUE(!res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Marshalling006 FAILED";
    }
    GTEST_LOG_(INFO) << "Marshalling006 End";
}

/*
 * @tc.name: Marshalling010
 * @tc.desc: Verify the Marshalling010 function.
 * @tc.type: FUNC
 * @tc.require: ICEU6Z
 */
HWTEST_F(CloudSyncCommonTest, Marshalling010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Marshalling010 Start";
    try {
        DownloadProgressObj downloadProgressObj;
        Parcel parcel;

        EXPECT_CALL(*parcel_, WriteString(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(false));
        EXPECT_CALL(*parcel_, WriteInt64(_)).WillOnce(Return(true)).WillOnce(Return(true));
        bool res = downloadProgressObj.Marshalling(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, WriteString(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt64(_)).WillOnce(Return(true))
            .WillOnce(Return(true)).WillOnce(Return(false));
        res = downloadProgressObj.Marshalling(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, WriteString(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt64(_)).WillOnce(Return(true))
            .WillOnce(Return(true)).WillOnce(Return(true))
            .WillOnce(Return(false));
        res = downloadProgressObj.Marshalling(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, WriteString(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt64(_)).WillOnce(Return(true))
            .WillOnce(Return(true)).WillOnce(Return(true))
            .WillOnce(Return(true)).WillOnce(Return(false));
        res = downloadProgressObj.Marshalling(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, WriteString(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt64(_)).WillOnce(Return(true))
            .WillOnce(Return(true)).WillOnce(Return(true))
            .WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(false));
        res = downloadProgressObj.Marshalling(parcel);
        EXPECT_TRUE(!res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Marshalling010 FAILED";
    }
    GTEST_LOG_(INFO) << "Marshalling010 End";
}

/*
 * @tc.name: Marshalling011
 * @tc.desc: Verify the Marshalling011 function.
 * @tc.type: FUNC
 * @tc.require: ICEU6Z
 */
HWTEST_F(CloudSyncCommonTest, Marshalling011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Marshalling011 Start";
    try {
        DownloadProgressObj downloadProgressObj;
        Parcel parcel;

        EXPECT_CALL(*parcel_, WriteString(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt64(_)).WillOnce(Return(true))
            .WillOnce(Return(true)).WillOnce(Return(true))
            .WillOnce(Return(true)).WillOnce(Return(true))
            .WillOnce(Return(true)).WillOnce(Return(false));
        bool res = downloadProgressObj.Marshalling(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, WriteString(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt64(_)).WillOnce(Return(true))
            .WillOnce(Return(true)).WillOnce(Return(true))
            .WillOnce(Return(true)).WillOnce(Return(true))
            .WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(false));
        res = downloadProgressObj.Marshalling(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, WriteString(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(true))
            .WillOnce(Return(true)).WillOnce(Return(false));
        EXPECT_CALL(*parcel_, WriteInt64(_)).WillOnce(Return(true))
            .WillOnce(Return(true)).WillOnce(Return(true))
            .WillOnce(Return(true)).WillOnce(Return(true))
            .WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(true));
        res = downloadProgressObj.Marshalling(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, WriteString(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(true))
            .WillOnce(Return(true)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt64(_)).WillOnce(Return(true))
            .WillOnce(Return(true)).WillOnce(Return(true))
            .WillOnce(Return(true)).WillOnce(Return(true))
            .WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(true));
        res = downloadProgressObj.Marshalling(parcel);
        EXPECT_TRUE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Marshalling011 FAILED";
    }
    GTEST_LOG_(INFO) << "Marshalling011 End";
}

/*
 * @tc.name: Marshalling009
 * @tc.desc: Verify the Marshalling009 function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, Marshalling009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Marshalling009 Start";
    try {
        DowngradeProgress progress;
        Parcel parcel;
        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(false));
        auto res = progress.Marshalling(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(false));
        res = progress.Marshalling(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt64(_)).WillOnce(Return(false));
        res = progress.Marshalling(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt64(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt64(_)).WillOnce(Return(false));
        res = progress.Marshalling(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(false));
        EXPECT_CALL(*parcel_, WriteInt64(_)).WillOnce(Return(true)).WillOnce(Return(true));
        res = progress.Marshalling(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true))
            .WillOnce(Return(true)).WillOnce(Return(false));
        EXPECT_CALL(*parcel_, WriteInt64(_)).WillOnce(Return(true)).WillOnce(Return(true));
        res = progress.Marshalling(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true))
            .WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(false));
        EXPECT_CALL(*parcel_, WriteInt64(_)).WillOnce(Return(true)).WillOnce(Return(true));
        res = progress.Marshalling(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true))
            .WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt64(_)).WillOnce(Return(true)).WillOnce(Return(true));
        res = progress.Marshalling(parcel);
        EXPECT_TRUE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Marshalling009 FAILED";
    }
    GTEST_LOG_(INFO) << "Marshalling009 End";
}

/*
 * @tc.name: Marshalling008
 * @tc.desc: Verify the Marshalling008 function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, Marshalling008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Marshalling008 Start";
    try {
        CloudFileInfo fileInfo;
        Parcel parcel;
        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(false));
        auto res = fileInfo.Marshalling(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt64(_)).WillOnce(Return(false));
        res = fileInfo.Marshalling(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(false));
        EXPECT_CALL(*parcel_, WriteInt64(_)).WillOnce(Return(true));
        res = fileInfo.Marshalling(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt64(_)).WillOnce(Return(true)).WillOnce(Return(false));
        res = fileInfo.Marshalling(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(false));
        EXPECT_CALL(*parcel_, WriteInt64(_)).WillOnce(Return(true)).WillOnce(Return(true));
        res = fileInfo.Marshalling(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt64(_)).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(false));
        res = fileInfo.Marshalling(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, WriteInt64(_)).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(true));
        res = fileInfo.Marshalling(parcel);
        EXPECT_TRUE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Marshalling008 FAILED";
    }
    GTEST_LOG_(INFO) << "Marshalling008 End";
}

/*
 * @tc.name: Unmarshalling001
 * @tc.desc: Verify the Unmarshalling001 function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, Unmarshalling001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Unmarshalling001 Start";
    try {
        auto switchDataObj = make_shared<SwitchDataObj>();
        Parcel parcel;
        EXPECT_CALL(*parcel_, ReadUint32(_)).WillOnce(Return(false));
        auto res = switchDataObj->Unmarshalling(parcel);
        EXPECT_TRUE(res == nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Unmarshalling001 FAILED";
    }
    GTEST_LOG_(INFO) << "Unmarshalling001 End";
}

/*
 * @tc.name: Unmarshalling002
 * @tc.desc: Verify the Unmarshalling002 function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, Unmarshalling002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Unmarshalling002 Start";
    try {
        auto downloadProgressObj = make_shared<DownloadProgressObj>();
        Parcel parcel;
        EXPECT_CALL(*parcel_, ReadString(_)).WillOnce(Return(false));
        auto res = downloadProgressObj->Unmarshalling(parcel);
        EXPECT_TRUE(res == nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Unmarshalling002 FAILED";
    }
    GTEST_LOG_(INFO) << "Unmarshalling002 End";
}

/*
 * @tc.name: Unmarshalling003
 * @tc.desc: Verify the Unmarshalling003 function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, Unmarshalling003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Unmarshalling003 Start";
    try {
        auto cleanOptions = make_shared<CleanOptions>();
        Parcel parcel;
        EXPECT_CALL(*parcel_, ReadUint32(_)).WillOnce(Return(false));
        auto res = cleanOptions->Unmarshalling(parcel);
        EXPECT_TRUE(res == nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Unmarshalling003 FAILED";
    }
    GTEST_LOG_(INFO) << "Unmarshalling003 End";
}

/*
 * @tc.name: Unmarshalling004
 * @tc.desc: Verify the Unmarshalling004 function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, Unmarshalling004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Unmarshalling004 Start";
    try {
        auto progress = make_shared<DowngradeProgress>();
        Parcel parcel;
        EXPECT_CALL(*parcel_, ReadInt32(_)).WillOnce(Return(false));
        auto res = progress->Unmarshalling(parcel);
        EXPECT_TRUE(res == nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Unmarshalling004 FAILED";
    }
    GTEST_LOG_(INFO) << "Unmarshalling004 End";
}

/*
 * @tc.name: Unmarshalling005
 * @tc.desc: Verify the Unmarshalling005 function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, Unmarshalling005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Unmarshalling005 Start";
    try {
        auto fileInfo = make_shared<CloudFileInfo>();
        Parcel parcel;
        EXPECT_CALL(*parcel_, ReadInt32(_)).WillOnce(Return(false));
        auto res = fileInfo->Unmarshalling(parcel);
        EXPECT_TRUE(res == nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Unmarshalling005 FAILED";
    }
    GTEST_LOG_(INFO) << "Unmarshalling005 End";
}

/*
 * @tc.name: ReadFromParcel001
 * @tc.desc: Verify the ReadFromParcel001 function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, ReadFromParcel001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadFromParcel001 Start";
    try {
        SwitchDataObj switchDataObj;
        Parcel parcel;
        EXPECT_CALL(*parcel_, ReadUint32(_)).WillOnce(Return(false));
        auto res = switchDataObj.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ReadFromParcel001 FAILED";
    }
    GTEST_LOG_(INFO) << "ReadFromParcel001 End";
}

/*
 * @tc.name: ReadFromParcel002
 * @tc.desc: Verify the ReadFromParcel002 function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, ReadFromParcel002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadFromParcel002 Start";
    try {
        SwitchDataObj switchDataObj;
        Parcel parcel;
        EXPECT_CALL(*parcel_, ReadUint32(_)).WillOnce(DoAll(SetArgReferee<0>(MAX_MAP_SIZE + 1), Return(true)));
        auto res = switchDataObj.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ReadFromParcel002 FAILED";
    }
    GTEST_LOG_(INFO) << "ReadFromParcel002 End";
}

/*
 * @tc.name: ReadFromParcel003
 * @tc.desc: Verify the ReadFromParcel003 function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, ReadFromParcel003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadFromParcel003 Start";
    try {
        SwitchDataObj switchDataObj;
        Parcel parcel;
        EXPECT_CALL(*parcel_, ReadUint32(_)).WillOnce(DoAll(SetArgReferee<0>(1), Return(true)));
        EXPECT_CALL(*parcel_, ReadString(_)).WillOnce(Return(false));
        auto res = switchDataObj.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, ReadUint32(_)).WillOnce(DoAll(SetArgReferee<0>(1), Return(true)));
        EXPECT_CALL(*parcel_, ReadString(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, ReadBool(_)).WillOnce(Return(false));
        res = switchDataObj.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, ReadUint32(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, ReadString(_)).WillOnce(DoAll(SetArgReferee<0>("123"), Return(true)));
        EXPECT_CALL(*parcel_, ReadBool(_)).WillOnce(DoAll(SetArgReferee<0>(false), Return(true)));
        res = switchDataObj.ReadFromParcel(parcel);
        EXPECT_TRUE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ReadFromParcel003 FAILED";
    }
    GTEST_LOG_(INFO) << "ReadFromParcel003 End";
}

/*
 * @tc.name: ReadFromParcel004
 * @tc.desc: Verify the ReadFromParcel004 function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, ReadFromParcel004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadFromParcel004 Start";
    try {
        CleanOptions cleanOptions;
        Parcel parcel;
        EXPECT_CALL(*parcel_, ReadUint32(_)).WillOnce(Return(false));
        auto res = cleanOptions.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ReadFromParcel004 FAILED";
    }
    GTEST_LOG_(INFO) << "ReadFromParcel004 End";
}

/*
 * @tc.name: ReadFromParcel005
 * @tc.desc: Verify the ReadFromParcel005 function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, ReadFromParcel005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadFromParcel005 Start";
    try {
        CleanOptions cleanOptions;
        Parcel parcel;
        EXPECT_CALL(*parcel_, ReadUint32(_)).WillOnce(DoAll(SetArgReferee<0>(MAX_MAP_SIZE + 1), Return(true)));
        auto res = cleanOptions.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ReadFromParcel005 FAILED";
    }
    GTEST_LOG_(INFO) << "ReadFromParcel005 End";
}

/*
 * @tc.name: ReadFromParcel006
 * @tc.desc: Verify the ReadFromParcel006 function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, ReadFromParcel006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadFromParcel006 Start";
    try {
        CleanOptions cleanOptions;
        Parcel parcel;

        EXPECT_CALL(*parcel_, ReadUint32(_)).WillOnce(DoAll(SetArgReferee<0>(1), Return(true)));
        EXPECT_CALL(*parcel_, ReadString(_)).WillOnce(Return(false));
        bool res = cleanOptions.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, ReadUint32(_)).WillOnce(DoAll(SetArgReferee<0>(1), Return(true)));
        EXPECT_CALL(*parcel_, ReadString(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, ReadInt32(_)).WillOnce(Return(false));
        res = cleanOptions.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, ReadUint32(_)).WillOnce(DoAll(SetArgReferee<0>(1), Return(true)));
        EXPECT_CALL(*parcel_, ReadString(_)).WillOnce(DoAll(SetArgReferee<0>("123"), Return(true)));
        EXPECT_CALL(*parcel_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(1), Return(true)));
        res = cleanOptions.ReadFromParcel(parcel);
        EXPECT_TRUE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ReadFromParcel006 FAILED";
    }
    GTEST_LOG_(INFO) << "ReadFromParcel006 End";
}

/*
 * @tc.name: ReadFromParcel007
 * @tc.desc: Verify the ReadFromParcel007 function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, ReadFromParcel007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadFromParcel007 Start";
    try {
        DownloadProgressObj downloadProgressObj;
        Parcel parcel;
        EXPECT_CALL(*parcel_, ReadString(_)).WillOnce(Return(false));
        auto res = downloadProgressObj.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, ReadString(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, ReadInt32(_)).WillOnce(Return(false));
        res = downloadProgressObj.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, ReadString(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, ReadInt32(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, ReadInt64(_)).WillOnce(Return(false));
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, ReadString(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, ReadInt32(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, ReadInt64(_)).WillOnce(Return(true)).WillOnce(Return(false));
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, ReadString(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, ReadInt32(_)).WillOnce(Return(true)).WillOnce(Return(false));
        EXPECT_CALL(*parcel_, ReadInt64(_)).WillOnce(Return(true)).WillOnce(Return(true));
        res = downloadProgressObj.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, ReadString(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, ReadInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, ReadInt64(_)).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(false));
        res = downloadProgressObj.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ReadFromParcel007 FAILED";
    }
    GTEST_LOG_(INFO) << "ReadFromParcel007 End";
}

/*
 * @tc.name: to_string
 * @tc.desc: Verify the to_string function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, to_string, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "to_string Start";
    try {
        DownloadProgressObj downloadProgressObj;
        auto res = downloadProgressObj.to_string();
        std::string expectStr =
            "DownloadProgressObj [path: ******** state: 0 downloaded: 0 total: 0 downloadErrorType: 0 "
            "downloadId: 0 batchState: 0 batchDownloadSize: 0 batchTotalSize: 0 "
            "batchSuccNum: 0 batchFailNum: 0 batchTotalNum: 0]";
        EXPECT_EQ(res, expectStr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " to_string FAILED";
    }
    GTEST_LOG_(INFO) << "to_string End";
}

/*
 * @tc.name: to_string
 * @tc.desc: Verify the to_string function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, DowngradeProgress_to_string, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "to_string Start";
    try {
        DowngradeProgress progress;
        auto res = progress.to_string();
        progress.state = DowngradeProgress::RUNNING;
        progress.stopReason = DowngradeProgress::NO_STOP;
        progress.downloadedSize = 0;
        progress.totalSize = 0;
        progress.successfulCount = 0;
        progress.failedCount = 0;
        progress.totalCount = 0;

        std::string expectStr =
            "DowngradeProgress [DownloadState: 0 downloadStopReason: 0 downloadedSize: 0 totalSize: 0 "
            "successfulCount: 0 failedCount: 0 totalCount: 0]";
        EXPECT_EQ(res, expectStr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " to_string FAILED";
    }
    GTEST_LOG_(INFO) << "to_string End";
}

/*
 * @tc.name: ReadFromParcel00
 * @tc.desc: Verify the ReadFromParcel00 function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, ReadFromParcel00, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadFromParcel00 Start";
    try {
        AssetInfoObj assetInfo;
        Parcel parcel;
        bool returnValues[5] = {true, true, true, true, true};
        int callCount = 0;

        EXPECT_CALL(*parcel_, ReadString(_)).Times(5).WillRepeatedly(Invoke([&]() {
            return returnValues[callCount++];
        }));
        auto res = assetInfo.ReadFromParcel(parcel);
        EXPECT_TRUE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ReadFromParcel00 FAILED";
    }
    GTEST_LOG_(INFO) << "ReadFromParcel00 End";
}

/*
 * @tc.name: ReadFromParcel100
 * @tc.desc: Verify the ReadFromParcel function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, ReadFromParcel100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadFromParcel100 Start";
    try {
        DowngradeProgress progress;
        Parcel parcel;
        EXPECT_CALL(*parcel_, ReadInt32(_)).WillOnce(Return(false));
        auto res = progress.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, ReadInt32(_)).WillOnce(Return(true)).WillOnce(Return(false));
        res = progress.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, ReadInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, ReadInt64(_)).WillOnce(Return(false));
        res = progress.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, ReadInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, ReadInt64(_)).WillOnce(Return(true)).WillOnce(Return(false));
        res = progress.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, ReadInt32(_)).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(false));
        EXPECT_CALL(*parcel_, ReadInt64(_)).WillOnce(Return(true)).WillOnce(Return(true));
        res = progress.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, ReadInt32(_)).WillOnce(Return(true)).WillOnce(Return(true))
            .WillOnce(Return(true)).WillOnce(Return(false));
        EXPECT_CALL(*parcel_, ReadInt64(_)).WillOnce(Return(true)).WillOnce(Return(true));
        res = progress.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, ReadInt32(_)).WillOnce(Return(true)).WillOnce(Return(true))
            .WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(false));
        EXPECT_CALL(*parcel_, ReadInt64(_)).WillOnce(Return(true)).WillOnce(Return(true));
        res = progress.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, ReadInt32(_)).WillOnce(Return(true)).WillOnce(Return(true))
            .WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, ReadInt64(_)).WillOnce(Return(true)).WillOnce(Return(true));
        res = progress.ReadFromParcel(parcel);
        EXPECT_TRUE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ReadFromParcel100 FAILED";
    }
    GTEST_LOG_(INFO) << "ReadFromParcel100 End";
}

/*
 * @tc.name: ReadFromParcel101
 * @tc.desc: Verify the ReadFromParcel function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, ReadFromParcel101, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadFromParcel101 Start";
    try {
        CloudFileInfo finleInfo;
        Parcel parcel;
        EXPECT_CALL(*parcel_, ReadInt32(_)).WillOnce(Return(false));
        auto res = finleInfo.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, ReadInt32(_)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, ReadInt64(_)).WillOnce(Return(false));
        res = finleInfo.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, ReadInt32(_)).WillOnce(Return(true)).WillOnce(Return(false));
        EXPECT_CALL(*parcel_, ReadInt64(_)).WillOnce(Return(true));
        res = finleInfo.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, ReadInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, ReadInt64(_)).WillOnce(Return(true)).WillOnce(Return(false));
        res = finleInfo.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, ReadInt32(_)).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(false));
        EXPECT_CALL(*parcel_, ReadInt64(_)).WillOnce(Return(true)).WillOnce(Return(true));
        res = finleInfo.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, ReadInt32(_)).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, ReadInt64(_)).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(false));
        res = finleInfo.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        EXPECT_CALL(*parcel_, ReadInt32(_)).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, ReadInt64(_)).WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(true));
        res = finleInfo.ReadFromParcel(parcel);
        EXPECT_TRUE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ReadFromParcel101 FAILED";
    }
    GTEST_LOG_(INFO) << "ReadFromParcel101 End";
}

/*
 * @tc.name: Marshalling007
 * @tc.desc: Verify the Marshalling007 function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, Marshalling007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Marshalling007 Start";
    try {
        AssetInfoObj assetInfo;
        Parcel parcel;
        bool returnValues[5] = {true, true, true, true, true};
        int callCount = 0;

        EXPECT_CALL(*parcel_, WriteString(_)).Times(5).WillRepeatedly(Invoke([&]() {
            return returnValues[callCount++];
        }));
        auto res = assetInfo.Marshalling(parcel);
        EXPECT_TRUE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Marshalling007 FAILED";
    }
    GTEST_LOG_(INFO) << "Marshalling007 End";
}

/*
 * @tc.name: Unmarshalling
 * @tc.desc: Verify the Unmarshalling function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, Unmarshalling, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Unmarshalling Start";
    try {
        auto assetInfo = make_shared<AssetInfoObj>();
        Parcel parcel;
        bool returnValues[5] = {true, true, true, true, true};
        int callCount = 0;

        EXPECT_CALL(*parcel_, ReadString(_)).Times(5).WillRepeatedly(Invoke([&]() {
            return returnValues[callCount++];
        }));
        auto res = assetInfo->Unmarshalling(parcel);
        EXPECT_TRUE(res != nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Unmarshalling FAILED";
    }
    GTEST_LOG_(INFO) << "Unmarshalling End";
}

/*
 * @tc.name: Unmarshalling
 * @tc.desc: Verify the Unmarshalling function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, UnmarshallingTest1, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnmarshallingTest1 Start";
    try {
        auto Info = make_shared<DentryFileInfoObj>();
        Parcel parcel;
        bool returnValues[4] = {true, true, true, true};
        int callCount = 0;

        EXPECT_CALL(*parcel_, ReadString(_)).Times(4).WillRepeatedly(Invoke([&]() {
            return returnValues[callCount++];
        }));
        EXPECT_CALL(*parcel_, ReadInt64(_)).WillOnce(Return(true)).WillOnce(Return(true));
        auto res = Info->Unmarshalling(parcel);
        EXPECT_TRUE(res != nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " UnmarshallingTest1 FAILED";
    }
    GTEST_LOG_(INFO) << "UnmarshallingTest1 End";
}

/*
 * @tc.name: Unmarshalling
 * @tc.desc: Verify the Unmarshalling function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCommonTest, UnmarshallingTest2, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnmarshallingTest2 Start";
    try {
        auto Info = make_shared<CleanFileInfoObj>();
        Parcel parcel;
        bool returnValues[3] = {true, true, true};
        int callCount = 0;

        EXPECT_CALL(*parcel_, ReadString(_)).Times(3).WillRepeatedly(Invoke([&]() {
            return returnValues[callCount++];
        }));
        EXPECT_CALL(*parcel_, ReadInt64(_)).WillOnce(Return(true)).WillOnce(Return(true));
        EXPECT_CALL(*parcel_, ReadStringVector(_)).WillOnce(Return(true));
        auto res = Info->Unmarshalling(parcel);
        EXPECT_TRUE(res != nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " UnmarshallingTest2 FAILED";
    }
    GTEST_LOG_(INFO) << "UnmarshallingTest2 End";
}
} // namespace FileManagement::CloudSync
} // namespace OHOS
