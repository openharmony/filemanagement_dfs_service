/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "decompress_update.h"
#include "decompress_kit.h"
#include "decompress_kit_mock.h"
#include "decompress_kit_constant.h"
#include "utils_log.h"

using namespace testing::ext;
using namespace testing;
using namespace OHOS::FileManagement::Decompress;

class DecompressUpdateManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DecompressUpdateManagerTest::SetUpTestCase(void)
{
    // DecompressKit's singleton pointer cannot be reset, so register exactly one
    // mock here for the whole suite (intentional process-lifetime instance,
    // never freed) and obtain it via GetInstance() in every test. This keeps the
    // branch tests mutually independent instead of relying on a specific test
    // having run first to register the mock.
    DecompressKit *mockKit = new (std::nothrow) DecompressKitMock();
    if (mockKit == nullptr) {
        return;
    }
    testing::Mock::AllowLeak(mockKit);
    DecompressKit::RegisterDecompressInstance(mockKit);
}

void DecompressUpdateManagerTest::TearDownTestCase(void)
{
}

void DecompressUpdateManagerTest::SetUp(void)
{
}

void DecompressUpdateManagerTest::TearDown(void)
{
}

// The nullptr-kit branch of HandleDecompressUpdate ("kitInstance == nullptr")
// is intentionally not exercised as a standalone test: this suite registers a
// single DecompressKitMock in SetUpTestCase so the branch tests are mutually
// independent, and the singleton cannot be reset afterward to recreate the null
// state. That branch is defensive only -- in production cloud_adapter
// self-registers DecompressKitImpl at load, so GetInstance() is never null in
// the cloudsync_sa process.

/**
 * @tc.name: HandleDecompressUpdate_002
 * @tc.desc: Test HandleDecompressUpdate when GetCloudVersionFilePath returns empty
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressUpdateManagerTest, HandleDecompressUpdate_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleDecompressUpdate_002 begin";
    DecompressKit *instance = DecompressKit::GetInstance();
    ASSERT_NE(instance, nullptr);
    EXPECT_CALL(*static_cast<DecompressKitMock *>(instance), GetCloudVersionFilePath())
        .WillOnce(Return(""));
    DecompressUpdateManager &manager = DecompressUpdateManager::GetInstance();
    manager.HandleDecompressUpdate();
    GTEST_LOG_(INFO) << "HandleDecompressUpdate_002 end";
}

/**
 * @tc.name: HandleDecompressUpdate_003
 * @tc.desc: Test HandleDecompressUpdate when IsUpdateVersionCompatible returns false
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressUpdateManagerTest, HandleDecompressUpdate_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleDecompressUpdate_003 begin";
    DecompressKit *instance = DecompressKit::GetInstance();
    ASSERT_NE(instance, nullptr);
    EXPECT_CALL(*static_cast<DecompressKitMock *>(instance), GetCloudVersionFilePath())
        .WillOnce(Return("/test/path"));
    EXPECT_CALL(*static_cast<DecompressKitMock *>(instance), IsUpdateVersionCompatible(_))
        .WillOnce(Return(false));
    DecompressUpdateManager &manager = DecompressUpdateManager::GetInstance();
    manager.HandleDecompressUpdate();
    GTEST_LOG_(INFO) << "HandleDecompressUpdate_003 end";
}

/**
 * @tc.name: HandleDecompressUpdate_004
 * @tc.desc: Test HandleDecompressUpdate when IsNeedCopy returns false
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressUpdateManagerTest, HandleDecompressUpdate_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleDecompressUpdate_004 begin";
    DecompressKit *instance = DecompressKit::GetInstance();
    ASSERT_NE(instance, nullptr);
    EXPECT_CALL(*static_cast<DecompressKitMock *>(instance), GetCloudVersionFilePath())
        .WillOnce(Return("/test/path"));
    EXPECT_CALL(*static_cast<DecompressKitMock *>(instance), IsUpdateVersionCompatible(_))
        .WillOnce(Return(true));
    EXPECT_CALL(*static_cast<DecompressKitMock *>(instance), IsNeedCopy(_, _))
        .WillOnce(Return(false));
    DecompressUpdateManager &manager = DecompressUpdateManager::GetInstance();
    manager.HandleDecompressUpdate();
    GTEST_LOG_(INFO) << "HandleDecompressUpdate_004 end";
}

/**
 * @tc.name: HandleDecompressUpdate_005
 * @tc.desc: Test HandleDecompressUpdate normal flow with all conditions true
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressUpdateManagerTest, HandleDecompressUpdate_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleDecompressUpdate_005 begin";
    DecompressKit *instance = DecompressKit::GetInstance();
    ASSERT_NE(instance, nullptr);
    EXPECT_CALL(*static_cast<DecompressKitMock *>(instance), GetCloudVersionFilePath())
        .WillOnce(Return("/test/path"));
    EXPECT_CALL(*static_cast<DecompressKitMock *>(instance), IsUpdateVersionCompatible(_))
        .WillOnce(Return(true));
    EXPECT_CALL(*static_cast<DecompressKitMock *>(instance), IsNeedCopy(_, _))
        .WillOnce(Return(true));
    EXPECT_CALL(*static_cast<DecompressKitMock *>(instance), DoParamDirCopy(_, _))
        .WillOnce(Return(true));
    DecompressUpdateManager &manager = DecompressUpdateManager::GetInstance();
    manager.HandleDecompressUpdate();
    GTEST_LOG_(INFO) << "HandleDecompressUpdate_005 end";
}

/**
 * @tc.name: HandleDecompressUpdate_006
 * @tc.desc: Test HandleDecompressUpdate when DoParamDirCopy returns false
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressUpdateManagerTest, HandleDecompressUpdate_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleDecompressUpdate_006 begin";
    DecompressKit *instance = DecompressKit::GetInstance();
    ASSERT_NE(instance, nullptr);
    EXPECT_CALL(*static_cast<DecompressKitMock *>(instance), GetCloudVersionFilePath())
        .WillOnce(Return("/test/path"));
    EXPECT_CALL(*static_cast<DecompressKitMock *>(instance), IsUpdateVersionCompatible(_))
        .WillOnce(Return(true));
    EXPECT_CALL(*static_cast<DecompressKitMock *>(instance), IsNeedCopy(_, _))
        .WillOnce(Return(true));
    EXPECT_CALL(*static_cast<DecompressKitMock *>(instance), DoParamDirCopy(_, _))
        .WillOnce(Return(false));
    DecompressUpdateManager &manager = DecompressUpdateManager::GetInstance();
    manager.HandleDecompressUpdate();
    GTEST_LOG_(INFO) << "HandleDecompressUpdate_006 end";
}
