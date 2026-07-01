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
const std::string UNSUPPORTED_LIST_FILE = "unsupported_list.json";
const std::string SYSTEM_FEATURE_FILE = "system_feature.json";
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
}

void DecompressConfigManagerTest::TearDownTestCase(void)
{
    std::string unsupportedListPath = TEST_CONFIG_DIR + "/" + UNSUPPORTED_LIST_FILE;
    std::string systemFeaturePath = TEST_CONFIG_DIR + "/" + SYSTEM_FEATURE_FILE;
    
    if (access(unsupportedListPath.c_str(), F_OK) == 0) {
        unlink(unsupportedListPath.c_str());
    }
    if (access(systemFeaturePath.c_str(), F_OK) == 0) {
        unlink(systemFeaturePath.c_str());
    }
}

void DecompressConfigManagerTest::SetUp(void)
{
    auto& manager = DecompressConfigManager::GetInstance();
    manager.SetConfigDir(TEST_CONFIG_DIR);
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
    std::string filePath = TEST_CONFIG_DIR + "/" + UNSUPPORTED_LIST_FILE;
    std::ofstream file(filePath);
    ASSERT_TRUE(file.is_open());
    
    nlohmann::json jsonObj;
    jsonObj["unsupportedList"] = {"com.test.app1", "com.test.app2", "com.test.app3"};
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
    std::string filePath = TEST_CONFIG_DIR + "/" + UNSUPPORTED_LIST_FILE;
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
    std::string filePath = TEST_CONFIG_DIR + "/" + UNSUPPORTED_LIST_FILE;
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
    std::string filePath = TEST_CONFIG_DIR + "/" + UNSUPPORTED_LIST_FILE;
    std::ofstream file(filePath);
    ASSERT_TRUE(file.is_open());
    
    nlohmann::json jsonObj;
    jsonObj["unsupportedList"] = nlohmann::json::array();
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
    std::string filePath = TEST_CONFIG_DIR + "/" + SYSTEM_FEATURE_FILE;
    std::ofstream file(filePath);
    ASSERT_TRUE(file.is_open());
    
    nlohmann::json jsonObj;
    jsonObj["systemFeature"] = true;
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
    std::string filePath = TEST_CONFIG_DIR + "/" + SYSTEM_FEATURE_FILE;
    std::ofstream file(filePath);
    ASSERT_TRUE(file.is_open());
    
    nlohmann::json jsonObj;
    jsonObj["systemFeature"] = false;
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
    std::string filePath = TEST_CONFIG_DIR + "/" + SYSTEM_FEATURE_FILE;
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
    std::string filePath = TEST_CONFIG_DIR + "/" + SYSTEM_FEATURE_FILE;
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
    std::string filePath = TEST_CONFIG_DIR + "/" + SYSTEM_FEATURE_FILE;
    
    auto& manager = DecompressConfigManager::GetInstance();
    
    std::ofstream file1(filePath);
    ASSERT_TRUE(file1.is_open());
    nlohmann::json jsonObj1;
    jsonObj1["systemFeature"] = true;
    file1 << jsonObj1.dump();
    file1.close();
    
    EXPECT_TRUE(manager.GetSystemFeature());
    
    std::ofstream file2(filePath);
    ASSERT_TRUE(file2.is_open());
    nlohmann::json jsonObj2;
    jsonObj2["systemFeature"] = false;
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