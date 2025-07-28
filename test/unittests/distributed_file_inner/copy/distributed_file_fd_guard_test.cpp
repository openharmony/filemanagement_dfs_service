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

#include "distributed_file_fd_guard.h"

#include <gtest/gtest.h>
#include <unistd.h>
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace OHOS::FileManagement;
using namespace testing;
using namespace testing::ext;
using namespace std;

class DistributedFileFdGuardTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedFileFdGuardTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DistributedFileFdGuardTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DistributedFileFdGuardTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void DistributedFileFdGuardTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: DistributedFileFdGuard_GetFD_0001
 * @tc.desc: test GetFD function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileFdGuardTest, DistributedFileFdGuard_GetFD_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileFdGuard_GetFD_0001 Start";

    int fd = 10;
    bool autoClose = false;
    FDGuard dfsuFDGuard(fd, autoClose);
    dfsuFDGuard.SetFD(fd, autoClose);
    int32_t ret = dfsuFDGuard.GetFD();
    EXPECT_EQ(ret, fd);

    GTEST_LOG_(INFO) << "DistributedFileFdGuard_GetFD_0001 End";
}

/**
 * @tc.name: DistributedFileFdGuard_GetFD_0002
 * @tc.desc: test GetFD function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileFdGuardTest, DistributedFileFdGuard_GetFD_0002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileFdGuard_GetFD_0002 Start";

    int fd = 5;
    bool autoClose = false;
    FDGuard dfsuFDGuard(fd, autoClose);
    dfsuFDGuard.SetFD(fd, autoClose);
    int32_t ret = dfsuFDGuard.GetFD();
    EXPECT_EQ(ret, fd);

    GTEST_LOG_(INFO) << "DistributedFileFdGuard_GetFD_0002 End";
}

/**
 * @tc.name: DistributedFileFdGuard_ClearFD_0001
 * @tc.desc: test ClearFD function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileFdGuardTest, DistributedFileFdGuard_ClearFD_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileFdGuard_ClearFD_0001 Start";

    int fd = 4;
    bool autoClose = false;
    FDGuard dfsuFDGuard(fd, autoClose);
    dfsuFDGuard.SetFD(fd, autoClose);
    dfsuFDGuard.ClearFD();
    int32_t ret = dfsuFDGuard.GetFD();
    EXPECT_NE(ret, fd);

    GTEST_LOG_(INFO) << "DistributedFileFdGuard_ClearFD_0001 End";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS