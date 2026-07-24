/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "decompress_config_manager.h"
#include "nlohmann/json.hpp"
#include "utils_log.h"

#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

using namespace testing::ext;
using namespace OHOS::FileManagement::CloudSync;

namespace {
const std::string TEST_CONFIG_DIR = "/data/service/el1/public/cloudfile/decompress_config";
const std::string TEST_SYS_CONFIG_DIR = "/data/service/el1/public/cloudfile/decompress_config_sys_test";
const std::string FAKE_DECOMPRESSION_FILE = "fake_decompress.json";
constexpr mode_t DIR_PERMISSION = 0711;
}

class DecompressConfigManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DecompressConfigManagerTest::SetUpTestCase(void)
{
    if (access(TEST_CONFIG_DIR.c_str(), F_OK) != 0) {
        mkdir(TEST_CONFIG_DIR.c_str(), DIR_PERMISSION);
    }
    if (access(TEST_SYS_CONFIG_DIR.c_str(), F_OK) != 0) {
        mkdir(TEST_SYS_CONFIG_DIR.c_str(), DIR_PERMISSION);
    }
}

void DecompressConfigManagerTest::TearDownTestCase(void)
{
    std::string filePath = TEST_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    if (access(filePath.c_str(), F_OK) == 0) {
        unlink(filePath.c_str());
    }
    std::string sysFilePath = TEST_SYS_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    if (access(sysFilePath.c_str(), F_OK) == 0) {
        unlink(sysFilePath.c_str());
    }
}

void DecompressConfigManagerTest::SetUp(void)
{
    auto& manager = DecompressConfigManager::GetInstance();
    manager.SetConfigDir(TEST_CONFIG_DIR);
    // sysConfigDir_ is private; the test target builds with `-Dprivate=public`
    // so it can be redirected to a controlled dir. Otherwise the dual-path
    // fallback would read the real /system/etc/decompress_config file on the
    // device and make the negative-case tests non-hermetic.
    manager.sysConfigDir_ = TEST_SYS_CONFIG_DIR;
    // Clean slate: ensure neither path carries a leftover config file so every
    // test starts hermetic regardless of declaration order.
    std::string defaultFile = TEST_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    std::string sysFile = TEST_SYS_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    if (access(defaultFile.c_str(), F_OK) == 0) {
        unlink(defaultFile.c_str());
    }
    if (access(sysFile.c_str(), F_OK) == 0) {
        unlink(sysFile.c_str());
    }
}

void DecompressConfigManagerTest::TearDown(void)
{
}

/**
 * @tc.name: GetUnsupportedList_001
 * @tc.desc: Test GetUnsupportedList when file exists with valid data
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressConfigManagerTest, GetUnsupportedList_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetUnsupportedList_001 begin";
    std::string filePath = TEST_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    std::ofstream file(filePath);
    ASSERT_TRUE(file.is_open());
    
    nlohmann::json jsonObj;
    jsonObj["unsupportedList"] = {"com.test.app1", "com.test.app2", "com.test.app3"};
    jsonObj["systemFeature"] = true;
    file << jsonObj.dump();
    file.close();
    
    auto& manager = DecompressConfigManager::GetInstance();
    std::vector<std::string> unsupportedList = manager.GetUnsupportedList();
    
    EXPECT_EQ(unsupportedList.size(), 3);
    EXPECT_EQ(unsupportedList[0], "com.test.app1");
    EXPECT_EQ(unsupportedList[1], "com.test.app2");
    EXPECT_EQ(unsupportedList[2], "com.test.app3");
    
    unlink(filePath.c_str());
    GTEST_LOG_(INFO) << "GetUnsupportedList_001 end";
}

/**
 * @tc.name: GetUnsupportedList_002
 * @tc.desc: Test GetUnsupportedList when file not exists
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressConfigManagerTest, GetUnsupportedList_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetUnsupportedList_002 begin";
    std::string filePath = TEST_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    if (access(filePath.c_str(), F_OK) == 0) {
        unlink(filePath.c_str());
    }
    
    auto& manager = DecompressConfigManager::GetInstance();
    std::vector<std::string> unsupportedList = manager.GetUnsupportedList();
    
    EXPECT_EQ(unsupportedList.size(), 0);
    GTEST_LOG_(INFO) << "GetUnsupportedList_002 end";
}

/**
 * @tc.name: GetUnsupportedList_003
 * @tc.desc: Test GetUnsupportedList when file has invalid format
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressConfigManagerTest, GetUnsupportedList_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetUnsupportedList_003 begin";
    std::string filePath = TEST_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    std::ofstream file(filePath);
    ASSERT_TRUE(file.is_open());
    
    file << "{invalid json format}";
    file.close();
    
    auto& manager = DecompressConfigManager::GetInstance();
    std::vector<std::string> unsupportedList = manager.GetUnsupportedList();
    
    EXPECT_EQ(unsupportedList.size(), 0);
    
    unlink(filePath.c_str());
    GTEST_LOG_(INFO) << "GetUnsupportedList_003 end";
}

/**
 * @tc.name: GetUnsupportedList_004
 * @tc.desc: Test GetUnsupportedList when file has empty list
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressConfigManagerTest, GetUnsupportedList_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetUnsupportedList_004 begin";
    std::string filePath = TEST_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    std::ofstream file(filePath);
    ASSERT_TRUE(file.is_open());
    
    nlohmann::json jsonObj;
    jsonObj["unsupportedList"] = nlohmann::json::array();
    jsonObj["systemFeature"] = true;
    file << jsonObj.dump();
    file.close();
    
    auto& manager = DecompressConfigManager::GetInstance();
    std::vector<std::string> unsupportedList = manager.GetUnsupportedList();
    
    EXPECT_EQ(unsupportedList.size(), 0);
    
    unlink(filePath.c_str());
    GTEST_LOG_(INFO) << "GetUnsupportedList_004 end";
}

/**
 * @tc.name: GetSystemFeature_001
 * @tc.desc: Test GetSystemFeature when file exists with true value
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressConfigManagerTest, GetSystemFeature_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSystemFeature_001 begin";
    std::string filePath = TEST_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    std::ofstream file(filePath);
    ASSERT_TRUE(file.is_open());
    
    nlohmann::json jsonObj;
    jsonObj["systemFeature"] = true;
    jsonObj["unsupportedList"] = nlohmann::json::array();
    file << jsonObj.dump();
    file.close();
    
    auto& manager = DecompressConfigManager::GetInstance();
    bool systemFeature = manager.GetSystemFeature();
    
    EXPECT_TRUE(systemFeature);
    
    unlink(filePath.c_str());
    GTEST_LOG_(INFO) << "GetSystemFeature_001 end";
}

/**
 * @tc.name: GetSystemFeature_002
 * @tc.desc: Test GetSystemFeature when file exists with false value
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressConfigManagerTest, GetSystemFeature_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSystemFeature_002 begin";
    std::string filePath = TEST_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    std::ofstream file(filePath);
    ASSERT_TRUE(file.is_open());
    
    nlohmann::json jsonObj;
    jsonObj["systemFeature"] = false;
    jsonObj["unsupportedList"] = nlohmann::json::array();
    file << jsonObj.dump();
    file.close();
    
    auto& manager = DecompressConfigManager::GetInstance();
    bool systemFeature = manager.GetSystemFeature();
    
    EXPECT_FALSE(systemFeature);
    
    unlink(filePath.c_str());
    GTEST_LOG_(INFO) << "GetSystemFeature_002 end";
}

/**
 * @tc.name: GetSystemFeature_003
 * @tc.desc: Test GetSystemFeature when file not exists
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressConfigManagerTest, GetSystemFeature_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSystemFeature_003 begin";
    std::string filePath = TEST_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    if (access(filePath.c_str(), F_OK) == 0) {
        unlink(filePath.c_str());
    }
    
    auto& manager = DecompressConfigManager::GetInstance();
    bool systemFeature = manager.GetSystemFeature();
    
    EXPECT_FALSE(systemFeature);
    GTEST_LOG_(INFO) << "GetSystemFeature_003 end";
}

/**
 * @tc.name: GetSystemFeature_004
 * @tc.desc: Test GetSystemFeature when file has invalid format
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressConfigManagerTest, GetSystemFeature_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSystemFeature_004 begin";
    std::string filePath = TEST_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    std::ofstream file(filePath);
    ASSERT_TRUE(file.is_open());
    
    file << "{invalid json format}";
    file.close();
    
    auto& manager = DecompressConfigManager::GetInstance();
    bool systemFeature = manager.GetSystemFeature();
    
    EXPECT_FALSE(systemFeature);
    
    unlink(filePath.c_str());
    GTEST_LOG_(INFO) << "GetSystemFeature_004 end";
}

/**
 * @tc.name: DynamicUpdate_001
 * @tc.desc: Test config file update is reflected immediately
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressConfigManagerTest, DynamicUpdate_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DynamicUpdate_001 begin";
    std::string filePath = TEST_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    
    auto& manager = DecompressConfigManager::GetInstance();
    
    std::ofstream file1(filePath);
    ASSERT_TRUE(file1.is_open());
    nlohmann::json jsonObj1;
    jsonObj1["systemFeature"] = true;
    jsonObj1["unsupportedList"] = nlohmann::json::array();
    file1 << jsonObj1.dump();
    file1.close();
    
    EXPECT_TRUE(manager.GetSystemFeature());
    
    std::ofstream file2(filePath);
    ASSERT_TRUE(file2.is_open());
    nlohmann::json jsonObj2;
    jsonObj2["systemFeature"] = false;
    jsonObj2["unsupportedList"] = nlohmann::json::array();
    file2 << jsonObj2.dump();
    file2.close();
    
    EXPECT_FALSE(manager.GetSystemFeature());
    
    unlink(filePath.c_str());
    GTEST_LOG_(INFO) << "DynamicUpdate_001 end";
}

/**
 * @tc.name: GetInstance_001
 * @tc.desc: Test GetInstance returns same instance (singleton pattern)
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressConfigManagerTest, GetInstance_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstance_001 begin";
    auto& instance1 = DecompressConfigManager::GetInstance();
    auto& instance2 = DecompressConfigManager::GetInstance();
    auto& instance3 = DecompressConfigManager::GetInstance();
    
    EXPECT_EQ(&instance1, &instance2);
    EXPECT_EQ(&instance2, &instance3);
    GTEST_LOG_(INFO) << "GetInstance_001 end";
}

/**
 * @tc.name: GetUnsupportedList_005
 * @tc.desc: Test GetUnsupportedList when file missing unsupportedList field
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressConfigManagerTest, GetUnsupportedList_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetUnsupportedList_005 begin";
    std::string filePath = TEST_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    std::ofstream file(filePath);
    ASSERT_TRUE(file.is_open());

    nlohmann::json jsonObj;
    jsonObj["systemFeature"] = true;
    file << jsonObj.dump();
    file.close();

    auto& manager = DecompressConfigManager::GetInstance();
    std::vector<std::string> unsupportedList = manager.GetUnsupportedList();

    EXPECT_EQ(unsupportedList.size(), 0);

    unlink(filePath.c_str());
    GTEST_LOG_(INFO) << "GetUnsupportedList_005 end";
}

/**
 * @tc.name: GetUnsupportedList_006
 * @tc.desc: Test GetUnsupportedList when unsupportedList is not an array
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressConfigManagerTest, GetUnsupportedList_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetUnsupportedList_006 begin";
    std::string filePath = TEST_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    std::ofstream file(filePath);
    ASSERT_TRUE(file.is_open());

    nlohmann::json jsonObj;
    jsonObj["unsupportedList"] = "not_an_array";
    jsonObj["systemFeature"] = true;
    file << jsonObj.dump();
    file.close();

    auto& manager = DecompressConfigManager::GetInstance();
    std::vector<std::string> unsupportedList = manager.GetUnsupportedList();

    EXPECT_EQ(unsupportedList.size(), 0);

    unlink(filePath.c_str());
    GTEST_LOG_(INFO) << "GetUnsupportedList_006 end";
}

/**
 * @tc.name: GetUnsupportedList_007
 * @tc.desc: Test GetUnsupportedList with mixed type items in array (only strings kept)
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressConfigManagerTest, GetUnsupportedList_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetUnsupportedList_007 begin";
    std::string filePath = TEST_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    std::ofstream file(filePath);
    ASSERT_TRUE(file.is_open());

    nlohmann::json jsonObj;
    jsonObj["unsupportedList"] = {"com.test.app1", 123, true, "com.test.app2"};
    jsonObj["systemFeature"] = true;
    file << jsonObj.dump();
    file.close();

    auto& manager = DecompressConfigManager::GetInstance();
    std::vector<std::string> unsupportedList = manager.GetUnsupportedList();

    EXPECT_EQ(unsupportedList.size(), 2);
    EXPECT_EQ(unsupportedList[0], "com.test.app1");
    EXPECT_EQ(unsupportedList[1], "com.test.app2");

    unlink(filePath.c_str());
    GTEST_LOG_(INFO) << "GetUnsupportedList_007 end";
}

/**
 * @tc.name: GetSystemFeature_005
 * @tc.desc: Test GetSystemFeature when file missing systemFeature field
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressConfigManagerTest, GetSystemFeature_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSystemFeature_005 begin";
    std::string filePath = TEST_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    std::ofstream file(filePath);
    ASSERT_TRUE(file.is_open());

    nlohmann::json jsonObj;
    jsonObj["unsupportedList"] = nlohmann::json::array();
    file << jsonObj.dump();
    file.close();

    auto& manager = DecompressConfigManager::GetInstance();
    bool systemFeature = manager.GetSystemFeature();

    EXPECT_FALSE(systemFeature);

    unlink(filePath.c_str());
    GTEST_LOG_(INFO) << "GetSystemFeature_005 end";
}

/**
 * @tc.name: GetSystemFeature_006
 * @tc.desc: Test GetSystemFeature when systemFeature is not a boolean
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressConfigManagerTest, GetSystemFeature_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSystemFeature_006 begin";
    std::string filePath = TEST_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    std::ofstream file(filePath);
    ASSERT_TRUE(file.is_open());

    nlohmann::json jsonObj;
    jsonObj["systemFeature"] = "not_a_boolean";
    jsonObj["unsupportedList"] = nlohmann::json::array();
    file << jsonObj.dump();
    file.close();

    auto& manager = DecompressConfigManager::GetInstance();
    bool systemFeature = manager.GetSystemFeature();

    EXPECT_FALSE(systemFeature);

    unlink(filePath.c_str());
    GTEST_LOG_(INFO) << "GetSystemFeature_006 end";
}

/**
 * @tc.name: CombinedConfig_001
 * @tc.desc: Test both unsupportedList and systemFeature read from same file
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressConfigManagerTest, CombinedConfig_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CombinedConfig_001 begin";
    std::string filePath = TEST_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    std::ofstream file(filePath);
    ASSERT_TRUE(file.is_open());

    nlohmann::json jsonObj;
    jsonObj["unsupportedList"] = {"com.test.app1", "com.test.app2"};
    jsonObj["systemFeature"] = true;
    file << jsonObj.dump();
    file.close();

    auto& manager = DecompressConfigManager::GetInstance();
    std::vector<std::string> unsupportedList = manager.GetUnsupportedList();
    bool systemFeature = manager.GetSystemFeature();

    EXPECT_EQ(unsupportedList.size(), 2);
    EXPECT_EQ(unsupportedList[0], "com.test.app1");
    EXPECT_EQ(unsupportedList[1], "com.test.app2");
    EXPECT_TRUE(systemFeature);

    unlink(filePath.c_str());
    GTEST_LOG_(INFO) << "CombinedConfig_001 end";
}

/**
 * @tc.name: CombinedConfig_002
 * @tc.desc: Test both unsupportedList and systemFeature with false systemFeature
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressConfigManagerTest, CombinedConfig_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CombinedConfig_002 begin";
    std::string filePath = TEST_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    std::ofstream file(filePath);
    ASSERT_TRUE(file.is_open());

    nlohmann::json jsonObj;
    jsonObj["unsupportedList"] = {"com.test.app3"};
    jsonObj["systemFeature"] = false;
    file << jsonObj.dump();
    file.close();

    auto& manager = DecompressConfigManager::GetInstance();
    std::vector<std::string> unsupportedList = manager.GetUnsupportedList();
    bool systemFeature = manager.GetSystemFeature();

    EXPECT_EQ(unsupportedList.size(), 1);
    EXPECT_EQ(unsupportedList[0], "com.test.app3");
    EXPECT_FALSE(systemFeature);

    unlink(filePath.c_str());
    GTEST_LOG_(INFO) << "CombinedConfig_002 end";
}

/**
 * @tc.name: ParseUnsupportedList_001
 * @tc.desc: Test ParseUnsupportedList with truncated JSON
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressConfigManagerTest, ParseUnsupportedList_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ParseUnsupportedList_001 begin";
    std::string filePath = TEST_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    std::ofstream file(filePath);
    ASSERT_TRUE(file.is_open());

    file << "{\"unsupportedList\":}";
    file.close();

    auto& manager = DecompressConfigManager::GetInstance();
    std::vector<std::string> unsupportedList = manager.GetUnsupportedList();

    EXPECT_EQ(unsupportedList.size(), 0);

    unlink(filePath.c_str());
    GTEST_LOG_(INFO) << "ParseUnsupportedList_001 end";
}

/**
 * @tc.name: ParseSystemFeature_001
 * @tc.desc: Test ParseSystemFeature with truncated JSON
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressConfigManagerTest, ParseSystemFeature_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ParseSystemFeature_001 begin";
    std::string filePath = TEST_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    std::ofstream file(filePath);
    ASSERT_TRUE(file.is_open());

    file << "{\"systemFeature\":}";
    file.close();

    auto& manager = DecompressConfigManager::GetInstance();
    bool systemFeature = manager.GetSystemFeature();

    EXPECT_FALSE(systemFeature);

    unlink(filePath.c_str());
    GTEST_LOG_(INFO) << "ParseSystemFeature_001 end";
}

/**
 * @tc.name: GetUnsupportedList_DefaultPriority_001
 * @tc.desc: Both default and system config exist; default takes priority
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressConfigManagerTest, GetUnsupportedList_DefaultPriority_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetUnsupportedList_DefaultPriority_001 begin";
    auto& manager = DecompressConfigManager::GetInstance();

    nlohmann::json sysJson;
    sysJson["unsupportedList"] = {"com.sys.app"};
    sysJson["systemFeature"] = true;
    std::string sysFile = TEST_SYS_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    std::ofstream sysOfs(sysFile);
    ASSERT_TRUE(sysOfs.is_open());
    sysOfs << sysJson.dump();
    sysOfs.close();

    nlohmann::json defJson;
    defJson["unsupportedList"] = {"com.default.app1", "com.default.app2"};
    defJson["systemFeature"] = true;
    std::string defFile = TEST_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    std::ofstream defOfs(defFile);
    ASSERT_TRUE(defOfs.is_open());
    defOfs << defJson.dump();
    defOfs.close();

    std::vector<std::string> unsupportedList = manager.GetUnsupportedList();
    EXPECT_EQ(unsupportedList.size(), 2);
    EXPECT_EQ(unsupportedList[0], "com.default.app1");
    EXPECT_EQ(unsupportedList[1], "com.default.app2");

    unlink(defFile.c_str());
    unlink(sysFile.c_str());
    GTEST_LOG_(INFO) << "GetUnsupportedList_DefaultPriority_001 end";
}

/**
 * @tc.name: GetUnsupportedList_FallbackToSys_001
 * @tc.desc: Default config missing; falls back to system config
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressConfigManagerTest, GetUnsupportedList_FallbackToSys_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetUnsupportedList_FallbackToSys_001 begin";
    auto& manager = DecompressConfigManager::GetInstance();

    // default file intentionally absent (SetUp guarantees a clean slate)
    nlohmann::json sysJson;
    sysJson["unsupportedList"] = {"com.sys.app1", "com.sys.app2"};
    sysJson["systemFeature"] = true;
    std::string sysFile = TEST_SYS_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    std::ofstream sysOfs(sysFile);
    ASSERT_TRUE(sysOfs.is_open());
    sysOfs << sysJson.dump();
    sysOfs.close();

    std::vector<std::string> unsupportedList = manager.GetUnsupportedList();
    EXPECT_EQ(unsupportedList.size(), 2);
    EXPECT_EQ(unsupportedList[0], "com.sys.app1");
    EXPECT_EQ(unsupportedList[1], "com.sys.app2");

    unlink(sysFile.c_str());
    GTEST_LOG_(INFO) << "GetUnsupportedList_FallbackToSys_001 end";
}

/**
 * @tc.name: GetUnsupportedList_DefaultParseFailFallsBack_001
 * @tc.desc: Default config has invalid JSON; falls back to system config
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressConfigManagerTest, GetUnsupportedList_DefaultParseFailFallsBack_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetUnsupportedList_DefaultParseFailFallsBack_001 begin";
    auto& manager = DecompressConfigManager::GetInstance();

    std::string defFile = TEST_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    std::ofstream defOfs(defFile);
    ASSERT_TRUE(defOfs.is_open());
    defOfs << "{invalid json}";
    defOfs.close();

    nlohmann::json sysJson;
    sysJson["unsupportedList"] = {"com.sys.app1"};
    sysJson["systemFeature"] = true;
    std::string sysFile = TEST_SYS_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    std::ofstream sysOfs(sysFile);
    ASSERT_TRUE(sysOfs.is_open());
    sysOfs << sysJson.dump();
    sysOfs.close();

    std::vector<std::string> unsupportedList = manager.GetUnsupportedList();
    EXPECT_EQ(unsupportedList.size(), 1);
    EXPECT_EQ(unsupportedList[0], "com.sys.app1");

    unlink(defFile.c_str());
    unlink(sysFile.c_str());
    GTEST_LOG_(INFO) << "GetUnsupportedList_DefaultParseFailFallsBack_001 end";
}

/**
 * @tc.name: GetUnsupportedList_BothMissing_001
 * @tc.desc: Neither default nor system config exists; returns empty list
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressConfigManagerTest, GetUnsupportedList_BothMissing_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetUnsupportedList_BothMissing_001 begin";
    auto& manager = DecompressConfigManager::GetInstance();
    // both files absent (SetUp guarantees a clean slate)
    std::vector<std::string> unsupportedList = manager.GetUnsupportedList();
    EXPECT_EQ(unsupportedList.size(), 0);
    GTEST_LOG_(INFO) << "GetUnsupportedList_BothMissing_001 end";
}

/**
 * @tc.name: GetSystemFeature_DefaultPriority_001
 * @tc.desc: Both default and system config exist; default takes priority
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressConfigManagerTest, GetSystemFeature_DefaultPriority_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSystemFeature_DefaultPriority_001 begin";
    auto& manager = DecompressConfigManager::GetInstance();

    nlohmann::json sysJson;
    sysJson["systemFeature"] = false;
    sysJson["unsupportedList"] = nlohmann::json::array();
    std::string sysFile = TEST_SYS_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    std::ofstream sysOfs(sysFile);
    ASSERT_TRUE(sysOfs.is_open());
    sysOfs << sysJson.dump();
    sysOfs.close();

    nlohmann::json defJson;
    defJson["systemFeature"] = true;
    defJson["unsupportedList"] = nlohmann::json::array();
    std::string defFile = TEST_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    std::ofstream defOfs(defFile);
    ASSERT_TRUE(defOfs.is_open());
    defOfs << defJson.dump();
    defOfs.close();

    EXPECT_TRUE(manager.GetSystemFeature());

    unlink(defFile.c_str());
    unlink(sysFile.c_str());
    GTEST_LOG_(INFO) << "GetSystemFeature_DefaultPriority_001 end";
}

/**
 * @tc.name: GetSystemFeature_FallbackToSys_001
 * @tc.desc: Default config missing; falls back to system config
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressConfigManagerTest, GetSystemFeature_FallbackToSys_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSystemFeature_FallbackToSys_001 begin";
    auto& manager = DecompressConfigManager::GetInstance();

    nlohmann::json sysJson;
    sysJson["systemFeature"] = true;
    sysJson["unsupportedList"] = nlohmann::json::array();
    std::string sysFile = TEST_SYS_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    std::ofstream sysOfs(sysFile);
    ASSERT_TRUE(sysOfs.is_open());
    sysOfs << sysJson.dump();
    sysOfs.close();

    EXPECT_TRUE(manager.GetSystemFeature());

    unlink(sysFile.c_str());
    GTEST_LOG_(INFO) << "GetSystemFeature_FallbackToSys_001 end";
}

/**
 * @tc.name: GetSystemFeature_DefaultParseFailFallsBack_001
 * @tc.desc: Default config has invalid JSON; falls back to system config
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressConfigManagerTest, GetSystemFeature_DefaultParseFailFallsBack_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSystemFeature_DefaultParseFailFallsBack_001 begin";
    auto& manager = DecompressConfigManager::GetInstance();

    std::string defFile = TEST_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    std::ofstream defOfs(defFile);
    ASSERT_TRUE(defOfs.is_open());
    defOfs << "{invalid json}";
    defOfs.close();

    nlohmann::json sysJson;
    sysJson["systemFeature"] = true;
    sysJson["unsupportedList"] = nlohmann::json::array();
    std::string sysFile = TEST_SYS_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    std::ofstream sysOfs(sysFile);
    ASSERT_TRUE(sysOfs.is_open());
    sysOfs << sysJson.dump();
    sysOfs.close();

    EXPECT_TRUE(manager.GetSystemFeature());

    unlink(defFile.c_str());
    unlink(sysFile.c_str());
    GTEST_LOG_(INFO) << "GetSystemFeature_DefaultParseFailFallsBack_001 end";
}

/**
 * @tc.name: GetSystemFeature_BothMissing_001
 * @tc.desc: Neither default nor system config exists; returns false
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressConfigManagerTest, GetSystemFeature_BothMissing_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSystemFeature_BothMissing_001 begin";
    auto& manager = DecompressConfigManager::GetInstance();
    EXPECT_FALSE(manager.GetSystemFeature());
    GTEST_LOG_(INFO) << "GetSystemFeature_BothMissing_001 end";
}

/**
 * @tc.name: GetUnsupportedList_TruncateAtMax_001
 * @tc.desc: Test GetUnsupportedList truncates when the list exceeds the max size (1000)
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressConfigManagerTest, GetUnsupportedList_TruncateAtMax_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetUnsupportedList_TruncateAtMax_001 begin";
    std::string filePath = TEST_CONFIG_DIR + "/" + FAKE_DECOMPRESSION_FILE;
    std::ofstream file(filePath);
    ASSERT_TRUE(file.is_open());

    nlohmann::json::array_t arr;
    for (int i = 0; i < 1005; ++i) {
        arr.push_back("com.app." + std::to_string(i));
    }
    nlohmann::json jsonObj;
    jsonObj["unsupportedList"] = arr;
    jsonObj["systemFeature"] = true;
    file << jsonObj.dump();
    file.close();

    auto& manager = DecompressConfigManager::GetInstance();
    std::vector<std::string> unsupportedList = manager.GetUnsupportedList();
    EXPECT_EQ(unsupportedList.size(), 1000);

    unlink(filePath.c_str());
    GTEST_LOG_(INFO) << "GetUnsupportedList_TruncateAtMax_001 end";
}