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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "assistant.h"
#include "file_utils.h"

namespace OHOS::FileManagement {
using namespace testing;
using namespace testing::ext;

class FileRangeLockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<CloudDisk::AssistantMock> insMock = nullptr;
};

void FileRangeLockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void FileRangeLockTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FileRangeLockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    insMock = std::make_shared<CloudDisk::AssistantMock>();
    CloudDisk::AssistantMock::EnableMock();
    CloudDisk::Assistant::ins = insMock;
}

void FileRangeLockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    CloudDisk::AssistantMock::DisableMock();
    CloudDisk::Assistant::ins = nullptr;
    insMock = nullptr;
}

/**
 * @tc.name: FilePosLockTest001
 * @tc.desc: Verify the FilePosLock function
 * @tc.type: FUNC
 */
HWTEST_F(FileRangeLockTest, FilePosLockTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FilePosLockTest001 Start";
    try {
        int fd = 0;
        off_t offset = 0;
        size_t size = 4096;
        EXPECT_CALL(*insMock, fcntl(_, _)).WillOnce(Return(-1));
        int ret = FileRangeLock::FilePosLock(fd, offset, size, F_WRLCK);
        EXPECT_NE(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FilePosLockTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "FilePosLockTest001 End";
}

/**a
 * @tc.name: FilePosLockTest002
 * @tc.desc: Verify the FilePosLock function
 * @tc.type: FUNC
 */
HWTEST_F(FileRangeLockTest, FilePosLockTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FilePosLockTest002 Start";
    try {
        int fd = 0;
        off_t offset = 0;
        size_t size = 4096;
        EXPECT_CALL(*insMock, fcntl(_, _)).WillOnce(Return(0));
        int ret = FileRangeLock::FilePosLock(fd, offset, size, F_WRLCK);
        EXPECT_EQ(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FilePosLockTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "FilePosLockTest002 End";
}
}