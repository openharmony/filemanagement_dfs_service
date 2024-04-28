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
#include "mountpoint/mount_manager.h"

#include <memory>
#include <unistd.h>

#include "gtest/gtest.h"

#include "mountpoint/mount_point.h"
#include "utils_directory.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing::ext;
using namespace std;

class MountManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: MountManagerTest_MountManager_0100
 * @tc.desc: Verify the MountManager class.
 * @tc.type: FUNC
 * @tc.require: I9KETY
 */
HWTEST_F(MountManagerTest, MountManagerTest_MountManager_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MountManagerTest_MountManager_0100 start";
    MountManager::GetInstance()->Umount("test");
    Utils::MountArgument mountArg{.userId_ = 100,
                                  .relativePath_ = "data/test",
                                  .accountless_ = true};
    unique_ptr<MountPoint> uniquePoint = std::make_unique<MountPoint>(mountArg);
    MountManager::GetInstance()->Mount(move(uniquePoint));

    auto smp = make_shared<MountPoint>(mountArg);
    weak_ptr<MountPoint> weakPoint = smp;
    MountManager::GetInstance()->Umount(weakPoint);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "MountManagerTest_MountManager_0100 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
