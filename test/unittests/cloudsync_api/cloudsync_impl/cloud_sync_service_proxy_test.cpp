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

#include "cloud_sync_service_proxy.h"
#include "dfs_error.h"
#include "i_cloud_download_callback_mock.h"
#include "i_cloud_sync_service_mock.h"
#include "service_callback_mock.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing::ext;
using namespace testing;
using namespace std;

class CloudSyncServiceProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    shared_ptr<CloudSyncServiceProxy> proxy_ = nullptr;
    sptr<CloudSyncServiceMock> mock_ = nullptr;
    sptr<CloudSyncCallbackMock> remote_ = nullptr;
    sptr<CloudDownloadCallbackMock> download_ = nullptr;
};

void CloudSyncServiceProxyTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudSyncServiceProxyTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudSyncServiceProxyTest::SetUp(void)
{
    mock_ = sptr(new CloudSyncServiceMock());
    proxy_ = make_shared<CloudSyncServiceProxy>(mock_);
    remote_ = sptr(new CloudSyncCallbackMock());
    download_ = sptr(new CloudDownloadCallbackMock());
    std::cout << "SetUp" << std::endl;
}

void CloudSyncServiceProxyTest::TearDown(void)
{
    proxy_ = nullptr;
    mock_ = nullptr;
    remote_ = nullptr;
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: UnRegisterCallbackInner001
 * @tc.desc: Verify the UnRegisterCallbackInner function.
 * @tc.type: FUNC
 * @tc.require: issueI7UYAL
 */
HWTEST_F(CloudSyncServiceProxyTest, UnRegisterCallbackInner001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnRegisterCallbackInner Start";
    string bundleName = "com.ohos.photos";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(-1));
    int result = proxy_->UnRegisterCallbackInner(bundleName);
    EXPECT_EQ(result, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "UnRegisterCallbackInner End";
}

/**
 * @tc.name: UnRegisterCallbackInner002
 * @tc.desc: Verify the RRegisterCallbackInner function.
 * @tc.type: FUNC
 * @tc.require: issueI7UYAL
 */
HWTEST_F(CloudSyncServiceProxyTest, UnRegisterCallbackInner002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnRegisterCallbackInner Start";
    string bundleName = "com.ohos.photos";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));
    int result = proxy_->UnRegisterCallbackInner(bundleName);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "UnRegisterCallbackInner End";
}


/**
 * @tc.name: RegisterCallbackInner001
 * @tc.desc: Verify the RegisterCallbackInner function.
 * @tc.type: FUNC
 * @tc.require: issueI7UYAL
 */
HWTEST_F(CloudSyncServiceProxyTest, RegisterCallbackInner001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterCallbackInner Start";
    string bundleName = "com.ohos.photos";

    int result = proxy_->RegisterCallbackInner(nullptr, bundleName);
    EXPECT_EQ(result, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "RegisterCallbackInner End";
}

/**
 * @tc.name: RegisterCallbackInner002
 * @tc.desc: Verify the RegisterCallbackInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, RegisterCallbackInner002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterCallbackInner Start";
    string bundleName = "com.ohos.photos";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(-1));

    int result = proxy_->RegisterCallbackInner(remote_, bundleName);
    EXPECT_EQ(result, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "RegisterCallbackInner End";
}

/**
 * @tc.name: RegisterCallbackInner003
 * @tc.desc: Verify the RegisterCallbackInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, RegisterCallbackInner003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterCallbackInner Start";
    string bundleName = "com.ohos.photos";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));

    int result = proxy_->RegisterCallbackInner(remote_, bundleName);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "RegisterCallbackInner End";
}

/**
 * @tc.name: StartSyncInner001
 * @tc.desc: Verify the StartSyncInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, StartSyncInner001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSyncInner Start";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(-1));

    bool forceFlag = true;
    int result = proxy_->StartSyncInner(forceFlag);
    EXPECT_EQ(result, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "StartSyncInner End";
}

/**
 * @tc.name: StartSyncInner002
 * @tc.desc: Verify the StartSyncInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, StartSyncInner002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSyncInner Start";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(2)
        .WillOnce(Invoke(mock_.GetRefPtr(), &CloudSyncServiceMock::InvokeSendRequest))
        .WillOnce(Return(EPERM));

    bool forceFlag = true;
    int result = proxy_->StartSyncInner(forceFlag);
    EXPECT_EQ(result, E_OK);

    result = proxy_->StartSyncInner(forceFlag);
    EXPECT_NE(result, E_OK);
    GTEST_LOG_(INFO) << "StartSyncInner End";
}

/**
 * @tc.name: TriggerSyncInnerInner001
 * @tc.desc: Verify the TriggerSyncInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, TriggerSyncInner001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TriggerSyncInner Start";
    string bundleName = "com.ohos.photos";
    int32_t userId = 100;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(-1));

    int result = proxy_->TriggerSyncInner(bundleName, userId);
    EXPECT_EQ(result, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "TriggerSyncInner End";
}

/**
 * @tc.name: TriggerSyncInner002
 * @tc.desc: Verify the TriggerSyncInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, TriggerSyncInner002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TriggerSyncInner Start";
    string bundleName = "com.ohos.photos";
    int32_t userId = 100;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));

    int result = proxy_->TriggerSyncInner(bundleName, userId);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "TriggerSyncInner End";
}

/**
 * @tc.name: GetSyncTimeInner001
 * @tc.desc: Verify the GetSyncTimeInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, GetSyncTimeInner001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncTimeInner Start";
    string bundleName = "com.ohos.photos";
    int64_t syncTime = 0;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(-1));

    int result = proxy_->GetSyncTimeInner(syncTime, bundleName);
    EXPECT_EQ(result, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "GetSyncTimeInner End";
}

/**
 * @tc.name: GetSyncTimeInner002
 * @tc.desc: Verify the GetSyncTimeInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, GetSyncTimeInner002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncTimeInner Start";
    string bundleName = "com.ohos.photos";
    int64_t syncTime = 0;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));
    
    int result = proxy_->GetSyncTimeInner(syncTime, bundleName);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "GetSyncTimeInner End";
}

/**
 * @tc.name: BatchDentryFileInsert001
 * @tc.desc: Verify the BatchCleanFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, BatchDentryFileInsert001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatchDentryFileInsert001 Start";
    std::vector<DentryFileInfoObj> fileInfoObj;
    std::vector<std::string> failCloudId;
    int ret = proxy_->BatchDentryFileInsert(fileInfoObj, failCloudId);
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "BatchDentryFileInsert001 End";
}

/**
 * @tc.name: BatchDentryFileInsert002
 * @tc.desc: Verify the BatchCleanFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, BatchDentryFileInsert002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatchDentryFileInsert002 Start";
    DentryFileInfo obj;
    std::vector<DentryFileInfoObj> fileInfoObj;
    fileInfoObj.push_back(obj);
    std::vector<std::string> failCloudId;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(-1));
    int ret = proxy_->BatchDentryFileInsert(fileInfoObj, failCloudId);
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "BatchDentryFileInsert002 End";
}

/**
 * @tc.name: BatchDentryFileInsert003
 * @tc.desc: Verify the BatchCleanFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, BatchDentryFileInsert003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatchDentryFileInsert003 Start";
    DentryFileInfo obj;
    std::vector<DentryFileInfoObj> fileInfoObj;
    fileInfoObj.push_back(obj);
    std::vector<std::string> failCloudId;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));
    int ret = proxy_->BatchDentryFileInsert(fileInfoObj, failCloudId);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "BatchDentryFileInsert003 End";
}

/**
 * @tc.name: CleanCacheInner001
 * @tc.desc: Verify the CleanCacheInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, CleanCacheInner001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanCacheInner Start";
    string uri = "";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(-1));

    int result = proxy_->CleanCacheInner(uri);
    EXPECT_EQ(result, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "CleanCacheInner End";
}

/**
 * @tc.name: CleanCacheInner002
 * @tc.desc: Verify the CleanCacheInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, CleanCacheInner002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanCacheInner Start";
    string uri = "";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));
    
    int result = proxy_->CleanCacheInner(uri);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "CleanCacheInner End";
}

/**
 * @tc.name: StopSyncInner001
 * @tc.desc: Verify the StopSyncInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, StopSyncInner001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopSyncInner Start";
    string bundleName = "com.ohos.photos";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(-1));

    int result = proxy_->StopSyncInner(bundleName);
    EXPECT_EQ(result, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "StopSyncInner End";
}

/**
 * @tc.name: StopSyncInner002
 * @tc.desc: Verify the StopSyncInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, StopSyncInner002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopSyncInner Start";
    string bundleName = "com.ohos.photos";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));

    int result = proxy_->StopSyncInner(bundleName);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StopSyncInner End";
}

/**
 * @tc.name: ChangeAppSwitch001
 * @tc.desc: Verify the ChangeAppSwitch function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, ChangeAppSwitch001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeAppSwitch Start";
    string accoutId = "100";
    string bundleName = "com.ohos.photos";
    bool status = true;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(-1));

    int result = proxy_->ChangeAppSwitch(accoutId, bundleName, status);
    EXPECT_EQ(result, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "ChangeAppSwitch End";
}

/**
 * @tc.name: ChangeAppSwitch002
 * @tc.desc: Verify the ChangeAppSwitch function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, ChangeAppSwitch002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeAppSwitch Start";
    string accoutId = "100";
    string bundleName = "com.ohos.photos";
    bool status = true;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));

    int result = proxy_->ChangeAppSwitch(accoutId, bundleName, status);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "ChangeAppSwitch End";
}

/**
 * @tc.name: Clean001
 * @tc.desc: Verify the Clean function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, Clean001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Clean Start";
    string accoutId = "100";
    CleanOptions cleanOptions;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(-1));

    int result = proxy_->Clean(accoutId, cleanOptions);
    EXPECT_EQ(result, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "Clean End";
}

/**
 * @tc.name: Clean002
 * @tc.desc: Verify the Clean function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, Clean002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Clean Start";
    string accoutId = "100";
    CleanOptions cleanOptions;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));

    int result = proxy_->Clean(accoutId, cleanOptions);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "Clean End";
}

/**
 * @tc.name: EnableCloud001
 * @tc.desc: Verify the EnableCloud function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, EnableCloud001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnableCloud Start";
    string accoutId = "100";
    SwitchDataObj switchData;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(-1));

    int result = proxy_->EnableCloud(accoutId, switchData);
    EXPECT_EQ(result, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "EnableCloud End";
}

/**
 * @tc.name: EnableCloud002
 * @tc.desc: Verify the EnableCloud function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, EnableCloud002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnableCloud Start";
    string accoutId = "100";
    SwitchDataObj switchData;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));

    int result = proxy_->EnableCloud(accoutId, switchData);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "EnableCloud End";
}

/**
 * @tc.name: DisableCloud001
 * @tc.desc: Verify the DisableCloud function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, DisableCloud001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DisableCloud Start";
    string accoutId = "100";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(-1));

    int result = proxy_->DisableCloud(accoutId);
    EXPECT_EQ(result, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "DisableCloud End";
}

/**
 * @tc.name: DisableCloud002
 * @tc.desc: Verify the DisableCloud function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, DisableCloud002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DisableCloud Start";
    string accoutId = "100";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));

    int result = proxy_->DisableCloud(accoutId);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "DisableCloud End";
}

/**
 * @tc.name: NotifyDataChange001
 * @tc.desc: Verify the NotifyDataChange function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, NotifyDataChange001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyDataChange Start";
    string accoutId = "100";
    string bundleName = "com.ohos.photos";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(-1));

    int result = proxy_->NotifyDataChange(accoutId, bundleName);
    EXPECT_EQ(result, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "NotifyDataChange End";
}

/**
 * @tc.name: NotifyDataChange002
 * @tc.desc: Verify the NotifyDataChange function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, NotifyDataChange002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyDataChange Start";
    string accoutId = "100";
    string bundleName = "com.ohos.photos";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));

    int result = proxy_->NotifyDataChange(accoutId, bundleName);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "NotifyDataChange End";
}

/**
 * @tc.name: NotifyEventChange001
 * @tc.desc: Verify the NotifyEventChange function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, NotifyEventChange001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyEventChange Start";
    int32_t userId = 100;
    string eventId = "";
    string extraData = "";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(-1));

    int result = proxy_->NotifyEventChange(userId, eventId, extraData);
    EXPECT_EQ(result, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "NotifyEventChange End";
}

/**
 * @tc.name: NotifyEventChange002
 * @tc.desc: Verify the NotifyEventChange function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, NotifyEventChange002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyEventChange Start";
    int32_t userId = 100;
    string eventId = "";
    string extraData = "";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));

    int result = proxy_->NotifyEventChange(userId, eventId, extraData);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "NotifyEventChange End";
}


/**
 * @tc.name: StartDownloadFile001
 * @tc.desc: Verify the StartDownloadFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, StartDownloadFile001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartDownloadFile Start";
    string uri = "file://media";
    int result = proxy_->StartDownloadFile(uri);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StartDownloadFile End";
}

/**
 * @tc.name: StartDownloadFile002
 * @tc.desc: Verify the StartDownloadFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, StartDownloadFile002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartDownloadFile Start";
    string uri = "";
    int result = proxy_->StartDownloadFile(uri);
    EXPECT_EQ(result, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "StartDownloadFile End";
}

/**
 * @tc.name: StartFileCache001
 * @tc.desc: Verify the StartFileCache function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, StartFileCache001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartFileCache Start";
    string uri = "file://media";
    std::vector<std::string> uriVec;
    uriVec.push_back(uri);
    int64_t downloadId = 0;
    int result = proxy_->StartFileCache(uriVec, downloadId);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StartFileCache End";
}

/**
 * @tc.name: StartFileCache002
 * @tc.desc: Verify the StartFileCache function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, StartFileCache002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartFileCache Start";
    string uri = "";
    std::vector<std::string> uriVec;
    uriVec.push_back(uri);
    int64_t downloadId = 0;
    int result = proxy_->StartFileCache(uriVec, downloadId);
    EXPECT_EQ(result, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "StartFileCache End";
}

/**
 * @tc.name: StopDownloadFile001
 * @tc.desc: Verify the StopDownloadFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, StopDownloadFile001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopDownloadFile Start";
    string uri = "file://media";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(-1));

    int result = proxy_->StopDownloadFile(uri);
    EXPECT_EQ(result, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "StopDownloadFile End";
}

/**
 * @tc.name: StopDownloadFile002
 * @tc.desc: Verify the StopDownloadFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, StopDownloadFile002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopDownloadFile Start";
    string uri = "file://media";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));

    int result = proxy_->StopDownloadFile(uri);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StopDownloadFile End";
}

/**
 * @tc.name: StopDownloadFile003
 * @tc.desc: Verify the StopDownloadFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, StopDownloadFile003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopDownloadFile Start";
    string uri = "";

    int result = proxy_->StopDownloadFile(uri);
    EXPECT_EQ(result, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "StopDownloadFile End";
}

/**
 * @tc.name: UploadAsset001
 * @tc.desc: Verify the UploadAsset function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, UploadAsset001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UploadAsset001 Start";
    int32_t userId = 100;
    string request = "test_request";
    string result;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(-1));
    int ret = proxy_->UploadAsset(userId, request, result);
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "UploadAsset001 End";
}

/**
 * @tc.name: UploadAsset002
 * @tc.desc: Verify the UploadAsset function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, UploadAsset002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UploadAsset002 Start";
    int32_t userId = 100;
    string request = "test_request";
    string result;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));
    int ret = proxy_->UploadAsset(userId, request, result);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "UploadAsset002 End";
}

/**
 * @tc.name: DownloadFile001
 * @tc.desc: Verify the DownloadFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, DownloadFile001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DownloadFile001 Start";
    int32_t userId = 100;
    string bundleName = "com.ohos.photos";
    AssetInfoObj assetInfoObj;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(-1));
    int ret = proxy_->DownloadFile(userId, bundleName, assetInfoObj);
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "DownloadFile001 End";
}

/**
 * @tc.name: DownloadFile002
 * @tc.desc: Verify the DownloadFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, DownloadFile002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DownloadFile002 Start";
    int32_t userId = 100;
    string bundleName = "com.ohos.photos";
    AssetInfoObj assetInfoObj;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));
    int ret = proxy_->DownloadFile(userId, bundleName, assetInfoObj);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DownloadFile002 End";
}

/**
 * @tc.name: DownloadFiles001
 * @tc.desc: Verify the DownloadFiles function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, DownloadFiles001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DownloadFiles001 Start";
    int32_t userId = 100;
    string bundleName = "com.ohos.photos";
    std::vector<AssetInfoObj> assetInfoObj;
    std::vector<bool> assetResultMap;
    int ret = proxy_->DownloadFiles(userId, bundleName, assetInfoObj, assetResultMap);
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "DownloadFiles001 End";
}

/**
 * @tc.name: DownloadFiles002
 * @tc.desc: Verify the DownloadFiles function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, DownloadFiles002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DownloadFiles002 Start";
    int32_t userId = 100;
    string bundleName = "com.ohos.photos";
    AssetInfoObj obj;
    std::vector<AssetInfoObj> assetInfoObj;
    assetInfoObj.push_back(obj);
    std::vector<bool> assetResultMap;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(-1));
    int ret = proxy_->DownloadFiles(userId, bundleName, assetInfoObj, assetResultMap);
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "DownloadFiles002 End";
}

/**
 * @tc.name: DownloadFiles003
 * @tc.desc: Verify the DownloadFiles function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, DownloadFiles003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DownloadFiles002 Start";
    int32_t userId = 100;
    string bundleName = "com.ohos.photos";
    AssetInfoObj obj;
    std::vector<AssetInfoObj> assetInfoObj;
    assetInfoObj.push_back(obj);
    std::vector<bool> assetResultMap;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));
    int ret = proxy_->DownloadFiles(userId, bundleName, assetInfoObj, assetResultMap);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DownloadFiles002 End";
}

/**
 * @tc.name: DownloadAsset001
 * @tc.desc: Verify the DownloadAsset function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, DownloadAsset001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DownloadAsset001 Start";
    uint64_t taskId = 100;
    int32_t userId = 100;
    string bundleName = "com.ohos.photos";
    string networkId = "";
    AssetInfoObj assetInfoObj;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(-1));
    int ret = proxy_->DownloadAsset(taskId, userId, bundleName, networkId, assetInfoObj);
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "DownloadAsset001 End";
}

/**
 * @tc.name: DownloadAsset002
 * @tc.desc: Verify the DownloadAsset function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, DownloadAsset002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DownloadAsset002 Start";
    uint64_t taskId = 100;
    int32_t userId = 100;
    string bundleName = "com.ohos.photos";
    string networkId = "";
    AssetInfoObj assetInfoObj;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));
    int ret = proxy_->DownloadAsset(taskId, userId, bundleName, networkId, assetInfoObj);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DownloadAsset002 End";
}

/**
 * @tc.name: RegisterDownloadAssetCallback001
 * @tc.desc: Verify the RegisterDownloadAssetCallback function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, RegisterDownloadAssetCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterDownloadAssetCallback Start";

    int ret = proxy_->RegisterDownloadAssetCallback(nullptr);
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "RegisterDownloadAssetCallback End";
}

/**
 * @tc.name: RegisterDownloadAssetCallback002
 * @tc.desc: Verify the RegisterDownloadAssetCallback function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, RegisterDownloadAssetCallback002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterDownloadAssetCallback Start";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(-1));

    int ret = proxy_->RegisterDownloadAssetCallback(remote_);
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "RegisterDownloadAssetCallback End";
}

/**
 * @tc.name: RegisterDownloadAssetCallback003
 * @tc.desc: Verify the RegisterDownloadAssetCallback function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, RegisterDownloadAssetCallback003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterDownloadAssetCallback Start";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));

    int ret = proxy_->RegisterDownloadAssetCallback(remote_);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "RegisterDownloadAssetCallback End";
}

/**
 * @tc.name: DeleteAsset001
 * @tc.desc: Verify the DeleteAsset function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, DeleteAsset001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteAsset001 Start";
    int32_t userId = 100;
    string uri = "file://media";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(-1));

    int ret = proxy_->DeleteAsset(userId, uri);
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "DeleteAsset001 End";
}

/**
 * @tc.name: DeleteAsset002
 * @tc.desc: Verify the DeleteAsset function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, DeleteAsset002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteAsset002 Start";
    int32_t userId = 100;
    string uri = "file://media";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));
    int ret = proxy_->DeleteAsset(userId, uri);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DeleteAsset002 End";
}

/**
 * @tc.name: RegisterDownloadFileCallback001
 * @tc.desc: Verify the RegisterDownloadFileCallback function.
 * @tc.type: FUNC
 * @tc.require: issueI7UYAL
 */
HWTEST_F(CloudSyncServiceProxyTest, RegisterDownloadFileCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterDownloadFileCallback Start";
    string bundleName = "com.ohos.photos";

    int result = proxy_->RegisterDownloadFileCallback(nullptr);
    EXPECT_EQ(result, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "RegisterDownloadFileCallback End";
}

/**
 * @tc.name: RegisterDownloadFileCallback002
 * @tc.desc: Verify the RegisterDownloadFileCallback function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, RegisterDownloadFileCallback002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterDownloadFileCallback Start";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(-1));

    int result = proxy_->RegisterDownloadFileCallback(remote_);
    EXPECT_EQ(result, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "RegisterDownloadFileCallback End";
}

/**
 * @tc.name: RegisterDownloadFileCallback003
 * @tc.desc: Verify the RegisterDownloadFileCallback function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, RegisterDownloadFileCallback003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterDownloadFileCallback Start";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));

    int result = proxy_->RegisterDownloadFileCallback(remote_);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "RegisterDownloadFileCallback End";
}

/**
 * @tc.name: UnregisterDownloadFileCallback001
 * @tc.desc: Verify the UnegisterDownloadFileCallback function.
 * @tc.type: FUNC
 * @tc.require: issueI7UYAL
 */
HWTEST_F(CloudSyncServiceProxyTest, UnregisterDownloadFileCallback001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterDownloadFileCallback Start";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(-1));
    int result = proxy_->UnregisterDownloadFileCallback();

    EXPECT_EQ(result, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "UnregisterDownloadFileCallback End";
}

/**
 * @tc.name: UnregisterDownloadFileCallback002
 * @tc.desc: Verify the UnegisterDownloadFileCallback function.
 * @tc.type: FUNC
 * @tc.require: issueI7UYAL
 */
HWTEST_F(CloudSyncServiceProxyTest, UnregisterDownloadFileCallback002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterDownloadFileCallback Start";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));
    int result = proxy_->UnregisterDownloadFileCallback();

    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "UnregisterDownloadFileCallback End";
}

/**
 * @tc.name: BatchCleanFile001
 * @tc.desc: Verify the BatchCleanFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, BatchCleanFile001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatchCleanFile001 Start";
    std::vector<CleanFileInfoObj> fileInfoObj;
    std::vector<std::string> failCloudId;
    int ret = proxy_->BatchCleanFile(fileInfoObj, failCloudId);
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "BatchCleanFile001 End";
}

/**
 * @tc.name: DeleteFilesInner002
 * @tc.desc: Verify the BatchCleanFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, BatchCleanFile002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatchCleanFile002 Start";
    CleanFileInfo obj;
    std::vector<CleanFileInfoObj> fileInfoObj;
    fileInfoObj.push_back(obj);
    std::vector<std::string> failCloudId;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(-1));
    int ret = proxy_->BatchCleanFile(fileInfoObj, failCloudId);
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "DownloadFiles002 End";
}

/**
 * @tc.name: BatchCleanFile003
 * @tc.desc: Verify the BatchCleanFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyTest, BatchCleanFile003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatchCleanFile003 Start";
    CleanFileInfo obj;
    std::vector<CleanFileInfoObj> fileInfoObj;
    fileInfoObj.push_back(obj);
    std::vector<std::string> failCloudId;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));
    int ret = proxy_->BatchCleanFile(fileInfoObj, failCloudId);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DeleteFilesInner003 End";
}
} // namespace Test
} // namespace FileManagement::CloudSync {
} // namespace OHOS