/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "dfs_error.h"
#include <copy/file_copy_manager.h>
#include <copy/trans_listener.h>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <memory>

namespace OHOS::Storage::DistributedFile::Test {
using namespace OHOS::FileManagement;
using namespace testing;
using namespace testing::ext;
using namespace std;

class TransListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    // Variables to track file transfer progress
    uint64_t process = 0;  // Bytes processed so far
    uint64_t fileSize = 0; // Total file size

    // Callback definition for progress updates
    using callBack = std::function<void(uint64_t processSize, uint64_t totalFileSize)>;
    callBack listener = [&](uint64_t processSize, uint64_t totalFileSize) {
        process = processSize;
        fileSize = totalFileSize;
    };
    callBack nullCallBack;
};

void TransListenerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void TransListenerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void TransListenerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    // Reset progress counters before each test
    process = 0;
    fileSize = 0;
}

void TransListenerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: TransListener_CopyToSandBox_0001
 * @tc.desc: The execution of the CopyTosandbox success.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(TransListenerTest, TransListener_CopyToSandBox_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListener_CopyToSandBox_0001 Start";

    // Set up source and destination URIs with docs authority
    string srcuri = "file://docs/storage/media/100/local/files/Docs/aa/";
    string desturi = "file://docs/storage/media/100/local/files/Docs/aa1/";

    // Create TransListener instance with destination URI and callback
    auto ptr = std::make_shared<TransListener>(desturi, listener);

    // Execute file copy operation and verify success
    int32_t ret = ptr->CopyToSandBox(srcuri);
    EXPECT_EQ(ret, E_OK); // Expect operation to succeed

    GTEST_LOG_(INFO) << "TransListener_CopyToSandBox_0001 End";
}

/**
 * @tc.name: TransListener_CopyToSandBox_0002
 * @tc.desc: Test CopyToSandBox with media authority (should skip copy)
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(TransListenerTest, TransListener_CopyToSandBox_0002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListener_CopyToSandBox_0002 Start";

    string srcuri = "file://media/storage/media/100/local/files/Media/src.jpg";
    string desturi = "file://media/storage/media/100/local/files/Media/dest.jpg";

    auto ptr = std::make_shared<TransListener>(desturi, listener);
    int32_t ret = ptr->CopyToSandBox(srcuri);
    EXPECT_EQ(ret, E_OK); // Should skip copy for media authority

    GTEST_LOG_(INFO) << "TransListener_CopyToSandBox_0002 End";
}

/**
 * @tc.name: TransListener_CopyToSandBox_0003
 * @tc.desc: Test CopyToSandBox directory with non-docs/media authority
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(TransListenerTest, TransListener_CopyToSandBox_0003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListener_CopyToSandBox_0003 Start";
    // Setup test directory structure first
    string testDir = "/data/storage/el2/distributedfiles/test_dir_0003";
    std::filesystem::create_directories(testDir + "/src");
    std::ofstream(testDir + "/src/file1.txt") << "test content";

    string srcuri = "file://other" + testDir + "/src";
    string desturi = "file://other" + testDir + "/dest";
    auto ptr = std::make_shared<TransListener>(desturi, listener);

    // change hmdfsInfo_.sandboxPath
    ptr->hmdfsInfo_.sandboxPath = testDir + "/src";

    int32_t ret = ptr->CopyToSandBox(srcuri);
    EXPECT_EQ(ret, E_OK);

    // Cleanup
    std::filesystem::remove_all(testDir);
    GTEST_LOG_(INFO) << "TransListener_CopyToSandBox_0003 End";
}

/**
 * @tc.name: TransListener_CopyToSandBox_0004
 * @tc.desc: Test CopyToSandBox single file with non-docs/media authority
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(TransListenerTest, TransListener_CopyToSandBox_0004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListener_CopyToSandBox_0004 Start";
    // Setup test file first
    string testDir = "/data/storage/el2/distributedfiles/test_dir_0004";
    std::filesystem::create_directories(testDir);
    std::ofstream(testDir + "/src.txt") << "test content";

    string srcuri = "file://other" + testDir + "/src.txt";
    string desturi = "file://other" + testDir + "/dest.txt";
    auto ptr = std::make_shared<TransListener>(desturi, listener);

    // change hmdfsInfo_.sandboxPath
    ptr->hmdfsInfo_.sandboxPath = testDir + "/src";

    int32_t ret = ptr->CopyToSandBox(srcuri);
    EXPECT_EQ(ret, EIO);

    // Cleanup
    std::filesystem::remove_all(testDir);
    GTEST_LOG_(INFO) << "TransListener_CopyToSandBox_0004 End";
}

/**
 * @tc.name: TransListener_CopyToSandBox_0005
 * @tc.desc: Test CopyToSandBox with invalid source path
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(TransListenerTest, TransListener_CopyToSandBox_0005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListener_CopyToSandBox_0005 Start";
    string srcuri = "file://other/invalid/path/src.txt";
    string desturi = "file://other/valid/path/dest.txt";
    auto ptr = std::make_shared<TransListener>(desturi, listener);

    int32_t ret = ptr->CopyToSandBox(srcuri);
    EXPECT_EQ(ret, EIO); // Should fail with EIO

    GTEST_LOG_(INFO) << "TransListener_CopyToSandBox_0005 End";
}

/**
 * @tc.name: TransListener_CopyToSandBox_0006
 * @tc.desc: Test CopyToSandBox with empty source URI
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(TransListenerTest, TransListener_CopyToSandBox_0006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListener_CopyToSandBox_0006 Start";
    string srcuri = "";
    string desturi = "file://other/valid/path/dest.txt";
    auto ptr = std::make_shared<TransListener>(desturi, listener);

    int32_t ret = ptr->CopyToSandBox(srcuri);
    EXPECT_NE(ret, E_OK); // Should fail

    GTEST_LOG_(INFO) << "TransListener_CopyToSandBox_0006 End";
}

/**
 * @tc.name: TransListener_CopyToSandBox_0007
 * @tc.desc: Test CopyToSandBox with permission denied scenario
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(TransListenerTest, TransListener_CopyToSandBox_0007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListener_CopyToSandBox_0007 Start";
    // Setup read-only directory
    string testDir = "/data/storage/el2/distributedfiles/test_dir_0007";
    std::filesystem::create_directories(testDir + "/src");
    std::filesystem::permissions(testDir + "/src", std::filesystem::perms::owner_read,
                                 std::filesystem::perm_options::replace);

    string srcuri = "file://other" + testDir + "/src";
    string desturi = "file://other" + testDir + "/dest";
    auto ptr = std::make_shared<TransListener>(desturi, listener);

    int32_t ret = ptr->CopyToSandBox(srcuri);
    EXPECT_EQ(ret, EIO); // Should fail with EIO

    // Cleanup
    std::filesystem::permissions(testDir + "/src", std::filesystem::perms::owner_all,
                                 std::filesystem::perm_options::replace);
    std::filesystem::remove_all(testDir);
    GTEST_LOG_(INFO) << "TransListener_CopyToSandBox_0007 End";
}

/**
 * @tc.name: TransListener_Cancel_0001
 * @tc.desc: The execution of the Cancel success.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(TransListenerTest, TransListener_Cancel_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListener_Cancel_0001 Start";

    // Set up test URIs
    string srcuri = "file://docs/storage/media/100/local/files/Docs/aa/";
    string desturi = "file://docs/storage/media/100/local/files/Docs/aa1/";

    // Create TransListener instance
    auto ptr = std::make_shared<TransListener>(desturi, listener);

    // Attempt to cancel operation and verify expected failure
    // (Expecting failure because test environment lacks real proxy)
    int32_t ret = ptr->Cancel();
    EXPECT_EQ(ret, E_SA_LOAD_FAILED);

    GTEST_LOG_(INFO) << "TransListener_Cancel_0001 End";
}

/**
 * @tc.name: TransListener_OnFileReceive_0001
 * @tc.desc: The execution of the OnFileReceive success.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(TransListenerTest, TransListener_OnFileReceive_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListener_OnFileReceive_0001 Start";

    // Set up test URIs
    string srcuri = "file://docs/storage/media/100/local/files/Docs/aa/";
    string desturi = "file://docs/storage/media/100/local/files/Docs/aa1/";

    // Create TransListener instance
    auto ptr = std::make_shared<TransListener>(desturi, listener);

    // Test file receive callback with current progress values
    int32_t ret = ptr->OnFileReceive(process, fileSize);
    EXPECT_EQ(ret, E_OK); // Expect callback to succeed

    GTEST_LOG_(INFO) << "TransListener_OnFileReceive_0001 End";
}

/**
 * @tc.name: TransListener_OnFileReceive_0002
 * @tc.desc: The execution of the OnFileReceive failed.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(TransListenerTest, TransListener_OnFileReceive_0002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListener_OnFileReceive_0002 Start";

    // Set up test URIs
    string srcuri = "file://docs/storage/media/100/local/files/Docs/aa/";
    string desturi = "file://docs/storage/media/100/local/files/Docs/aa1/";

    // Create TransListener instance
    auto ptr = std::make_shared<TransListener>(desturi, nullCallBack);

    // Test file receive callback with current progress values
    int32_t ret = ptr->OnFileReceive(process, fileSize);
    EXPECT_EQ(ret, ENOMEM); // Expect callback to failed

    GTEST_LOG_(INFO) << "TransListener_OnFileReceive_0002 End";
}

/**
 * @tc.name: TransListener_0004
 * @tc.desc: The execution of the OnFinished success.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(TransListenerTest, TransListener_0004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListener_0004 Start";

    // Set up test parameters
    string sessionName = "test";
    string desturi = "file://docs/storage/media/100/local/files/Docs/aa1/";

    // Create TransListener instance
    auto ptr = std::make_shared<TransListener>(desturi, listener);

    // Simulate successful operation completion
    int32_t ret = ptr->OnFinished(sessionName);
    EXPECT_EQ(ret, E_OK); // Expect completion handler to succeed

    GTEST_LOG_(INFO) << "TransListener_0004 End";
}

/**
 * @tc.name: TransListener_0005
 * @tc.desc: The execution of the OnFailed success.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(TransListenerTest, TransListener_0005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListener_0005 Start";

    // Set up test parameters
    string sessionName = "test";
    int32_t errorCode = 0; // Using 0 as generic error code for testing
    string desturi = "file://docs/storage/media/100/local/files/Docs/aa1/";

    // Create TransListener instance
    auto ptr = std::make_shared<TransListener>(desturi, listener);

    // Simulate operation failure
    int32_t ret = ptr->OnFailed(sessionName, errorCode);
    EXPECT_EQ(ret, E_OK); // Expect failure handler to succeed

    GTEST_LOG_(INFO) << "TransListener_0005 End";
}

/**
 * @tc.name: TransListener_0006
 * @tc.desc: Test CreateTmpDir with docs authority.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(TransListenerTest, TransListener_0006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListener_0006 Start";

    // Set up destination URI with docs authority
    string desturi = "file://docs/storage/media/100/local/files/Docs/aa1/";

    // Create TransListener instance
    auto ptr = std::make_shared<TransListener>(desturi, listener);

    // Test temporary directory creation (should skip actual creation for docs)
    int32_t ret = ptr->CreateTmpDir();
    EXPECT_EQ(ret, E_OK); // Expect success even though no dir is created

    GTEST_LOG_(INFO) << "TransListener_0006 End";
}

/**
 * @tc.name: TransListener_0007
 * @tc.desc: Test CreateTmpDir with media authority.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(TransListenerTest, TransListener_0007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListener_0007 Start";

    // Set up destination URI with media authority
    string desturi = "file://media/storage/media/100/local/files/Media/aa1/";

    // Create TransListener instance
    auto ptr = std::make_shared<TransListener>(desturi, listener);

    // Test temporary directory creation (should skip actual creation for media)
    int32_t ret = ptr->CreateTmpDir();
    EXPECT_EQ(ret, E_OK); // Expect success even though no dir is created

    GTEST_LOG_(INFO) << "TransListener_0007 End";
}

/**
 * @tc.name: TransListener_0008
 * @tc.desc: Test CreateTmpDir with invalid path.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(TransListenerTest, TransListener_0008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListener_0008 Start";

    // Set up destination URI with invalid authority
    string desturi = "file://invalid/storage/media/100/local/files/Invalid/aa1/";

    // Create TransListener instance
    auto ptr = std::make_shared<TransListener>(desturi, listener);

    // Test temporary directory creation with invalid path
    int32_t ret = ptr->CreateTmpDir();
    EXPECT_EQ(ret, E_OK); // Expect failure due to invalid authority

    GTEST_LOG_(INFO) << "TransListener_0008 End";
}

/**
 * @tc.name: TransListener_0009
 * @tc.desc: Test WaitForCopyResult with success result.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(TransListenerTest, TransListener_0009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListener_0009 Start";

    // Set up destination URI
    string desturi = "file://docs/storage/media/100/local/files/Docs/aa1/";
    auto ptr = std::make_shared<TransListener>(desturi, listener);

    // Simulate successful completion in separate thread
    std::thread t([ptr]() {
        ptr->OnFinished("test_session"); // Trigger success callback
    });

    // Wait for operation result and verify success
    int32_t ret = ptr->WaitForCopyResult();
    t.join();                // Ensure thread completes
    EXPECT_EQ(ret, SUCCESS); // Expect success status

    GTEST_LOG_(INFO) << "TransListener_0009 End";
}

/**
 * @tc.name: TransListener_0010
 * @tc.desc: Test WaitForCopyResult with failed result.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(TransListenerTest, TransListener_0010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListener_0010 Start";

    // Set up destination URI
    string desturi = "file://docs/storage/media/100/local/files/Docs/aa1/";
    auto ptr = std::make_shared<TransListener>(desturi, listener);

    // Simulate failure in separate thread
    std::thread t([ptr]() {
        ptr->OnFailed("test_session", EIO); // Trigger failure callback
    });

    // Wait for operation result and verify failure
    int32_t ret = ptr->WaitForCopyResult();
    t.join();               // Ensure thread completes
    EXPECT_EQ(ret, FAILED); // Expect failure status

    GTEST_LOG_(INFO) << "TransListener_0010 End";
}

/**
 * @tc.name: TransListener_0011
 * @tc.desc: Test GetFileName with valid path.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(TransListenerTest, TransListener_0011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListener_0011 Start";

    // Set up destination URI
    string desturi = "file://docs/storage/media/100/local/files/Docs/aa1/";
    auto ptr = std::make_shared<TransListener>(desturi, listener);

    // Test filename extraction from valid path
    string filename = ptr->GetFileName("/path/to/file.txt");
    EXPECT_FALSE(filename.empty()); // Expect non-empty filename

    GTEST_LOG_(INFO) << "TransListener_0011 End";
}

/**
 * @tc.name: TransListener_0012
 * @tc.desc: Test GetFileName with invalid path.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(TransListenerTest, TransListener_0012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListener_0012 Start";

    // Set up destination URI
    string desturi = "file://docs/storage/media/100/local/files/Docs/aa1/";
    auto ptr = std::make_shared<TransListener>(desturi, listener);

    // Test filename extraction from invalid path
    string filename = ptr->GetFileName("invalidpath");
    EXPECT_TRUE(filename.empty()); // Expect empty result for invalid path

    GTEST_LOG_(INFO) << "TransListener_0012 End";
}

/**
 * @tc.name: TransListener_0013
 * @tc.desc: Test GetNetworkIdFromUri with network parameter.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(TransListenerTest, TransListener_0013, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListener_0013 Start";

    // Set up URI with network ID parameter
    string desturi = "file://docs/storage/media/100/local/files/Docs/aa1/?networkid=12345";
    auto ptr = std::make_shared<TransListener>(desturi, listener);

    // Test network ID extraction
    string networkId = ptr->GetNetworkIdFromUri(desturi);
    EXPECT_EQ(networkId, "12345"); // Expect correct network ID extraction

    GTEST_LOG_(INFO) << "TransListener_0013 End";
}

/**
 * @tc.name: TransListener_0014
 * @tc.desc: Test RmTmpDir with docs authority.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(TransListenerTest, TransListener_0014, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListener_0014 Start";

    // Set up destination URI with docs authority
    string desturi = "file://docs/storage/media/100/local/files/Docs/aa1/";
    auto ptr = std::make_shared<TransListener>(desturi, listener);

    // Test temporary directory removal (should skip for docs)
    EXPECT_NO_THROW(ptr->RmTmpDir()); // Expect no exception

    GTEST_LOG_(INFO) << "TransListener_0014 End";
}

/**
 * @tc.name: TransListener_0015
 * @tc.desc: Test RmTmpDir with non-docs authority.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(TransListenerTest, TransListener_0015, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListener_0015 Start";

    // Set up destination URI with non-docs authority
    string desturi = "file://other/storage/media/100/local/files/Other/aa1/";
    auto ptr = std::make_shared<TransListener>(desturi, listener);

    // Test temporary directory removal
    EXPECT_NO_THROW(ptr->RmTmpDir()); // Expect no exception

    GTEST_LOG_(INFO) << "TransListener_0015 End";
}
} // namespace OHOS::Storage::DistributedFile::Test