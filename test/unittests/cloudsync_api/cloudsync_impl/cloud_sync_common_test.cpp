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
#include "dfs_error.h"
#include "iservice_registry.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing::ext;
using namespace testing;
using namespace std;

class CloudSyncCommonTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudSyncCommonTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudSyncCommonTest::TearDownTestCase(void)
{
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
        string strSwitchData = "continue";
        bool boolSwitchData = false;
        switchDataObj.switchData.insert({strSwitchData, boolSwitchData});
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
        Parcel parcel;

        auto res = switchDataObj.Marshalling(parcel);
        EXPECT_TRUE(!res);

        string strSwitchData = "continue";
        bool boolSwitchData = true;
        switchDataObj.switchData.insert({strSwitchData, boolSwitchData});
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
        Parcel parcel;

        string strSwitchData = "strSwitchData";
        bool boolSwitchData = false;
        switchDataObj.switchData.insert({strSwitchData, boolSwitchData});
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
        downloadProgressObj.path = "continue";
        downloadProgressObj.state = downloadProgressObj.RUNNING;
        downloadProgressObj.downloadedSize = 1;
        downloadProgressObj.totalSize = 1;
        auto res = downloadProgressObj.Marshalling(parcel);
        EXPECT_TRUE(!res);

        downloadProgressObj.path = "continue";
        downloadProgressObj.state = downloadProgressObj.RUNNING;
        downloadProgressObj.downloadedSize = 1;
        downloadProgressObj.totalSize = 0;
        res = downloadProgressObj.Marshalling(parcel);
        EXPECT_TRUE(!res);

        downloadProgressObj.path = "continue";
        downloadProgressObj.state = downloadProgressObj.RUNNING;
        downloadProgressObj.downloadedSize = 0;
        downloadProgressObj.totalSize = 0;
        res = downloadProgressObj.Marshalling(parcel);
        EXPECT_TRUE(!res);

        downloadProgressObj.path = "continue";
        downloadProgressObj.state = downloadProgressObj.STOPPED;
        downloadProgressObj.downloadedSize = 0;
        downloadProgressObj.totalSize = 0;
        res = downloadProgressObj.Marshalling(parcel);
        EXPECT_TRUE(!res);

        downloadProgressObj.path = "path";
        downloadProgressObj.state = downloadProgressObj.STOPPED;
        downloadProgressObj.downloadedSize = 0;
        downloadProgressObj.totalSize = 0;
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
        Parcel parcel;

        auto res = cleanOptions.Marshalling(parcel);
        EXPECT_TRUE(!res);

        string strActionsData = "continue";
        int32_t intActionsData = 1;
        cleanOptions.appActionsData.insert({strActionsData, intActionsData});
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
        Parcel parcel;
        string strActionsData = "strActionsData";
        int32_t intActionsData = 0;
        cleanOptions.appActionsData.insert({strActionsData, intActionsData});
        auto res = cleanOptions.Marshalling(parcel);
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
        Parcel parcel;

        auto res = cleanOptions.Marshalling(parcel);
        EXPECT_TRUE(!res);

        string strActionsData = "continue";
        int32_t intActionsData = 0;
        cleanOptions.appActionsData.insert({strActionsData, intActionsData});
        res = cleanOptions.Marshalling(parcel);
        EXPECT_TRUE(!res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Marshalling006 FAILED";
    }
    GTEST_LOG_(INFO) << "Marshalling006 End";
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
        auto res = cleanOptions->Unmarshalling(parcel);
        EXPECT_TRUE(res == nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Unmarshalling003 FAILED";
    }
    GTEST_LOG_(INFO) << "Unmarshalling003 End";
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
        parcel.dataSize_ = 0;
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
        parcel.dataSize_ = 1025;
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
        parcel.dataSize_ = 3;
        auto res = switchDataObj.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        parcel.data_ = new uint8_t();
        parcel.writable_ = 0;
        res = switchDataObj.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        parcel.writable_ = 1;
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
        parcel.dataSize_ = 0;
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
        parcel.dataSize_ = 1025;
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
        parcel.dataSize_ = 3;
        auto res = cleanOptions.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        parcel.data_ = new uint8_t();
        parcel.readCursor_ = 0;
        res = cleanOptions.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        parcel.readCursor_ = 1;
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
        auto res = downloadProgressObj.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        parcel.data_ = new uint8_t();
        parcel.readCursor_ = 0;
        res = downloadProgressObj.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        parcel.readCursor_ = 1;
        res = downloadProgressObj.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        downloadProgressObj.downloadedSize = 0;
        res = downloadProgressObj.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        downloadProgressObj.downloadedSize = 1;
        downloadProgressObj.totalSize = 0;
        res = downloadProgressObj.ReadFromParcel(parcel);
        EXPECT_TRUE(!res);

        downloadProgressObj.totalSize = 1;
        res = downloadProgressObj.ReadFromParcel(parcel);
        EXPECT_TRUE(res);
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
        std::string expectStr = "DownloadProgressObj [path:  state: 0 downloaded: 0 total: 0 downloadErrorType: 0]";
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
        auto res = assetInfo.ReadFromParcel(parcel);
        EXPECT_TRUE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ReadFromParcel00 FAILED";
    }
    GTEST_LOG_(INFO) << "ReadFromParcel00 End";
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
        auto res = assetInfo->Unmarshalling(parcel);
        EXPECT_TRUE(res != nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Unmarshalling FAILED";
    }
    GTEST_LOG_(INFO) << "Unmarshalling End";
}
} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS
