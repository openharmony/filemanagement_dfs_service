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
#include "mountpoint/mount_point.h"

#include <memory>
#include <unistd.h>

#include "gtest/gtest.h"

#include "utils_directory.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing::ext;
using namespace std;

class MountPointTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: MountPointTest_MountPoint_0100
 * @tc.desc: Verify the MountPoint class.
 * @tc.type: FUNC
 * @tc.require: I9KETY
 */
HWTEST_F(MountPointTest, MountPointTest_MountPoint_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MountPointTest_MountPoint_0100 start";
    Utils::MountArgument mountArg{.userId_ = 100,
                                  .relativePath_ = "data/test",
                                  .accountless_ = true};
    MountPoint point(mountArg);
    EXPECT_EQ(point.GetID(), point.idGen_ - 1);
    EXPECT_EQ(point.isAccountLess(), true);
    EXPECT_EQ(point.ToString(), "");
    EXPECT_EQ(point.GetMountArgument().GetFullDst(), "/mnt/hmdfs/100/data/test");
    EXPECT_EQ(point.GetMountArgument().GetFullSrc(), "/data/service/el2/100/hmdfs/data/test");
    MountPoint point2(mountArg);
    EXPECT_EQ(point == point2, true);

    Utils::MountArgument mountArg2;
    MountPoint point3(mountArg2);
    EXPECT_EQ(point == point3, false);

    point.Mount();
    point.Umount();
    GTEST_LOG_(INFO) << "MountPointTest_MountPoint_0100 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
