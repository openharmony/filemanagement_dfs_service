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

#include "utils_directory.h"

#include <sys/types.h>
#include <unistd.h>
#include <system_error>

#include "directory_ex.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Utils {
using namespace testing;
using namespace testing::ext;
using namespace std;

class UtilsDirectoryTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void UtilsDirectoryTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void UtilsDirectoryTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void UtilsDirectoryTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void UtilsDirectoryTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: ForceCreateDirectoryTest001
 * @tc.desc: Verify the ForceCreateDirectory function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(UtilsDirectoryTest, ForceCreateDirectoryTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForceCreateDirectoryTest001 Start";
    try {
        string path = "/data/tdd";
        ForceCreateDirectory(path);
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << " ForceCreateDirectoryTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "ForceCreateDirectoryTest001 End";
}

/**
 * @tc.name: ForceCreateDirectoryTest002
 * @tc.desc: Verify the ForceCreateDirectoryTest002 function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(UtilsDirectoryTest, ForceCreateDirectoryTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForceCreateDirectoryTest002 Start";
    try {
        string path = "/data/tdd";
        mode_t mode = (S_IRWXU | S_IRWXG | S_IRWXO); // 00777
        ForceCreateDirectory(path, mode);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ForceCreateDirectoryTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "ForceCreateDirectoryTest002 End";
}

/**
 * @tc.name: ForceCreateDirectoryTest003
 * @tc.desc: Verify the ForceCreateDirectoryTest003 function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(UtilsDirectoryTest, ForceCreateDirectoryTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForceCreateDirectoryTest003 Start";
    try {
        string path = "/data/tdd";
        mode_t mode = (S_IRWXU | S_IRWXG | S_IRWXO);
        uid_t uid = UID_ROOT;
        gid_t gid = 0; // root
        ForceCreateDirectory(path, mode, uid, gid);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ForceCreateDirectoryTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "ForceCreateDirectoryTest003 End";
}

/**
 * @tc.name: ForceRemoveDirectoryTest001
 * @tc.desc: Verify the ForceRemoveDirectory function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(UtilsDirectoryTest, ForceRemoveDirectoryTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForceRemoveDirectory001 Start";
    try {
        string path = "/data/tdd";
        ForceRemoveDirectory(path);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ForceRemoveDirectoryTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "ForceRemoveDirectoryTest001 End";
}
} // namespace Utils
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS