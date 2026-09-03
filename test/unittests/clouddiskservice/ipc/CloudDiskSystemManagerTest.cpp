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


#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "cloud_disk_error.h"
#include "cloud_disk_service_manager_mock.h"
#include "cloud_disk_system_manager.h"

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;

class SystemProgressObserver final : public CloudDiskProgressObserver {
public:
    void OnProgress(const CloudDiskHydrateProgress &progress) override { latest = progress; }
    CloudDiskHydrateProgress latest;
};

class CloudDiskSystemManagerTest : public testing::Test {
public:
    void TearDown() override
    {
        Mock::VerifyAndClearExpectations(&CloudDiskServiceManagerMock::GetInstance());
    }
};
/**
 * @tc.name: Forwarding_001
 * @tc.desc: Forward absolute path, explicit type and priority, and preserve DFS errors.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSystemManagerTest, Forwarding_001, TestSize.Level2)
{
    CloudDiskSystemManager manager;
#ifdef SUPPORT_CLOUD_DISK_MANAGER
    auto &mock = CloudDiskServiceManagerMock::GetInstance();
    EXPECT_CALL(mock, StartHydrationByPath("/absolute/file", 0, 2)).WillOnce(Return(E_TRY_AGAIN));
    EXPECT_EQ(manager.HydratePlaceholder("/absolute/file", 0, 2), E_TRY_AGAIN);
    EXPECT_CALL(mock, StartHydrationByPath("/absolute/file", 1, 0)).WillOnce(Return(E_OK));
    EXPECT_EQ(manager.HydratePlaceholder("/absolute/file", 1, 0), E_OK);
    EXPECT_CALL(mock, DehydrateFileByPath("/absolute/file")).WillOnce(Return(E_DEHYDRATE_DENIED));
    EXPECT_EQ(manager.DehydrateFile("/absolute/file"), E_DEHYDRATE_DENIED);
#else
    EXPECT_EQ(CloudDiskSystemManager::CheckAccess(), E_NOT_SUPPORT);
    EXPECT_EQ(manager.HydratePlaceholder("/absolute/file", 0, 2), E_NOT_SUPPORT);
    EXPECT_EQ(manager.DehydrateFile("/absolute/file"), E_NOT_SUPPORT);
    EXPECT_EQ(manager.RegisterProgressCallback(nullptr), E_NOT_SUPPORT);
    EXPECT_EQ(manager.UnregisterProgressCallback(), E_NOT_SUPPORT);
#endif
}
/**
 * @tc.name: Subscription_001
 * @tc.desc: Bridge unsigned progress and unregister the exact per-accessor callback on destruction.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSystemManagerTest, Subscription_001, TestSize.Level2)
{
#ifdef SUPPORT_CLOUD_DISK_MANAGER
    auto &mock = CloudDiskServiceManagerMock::GetInstance();
    auto observer = std::make_shared<SystemProgressObserver>();
    sptr<ICloudDiskProgressCallback> callback;
    EXPECT_CALL(mock, RegisterProgressCallback(_)).WillOnce(DoAll(SaveArg<0>(&callback), Return(E_OK)));
    {
        CloudDiskSystemManager manager;
        ASSERT_EQ(manager.RegisterProgressCallback(observer), E_OK);
        ASSERT_NE(callback, nullptr);
        HydrateProgress progress;
        progress.filePath = "/original/file";
        progress.state = 2;
        progress.processedSize = uint64_t{1} << 63;
        progress.totalSize = progress.processedSize + 1;
        callback->OnProgress(progress);
        EXPECT_EQ(observer->latest.filePath, progress.filePath);
        EXPECT_EQ(observer->latest.state, progress.state);
        EXPECT_EQ(observer->latest.processedSize, progress.processedSize);
        EXPECT_EQ(observer->latest.totalSize, progress.totalSize);
        EXPECT_CALL(mock, UnregisterProgressCallback(callback)).WillOnce(Return(E_OK));
    }
#endif
}
/**
 * @tc.name: ErrorMapping_001
 * @tc.desc: Keep system permission and placeholder error codes distinct from generic retry errors.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSystemManagerTest, ErrorMapping_001, TestSize.Level2)
{
    for (int32_t code : {202, 34400017, 34400018, 34400019, 34400020, 34400021, 34400022,
        34400028, 34400029, 34400030, 34400031, 34400032}) {
        EXPECT_EQ(GetCloudDiskErrorInfo(code).code, code);
        EXPECT_EQ(GetCloudDiskApiErrorInfo(code).code, code);
    }
}
} // namespace OHOS::FileManagement::CloudDiskService::Test
