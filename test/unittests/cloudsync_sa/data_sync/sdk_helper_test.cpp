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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "data_sync_manager.h"
#include "dfs_error.h"
#include "dk_record.h"
#include "gallery_data_syncer.h"
#include "sdk_helper.h"

/* template link */
namespace OHOS::FileManagement::CloudSync {
template<typename T, typename RET, typename... ARGS>
std::function<RET(ARGS...)> DataSyncer::AsyncCallback(RET (T::*f)(ARGS...))
{
    return taskRunner_->AsyncCallback<DataSyncer>(f, this);
}
} // namespace OHOS::FileManagement::CloudSync

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class SdkHelperTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<SdkHelper> sdkHelper_;
};
void SdkHelperTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    sdkHelper_ = std::make_shared<SdkHelper>();
}

void SdkHelperTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    sdkHelper_ = nullptr;
}

#undef LOGI
void LOGI(string content)
{
    GTEST_LOG_(INFO) << content;
}

void SdkHelperTest::SetUp(void) {}

void SdkHelperTest::TearDown(void) {}

/**
 * @tc.name: GetAssetReadSessionErrTest,
 * @tc.desc: Verify the GetAssetReadSession function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, GetAssetReadSessionErrTest, TestSize.Level1)
{
    auto ret = sdkHelper_->GetAssetReadSession({}, {}, {}, {});
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: InitDriveKitInstanceFailTest
 * @tc.desc: Verify the Init function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, InitDriveKitInstanceFailTest, TestSize.Level1)
{
    int32_t userId = -1;
    string appBundleName = "com.ohos.test";
    auto ret = sdkHelper_->Init(userId, appBundleName);
    EXPECT_EQ(E_CLOUD_SDK, ret);
}

/**
 * @tc.name: InitContainerFailTest
 * @tc.desc: Verify the Init function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, InitContainerFailTest, TestSize.Level1)
{
    int32_t userId = 100;
    string appBundleName = "com.ohos.test.mock";
    auto ret = sdkHelper_->Init(userId, appBundleName);
    EXPECT_EQ(E_CLOUD_SDK, ret);
}

/**
 * @tc.name: InitPrivateDatabaseFailTest
 * @tc.desc: Verify the Init function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, InitPrivateDatabaseFailTest, TestSize.Level1)
{
    int32_t userId = 100;
    string appBundleName = "com.ohos.test";
    auto ret = sdkHelper_->Init(userId, appBundleName);
    EXPECT_EQ(E_CLOUD_SDK, ret);
}

/**
 * @tc.name: InitOKTest
 * @tc.desc: Verify the Init function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, InitOKTest, TestSize.Level1)
{
    int32_t userId = 100;
    string appBundleName = "com.ohos.test12312";
    auto ret = sdkHelper_->Init(userId, appBundleName);
    EXPECT_EQ(E_OK, ret);
}

/**
 * @tc.name: GetLockOKTest
 * @tc.desc: Verify the GetLock function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, GetLockOKTest, TestSize.Level1)
{
    DriveKit::DKLock lock_;
    auto ret = sdkHelper_->GetLock(lock_);
    EXPECT_EQ(E_OK, ret);
    sdkHelper_->DeleteLock(lock_);
}

/**
 * @tc.name: GetLockErrTest
 * @tc.desc: Verify the GetLock function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, GetLockErrTest, TestSize.Level1)
{
    DriveKit::DKLock lock_;
    lock_.lockInterval = -1;
    auto ret = sdkHelper_->GetLock(lock_);
    EXPECT_NE(E_OK, ret);
}

/**
 * @tc.name: FetchRecordsOKTest
 * @tc.desc: Verify the FetchRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, FetchRecordsOKTest, TestSize.Level1)
{
    FetchCondition cond;
    int32_t ret = sdkHelper_->FetchRecords(nullptr, cond, "", nullptr);
    EXPECT_EQ(E_OK, ret);
}

/**
 * @tc.name: FetchRecordsErrTest
 * @tc.desc: Verify the FetchRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, FetchRecordsErrTest, TestSize.Level1)
{
    FetchCondition cond;
    int32_t ret = sdkHelper_->FetchRecords(nullptr, cond, "err", nullptr);
    EXPECT_NE(E_OK, ret);
}

/**
 * @tc.name: GetStartCursorOKTest
 * @tc.desc: Verify the GetStartCursor function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, GetStartCursorOKTest, TestSize.Level1)
{
    std::string tempStartCursor;
    int32_t ret = sdkHelper_->GetStartCursor("", tempStartCursor);
    EXPECT_EQ(E_OK, ret);
}

/**
 * @tc.name: GetStartCursorErrTest
 * @tc.desc: Verify the GetStartCursor function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, GetStartCursorErrTest, TestSize.Level1)
{
    std::string tempStartCursor;
    int32_t ret = sdkHelper_->GetStartCursor("err", tempStartCursor);
    EXPECT_NE(E_OK, ret);
}

/**
 * @tc.name: GetAssetReadSessionOKTest,
 * @tc.desc: Verify the GetAssetReadSession function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, GetAssetReadSessionOKTest, TestSize.Level1)
{
    auto ret = sdkHelper_->GetAssetReadSession({}, {}, {}, {});
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.name: SaveSubscriptionOKTest,
 * @tc.desc: Verify the SaveSubscription function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, SaveSubscriptionOKTest, TestSize.Level1)
{
    auto ret = sdkHelper_->SaveSubscription(nullptr);
    EXPECT_EQ(E_OK, ret);
}

/**
 * @tc.name: SaveSubscriptionErrTest,
 * @tc.desc: Verify the SaveSubscription function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, SaveSubscriptionErrTest, TestSize.Level1)
{
    auto ret = sdkHelper_->SaveSubscription(
        [](auto, shared_ptr<DriveKit::DKContainer>, DriveKit::DKSubscriptionResult &res) {});
    EXPECT_NE(E_OK, ret);
}

/**
 * @tc.name: DeleteSubscriptionOKTest,
 * @tc.desc: Verify the DeleteSubscription function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, DeleteSubscriptionOKTest, TestSize.Level1)
{
    auto ret = sdkHelper_->DeleteSubscription(nullptr);
    EXPECT_EQ(E_OK, ret);
}

/**
 * @tc.name: DeleteSubscriptionErrTest,
 * @tc.desc: Verify the DeleteSubscription function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, DeleteSubscriptionErrTest, TestSize.Level1)
{
    auto ret = sdkHelper_->DeleteSubscription([](auto, DriveKit::DKError err) {});
    EXPECT_NE(E_OK, ret);
}

/**
 * @tc.name: FetchRecordWithIdOKTest
 * @tc.desc: Verify the FetchRecordWithId function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, FetchRecordWithIdOKTest, TestSize.Level1)
{
    FetchCondition cond;
    int32_t ret = sdkHelper_->FetchRecordWithId(nullptr, cond, "", nullptr);
    EXPECT_EQ(E_OK, ret);
}

/**
 * @tc.name: FetchRecordWithIdErrTest
 * @tc.desc: Verify the FetchRecordWithId function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, FetchRecordWithIdErrTest, TestSize.Level1)
{
    FetchCondition cond;
    int32_t ret = sdkHelper_->FetchRecordWithId(nullptr, cond, "err", nullptr);
    EXPECT_NE(E_OK, ret);
}

/**
 * @tc.name: CancelDownloadAssetsOKTest
 * @tc.desc: Verify the CancelDownloadAssets function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, CancelDownloadAssetsOKTest, TestSize.Level1)
{
    int32_t id = 0;
    int32_t ret = sdkHelper_->CancelDownloadAssets(id);
    EXPECT_EQ(E_OK, ret);
}
} // namespace OHOS::FileManagement::CloudSync::Test