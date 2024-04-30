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

#include <memory>
#include <unistd.h>

#include "gtest/gtest.h"
#include "network/softbus/softbus_session_listener.h"
#include "sandbox_helper.h"

#include "dfs_error.h"
#include "utils_log.h"

using namespace OHOS::AppFileService;
using namespace OHOS::FileManagement;
using namespace std;
namespace {
const string TEST_URI = "file://com.demo.a/data/test/el2/base/files/test.txt";
const string TEST_ERR_URI = "file://com.demo.a/data/test/el2/base/files/test2.txt";
const string TEST_MEDIA_URI = "file://media/data/test/el2/base/files/test.txt";
const string TEST_PATH = "/data/test/";
const string TEST_ERR_PATH = "/data/test2/";
}

namespace OHOS {
namespace AppFileService {
int32_t SandboxHelper::GetPhysicalPath(const std::string &fileUri, const std::string &userId,
                                       std::string &physicalPath)
{
    if (fileUri == TEST_ERR_URI) {
        return -EINVAL;
    }

    if (fileUri == TEST_URI || fileUri == TEST_MEDIA_URI) {
        physicalPath = TEST_PATH;
        return E_OK;
    }

    physicalPath = TEST_ERR_PATH;
    return E_OK;
}

bool SandboxHelper::CheckValidPath(const std::string &filePath)
{
    if (filePath == TEST_ERR_PATH) {
        return false;
    }

    return true;
}
}
}

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing::ext;

class SoftBusSessionListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};
/**
 * @tc.name: SoftBusSessionListenerTest_GetBundleName_0100
 * @tc.desc: test GetBundleName function.
 * @tc.type: FUNC
 * @tc.require: I9JKYU
 */
HWTEST_F(SoftBusSessionListenerTest, SoftBusSessionListenerTest_GetBundleName_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftBusSessionListenerTest_GetBundleName_0100 start";
    string bundleName = SoftBusSessionListener::GetBundleName(TEST_URI);
    EXPECT_EQ(bundleName, "com.demo.a");
    string testUri = "datashare:///com.demo.a/data/test/el2/base/files/test.txt";
    bundleName = SoftBusSessionListener::GetBundleName(testUri);
    EXPECT_EQ(bundleName, "");
    testUri = "file://";
    bundleName = SoftBusSessionListener::GetBundleName(testUri);
    EXPECT_EQ(bundleName, "");
    testUri = "file://com.demo.a";
    bundleName = SoftBusSessionListener::GetBundleName(testUri);
    EXPECT_EQ(bundleName, "");
    GTEST_LOG_(INFO) << "SoftBusSessionListenerTest_GetBundleName_0100 end";
}

/**
 * @tc.name: SoftBusSessionListenerTest_GetLocalUri_0100
 * @tc.desc: test GetLocalUri function.
 * @tc.type: FUNC
 * @tc.require: I9JKYU
 */
HWTEST_F(SoftBusSessionListenerTest, SoftBusSessionListenerTest_GetLocalUri_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftBusSessionListenerTest_GetLocalUri_0100 start";
    string localUri = SoftBusSessionListener::GetLocalUri(TEST_URI);
    EXPECT_EQ(localUri, "");
    string testUri2 = TEST_URI + "?networkid=454553656565656656";
    localUri = SoftBusSessionListener::GetLocalUri(testUri2);
    EXPECT_EQ(localUri, TEST_URI);
    GTEST_LOG_(INFO) << "SoftBusSessionListenerTest_GetLocalUri_0100 end";
}

/**
 * @tc.name: SoftBusSessionListenerTest_GetSandboxPath_0100
 * @tc.desc: test GetSandboxPath function.
 * @tc.type: FUNC
 * @tc.require: I9JKYU
 */
HWTEST_F(SoftBusSessionListenerTest, SoftBusSessionListenerTest_GetSandboxPath_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftBusSessionListenerTest_GetSandboxPath_0100 start";
    string sandboxPath = SoftBusSessionListener::GetSandboxPath(TEST_URI);
    EXPECT_EQ(sandboxPath, "");
    string testUri2 = "file://com.demo.a/distributedfiles/.remote_share/data/test/el2/base/files/test.txt";
    sandboxPath = SoftBusSessionListener::GetSandboxPath(testUri2);
    EXPECT_EQ(sandboxPath, "data/test/el2/base/files/test.txt");
    GTEST_LOG_(INFO) << "SoftBusSessionListenerTest_GetSandboxPath_0100 end";
}

/**
 * @tc.name: SoftBusSessionListenerTest_GetRealPath_0100
 * @tc.desc: test GetRealPath function.
 * @tc.type: FUNC
 * @tc.require: I9JKYU
 */
HWTEST_F(SoftBusSessionListenerTest, SoftBusSessionListenerTest_GetRealPath_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftBusSessionListenerTest_GetRealPath_0100 start";
    string netWorkId = "?networkid=454553656565656656";
    string realPath = SoftBusSessionListener::GetRealPath(TEST_URI);
    EXPECT_EQ(realPath, "");
    string testUri2 = "datashare:///com.demo.a/data/test/el2/base/files/test.txt" + netWorkId;
    realPath = SoftBusSessionListener::GetRealPath(testUri2);
    EXPECT_EQ(realPath, "");
    string testUri3 = TEST_URI + netWorkId;
    realPath = SoftBusSessionListener::GetRealPath(testUri3);
    EXPECT_EQ(realPath, "");

    string testUri4 = "file://com.demo.a/distributedfiles/.remote_share/data/test/el2/base/files/test.txt";
    testUri4 += netWorkId;
    realPath = SoftBusSessionListener::GetRealPath(testUri4);
    EXPECT_EQ(realPath, TEST_PATH);

    string testUri5 = "file://com.demo.a/distributedfiles/.remote_share/data/test/el2/base/files/test2.txt";
    testUri5 += netWorkId;
    realPath = SoftBusSessionListener::GetRealPath(testUri5);
    EXPECT_EQ(realPath, "");

    string testUri6 = "file://docs/data/test/el2/base/files/test2.txt" + netWorkId;
    realPath = SoftBusSessionListener::GetRealPath(testUri6);
    EXPECT_EQ(realPath, "");

    string testUri7 = TEST_MEDIA_URI + netWorkId;
    realPath = SoftBusSessionListener::GetRealPath(testUri7);
    EXPECT_EQ(realPath, TEST_PATH);
    GTEST_LOG_(INFO) << "SoftBusSessionListenerTest_GetRealPath_0100 end";
}

/**
 * @tc.name: SoftBusSessionListenerTest_GetFileName_0100
 * @tc.desc: test GetFileName function.
 * @tc.type: FUNC
 * @tc.require: I9JKYU
 */
HWTEST_F(SoftBusSessionListenerTest, SoftBusSessionListenerTest_GetFileName_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftBusSessionListenerTest_GetSandboxPath_0100 start";
    vector<string> fileList;
    fileList.push_back("/data/test/test1/t1.txt");
    fileList.push_back("/data/test/test1/t2.txt");
    vector<string> rltList;
    rltList = SoftBusSessionListener::GetFileName(fileList, "/data/test");
    EXPECT_EQ(rltList.size(), 2);
    EXPECT_EQ(rltList[0], "test1/t1.txt");
    EXPECT_EQ(rltList[1], "test1/t2.txt");
    rltList.clear();
    rltList = SoftBusSessionListener::GetFileName(fileList, "/data/test/test1/t1.txt");
    EXPECT_EQ(rltList.size(), 1);
    EXPECT_EQ(rltList[0], "t1.txt");
    GTEST_LOG_(INFO) << "SoftBusSessionListenerTest_GetSandboxPath_0100 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
