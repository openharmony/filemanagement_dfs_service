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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

#include "dfs_error.h"
#include "daemon_execute.h"
#include "softbus_handler_asset_mock.h"
#include "softbus_handler_mock.h"
#include "all_connect_manager_mock.h"

namespace OHOS::Storage::DistributedFile::Test {
using namespace OHOS::FileManagement;
using namespace testing;
using namespace testing::ext;
using namespace std;
constexpr int32_t BLOCK_INTERVAL_SEND_FILE = 8 * 1000;
class DaemonExecuteTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DaemonExecute> daemonExecute_;

public:
    static inline std::shared_ptr<SoftBusHandlerAssetMock> softBusHandlerAssetMock_ = nullptr;
    static inline std::shared_ptr<SoftBusHandlerMock> softBusHandlerMock_ = nullptr;
    static inline std::shared_ptr<AllConnectManagerMock> allConnectManagerMock_ = nullptr;
};

void DaemonExecuteTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    softBusHandlerAssetMock_ = std::make_shared<SoftBusHandlerAssetMock>();
    ISoftBusHandlerAssetMock::iSoftBusHandlerAssetMock_ = softBusHandlerAssetMock_;
    allConnectManagerMock_ = std::make_shared<AllConnectManagerMock>();
    IAllConnectManagerMock::iAllConnectManagerMock_ = allConnectManagerMock_;
    softBusHandlerMock_ = std::make_shared<SoftBusHandlerMock>();
    ISoftBusHandlerMock::iSoftBusHandlerMock_ = softBusHandlerMock_;

    std::string path =  "/mnt/hmdfs/100/account/device_view/local/data/com.huawei.hmos.example/docs";
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directory(path);
    }
    std::ofstream file1(path + "/1.txt");
    std::ofstream file2(path + "/2.txt");
}

void DaemonExecuteTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    ISoftBusHandlerAssetMock::iSoftBusHandlerAssetMock_ = nullptr;
    softBusHandlerAssetMock_ = nullptr;

    std::string path =  "/mnt/hmdfs/100/account/device_view/local/data/com.huawei.hmos.example";
    if (std::filesystem::exists(path)) {
        std::filesystem::remove_all(path);
    }
}

void DaemonExecuteTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    daemonExecute_ = std::make_shared<DaemonExecute>();
}

void DaemonExecuteTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    daemonExecute_ = nullptr;
}

// /**
//  * @tc.name: DaemonExecute_ProcessEvent_001
//  * @tc.desc: verify ProcessEvent.
//  * @tc.type: FUNC
//  * @tc.require: I7TDJK
//  */
// HWTEST_F(DaemonExecuteTest, DaemonExecute_ProcessEvent_001, TestSize.Level1)
// {
//     auto eventptr = AppExecFwk::InnerEvent::Get(DEAMON_EXECUTE_PUSH_ASSET, nullptr, 0);
//     eventptr.reset(nullptr);
//     daemonExecute_->ProcessEvent(eventptr);

//     auto event = AppExecFwk::InnerEvent::Get(999, nullptr, 0);
//     daemonExecute_->ProcessEvent(event);

//     event = AppExecFwk::InnerEvent::Get(DEAMON_EXECUTE_PUSH_ASSET, nullptr, 0);
//     daemonExecute_->ProcessEvent(event);
// }


/**
 * @tc.name: DaemonExecute_ExecutePushAsset_001
 * @tc.desc: verify ExecutePushAsset.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonExecuteTest, DaemonExecute_ExecutePushAsset_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonExecute_ExecutePushAsset_001 begin";
    std::shared_ptr<PushAssetData> pushData1 = nullptr;
    auto eventptr = AppExecFwk::InnerEvent::Get(DEAMON_EXECUTE_PUSH_ASSET, pushData1, 0);
    eventptr.reset(nullptr);
    daemonExecute_->ExecutePushAsset(eventptr);

    std::shared_ptr<PushAssetData> pushData = nullptr;
    auto event = AppExecFwk::InnerEvent::Get(DEAMON_EXECUTE_PUSH_ASSET, pushData, 0);
    daemonExecute_->ExecutePushAsset(event);

    int32_t userId = 100;
    sptr<AssetObj> assetObj = nullptr;
    pushData = std::make_shared<PushAssetData>(userId, assetObj);
    event = AppExecFwk::InnerEvent::Get(DEAMON_EXECUTE_PUSH_ASSET, pushData, 0);
    daemonExecute_->ExecutePushAsset(event);

    assetObj = new (std::nothrow) AssetObj();
    pushData = std::make_shared<PushAssetData>(userId, assetObj);
    event = AppExecFwk::InnerEvent::Get(DEAMON_EXECUTE_PUSH_ASSET, pushData, 0);
    EXPECT_CALL(*softBusHandlerAssetMock_, AssetBind(_, _)).WillOnce(Return(-1));
    daemonExecute_->ExecutePushAsset(event);

    assetObj->uris_.push_back("test");
    assetObj->srcBundleName_ = "srcBundleName_";
    pushData = std::make_shared<PushAssetData>(userId, assetObj);
    event = AppExecFwk::InnerEvent::Get(DEAMON_EXECUTE_PUSH_ASSET, pushData, 0);
    EXPECT_CALL(*softBusHandlerAssetMock_, AssetBind(_, _)).WillOnce(Return(E_OK));
    daemonExecute_->ExecutePushAsset(event);

    assetObj->uris_.clear();
    assetObj->uris_.push_back("file://com.huawei.hmos.example/data/storage/el2/distributedfiles/docs/1.txt");
    assetObj->srcBundleName_ = "com.huawei.hmos.example";
    pushData = std::make_shared<PushAssetData>(userId, assetObj);
    event = AppExecFwk::InnerEvent::Get(DEAMON_EXECUTE_PUSH_ASSET, pushData, 0);
    EXPECT_CALL(*softBusHandlerAssetMock_, AssetBind(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*softBusHandlerAssetMock_, AssetSendFile(_, _, _)).WillOnce(Return(-1));
    daemonExecute_->ExecutePushAsset(event);

    EXPECT_CALL(*softBusHandlerAssetMock_, AssetBind(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*softBusHandlerAssetMock_, AssetSendFile(_, _, _)).WillOnce(Return(E_OK));
    daemonExecute_->ExecutePushAsset(event);

    assetObj->uris_.push_back("file://com.huawei.hmos.example/data/storage/el2/distributedfiles/docs/2.txt");
    pushData = std::make_shared<PushAssetData>(userId, assetObj);
    event = AppExecFwk::InnerEvent::Get(DEAMON_EXECUTE_PUSH_ASSET, pushData, 0);
    EXPECT_CALL(*softBusHandlerAssetMock_, AssetBind(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*softBusHandlerAssetMock_, CompressFile(_, _, _)).WillOnce(Return(-1));
    daemonExecute_->ExecutePushAsset(event);
    GTEST_LOG_(INFO) << "DaemonExecute_ExecutePushAsset_001 end";
}

/**
 * @tc.name: DaemonExecute_ExecuteRequestSendFile_001
 * @tc.desc: verify ExecuteRequestSendFile.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonExecuteTest, DaemonExecute_ExecuteRequestSendFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonExecute_ExecuteRequestSendFile_001 begin";
    std::shared_ptr<RequestSendFileData> requestSendFileData1 = nullptr;
    auto eventptr = AppExecFwk::InnerEvent::Get(DEAMON_EXECUTE_REQUEST_SEND_FILE, requestSendFileData1, 0);
    eventptr.reset(nullptr);
    daemonExecute_->ExecuteRequestSendFile(eventptr);

    std::shared_ptr<RequestSendFileData> requestSendFileData = nullptr;
    auto event = AppExecFwk::InnerEvent::Get(DEAMON_EXECUTE_REQUEST_SEND_FILE, requestSendFileData, 0);
    daemonExecute_->ExecuteRequestSendFile(event);

    requestSendFileData = std::make_shared<RequestSendFileData>("", "", "", "", nullptr);
    event = AppExecFwk::InnerEvent::Get(DEAMON_EXECUTE_REQUEST_SEND_FILE, requestSendFileData, 0);
    daemonExecute_->ExecuteRequestSendFile(event);

    auto requestSendFileBlock = std::make_shared<BlockObject<int32_t>>(BLOCK_INTERVAL_SEND_FILE, ERR_BAD_VALUE);
    requestSendFileData = std::make_shared<RequestSendFileData>("", "", "", "", requestSendFileBlock);
    event = AppExecFwk::InnerEvent::Get(DEAMON_EXECUTE_REQUEST_SEND_FILE, requestSendFileData, 0);
    daemonExecute_->ExecuteRequestSendFile(event);
    EXPECT_EQ(requestSendFileBlock->GetValue(), ERR_BAD_VALUE);

    requestSendFileData = std::make_shared<RequestSendFileData>("test", "", "", "", requestSendFileBlock);
    event = AppExecFwk::InnerEvent::Get(DEAMON_EXECUTE_REQUEST_SEND_FILE, requestSendFileData, 0);
    daemonExecute_->ExecuteRequestSendFile(event);
    EXPECT_EQ(requestSendFileBlock->GetValue(), ERR_BAD_VALUE);

    requestSendFileData = std::make_shared<RequestSendFileData>("test", "", "test", "", requestSendFileBlock);
    event = AppExecFwk::InnerEvent::Get(DEAMON_EXECUTE_REQUEST_SEND_FILE, requestSendFileData, 0);
    daemonExecute_->ExecuteRequestSendFile(event);
    EXPECT_EQ(requestSendFileBlock->GetValue(), ERR_BAD_VALUE);

    requestSendFileData = std::make_shared<RequestSendFileData>("test", "", "test", "test", requestSendFileBlock);
    event = AppExecFwk::InnerEvent::Get(DEAMON_EXECUTE_REQUEST_SEND_FILE, requestSendFileData, 0);
    EXPECT_CALL(*allConnectManagerMock_, ApplyAdvancedResource(_, _)).WillOnce(Return(-1));
    daemonExecute_->ExecuteRequestSendFile(event);
    EXPECT_EQ(requestSendFileBlock->GetValue(), ERR_APPLY_RESULT);
    GTEST_LOG_(INFO) << "DaemonExecute_ExecuteRequestSendFile_001 end";
}

/**
 * @tc.name: DaemonExecute_RequestSendFileInner_001
 * @tc.desc: verify RequestSendFileInner.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonExecuteTest, DaemonExecute_RequestSendFileInner_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonExecute_RequestSendFileInner_001 begin";
    std::string srcUri;
    std::string dstPath;
    std::string dstDeviceId;
    std::string sessionName;
    EXPECT_CALL(*allConnectManagerMock_, ApplyAdvancedResource(_, _)).WillOnce(Return(-1));
    EXPECT_EQ(daemonExecute_->RequestSendFileInner(srcUri, dstPath, dstDeviceId, sessionName), -1);

    EXPECT_CALL(*allConnectManagerMock_, ApplyAdvancedResource(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*softBusHandlerMock_, OpenSession(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(daemonExecute_->RequestSendFileInner(srcUri, dstPath, dstDeviceId, sessionName), -1);

    EXPECT_CALL(*allConnectManagerMock_, ApplyAdvancedResource(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*softBusHandlerMock_, OpenSession(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*softBusHandlerMock_, CopySendFile(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(daemonExecute_->RequestSendFileInner(srcUri, dstPath, dstDeviceId, sessionName), -1);

    EXPECT_CALL(*allConnectManagerMock_, ApplyAdvancedResource(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*softBusHandlerMock_, OpenSession(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*softBusHandlerMock_, CopySendFile(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(daemonExecute_->RequestSendFileInner(srcUri, dstPath, dstDeviceId, sessionName), E_OK);
    GTEST_LOG_(INFO) << "DaemonExecute_RequestSendFileInner_001 end";
}

/**
 * @tc.name: DaemonExecute_GetFileList_001
 * @tc.desc: verify GetFileList.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonExecuteTest, DaemonExecute_GetFileList_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonExecute_GetFileList_001 begin";
    std::vector<std::string> uris;
    int32_t userId = 100;
    std::string srcBundleName;

    uris.push_back("test");
    srcBundleName = "srcBundleName";
    auto ret = daemonExecute_->GetFileList(uris, userId, srcBundleName);
    EXPECT_TRUE(ret.empty());

    uris.clear();
    uris.push_back("file://com.huawei.hmos.example/data/storage/el2/distributeles/docs/1.txt");
    ret = daemonExecute_->GetFileList(uris, userId, srcBundleName);
    EXPECT_TRUE(ret.empty());

    uris.clear();
    uris.push_back("file://com.huawei.hmos.example/data/storage/el2/distributedfiles/docs/a/1.txt");
    ret = daemonExecute_->GetFileList(uris, userId, srcBundleName);
    EXPECT_TRUE(ret.empty());

    uris.clear();
    uris.push_back("file://com.huawei.hmos.example/data/storage/el2/distributedfiles/docs");
    ret = daemonExecute_->GetFileList(uris, userId, srcBundleName);
    EXPECT_TRUE(ret.empty());

    uris.clear();
    uris.push_back("file://com.huawei.hmos.example/data/storage/el2/distributedfiles/docs");
    ret = daemonExecute_->GetFileList(uris, userId, srcBundleName);
    EXPECT_TRUE(ret.empty());

    uris.clear();
    uris.push_back("file://com.huawei.hmos.example/data/storage/el2/distributedfiles/docs/1.txt");
    ret = daemonExecute_->GetFileList(uris, userId, srcBundleName);
    EXPECT_TRUE(!ret.empty());
    GTEST_LOG_(INFO) << "DaemonExecute_GetFileList_001 end";
}

/**
 * @tc.name: DaemonExecute_HandleZip_001
 * @tc.desc: verify HandleZip.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DaemonExecuteTest, DaemonExecute_HandleZip_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonExecute_HandleZip_001 begin";
    std::vector<std::string> fileList;
    sptr<AssetObj> assetObj(new (std::nothrow) AssetObj());
    std::string sendFileName;
    bool isSingleFile;

    fileList.emplace_back("/mnt/hmdfs/100/account/device_view/local/data/com.huawei.hmos.example/docs/1.txt");
    EXPECT_EQ(daemonExecute_->HandleZip(fileList, assetObj, sendFileName, isSingleFile), E_OK);

    fileList.emplace_back("/mnt/hmdfs/100/account/device_view/local/data/com.huawei.hmos.example/docs/2.txt");
    assetObj->srcBundleName_ = "com.huawei.hmos.examplesa";
    EXPECT_EQ(daemonExecute_->HandleZip(fileList, assetObj, sendFileName, isSingleFile), E_ZIP);

    assetObj->srcBundleName_ = "com.huawei.hmos.example";
    EXPECT_CALL(*softBusHandlerAssetMock_, CompressFile(_, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(daemonExecute_->HandleZip(fileList, assetObj, sendFileName, isSingleFile), E_ZIP);

    EXPECT_CALL(*softBusHandlerAssetMock_, CompressFile(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(daemonExecute_->HandleZip(fileList, assetObj, sendFileName, isSingleFile), E_OK);
    GTEST_LOG_(INFO) << "DaemonExecute_HandleZip_001 end";
}
}