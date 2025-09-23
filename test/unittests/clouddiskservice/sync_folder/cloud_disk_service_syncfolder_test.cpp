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
#include "cloud_disk_service_syncfolder.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CloudDiskServiceSyncFolderTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudDiskServiceSyncFolderTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudDiskServiceSyncFolderTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudDiskServiceSyncFolderTest::SetUp()
{
}

void CloudDiskServiceSyncFolderTest::TearDown()
{
}

/**
 * @tc.name: RegisterSyncFolderTest001
 * @tc.desc: Verify the RegisterSyncFolder function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceSyncFolderTest, RegisterSyncFolderTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSyncFolderTest001 start";
    try {
        int32_t userId = 1;
        uint64_t syncFolderIndex = 1;
        string path = "path";
        CloudDiskServiceSyncFolder::RegisterSyncFolder(userId, syncFolderIndex, path);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterSyncFolderTest001 failed";
    }
    GTEST_LOG_(INFO) << "RegisterSyncFolderTest001 end";
}

/**
 * @tc.name: UnRegisterSyncFolderTest001
 * @tc.desc: Verify the UnRegisterSyncFolder function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceSyncFolderTest, UnRegisterSyncFolderTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnRegisterSyncFolderTest001 start";
    try {
        int32_t userId = 1;
        uint64_t syncFolderIndex = 1;
        CloudDiskServiceSyncFolder::UnRegisterSyncFolder(userId, syncFolderIndex);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnRegisterSyncFolderTest001 failed";
    }
    GTEST_LOG_(INFO) << "UnRegisterSyncFolderTest001 end";
}

/**
 * @tc.name: RegisterSyncFolderChangesTest001
 * @tc.desc: Verify the RegisterSyncFolderChanges function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceSyncFolderTest, RegisterSyncFolderChangesTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSyncFolderChangesTest001 start";
    try {
        int32_t userId = 1;
        uint64_t syncFolderIndex = 1;
        CloudDiskServiceSyncFolder::RegisterSyncFolderChanges(userId, syncFolderIndex);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterSyncFolderChangesTest001 failed";
    }
    GTEST_LOG_(INFO) << "RegisterSyncFolderChangesTest001 end";
}

/**
 * @tc.name: UnRegisterSyncFolderChangesTest001
 * @tc.desc: Verify the UnRegisterSyncFolderChanges function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceSyncFolderTest, UnRegisterSyncFolderChangesTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnRegisterSyncFolderChangesTest001 start";
    try {
        int32_t userId = 1;
        uint64_t syncFolderIndex = 1;
        CloudDiskServiceSyncFolder::UnRegisterSyncFolderChanges(userId, syncFolderIndex);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnRegisterSyncFolderChangesTest001 failed";
    }
    GTEST_LOG_(INFO) << "UnRegisterSyncFolderChangesTest001 end";
}

/**
 * @tc.name: GetSyncFolderChangesTest001
 * @tc.desc: Verify the GetSyncFolderChanges function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceSyncFolderTest, GetSyncFolderChangesTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncFolderChangesTest001 start";
    try {
        int32_t userId = 1;
        uint64_t syncFolderIndex = 1;
        uint64_t start = 1;
        uint64_t count = 1;
        ChangesResult changesResult;
        CloudDiskServiceSyncFolder::GetSyncFolderChanges(userId, syncFolderIndex, start, count, changesResult);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSyncFolderChangesTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetSyncFolderChangesTest001 end";
}

/**
 * @tc.name: SetSyncFolderChangesTest001
 * @tc.desc: Verify the SetSyncFolderChanges function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceSyncFolderTest, SetSyncFolderChangesTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetSyncFolderChangesTest001 start";
    try {
        EventInfo eventInfo;
        CloudDiskServiceSyncFolder::SetSyncFolderChanges(eventInfo);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetSyncFolderChangesTest001 failed";
    }
    GTEST_LOG_(INFO) << "SetSyncFolderChangesTest001 end";
}

/**
 * @tc.name: CloudDiskServiceClearAllTest001
 * @tc.desc: Verify the CloudDiskServiceClearAll function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceSyncFolderTest, CloudDiskServiceClearAllTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDiskServiceClearAllTest001 start";
    try {
        CloudDiskServiceSyncFolder::CloudDiskServiceClearAll();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudDiskServiceClearAllTest001 failed";
    }
    GTEST_LOG_(INFO) << "CloudDiskServiceClearAllTest001 end";
}
}