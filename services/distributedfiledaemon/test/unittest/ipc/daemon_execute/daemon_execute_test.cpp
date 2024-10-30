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

#include <filesystem>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "all_connect_manager_mock.h"
#include "daemon_execute.h"
#include "dfs_error.h"
#include "sandbox_helper.h"
#include "softbus_handler_asset_mock.h"
#include "softbus_handler_mock.h"

namespace {
std::string g_physicalPath;
int32_t g_getPhysicalPath = 0;
bool g_checkValidPath = false;
bool g_isFolder;
} // namespace

namespace OHOS::AppFileService {
int32_t SandboxHelper::GetPhysicalPath(const std::string &fileUri, const std::string &userId, std::string &physicalPath)
{
    physicalPath = g_physicalPath;
    return g_getPhysicalPath;
}

bool SandboxHelper::CheckValidPath(const std::string &filePath)
{
    return g_checkValidPath;
}
} // namespace OHOS::AppFileService

namespace OHOS::Storage::DistributedFile::Utils {
bool IsFolder(const std::string &name)
{
    return g_isFolder;
}
} // namespace OHOS::Storage::DistributedFile::Utils

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

    std::string path = "/mnt/hmdfs/100/account/device_view/local/data/com.example.app";
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directory(path);
        std::filesystem::create_directory(path + "/docs");
    }
    std::ofstream file1(path + "/1.txt");
    std::ofstream file2(path + "/2.txt");
}

void DaemonExecuteTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    ISoftBusHandlerAssetMock::iSoftBusHandlerAssetMock_ = nullptr;
    softBusHandlerAssetMock_ = nullptr;
    IAllConnectManagerMock::iAllConnectManagerMock_ = nullptr;
    allConnectManagerMock_ = nullptr;
    ISoftBusHandlerMock::iSoftBusHandlerMock_ = nullptr;
    softBusHandlerMock_ = nullptr;

    std::string path = "/mnt/hmdfs/100/account/device_view/local/data/com.example.app";
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
    assetObj->uris_.push_back("file://com.example.app/data/storage/el2/distributedfiles/docs/1.txt");
    assetObj->srcBundleName_ = "com.example.app";
    g_getPhysicalPath = E_OK;
    g_checkValidPath = true;
    g_isFolder = false;
    g_physicalPath = "/mnt/hmdfs/100/account/device_view/local/data/com.example.app/docs/1.txt";
    pushData = std::make_shared<PushAssetData>(userId, assetObj);
    event = AppExecFwk::InnerEvent::Get(DEAMON_EXECUTE_PUSH_ASSET, pushData, 0);
    EXPECT_CALL(*softBusHandlerAssetMock_, AssetBind(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*softBusHandlerAssetMock_, AssetSendFile(_, _, _)).WillOnce(Return(-1));
    daemonExecute_->ExecutePushAsset(event);

    EXPECT_CALL(*softBusHandlerAssetMock_, AssetBind(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*softBusHandlerAssetMock_, AssetSendFile(_, _, _)).WillOnce(Return(E_OK));
    daemonExecute_->ExecutePushAsset(event);

    assetObj->uris_.push_back("file://com.example.app/data/storage/el2/distributedfiles/docs/2.txt");
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
    EXPECT_EQ(daemonExecute_->RequestSendFileInner(srcUri, dstPath, dstDeviceId, sessionName), ERR_APPLY_RESULT);

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
    srcBundleName = "com.example.app";
    auto ret = daemonExecute_->GetFileList(uris, userId, srcBundleName);
    EXPECT_TRUE(ret.empty());

    uris.clear();
    uris.push_back("file://com.example.app/data/storage/el2/distributedfiles/docs/1.txt");
    g_getPhysicalPath = -1;
    ret = daemonExecute_->GetFileList(uris, userId, srcBundleName);
    EXPECT_TRUE(ret.empty());

    ret = daemonExecute_->GetFileList(uris, userId, srcBundleName);
    g_getPhysicalPath = E_OK;
    g_checkValidPath = false;
    EXPECT_TRUE(ret.empty());

    g_checkValidPath = true;
    g_isFolder = true;
    ret = daemonExecute_->GetFileList(uris, userId, srcBundleName);
    EXPECT_TRUE(ret.empty());

    g_isFolder = false;
    g_physicalPath = "test";
    ret = daemonExecute_->GetFileList(uris, userId, srcBundleName);
    EXPECT_TRUE(!ret.empty());

    g_physicalPath = "/mnt/hmdfs/100/account/device_view/local/data/com.example.app/docs/1.txt";
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

    fileList.emplace_back("/mnt/hmdfs/100/account/device_view/local/data/com.example.app/docs/1.txt");
    EXPECT_EQ(daemonExecute_->HandleZip(fileList, assetObj, sendFileName, isSingleFile), E_OK);

    fileList.emplace_back("/mnt/hmdfs/100/account/device_view/local/data/com.example.app/docs/2.txt");
    assetObj->srcBundleName_ = "com.example.appsa";
    EXPECT_EQ(daemonExecute_->HandleZip(fileList, assetObj, sendFileName, isSingleFile), E_ZIP);

    assetObj->srcBundleName_ = "com.example.app";
    EXPECT_CALL(*softBusHandlerAssetMock_, CompressFile(_, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(daemonExecute_->HandleZip(fileList, assetObj, sendFileName, isSingleFile), E_ZIP);

    EXPECT_CALL(*softBusHandlerAssetMock_, CompressFile(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(daemonExecute_->HandleZip(fileList, assetObj, sendFileName, isSingleFile), E_OK);
    GTEST_LOG_(INFO) << "DaemonExecute_HandleZip_001 end";
}
} // namespace OHOS::Storage::DistributedFile::Test