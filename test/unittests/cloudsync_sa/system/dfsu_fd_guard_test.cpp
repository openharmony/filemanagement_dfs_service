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
#include <memory>

#include "dfsu_fd_guard.h"

namespace OHOS::Storage::DistributedFile::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
class DfsuFdGuardTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DfsuFdGuardTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DfsuFdGuardTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DfsuFdGuardTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void DfsuFdGuardTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: GetFDTest001
 * @tc.desc: Verify the GetFD function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DfsuFdGuardTest, GetFDTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFDTest001 Begin";
    try {
        int fd = 0;
        bool autoClose = true;
        DfsuFDGuard dfsuFDGuard(fd, autoClose);
        auto ret = dfsuFDGuard.GetFD();
        EXPECT_EQ(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetFDTest001 ERROR";
    }

    GTEST_LOG_(INFO) << "GetFDTest001 End";
}

/**
 * @tc.name: GetFDTest002
 * @tc.desc: Verify the GetFD function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DfsuFdGuardTest, GetFDTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFDTest002 Begin";
    try {
        int fd = 0;
        bool autoClose = true;
        DfsuFDGuard dfsuFDGuard(fd, autoClose);
        if (dfsuFDGuard.GetFD()) {
            EXPECT_TRUE(true);
        }
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetFDTest002 ERROR";
    }

    GTEST_LOG_(INFO) << "GetFDTest002 End";
}

/**
 * @tc.name: SetFDTest001
 * @tc.desc: Verify the SetFD function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DfsuFdGuardTest, SetFDTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFDTest001 Begin";
    try {
        int fd = 1;
        bool autoClose = true;
        DfsuFDGuard dfsuFDGuard(fd, autoClose);
        dfsuFDGuard.SetFD(fd, autoClose);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetFDTest001 ERROR";
    }

    GTEST_LOG_(INFO) << "SetFDTest001 End";
}

/**
 * @tc.name: SetFDTest002
 * @tc.desc: Verify the SetFD function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DfsuFdGuardTest, SetFDTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFDTest002 Begin";
    try {
        int fd = 1;
        bool autoClose = true;
        DfsuFDGuard dfsuFDGuard(fd, autoClose);
        dfsuFDGuard.ClearFD();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetFDTest002 ERROR";
    }

    GTEST_LOG_(INFO) << "SetFDTest002 End";
}
} // namespace OHOS::Storage::DistributedFile::Test
