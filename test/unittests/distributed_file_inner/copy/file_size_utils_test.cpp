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
#include <copy/file_copy_manager.h>
#include <copy/file_size_utils.h>
#include <gtest/gtest.h>
#include "dfs_error.h"

namespace OHOS::Storage::DistributedFile::Test {
using namespace OHOS::FileManagement;
using namespace testing;
using namespace testing::ext;
using namespace std;

class FileSizeUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    uint64_t process = 0;
    uint64_t fileSize = 0;
    using callBack = std::function<void(uint64_t processSize, uint64_t totalFileSize)>;
    callBack listener = [&](uint64_t processSize, uint64_t totalFileSize) {
        process = processSize;
        fileSize = totalFileSize;
    };
};

void FileSizeUtilsTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void FileSizeUtilsTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FileSizeUtilsTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FileSizeUtilsTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
* @tc.name: FileSizeUtils_0001
* @tc.desc: The execution of the GetSize failed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileSizeUtilsTest, FileSizeUtils_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileSizeUtils_0001 Start";
    string srcuri = "file://docs/storage/media/100/local/files/Docs/aa/";
    string desturi = "file://docs/storage/media/100/local/files/Docs/aa1/";
    bool isSrcUri = true;
    auto ptr = std::make_shared<FileSizeUtils>();
    int32_t ret = ptr->GetSize(srcuri, isSrcUri, fileSize);
    EXPECT_EQ(ret, 2);
    GTEST_LOG_(INFO) << "FileSizeUtils_0001 End";
}
}
