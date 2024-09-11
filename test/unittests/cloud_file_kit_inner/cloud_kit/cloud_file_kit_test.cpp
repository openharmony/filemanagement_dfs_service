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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "cloud_file_kit.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement::CloudFile {
namespace Test {
using namespace testing::ext;
using namespace std;


class CloudFileKitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudFileKitTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudFileKitTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudFileKitTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void CloudFileKitTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: StartTest001
 * @tc.desc: Verify the Start function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudFileKitTest, RegisterCloudInstanceTest001, TestSize.Level1)
{
    CloudFileKit *firstInstance = new CloudFileKit();
    EXPECT_TRUE(CloudFileKit::RegisterCloudInstance(firstInstance));

    CloudFileKit *secondInstance = new CloudFileKit();
    EXPECT_FALSE(CloudFileKit::RegisterCloudInstance(secondInstance));

    delete firstInstance;
    delete secondInstance;
}

} // namespace Test
} // namespace FileManagement::CloudFile
} // namespace OHOS