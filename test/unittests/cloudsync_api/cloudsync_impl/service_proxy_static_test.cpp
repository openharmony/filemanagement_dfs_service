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
#include <fstream>
#include <string>

#include "service_proxy.cpp"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;

class ServiceProxyStaticTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ServiceProxyStaticTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void ServiceProxyStaticTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void ServiceProxyStaticTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void ServiceProxyStaticTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: GetCallerBundleNameTest001
 * @tc.desc: Verify GetCallerBundleName returns valid bundle name
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(ServiceProxyStaticTest, GetCallerBundleNameTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCallerBundleNameTest001 Begin";
    try {
        std::string bundleName = GetCallerBundleName();
        
        EXPECT_NE(bundleName, "");
        EXPECT_NE(bundleName, "-");
        GTEST_LOG_(INFO) << "bundleName: " << bundleName;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCallerBundleNameTest001 Error";
    }
    GTEST_LOG_(INFO) << "GetCallerBundleNameTest001 End";
}

/**
 * @tc.name: GetCallerBundleNameTest002
 * @tc.desc: Verify GetCallerBundleName extracts last segment from path
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(ServiceProxyStaticTest, GetCallerBundleNameTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCallerBundleNameTest002 Begin";
    try {
        std::string bundleName = GetCallerBundleName();
        
        EXPECT_EQ(bundleName.find('/'), std::string::npos);
        GTEST_LOG_(INFO) << "bundleName: " << bundleName;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCallerBundleNameTest002 Error";
    }
    GTEST_LOG_(INFO) << "GetCallerBundleNameTest002 End";
}

/**
 * @tc.name: GetCallerBundleNameTest003
 * @tc.desc: Verify GetCallerBundleName does not throw exception
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(ServiceProxyStaticTest, GetCallerBundleNameTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCallerBundleNameTest003 Begin";
    try {
        std::string bundleName = GetCallerBundleName();
        
        EXPECT_TRUE(!bundleName.empty());
        GTEST_LOG_(INFO) << "bundleName: " << bundleName;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCallerBundleNameTest003 Error";
    }
    GTEST_LOG_(INFO) << "GetCallerBundleNameTest003 End";
}

/**
 * @tc.name: GetCallerBundleNameTest004
 * @tc.desc: Verify GetCallerBundleName handles path with multiple slashes
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(ServiceProxyStaticTest, GetCallerBundleNameTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCallerBundleNameTest004 Begin";
    try {
        std::ifstream cmdlineFile("/proc/self/cmdline");
        std::string rawName;
        std::getline(cmdlineFile, rawName, '\0');
        
        size_t lastSlash = rawName.find_last_of('/');
        std::string expectedName = rawName;
        if (lastSlash != std::string::npos) {
            expectedName = rawName.substr(lastSlash + 1);
        }
        if (expectedName.empty()) {
            expectedName = "-";
        }
        
        std::string bundleName = GetCallerBundleName();
        EXPECT_EQ(bundleName, expectedName);
        GTEST_LOG_(INFO) << "rawName: " << rawName << ", bundleName: " << bundleName;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCallerBundleNameTest004 Error";
    }
    GTEST_LOG_(INFO) << "GetCallerBundleNameTest004 End";
}

/**
 * @tc.name: GetCallerBundleNameTest005
 * @tc.desc: Verify GetCallerBundleName consistency across multiple calls
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(ServiceProxyStaticTest, GetCallerBundleNameTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCallerBundleNameTest005 Begin";
    try {
        std::string firstCall = GetCallerBundleName();
        std::string secondCall = GetCallerBundleName();
        std::string thirdCall = GetCallerBundleName();
        
        EXPECT_EQ(firstCall, secondCall);
        EXPECT_EQ(secondCall, thirdCall);
        GTEST_LOG_(INFO) << "All calls return: " << firstCall;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCallerBundleNameTest005 Error";
    }
    GTEST_LOG_(INFO) << "GetCallerBundleNameTest005 End";
}

} // namespace OHOS::FileManagement::CloudSync::Test