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

#include "all_connect/all_connect_manager.h"

#include <memory>
#include <thread>

#include "dfs_error.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "library_func_mock.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
int32_t CollaborationMockFail(ServiceCollaborationManager_API *exportapi)
{
    return -1;
}

int32_t CollaborationMockSuccess(ServiceCollaborationManager_API *exportapi)
{
    return 0;
}
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace OHOS::FileManagement;

class AllConnectManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

public:
    static inline std::shared_ptr<LibraryFuncMock> libraryFuncMock_ = nullptr;
    static inline std::string peerNetworkId = "testNetworkId";
};

void AllConnectManagerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "AllConnectManagerTest SetUpTestCase";
    libraryFuncMock_ = std::make_shared<LibraryFuncMock>();
    LibraryFuncMock::libraryFunc_ = libraryFuncMock_;
}

void AllConnectManagerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "AllConnectManagerTest TearDownTestCase";
    LibraryFuncMock::libraryFunc_ = nullptr;
    libraryFuncMock_ = nullptr;
}

void AllConnectManagerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void AllConnectManagerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: AllConnectManagerTest_InitAllConnectManager_001
 * @tc.desc: verify InitAllConnectManager GetAllConnectSoLoad fail.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, InitAllConnectManager_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitAllConnectManager_001 start";
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();

        EXPECT_CALL(*libraryFuncMock_, realpath(_, _)).WillOnce(Return(nullptr));

        int32_t ret = allConnectManager.InitAllConnectManager();
        EXPECT_EQ(ret, FileManagement::ERR_DLOPEN);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "InitAllConnectManager_001 end";
}

/**
 * @tc.name: AllConnectManagerTest_InitAllConnectManager_002
 * @tc.desc: verify InitAllConnectManager RegisterLifecycleCallback fail.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, InitAllConnectManager_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitAllConnectManager_002 start";
    char soPath[] = "test.so";
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();

        EXPECT_CALL(*libraryFuncMock_, realpath(_, _)).WillOnce(Return(soPath));
        EXPECT_CALL(*libraryFuncMock_, dlopen(_, _)).WillOnce(Return(reinterpret_cast<void *>(0x1)));
        EXPECT_CALL(*libraryFuncMock_, dlsym(_, _))
            .WillOnce(Return(reinterpret_cast<void *>(&CollaborationMockSuccess)));
        allConnectManager.allConnect_.ServiceCollaborationManager_RegisterLifecycleCallback = nullptr;

        int32_t ret = allConnectManager.InitAllConnectManager();
        EXPECT_EQ(ret, FileManagement::ERR_DLOPEN);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "InitAllConnectManager_002 end";
}

/**
 * @tc.name: AllConnectManagerTest_InitAllConnectManager_003
 * @tc.desc: verify InitAllConnectManager success.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, InitAllConnectManager_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitAllConnectManager_003 start";
    char soPath[] = "test.so";
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();

        EXPECT_CALL(*libraryFuncMock_, realpath(_, _)).WillOnce(Return(soPath));
        EXPECT_CALL(*libraryFuncMock_, dlopen(_, _)).WillOnce(Return(reinterpret_cast<void *>(0x1)));
        EXPECT_CALL(*libraryFuncMock_, dlsym(_, _))
            .WillOnce(Return(reinterpret_cast<void *>(&CollaborationMockSuccess)));
        allConnectManager.allConnect_.ServiceCollaborationManager_RegisterLifecycleCallback = [](
            const char *serviceName,
            ServiceCollaborationManager_Callback *callback) -> int32_t {
            return 0;
        };

        int32_t ret = allConnectManager.InitAllConnectManager();
        EXPECT_EQ(ret, FileManagement::ERR_OK);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "InitAllConnectManager_003 end";
}

/**
 * @tc.name: AllConnectManagerTest_UnInitAllConnectManager_001
 * @tc.desc: verify UnRegisterLifecycleCallback fail, UnInitAllConnectManager succcess.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, UnInitAllConnectManager_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnInitAllConnectManager_001 start";
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();

        allConnectManager.dllHandle_ = nullptr;
        EXPECT_CALL(*libraryFuncMock_, dlclose(_)).WillOnce(Return(0));


        int32_t ret = allConnectManager.UnInitAllConnectManager();
        EXPECT_EQ(ret, FileManagement::ERR_OK);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "UnInitAllConnectManager_001 end";
}

/**
 * @tc.name: AllConnectManagerTest_PublishServiceState_001
 * @tc.desc: verify PublishServiceState dllHandle_ is nullptr.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, PublishServiceState_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PublishServiceState_001 start";
    ServiceCollaborationManagerBussinessStatus state = ServiceCollaborationManagerBussinessStatus::SCM_IDLE;
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();

        allConnectManager.dllHandle_ = nullptr;

        int32_t ret = allConnectManager.PublishServiceState(peerNetworkId, state);
        EXPECT_EQ(ret, FileManagement::ERR_OK);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "PublishServiceState_001 end";
}

/**
 * @tc.name: AllConnectManagerTest_PublishServiceState_002
 * @tc.desc: verify PublishServiceState allConnect_ is nullptr.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, PublishServiceState_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PublishServiceState_002 start";
    ServiceCollaborationManagerBussinessStatus state = ServiceCollaborationManagerBussinessStatus::SCM_IDLE;
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();

        allConnectManager.dllHandle_ = (void *)0x1;
        allConnectManager.allConnect_.ServiceCollaborationManager_PublishServiceState = nullptr;

        int32_t ret = allConnectManager.PublishServiceState(peerNetworkId, state);
        EXPECT_EQ(ret, FileManagement::ERR_DLOPEN);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "PublishServiceState_002 end";
}

/**
 * @tc.name: AllConnectManagerTest_PublishServiceState_003
 * @tc.desc: verify PublishServiceState fail.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, PublishServiceState_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PublishServiceState_003 start";
    ServiceCollaborationManagerBussinessStatus state = ServiceCollaborationManagerBussinessStatus::SCM_IDLE;
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();

        allConnectManager.dllHandle_ = (void *)0x1;
        allConnectManager.allConnect_.ServiceCollaborationManager_PublishServiceState = [](
            const char *peerNetworkId,
            const char *serviceName,
            const char *extraInfo,
            ServiceCollaborationManagerBussinessStatus state) -> int32_t {
            return 1;
        };

        int32_t ret = allConnectManager.PublishServiceState(peerNetworkId, state);
        EXPECT_EQ(ret, FileManagement::ERR_PUBLISH_STATE);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "PublishServiceState_003 end";
}

/**
 * @tc.name: AllConnectManagerTest_PublishServiceState_004
 * @tc.desc: verify PublishServiceState success.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, PublishServiceState_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PublishServiceState_004 start";
    ServiceCollaborationManagerBussinessStatus state = ServiceCollaborationManagerBussinessStatus::SCM_IDLE;
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();

        allConnectManager.dllHandle_ = (void *)0x1;
        allConnectManager.allConnect_.ServiceCollaborationManager_PublishServiceState = [](
            const char *peerNetworkId,
            const char *serviceName,
            const char *extraInfo,
            ServiceCollaborationManagerBussinessStatus state) -> int32_t {
            return 0;
        };

        int32_t ret = allConnectManager.PublishServiceState(peerNetworkId, state);
        EXPECT_EQ(ret, FileManagement::ERR_OK);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "PublishServiceState_004 end";
}

/**
 * @tc.name: AllConnectManagerTest_ApplyAdvancedResource_001
 * @tc.desc: verify ApplyAdvancedResource dllHandle_ is nullptr.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, ApplyAdvancedResource_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ApplyAdvancedResource_001 start";
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();

        allConnectManager.dllHandle_ = nullptr;

        int32_t ret = allConnectManager.ApplyAdvancedResource(peerNetworkId, nullptr);
        EXPECT_EQ(ret, FileManagement::ERR_OK);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "ApplyAdvancedResource_001 end";
}

/**
 * @tc.name: AllConnectManagerTest_ApplyAdvancedResource_002
 * @tc.desc: verify ApplyAdvancedResource allConnect_ is nullptr.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, ApplyAdvancedResource_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ApplyAdvancedResource_002 start";
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();

        allConnectManager.dllHandle_ = (void *)0x1;
        allConnectManager.allConnect_.ServiceCollaborationManager_ApplyAdvancedResource = nullptr;

        int32_t ret = allConnectManager.ApplyAdvancedResource(peerNetworkId, nullptr);
        EXPECT_EQ(ret, FileManagement::ERR_DLOPEN);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "ApplyAdvancedResource_002 end";
}

/**
 * @tc.name: AllConnectManagerTest_ApplyAdvancedResource_003
 * @tc.desc: verify ApplyAdvancedResource fail.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, ApplyAdvancedResource_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ApplyAdvancedResource_003 start";
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();

        allConnectManager.dllHandle_ = (void *)0x1;
        allConnectManager.applyResultBlock_ = nullptr;
        allConnectManager.allConnect_.ServiceCollaborationManager_ApplyAdvancedResource = [](
            const char *peerNetworkId,
            const char *serviceName,
            ServiceCollaborationManager_ResourceRequestInfoSets *resourceRequest,
            ServiceCollaborationManager_Callback *callback) -> int32_t {
            return 1;
        };

        int32_t ret = allConnectManager.ApplyAdvancedResource(peerNetworkId, nullptr);
        EXPECT_EQ(ret, FileManagement::ERR_APPLY_RESULT);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "ApplyAdvancedResource_003 end";
}

/**
 * @tc.name: AllConnectManagerTest_ApplyAdvancedResource_004
 * @tc.desc: verify ApplyAdvancedResource fail.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, ApplyAdvancedResource_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ApplyAdvancedResource_004 start";
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();

        allConnectManager.dllHandle_ = (void *)0x1;
        allConnectManager.applyResultBlock_ = nullptr;
        allConnectManager.allConnect_.ServiceCollaborationManager_ApplyAdvancedResource = [](
            const char *peerNetworkId,
            const char *serviceName,
            ServiceCollaborationManager_ResourceRequestInfoSets *resourceRequest,
            ServiceCollaborationManager_Callback *callback) -> int32_t {
            return 0;
        };
        std::thread([&allConnectManager]() {
            int32_t num = 10;
            while (allConnectManager.applyResultBlock_ == nullptr && num-- > 0) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            if (allConnectManager.applyResultBlock_ == nullptr) {
                allConnectManager.applyResultBlock_ = std::make_shared<BlockObject<bool>>(
                    AllConnectManager::BLOCK_INTERVAL_ALLCONNECT, false);
            }
            allConnectManager.applyResultBlock_->SetValue(false);
        }).detach();

        int32_t ret = allConnectManager.ApplyAdvancedResource(peerNetworkId, nullptr);
        EXPECT_EQ(ret, FileManagement::ERR_APPLY_RESULT);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "ApplyAdvancedResource_004 end";
}

/**
 * @tc.name: AllConnectManagerTest_ApplyAdvancedResource_005
 * @tc.desc: verify ApplyAdvancedResource success.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, ApplyAdvancedResource_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ApplyAdvancedResource_005 start";
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();

        allConnectManager.dllHandle_ = (void *)0x1;
        allConnectManager.applyResultBlock_ = nullptr;
        allConnectManager.allConnect_.ServiceCollaborationManager_ApplyAdvancedResource = [](
            const char *peerNetworkId,
            const char *serviceName,
            ServiceCollaborationManager_ResourceRequestInfoSets *resourceRequest,
            ServiceCollaborationManager_Callback *callback) -> int32_t {
            return 0;
        };
        std::thread([&allConnectManager]() {
            int32_t num = 10;
            while (allConnectManager.applyResultBlock_ == nullptr && num-- > 0) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            if (allConnectManager.applyResultBlock_ == nullptr) {
                allConnectManager.applyResultBlock_ = std::make_shared<BlockObject<bool>>(
                    AllConnectManager::BLOCK_INTERVAL_ALLCONNECT, false);
            }
            allConnectManager.applyResultBlock_->SetValue(true);
        }).detach();

        int32_t ret = allConnectManager.ApplyAdvancedResource(peerNetworkId, nullptr);
        EXPECT_EQ(ret, FileManagement::ERR_OK);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "ApplyAdvancedResource_005 end";
}

/**
 * @tc.name: AllConnectManagerTest_GetAllConnectSoLoad_001
 * @tc.desc: verify GetAllConnectSoLoad fail.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, GetAllConnectSoLoad_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAllConnectSoLoad_001 start";
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();

        EXPECT_CALL(*libraryFuncMock_, realpath(_, _)).WillOnce(Return(nullptr));

        int32_t ret = allConnectManager.GetAllConnectSoLoad();
        EXPECT_EQ(ret, FileManagement::ERR_DLOPEN);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "GetAllConnectSoLoad_001 end";
}

/**
 * @tc.name: AllConnectManagerTest_GetAllConnectSoLoad_002
 * @tc.desc: verify GetAllConnectSoLoad fail.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, GetAllConnectSoLoad_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAllConnectSoLoad_002 start";
    char soPath[] = "test.so";
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();
        
        EXPECT_CALL(*libraryFuncMock_, realpath(_, _)).WillOnce(Return(soPath));
        EXPECT_CALL(*libraryFuncMock_, dlopen(_, _)).WillOnce(Return(reinterpret_cast<void *>(0x0)));

        int32_t ret = allConnectManager.GetAllConnectSoLoad();
        EXPECT_EQ(ret, FileManagement::ERR_DLOPEN);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "GetAllConnectSoLoad_002 end";
}

/**
 * @tc.name: AllConnectManagerTest_GetAllConnectSoLoad_003
 * @tc.desc: verify GetAllConnectSoLoad fail.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, GetAllConnectSoLoad_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAllConnectSoLoad_003 start";
    char soPath[] = "test.so";
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();
        
        EXPECT_CALL(*libraryFuncMock_, realpath(_, _)).WillOnce(Return(soPath));
        EXPECT_CALL(*libraryFuncMock_, dlopen(_, _)).WillOnce(Return(reinterpret_cast<void *>(0x1)));
        EXPECT_CALL(*libraryFuncMock_, dlsym(_, _)).WillOnce(Return(nullptr));
        EXPECT_CALL(*libraryFuncMock_, dlclose(_)).WillOnce(Return(0));

        int32_t ret = allConnectManager.GetAllConnectSoLoad();
        EXPECT_EQ(ret, FileManagement::ERR_DLOPEN);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "GetAllConnectSoLoad_003 end";
}

/**
 * @tc.name: AllConnectManagerTest_GetAllConnectSoLoad_004
 * @tc.desc: verify GetAllConnectSoLoad fail.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, GetAllConnectSoLoad_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAllConnectSoLoad_004 start";
    char soPath[] = "test.so";
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();
        
        EXPECT_CALL(*libraryFuncMock_, realpath(_, _)).WillOnce(Return(soPath));
        EXPECT_CALL(*libraryFuncMock_, dlopen(_, _)).WillOnce(Return(reinterpret_cast<void *>(0x1)));
        EXPECT_CALL(*libraryFuncMock_, dlsym(_, _))
            .WillOnce(Return(reinterpret_cast<void *>(&CollaborationMockFail)));
        EXPECT_CALL(*libraryFuncMock_, dlclose(_)).WillOnce(Return(0));

        int32_t ret = allConnectManager.GetAllConnectSoLoad();
        EXPECT_EQ(ret, FileManagement::ERR_DLOPEN);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "GetAllConnectSoLoad_004 end";
}

/**
 * @tc.name: AllConnectManagerTest_GetAllConnectSoLoad_005
 * @tc.desc: verify GetAllConnectSoLoad success.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, GetAllConnectSoLoad_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAllConnectSoLoad_005 start";
    char soPath[] = "test.so";
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();
        
        EXPECT_CALL(*libraryFuncMock_, realpath(_, _)).WillOnce(Return(soPath));
        EXPECT_CALL(*libraryFuncMock_, dlopen(_, _)).WillOnce(Return(reinterpret_cast<void *>(0x1)));
        EXPECT_CALL(*libraryFuncMock_, dlsym(_, _))
            .WillOnce(Return(reinterpret_cast<void *>(&CollaborationMockSuccess)));

        int32_t ret = allConnectManager.GetAllConnectSoLoad();
        EXPECT_EQ(ret, FileManagement::ERR_OK);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "GetAllConnectSoLoad_005 end";
}

/**
 * @tc.name: AllConnectManagerTest_RegisterLifecycleCallback_001
 * @tc.desc: verify RegisterLifecycleCallback dllHandle_ is nullptr.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, RegisterLifecycleCallback_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterLifecycleCallback_001 start";
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();
        
        allConnectManager.dllHandle_ = nullptr;
        
        int32_t ret = allConnectManager.RegisterLifecycleCallback();
        EXPECT_EQ(ret, FileManagement::ERR_DLOPEN);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "RegisterLifecycleCallback_001 end";
}

/**
 * @tc.name: AllConnectManagerTest_RegisterLifecycleCallback_002
 * @tc.desc: verify RegisterLifecycleCallback allConnect_ is nullptr.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, RegisterLifecycleCallback_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterLifecycleCallback_002 start";
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();
        
        allConnectManager.dllHandle_ = (void *)0x1;
        allConnectManager.allConnect_.ServiceCollaborationManager_RegisterLifecycleCallback = nullptr;
        
        int32_t ret = allConnectManager.RegisterLifecycleCallback();
        EXPECT_EQ(ret, FileManagement::ERR_DLOPEN);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "RegisterLifecycleCallback_002 end";
}

/**
 * @tc.name: AllConnectManagerTest_RegisterLifecycleCallback_003
 * @tc.desc: verify RegisterLifecycleCallback fail.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, RegisterLifecycleCallback_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterLifecycleCallback_003 start";
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();
        
        allConnectManager.dllHandle_ = (void *)0x1;
        allConnectManager.allConnect_.ServiceCollaborationManager_RegisterLifecycleCallback = [](
            const char *serviceName,
            ServiceCollaborationManager_Callback *callback) -> int32_t {
            return 1;
        };
        
        int32_t ret = allConnectManager.RegisterLifecycleCallback();
        EXPECT_EQ(ret, FileManagement::ERR_ALLCONNECT);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "RegisterLifecycleCallback_003 end";
}

/**
 * @tc.name: AllConnectManagerTest_RegisterLifecycleCallback_004
 * @tc.desc: verify RegisterLifecycleCallback success.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, RegisterLifecycleCallback_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterLifecycleCallback_004 start";
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();
        
        allConnectManager.dllHandle_ = (void *)0x1;
        allConnectManager.allConnect_.ServiceCollaborationManager_RegisterLifecycleCallback = [](
            const char *serviceName,
            ServiceCollaborationManager_Callback *callback) -> int32_t {
            return 0;
        };
        
        int32_t ret = allConnectManager.RegisterLifecycleCallback();
        EXPECT_EQ(ret, FileManagement::ERR_OK);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "RegisterLifecycleCallback_004 end";
}

/**
 * @tc.name: AllConnectManagerTest_UnRegisterLifecycleCallback_001
 * @tc.desc: verify UnRegisterLifecycleCallback dllHandle_ is nullptr.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, UnRegisterLifecycleCallback_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnRegisterLifecycleCallback_001 start";
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();
        
        allConnectManager.dllHandle_ = nullptr;
        
        int32_t ret = allConnectManager.UnRegisterLifecycleCallback();
        EXPECT_EQ(ret, FileManagement::ERR_DLOPEN);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "UnRegisterLifecycleCallback_001 end";
}

/**
 * @tc.name: AllConnectManagerTest_UnRegisterLifecycleCallback_002
 * @tc.desc: verify UnRegisterLifecycleCallback allConnect_ is nullptr.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, UnRegisterLifecycleCallback_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnRegisterLifecycleCallback_002 start";
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();
        
        allConnectManager.dllHandle_ = (void *)0x1;
        allConnectManager.allConnect_.ServiceCollaborationManager_UnRegisterLifecycleCallback = nullptr;
        
        int32_t ret = allConnectManager.UnRegisterLifecycleCallback();
        EXPECT_EQ(ret, FileManagement::ERR_DLOPEN);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "UnRegisterLifecycleCallback_002 end";
}

/**
 * @tc.name: AllConnectManagerTest_UnRegisterLifecycleCallback_003
 * @tc.desc: verify UnRegisterLifecycleCallback fail.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, UnRegisterLifecycleCallback_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnRegisterLifecycleCallback_003 start";
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();
        
        allConnectManager.dllHandle_ = (void *)0x1;
        allConnectManager.allConnect_.ServiceCollaborationManager_UnRegisterLifecycleCallback = [](
            const char *serviceName) -> int32_t {
            return 1;
        };
        
        int32_t ret = allConnectManager.UnRegisterLifecycleCallback();
        EXPECT_EQ(ret, FileManagement::ERR_ALLCONNECT);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "UnRegisterLifecycleCallback_003 end";
}

/**
 * @tc.name: AllConnectManagerTest_UnRegisterLifecycleCallback_004
 * @tc.desc: verify UnRegisterLifecycleCallback success.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, UnRegisterLifecycleCallback_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnRegisterLifecycleCallback_004 start";
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();
        
        allConnectManager.dllHandle_ = (void *)0x1;
        allConnectManager.allConnect_.ServiceCollaborationManager_UnRegisterLifecycleCallback = [](
            const char *serviceName) -> int32_t {
            return 0;
        };
        
        int32_t ret = allConnectManager.UnRegisterLifecycleCallback();
        EXPECT_EQ(ret, FileManagement::ERR_OK);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "UnRegisterLifecycleCallback_004 end";
}

/**
 * @tc.name: AllConnectManagerTest_OnStop_001
 * @tc.desc: verify OnStop.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, OnStop_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStop_001 start";
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();
        
        int32_t ret = allConnectManager.OnStop("");
        EXPECT_EQ(ret, FileManagement::ERR_OK);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "OnStop_001 end";
}

/**
 * @tc.name: AllConnectManagerTest_ApplyResult_001
 * @tc.desc: verify ApplyResult result is REJECT.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, ApplyResult_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ApplyResult_001 start";
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();
        
        allConnectManager.applyResultBlock_ = std::make_shared<BlockObject<bool>>(
            AllConnectManager::BLOCK_INTERVAL_ALLCONNECT, false);
        
        int32_t ret = allConnectManager.ApplyResult(0, REJECT, "");
        EXPECT_EQ(ret, FileManagement::ERR_APPLY_RESULT);
        EXPECT_EQ(allConnectManager.applyResultBlock_ -> GetValue(), false);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "ApplyResult_001 end";
}

/**
 * @tc.name: AllConnectManagerTest_ApplyResult_002
 * @tc.desc: verify ApplyResult result is PASS.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, ApplyResult_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ApplyResult_002 start";
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();
        
        allConnectManager.applyResultBlock_ = std::make_shared<BlockObject<bool>>(
            AllConnectManager::BLOCK_INTERVAL_ALLCONNECT, false);
        
        int32_t ret = allConnectManager.ApplyResult(0, PASS, "");
        EXPECT_EQ(ret, FileManagement::ERR_OK);
        EXPECT_EQ(allConnectManager.applyResultBlock_->GetValue(), true);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "ApplyResult_002 end";
}

/**
 * @tc.name: AllConnectManagerTest_BuildResourceRequest_001
 * @tc.desc: verify BuildResourceRequest.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AllConnectManagerTest, BuildResourceRequest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BuildResourceRequest_001 start";
    try {
        auto &allConnectManager = AllConnectManager::GetInstance();
        
        auto ret = allConnectManager.BuildResourceRequest();
        EXPECT_TRUE(ret != nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "BuildResourceRequest_001 end";
}
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
