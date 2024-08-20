/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "network/softbus/softbus_asset_recv_listener.h"

#include <memory>
#include <unistd.h>

#include "gtest/gtest.h"

#include "asset_obj.h"
#include "dfs_error.h"
#include "network/softbus/softbus_handler_asset.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing::ext;
using namespace std;
using namespace OHOS::FileManagement;
const string testPath = "/data/test";

class SoftbusAssetRecvListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void SoftbusAssetRecvListenerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void SoftbusAssetRecvListenerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void SoftbusAssetRecvListenerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void SoftbusAssetRecvListenerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: SoftbusAssetRecvListenerTest_OnFile_0100
 * @tc.desc: test OnFile function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusAssetRecvListenerTest, SoftbusAssetRecvListenerTest_OnFile_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAssetRecvListenerTest_OnFile_0100 start";
    try {
        FileEvent event = {.type = FILE_EVENT_RECV_START,
                           .fileCnt = 0,
                           .bytesProcessed = 0,
                           .bytesTotal = 0};
        int32_t socket = 1;
        SoftbusAssetRecvListener::OnFile(socket, nullptr);
        SoftbusAssetRecvListener::OnFile(socket, &event);

        event.fileCnt = 1;
        SoftbusAssetRecvListener::OnFile(socket, &event);
        const char *filePath = "test";
        event.files = &filePath;
        SoftBusHandlerAsset::GetInstance().clientInfoMap_.insert(std::make_pair(socket, "test"));
        SoftbusAssetRecvListener::OnFile(socket, &event);
        SoftBusHandlerAsset::GetInstance().clientInfoMap_.erase(socket);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "SoftbusAssetRecvListenerTest_OnFile_0100 end";
}

/**
 * @tc.name: SoftbusAssetRecvListenerTest_OnFile_0200
 * @tc.desc: test OnFile function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusAssetRecvListenerTest, SoftbusAssetRecvListenerTest_OnFile_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAssetRecvListenerTest_OnFile_0200 start";
    try {
        FileEvent event = {.type = FILE_EVENT_RECV_FINISH,
                           .fileCnt = 0,
                           .bytesProcessed = 0,
                           .bytesTotal = 0};
        int32_t socket = 1;
        SoftbusAssetRecvListener::OnFile(socket, &event);

        event.fileCnt = 1;
        SoftbusAssetRecvListener::OnFile(socket, &event);
        SoftBusHandlerAsset::GetInstance().clientInfoMap_.insert(std::make_pair(socket, "test"));
        const char *filePath = "test";
        event.files = &filePath;
        SoftbusAssetRecvListener::OnFile(socket, &event);
        SoftBusHandlerAsset::GetInstance().clientInfoMap_.erase(socket);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "SoftbusAssetRecvListenerTest_OnFile_0200 end";
}

/**
 * @tc.name: SoftbusAssetRecvListenerTest_OnFile_0300
 * @tc.desc: test OnFile function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusAssetRecvListenerTest, SoftbusAssetRecvListenerTest_OnFile_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAssetRecvListenerTest_OnFile_0300 start";
    try {
        FileEvent event = {.type = FILE_EVENT_RECV_ERROR,
                           .fileCnt = 0,
                           .bytesProcessed = 0,
                           .bytesTotal = 0};
        int32_t socket = 1;
        SoftbusAssetRecvListener::OnFile(socket, &event);
        SoftBusHandlerAsset::GetInstance().clientInfoMap_.insert(std::make_pair(socket, "test"));
        SoftbusAssetRecvListener::OnFile(socket, &event);
        EXPECT_EQ(SoftBusHandlerAsset::GetInstance().GetClientInfo(socket), "");
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "SoftbusAssetRecvListenerTest_OnFile_0300 end";
}

/**
 * @tc.name: SoftbusAssetRecvListenerTest_OnFile_0400
 * @tc.desc: test OnFile function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusAssetRecvListenerTest, SoftbusAssetRecvListenerTest_OnFile_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAssetRecvListenerTest_OnFile_0400 start";
    try {
        FileEvent event = {.type = FILE_EVENT_RECV_UPDATE_PATH,
                           .fileCnt = 0,
                           .bytesProcessed = 0,
                           .bytesTotal = 0};
        int32_t socket = 1;
        SoftbusAssetRecvListener::OnFile(socket, &event);
        EXPECT_EQ(string((event.UpdateRecvPath)()), "/mnt/hmdfs/100/account/device_view/local/data/");
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "SoftbusAssetRecvListenerTest_OnFile_0400 end";
}

/**
 * @tc.name: SoftbusAssetRecvListenerTest_MoveAsset_0100
 * @tc.desc: test MoveAsset function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusAssetRecvListenerTest, SoftbusAssetRecvListenerTest_MoveAsset_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAssetRecvListenerTest_MoveAsset_0100 start";
    std::vector<std::string> fileList;
    fileList.push_back("test");
    EXPECT_EQ(SoftbusAssetRecvListener::MoveAsset(fileList, true), false);

    fileList[0].clear();
    fileList[0] = "ASSET_TEMP/";
    EXPECT_EQ(SoftbusAssetRecvListener::MoveAsset(fileList, true), false);

    fileList[0] += ".asset_single?";
    EXPECT_EQ(SoftbusAssetRecvListener::MoveAsset(fileList, true), false);

    fileList[0].clear();
    fileList[0] = "/data/test/ASSET_TEMP/test1.txt.asset_single?";
    EXPECT_EQ(SoftbusAssetRecvListener::MoveAsset(fileList, true), false);

    fileList.clear();
    fileList.push_back("test");
    EXPECT_EQ(SoftbusAssetRecvListener::MoveAsset(fileList, false), false);
    fileList[0].clear();
    fileList[0] = "ASSET_TEMP/";
    EXPECT_EQ(SoftbusAssetRecvListener::MoveAsset(fileList, false), false);

    fileList[0].clear();
    fileList[0] = "ASSET_TEMP/";
    GTEST_LOG_(INFO) << "SoftbusAssetRecvListenerTest_MoveAsset_0100 end";
}

/**
 * @tc.name: SoftbusAssetRecvListenerTest_RemoveAsset_0100
 * @tc.desc: test RemoveAsset function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusAssetRecvListenerTest, SoftbusAssetRecvListenerTest_RemoveAsset_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAssetRecvListenerTest_RemoveAsset_0100 start";
    string fileName = "test";
    EXPECT_EQ(SoftbusAssetRecvListener::RemoveAsset(fileName), false);

    fileName.clear();
    fileName = "/data/test/ASSET_TEMP/";
    EXPECT_EQ(SoftbusAssetRecvListener::RemoveAsset(fileName), false);
    
    fileName.clear();
    fileName = "/data/test/ASSET_TEMP/";
    SoftBusHandlerAsset::GetInstance().MkDirRecurse(fileName, S_IRWXU | S_IRWXG | S_IXOTH);
    EXPECT_EQ(SoftbusAssetRecvListener::RemoveAsset(fileName), true);
    GTEST_LOG_(INFO) << "SoftbusAssetRecvListenerTest_RemoveAsset_0100 end";
}

/**
 * @tc.name: SoftbusAssetRecvListenerTest_JudgeSingleFile_0100
 * @tc.desc: test JudgeSingleFile function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusAssetRecvListenerTest, SoftbusAssetRecvListenerTest_JudgeSingleFile_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAssetRecvListenerTest_JudgeSingleFile_0100 start";
    string fileName = "/data/test/ASSET_TEMP/test1.txt.asset_single?";
    EXPECT_EQ(SoftbusAssetRecvListener::JudgeSingleFile(fileName), true);

    fileName = "test";
    EXPECT_EQ(SoftbusAssetRecvListener::JudgeSingleFile(fileName), false);
    GTEST_LOG_(INFO) << "SoftbusAssetRecvListenerTest_JudgeSingleFile_0100 end";
}

/**
 * @tc.name: SoftbusAssetRecvListenerTest_HandleSingleFile_0100
 * @tc.desc: test HandleSingleFile function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusAssetRecvListenerTest, SoftbusAssetRecvListenerTest_HandleSingleFile_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAssetRecvListenerTest_HandleSingleFile_0100 start";
    string fileName = "demoA/ASSET_TEMP/test.asset_single?";
    sptr<AssetObj> assetObj = new (std::nothrow) AssetObj();
    assetObj->dstBundleName_ = "demoB";
    EXPECT_EQ(SoftbusAssetRecvListener::HandleSingleFile(2, fileName, assetObj), ERR_BAD_VALUE);

    fileName = "demoB/ASSET_TEMP/test.asset_single?";
    EXPECT_EQ(SoftbusAssetRecvListener::HandleSingleFile(2, fileName, assetObj), ERR_BAD_VALUE);
    GTEST_LOG_(INFO) << "SoftbusAssetRecvListenerTest_HandleSingleFile_0100 end";
}

/**
 * @tc.name: SoftbusAssetRecvListenerTest_HandleZipFile_0100
 * @tc.desc: test HandleZipFile function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusAssetRecvListenerTest, SoftbusAssetRecvListenerTest_HandleZipFile_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAssetRecvListenerTest_HandleZipFile_0100 start";
    string fileName = "test";
    sptr<AssetObj> assetObj = new (std::nothrow) AssetObj();
    assetObj->dstBundleName_ = "demoB";
    EXPECT_EQ(SoftbusAssetRecvListener::HandleZipFile(2, fileName, assetObj), ERR_BAD_VALUE);

    fileName = ".asset_zip?";
    EXPECT_EQ(SoftbusAssetRecvListener::HandleZipFile(2, fileName, assetObj), ERR_BAD_VALUE);
    GTEST_LOG_(INFO) << "SoftbusAssetRecvListenerTest_HandleZipFile_0100 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
