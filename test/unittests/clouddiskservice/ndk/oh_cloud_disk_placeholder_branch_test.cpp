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

#include <limits>
#include <string>
#include <vector>

#include "cloud_disk_service_manager_mock.h"
#include "oh_cloud_disk_manager.h"
#include "oh_cloud_disk_utils.h"

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;

namespace {
struct CallbackObservation {
    int32_t count = 0;
    OH_CloudDisk_CallbackType type = OH_CLOUD_DISK_CALLBACK_TYPE_FETCH_DATA;
    std::string syncFolder;
    std::string filePath;
    std::vector<uint8_t> reqKey;
    OH_CloudDisk_HydratePriority priority = ::OH_CLOUD_DISK_HYDRATE_PRIORITY_LOW;
};

CallbackObservation g_observation;

void RecordPlaceholderCallback(const OH_CloudDisk_CallbackReqHead reqHead, OH_CloudDisk_CallbackContext context)
{
    ++g_observation.count;
    g_observation.type = reqHead.callbackType;
    g_observation.syncFolder.assign(reqHead.syncFolderPath.value, reqHead.syncFolderPath.length);
    if (reqHead.reqKey.data != nullptr) {
        g_observation.reqKey.assign(reqHead.reqKey.data, reqHead.reqKey.data + reqHead.reqKey.dataSize);
    }
    if (reqHead.callbackType == OH_CLOUD_DISK_CALLBACK_TYPE_FETCH_DATA && context.fetchData != nullptr) {
        g_observation.filePath.assign(context.fetchData->filePath.value, context.fetchData->filePath.length);
        g_observation.priority = context.fetchData->priority;
    } else if (reqHead.callbackType == OH_CLOUD_DISK_CALLBACK_TYPE_CANCEL_FETCH_DATA &&
               context.cancelFetchData != nullptr) {
        g_observation.filePath.assign(context.cancelFetchData->value, context.cancelFetchData->length);
    } else if (reqHead.callbackType == OH_CLOUD_DISK_CALLBACK_TYPE_DEHYDRATE && context.dehydrateData != nullptr) {
        g_observation.filePath.assign(context.dehydrateData->filePath.value, context.dehydrateData->filePath.length);
        context.dehydrateData->allow = true;
    }
}
} // namespace

class OhCloudDiskPlaceholderBranchTest : public testing::Test {
public:
    void SetUp() override
    {
        g_observation = {};
        Mock::VerifyAndClearExpectations(&CloudDiskServiceManagerMock::GetInstance());
    }

    void TearDown() override
    {
        Mock::VerifyAndClearExpectations(&CloudDiskServiceManagerMock::GetInstance());
    }

    static CloudDisk_SyncFolderPath SyncFolder(std::string &value)
    {
        return {value.data(), value.size()};
    }

    static CloudDisk_PathInfo Path(std::string &value)
    {
        return {value.data(), value.size()};
    }
};

/**
 * @tc.name: RegisterCallbackTable_001
 * @tc.desc: Validate callback registration and preserve all callback request fields.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskPlaceholderBranchTest, RegisterCallbackTable_001, TestSize.Level1)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    std::string syncFolder = "/sync";
    std::string filePath = "dir/file.txt";
    auto syncFolderPath = SyncFolder(syncFolder);
    CloudDisk_SyncFolderPath invalidPath{nullptr, 1};
    auto &mock = CloudDiskServiceManagerMock::GetInstance();
    EXPECT_CALL(mock, RegisterCallbackTable(_, _)).Times(0);
    EXPECT_EQ(OH_CloudDisk_RegisterCallbackTable(invalidPath, RecordPlaceholderCallback), CLOUD_DISK_INVALID_ARG);
    EXPECT_EQ(OH_CloudDisk_RegisterCallbackTable(syncFolderPath, nullptr), CLOUD_DISK_INVALID_ARG);

    Mock::VerifyAndClearExpectations(&mock);
    std::shared_ptr<CloudDiskServiceCallbackTable> callbackTable;
    EXPECT_CALL(mock, RegisterCallbackTable(syncFolder, _)).WillOnce(DoAll(SaveArg<1>(&callbackTable), Return(E_OK)));
    ASSERT_EQ(OH_CloudDisk_RegisterCallbackTable(syncFolderPath, RecordPlaceholderCallback), CLOUD_DISK_OK);
    ASSERT_NE(callbackTable, nullptr);

    std::vector<uint8_t> reqKey{1, 2, 3};
    CloudDiskCallbackReqHead head{
        {syncFolder.data(), syncFolder.size()}, CloudDiskCallbackType::FETCH_DATA, {reqKey.data(), reqKey.size()}};
    CloudDiskFetchDataRequest fetch{{filePath.data(), filePath.size()}, CLOUD_DISK_HYDRATE_PRIORITY_HIGH};
    CloudDiskCallbackContext context{};
    context.fetchData = &fetch;
    callbackTable->OnCallback(head, context);
    EXPECT_EQ(g_observation.count, 1);
    EXPECT_EQ(g_observation.type, OH_CLOUD_DISK_CALLBACK_TYPE_FETCH_DATA);
    EXPECT_EQ(g_observation.syncFolder, syncFolder);
    EXPECT_EQ(g_observation.filePath, filePath);
    EXPECT_EQ(g_observation.reqKey, reqKey);
    EXPECT_EQ(g_observation.priority, ::OH_CLOUD_DISK_HYDRATE_PRIORITY_HIGH);

    context.fetchData = nullptr;
    callbackTable->OnCallback(head, context);
    EXPECT_EQ(g_observation.count, 1);
#else
    std::string syncFolder = "/sync";
    EXPECT_EQ(OH_CloudDisk_RegisterCallbackTable(SyncFolder(syncFolder), RecordPlaceholderCallback),
              CLOUD_DISK_NOT_SUPPORTED);
#endif
}

/**
 * @tc.name: RegisterCallbackTable_003
 * @tc.desc: Preserve cancel, dehydrate, null-context, and unsupported callback behavior.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskPlaceholderBranchTest, RegisterCallbackTable_003, TestSize.Level1)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    std::string syncFolder = "/sync";
    std::string filePath = "dir/file.txt";
    std::shared_ptr<CloudDiskServiceCallbackTable> callbackTable;
    auto &mock = CloudDiskServiceManagerMock::GetInstance();
    EXPECT_CALL(mock, RegisterCallbackTable(syncFolder, _)).WillOnce(DoAll(SaveArg<1>(&callbackTable), Return(E_OK)));
    ASSERT_EQ(OH_CloudDisk_RegisterCallbackTable(SyncFolder(syncFolder), RecordPlaceholderCallback), CLOUD_DISK_OK);
    ASSERT_NE(callbackTable, nullptr);

    std::vector<uint8_t> reqKey{1, 2, 3};
    CloudDiskCallbackReqHead head{{syncFolder.data(), syncFolder.size()},
                                  CloudDiskCallbackType::CANCEL_FETCH_DATA,
                                  {reqKey.data(), reqKey.size()}};
    CloudDiskCallbackContext context{};

    CloudDiskPathInfo pathInfo{filePath.data(), filePath.size()};
    context.cancelFetchData = &pathInfo;
    callbackTable->OnCallback(head, context);
    EXPECT_EQ(g_observation.count, 1);
    EXPECT_EQ(g_observation.type, OH_CLOUD_DISK_CALLBACK_TYPE_CANCEL_FETCH_DATA);
    EXPECT_EQ(g_observation.filePath, filePath);
    context.cancelFetchData = nullptr;
    callbackTable->OnCallback(head, context);
    EXPECT_EQ(g_observation.count, 1);

    head.callbackType = CloudDiskCallbackType::DEHYDRATE;
    CloudDiskDehydrateInfo dehydrate{pathInfo, false};
    context.dehydrateData = &dehydrate;
    callbackTable->OnCallback(head, context);
    EXPECT_EQ(g_observation.count, 2);
    EXPECT_TRUE(dehydrate.allow);
    context.dehydrateData = nullptr;
    callbackTable->OnCallback(head, context);
    EXPECT_EQ(g_observation.count, 2);

    head.callbackType = static_cast<CloudDiskCallbackType>(99);
    callbackTable->OnCallback(head, context);
    EXPECT_EQ(g_observation.count, 2);
#endif
}

/**
 * @tc.name: UnregisterCallbackTable_001
 * @tc.desc: Validate callback unregistration and map success and service errors.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskPlaceholderBranchTest, UnregisterCallbackTable_001, TestSize.Level1)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    std::string syncFolder = "/sync";
    auto syncFolderPath = SyncFolder(syncFolder);
    CloudDisk_SyncFolderPath invalidPath{nullptr, 1};
    auto &mock = CloudDiskServiceManagerMock::GetInstance();
    EXPECT_EQ(OH_CloudDisk_UnregisterCallbackTable(invalidPath), CLOUD_DISK_INVALID_ARG);
    EXPECT_CALL(mock, UnregisterCallbackTable(syncFolder)).WillOnce(Return(E_OK));
    EXPECT_EQ(OH_CloudDisk_UnregisterCallbackTable(syncFolderPath), CLOUD_DISK_OK);
    Mock::VerifyAndClearExpectations(&mock);
    EXPECT_CALL(mock, UnregisterCallbackTable(syncFolder)).WillOnce(Return(E_IPC_FAILED));
    EXPECT_EQ(OH_CloudDisk_UnregisterCallbackTable(syncFolderPath), CLOUD_DISK_IPC_FAILED);
#else
    std::string syncFolder = "/sync";
    EXPECT_EQ(OH_CloudDisk_UnregisterCallbackTable(SyncFolder(syncFolder)), CLOUD_DISK_NOT_SUPPORTED);
#endif
}

/**
 * @tc.name: CreatePlaceholder_001
 * @tc.desc: Validate paths, empty custom-info variants, metadata forwarding, and service errors.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskPlaceholderBranchTest, CreatePlaceholder_001, TestSize.Level1)
{
    std::string syncFolder = "/sync";
    std::string filePath = "file.txt";
    auto syncFolderPath = SyncFolder(syncFolder);
    auto pathInfo = Path(filePath);
    CloudDisk_SyncFolderPath invalidSync{nullptr, 1};
    CloudDisk_PathInfo invalidPath{nullptr, 1};
    OH_CloudDisk_PlaceholderInfo info{10, 20, 30};
    auto &mock = CloudDiskServiceManagerMock::GetInstance();
    EXPECT_CALL(mock, CreatePlaceholderFile(_, _, _, _)).Times(0);
    EXPECT_EQ(OH_CloudDisk_CreatePlaceholder(invalidSync, pathInfo, info, nullptr), CLOUD_DISK_INVALID_ARG);
    EXPECT_EQ(OH_CloudDisk_CreatePlaceholder(syncFolderPath, invalidPath, info, nullptr), CLOUD_DISK_INVALID_ARG);

    Mock::VerifyAndClearExpectations(&mock);
    OH_CloudDisk_PlaceholderCustomInfo emptyWithLength{3, nullptr};
    EXPECT_CALL(mock, CreatePlaceholderFile(_, _, _, _)).Times(0);
    EXPECT_EQ(OH_CloudDisk_CreatePlaceholder(syncFolderPath, pathInfo, info, &emptyWithLength), CLOUD_DISK_INVALID_ARG);

    Mock::VerifyAndClearExpectations(&mock);
    uint8_t value = 1;
    OH_CloudDisk_PlaceholderCustomInfo emptyWithPointer{0, &value};
    EXPECT_CALL(mock, CreatePlaceholderFile(syncFolder, filePath, _, _)).WillOnce(Return(E_FILE_ALREADY_EXISTS));
    EXPECT_EQ(OH_CloudDisk_CreatePlaceholder(syncFolderPath, pathInfo, info, &emptyWithPointer),
              OH_CLOUD_DISK_FILE_ALREADY_EXISTS);
}

/**
 * @tc.name: IsPlaceholderFile_001
 * @tc.desc: Validate output and path arguments and preserve service success and failure behavior.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskPlaceholderBranchTest, IsPlaceholderFile_001, TestSize.Level1)
{
    std::string syncFolder = "/sync";
    std::string filePath = "file.txt";
    auto syncFolderPath = SyncFolder(syncFolder);
    auto pathInfo = Path(filePath);
    CloudDisk_SyncFolderPath invalidSync{nullptr, 1};
    CloudDisk_PathInfo invalidPath{nullptr, 1};
    auto &mock = CloudDiskServiceManagerMock::GetInstance();
    EXPECT_CALL(mock, IsPlaceholderFile(_, _, _)).Times(0);
    EXPECT_EQ(OH_CloudDisk_IsPlaceholderFile(syncFolderPath, pathInfo, nullptr), CLOUD_DISK_INVALID_ARG);
    bool result = true;
    EXPECT_EQ(OH_CloudDisk_IsPlaceholderFile(invalidSync, pathInfo, &result), CLOUD_DISK_INVALID_ARG);
    EXPECT_FALSE(result);
    result = true;
    EXPECT_EQ(OH_CloudDisk_IsPlaceholderFile(syncFolderPath, invalidPath, &result), CLOUD_DISK_INVALID_ARG);
    EXPECT_FALSE(result);

    Mock::VerifyAndClearExpectations(&mock);
    EXPECT_CALL(mock, IsPlaceholderFile(syncFolder, filePath, _)).WillOnce(DoAll(SetArgReferee<2>(true), Return(E_OK)));
    EXPECT_EQ(OH_CloudDisk_IsPlaceholderFile(syncFolderPath, pathInfo, &result), CLOUD_DISK_OK);
    EXPECT_TRUE(result);

    Mock::VerifyAndClearExpectations(&mock);
    EXPECT_CALL(mock, IsPlaceholderFile(syncFolder, filePath, _)).WillOnce(Return(E_FILE_NOT_EXIST));
    EXPECT_EQ(OH_CloudDisk_IsPlaceholderFile(syncFolderPath, pathInfo, &result), OH_CLOUD_DISK_FILE_NOT_EXIST);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: GetPlaceholderCustomInfo_001
 * @tc.desc: Validate output-buffer contracts and the empty custom-information success path.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskPlaceholderBranchTest, GetPlaceholderCustomInfo_001, TestSize.Level2)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    std::string syncFolder = "/sync";
    std::string filePath = "file.txt";
    auto syncFolderPath = SyncFolder(syncFolder);
    auto pathInfo = Path(filePath);
    CloudDisk_SyncFolderPath invalidSync{nullptr, 1};
    CloudDisk_PathInfo invalidPath{nullptr, 1};
    auto &mock = CloudDiskServiceManagerMock::GetInstance();
    uint8_t buffer[2] = {};
    size_t length = sizeof(buffer);
    EXPECT_CALL(mock, GetPlaceholderCustomInfo(_, _, _)).Times(0);
    EXPECT_EQ(OH_CloudDisk_GetPlaceholderCustomInfo(syncFolderPath, pathInfo, buffer, nullptr), CLOUD_DISK_INVALID_ARG);
    EXPECT_EQ(OH_CloudDisk_GetPlaceholderCustomInfo(invalidSync, pathInfo, buffer, &length), CLOUD_DISK_INVALID_ARG);
    EXPECT_EQ(OH_CloudDisk_GetPlaceholderCustomInfo(syncFolderPath, invalidPath, buffer, &length),
              CLOUD_DISK_INVALID_ARG);
    EXPECT_EQ(OH_CloudDisk_GetPlaceholderCustomInfo(syncFolderPath, pathInfo, nullptr, &length),
              CLOUD_DISK_INVALID_ARG);
    EXPECT_EQ(length, 0U);

    Mock::VerifyAndClearExpectations(&mock);
    length = 0;
    EXPECT_CALL(mock, GetPlaceholderCustomInfo(syncFolder, filePath, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(OH_CloudDisk_GetPlaceholderCustomInfo(syncFolderPath, pathInfo, nullptr, &length), CLOUD_DISK_OK);
    EXPECT_EQ(length, 0U);

    Mock::VerifyAndClearExpectations(&mock);
    PlaceholderCustomInfo customInfo;
    customInfo.data = {1};
    EXPECT_CALL(mock, GetPlaceholderCustomInfo(syncFolder, filePath, _))
        .WillOnce(DoAll(SetArgReferee<2>(customInfo), Return(E_OK)));
    EXPECT_EQ(OH_CloudDisk_GetPlaceholderCustomInfo(syncFolderPath, pathInfo, nullptr, &length),
              CLOUD_DISK_INVALID_ARG);
    EXPECT_EQ(length, 1U);
#else
    std::string syncFolder = "/sync";
    std::string filePath = "file.txt";
    size_t length = 2;
    EXPECT_EQ(OH_CloudDisk_GetPlaceholderCustomInfo(SyncFolder(syncFolder), Path(filePath), nullptr, &length),
              CLOUD_DISK_NOT_SUPPORTED);
    EXPECT_EQ(length, 0U);
#endif
}

/**
 * @tc.name: HydratePlaceholder_003
 * @tc.desc: Reject malformed pointed-to paths and the lower hydration-priority boundary violation.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskPlaceholderBranchTest, HydratePlaceholder_003, TestSize.Level2)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    std::string syncFolder = "/sync";
    std::string filePath = "file.txt";
    auto syncFolderPath = SyncFolder(syncFolder);
    auto pathInfo = Path(filePath);
    CloudDisk_SyncFolderPath invalidSync{nullptr, 1};
    CloudDisk_PathInfo invalidPath{nullptr, 1};
    auto invalidPriority = static_cast<OH_CloudDisk_HydratePriority>(-1);
    EXPECT_CALL(CloudDiskServiceManagerMock::GetInstance(), StartHydration(_, _, _)).Times(0);
    EXPECT_EQ(OH_CloudDisk_HydratePlaceholder(&invalidSync, &pathInfo, OH_CLOUD_DISK_CALLBACK_TYPE_FETCH_DATA,
                                              ::OH_CLOUD_DISK_HYDRATE_PRIORITY_NORMAL),
              CLOUD_DISK_INVALID_ARG);
    EXPECT_EQ(OH_CloudDisk_HydratePlaceholder(&syncFolderPath, &invalidPath, OH_CLOUD_DISK_CALLBACK_TYPE_FETCH_DATA,
                                              ::OH_CLOUD_DISK_HYDRATE_PRIORITY_NORMAL),
              CLOUD_DISK_INVALID_ARG);
    EXPECT_EQ(OH_CloudDisk_HydratePlaceholder(&syncFolderPath, &pathInfo, OH_CLOUD_DISK_CALLBACK_TYPE_FETCH_DATA,
                                              invalidPriority),
              CLOUD_DISK_INVALID_ARG);
#else
    SUCCEED();
#endif
}

/**
 * @tc.name: Execute_004
 * @tc.desc: Cover request-header and callback-context validation not requiring service IPC.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskPlaceholderBranchTest, Execute_004, TestSize.Level2)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    constexpr size_t oversizedRequestKey = MAX_CALLBACK_REQUEST_KEY_SIZE + 1;
    std::string syncFolder = "/sync";
    std::string filePath = "file.txt";
    uint8_t keyData[oversizedRequestKey] = {};
    uint8_t dataByte = 1;
    OH_CloudDisk_CallbackReqHead head{SyncFolder(syncFolder), OH_CLOUD_DISK_CALLBACK_TYPE_FETCH_DATA, {keyData, 1}};
    OH_CloudDisk_FetchDataRequest request{Path(filePath), ::OH_CLOUD_DISK_HYDRATE_PRIORITY_NORMAL};
    OH_CloudDisk_CallbackContext context{};
    context.fetchData = &request;
    OH_CloudDisk_FetchData fetchData{0, 1, 1, {&dataByte, 1}, true};
    OH_CloudDisk_CallbackResponse response{};
    response.fetchData = &fetchData;
    EXPECT_CALL(CloudDiskServiceManagerMock::GetInstance(), Execute(_)).Times(0);

    head.reqKey = {nullptr, 1};
    EXPECT_EQ(OH_CloudDisk_Execute(head, context, response), CLOUD_DISK_INVALID_ARG);
    head.reqKey = {keyData, 0};
    EXPECT_EQ(OH_CloudDisk_Execute(head, context, response), CLOUD_DISK_INVALID_ARG);
    head.reqKey = {keyData, oversizedRequestKey};
    EXPECT_EQ(OH_CloudDisk_Execute(head, context, response), CLOUD_DISK_INVALID_ARG);
    head.reqKey = {keyData, 1};
    head.syncFolderPath = {nullptr, 1};
    EXPECT_EQ(OH_CloudDisk_Execute(head, context, response), CLOUD_DISK_INVALID_ARG);
    head.syncFolderPath = SyncFolder(syncFolder);
    response.fetchData = nullptr;
    EXPECT_EQ(OH_CloudDisk_Execute(head, context, response), CLOUD_DISK_INVALID_ARG);
    response.fetchData = &fetchData;
    request.filePath = {nullptr, 1};
    EXPECT_EQ(OH_CloudDisk_Execute(head, context, response), CLOUD_DISK_INVALID_ARG);
    request.filePath = Path(filePath);
    fetchData = {0, 2, 1, {keyData, 2}, true};
    EXPECT_EQ(OH_CloudDisk_Execute(head, context, response), CLOUD_DISK_INVALID_ARG);
    fetchData = {2, 0, 1, {nullptr, 0}, true};
    EXPECT_EQ(OH_CloudDisk_Execute(head, context, response), CLOUD_DISK_INVALID_ARG);

    head.callbackType = OH_CLOUD_DISK_CALLBACK_TYPE_CANCEL_FETCH_DATA;
    context.cancelFetchData = nullptr;
    EXPECT_EQ(OH_CloudDisk_Execute(head, context, response), CLOUD_DISK_INVALID_ARG);
    CloudDisk_PathInfo invalidPath{nullptr, 1};
    context.cancelFetchData = &invalidPath;
    EXPECT_EQ(OH_CloudDisk_Execute(head, context, response), CLOUD_DISK_INVALID_ARG);
#else
    SUCCEED();
#endif
}

/**
 * @tc.name: Execute_005
 * @tc.desc: Accept empty-file completion and the exact 128 KiB Execute payload boundary.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskPlaceholderBranchTest, Execute_005, TestSize.Level1)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    std::string syncFolder = "/sync";
    std::string filePath = "file.txt";
    uint8_t key = 1;
    OH_CloudDisk_CallbackReqHead head{SyncFolder(syncFolder), OH_CLOUD_DISK_CALLBACK_TYPE_FETCH_DATA, {&key, 1}};
    OH_CloudDisk_FetchDataRequest request{Path(filePath), ::OH_CLOUD_DISK_HYDRATE_PRIORITY_NORMAL};
    OH_CloudDisk_CallbackContext context{};
    context.fetchData = &request;
    OH_CloudDisk_CallbackResponse response{};
    OH_CloudDisk_FetchData empty{0, 0, 0, {nullptr, 0}, true};
    response.fetchData = &empty;
    auto &mock = CloudDiskServiceManagerMock::GetInstance();
    EXPECT_CALL(mock, Execute(_)).WillOnce(Invoke([](const CallbackExecuteRequest &inner) {
        EXPECT_TRUE(inner.data.empty());
        EXPECT_TRUE(inner.isComplete);
        EXPECT_EQ(inner.totalSize, 0U);
        return E_OK;
    }));
    EXPECT_EQ(OH_CloudDisk_Execute(head, context, response), CLOUD_DISK_OK);

    Mock::VerifyAndClearExpectations(&mock);
    std::vector<uint8_t> data(MAX_EXECUTE_DATA_SIZE, 7);
    OH_CloudDisk_FetchData boundary{0, data.size(), data.size(), {data.data(), data.size()}, true};
    response.fetchData = &boundary;
    EXPECT_CALL(mock, Execute(_)).WillOnce(Invoke([&data](const CallbackExecuteRequest &inner) {
        EXPECT_EQ(inner.data, data);
        EXPECT_EQ(inner.size, MAX_EXECUTE_DATA_SIZE);
        return E_OK;
    }));
    EXPECT_EQ(OH_CloudDisk_Execute(head, context, response), CLOUD_DISK_OK);
#else
    SUCCEED();
#endif
}

/**
 * @tc.name: ConvertToErrorCode_001
 * @tc.desc: Verify every placeholder-specific mapping and the unknown-error fallback.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskPlaceholderBranchTest, ConvertToErrorCode_001, TestSize.Level1)
{
    const std::vector<std::pair<int32_t, CloudDisk_ErrorCode>> cases = {
        {E_NOT_A_PLACEHOLDER, OH_CLOUD_DISK_NOT_A_PLACEHOLDER},
        {E_IS_A_PLACEHOLDER, OH_CLOUD_DISK_IS_A_PLACEHOLDER},
        {E_HYDRATE_IN_PROGRESS, OH_CLOUD_DISK_HYDRATE_IN_PROGRESS},
        {E_FILE_ALREADY_EXISTS, OH_CLOUD_DISK_FILE_ALREADY_EXISTS},
        {E_NO_SPACE_LEFT, OH_CLOUD_DISK_NO_SPACE_LEFT},
        {E_CALLBACK_NOT_REGISTERED, OH_CLOUD_DISK_CALLBACK_NOT_REGISTERED},
        {E_CALLBACK_ALREADY_REGISTERED, OH_CLOUD_DISK_CALLBACK_ALREADY_REGISTERED},
        {E_NOT_A_DIRECTORY, OH_CLOUD_DISK_NOT_A_DIRECTORY},
        {E_FILE_NOT_EXIST, OH_CLOUD_DISK_FILE_NOT_EXIST},
        {E_NAME_TOO_LONG, OH_CLOUD_DISK_NAME_TOO_LONG},
        {E_FILE_TOO_LARGE, OH_CLOUD_DISK_FILE_TOO_LARGE},
        {E_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND, OH_CLOUD_DISK_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND},
        {E_PLACEHOLDER_NOT_FULLY_HYDRATED, OH_CLOUD_DISK_PLACEHOLDER_NOT_FULLY_HYDRATED},
        {E_DEHYDRATE_DENIED, OH_CLOUD_DISK_DEHYDRATE_DENIED},
        {E_CANCELLED, OH_CLOUD_DISK_CANCELLED},
        {E_ALREADY_HYDRATED, OH_CLOUD_DISK_ALREADY_HYDRATED},
        {E_NO_HYDRATION_IN_PROGRESS, OH_CLOUD_DISK_NO_HYDRATION_IN_PROGRESS},
        {E_INVALID_PLACEHOLDER_STATE, OH_CLOUD_DISK_INVALID_PLACEHOLDER_STATE},
        {E_HYDRATION_TASK_LIMIT_REACHED, OH_CLOUD_DISK_HYDRATION_TASK_LIMIT_REACHED},
    };
    for (const auto &[inner, expected] : cases) {
        EXPECT_EQ(ConvertToErrorCode(inner), expected);
    }
    EXPECT_EQ(ConvertToErrorCode(std::numeric_limits<int32_t>::max()), CLOUD_DISK_TRY_AGAIN);
}
} // namespace OHOS::FileManagement::CloudDiskService::Test
