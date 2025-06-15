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

#include "distributed_file_daemon_proxy.h"
#include "file_mount_manager.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "dfs_error.h"

namespace OHOS::Storage::DistributedFile::Test {
using namespace OHOS::FileManagement;
using namespace testing;
using namespace testing::ext;
using namespace std;

class FileMountManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
public:
    static inline shared_ptr<FileMountManager> filemountmanager_ = nullptr;
};

void FileMountManagerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void FileMountManagerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FileMountManagerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FileMountManagerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FileMountManager_GetDfsUrisDirFromLocal_0100
 * @tc.desc: The execution of the GetDfsUrisDirFromLocal failed.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileMountManagerTest, FileMountManager_GetDfsUrisDirFromLocal_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMountManager_GetDfsUrisDirFromLocal_0100 Start";
    std::vector<std::string> uriList = {""};
    int32_t userId = 101;
    std::string str = "test";
    AppFileService::ModuleRemoteFileShare::HmdfsUriInfo uriInfo;
    uriInfo.uriStr = "test";
    uriInfo.fileSize = 2;
    std::unordered_map<std::string, AppFileService::ModuleRemoteFileShare::HmdfsUriInfo> uriToDfsUriMaps;
    uriToDfsUriMaps.insert({str, uriInfo});

    auto ret = filemountmanager_->GetDfsUrisDirFromLocal(uriList, userId, uriToDfsUriMaps);
    EXPECT_EQ(ret, E_SA_LOAD_FAILED);
    GTEST_LOG_(INFO) << "FileMountManager_GetDfsUrisDirFromLocal_0100 End";
}

}}