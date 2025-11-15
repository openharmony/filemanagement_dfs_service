/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef DATA_HELPER_MOCK_H
#define DATA_HELPER_MOCK_H
#include <gmock/gmock.h>
#include <memory>
#include <vector>

#include "datashare_helper.h"
#include "settings_data_manager.h"
#include "os_account_manager.h"

namespace OHOS::FileManagement::CloudFile {
using namespace OHOS::DataShare;
using namespace OHOS::FileManagement::CloudSync;

class DataShareHelperMock : public DataShareHelper {
public:
    MOCK_METHOD5(Creator,
                 std::shared_ptr<DataShareHelper>(const std::string &strUri,
                                                  const CreateOptions &options,
                                                  const std::string &bundleName,
                                                  const int waitTime,
                                                  bool isSystem));
    MOCK_METHOD0(Release, bool());
    MOCK_METHOD2(GetFileTypes, std::vector<std::string>(Uri &uri, const std::string &mimeTypeFilter));
    MOCK_METHOD2(OpenFile, int(Uri &uri, const std::string &mode));
    MOCK_METHOD3(OpenFileWithErrCode, int(Uri &uri, const std::string &mode, int32_t &errCode));
    MOCK_METHOD2(OpenRawFile, int(Uri &uri, const std::string &mode));
    MOCK_METHOD2(Insert, int(Uri &uri, const DataShareValuesBucket &value));
    MOCK_METHOD3(InsertExt, int(Uri &uri, const DataShareValuesBucket &value, std::string &result));
    MOCK_METHOD3(Update, int(Uri &uri, const DataSharePredicates &predicates, const DataShareValuesBucket &value));
    MOCK_METHOD2(BatchUpdate, int(const UpdateOperations &operations, std::vector<BatchUpdateResult> &results));
    MOCK_METHOD2(Delete, int(Uri &uri, const DataSharePredicates &predicates));
    MOCK_METHOD4(Query,
                 std::shared_ptr<DataShareResultSet>(Uri &uri,
                                                     const DataSharePredicates &predicates,
                                                     std::vector<std::string> &columns,
                                                     DatashareBusinessError *businessError));
    MOCK_METHOD1(GetType, std::string(Uri &uri));
    MOCK_METHOD2(BatchInsert, int(Uri &uri, const std::vector<DataShareValuesBucket> &values));
    MOCK_METHOD2(ExecuteBatch, int(const std::vector<OperationStatement> &statements, ExecResultSet &result));
    MOCK_METHOD2(RegisterObserver, int(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver));
    MOCK_METHOD2(UnregisterObserver, int(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver));
    MOCK_METHOD1(NotifyChange, void(const Uri &uri));
    MOCK_METHOD3(RegisterObserverExtProvider,
                 int(const Uri &uri, std::shared_ptr<DataShareObserver> dataObserver, bool isDescendants));
    MOCK_METHOD2(UnregisterObserverExtProvider, int(const Uri &uri, std::shared_ptr<DataShareObserver> dataObserver));
    MOCK_METHOD1(NotifyChangeExtProvider, void(const DataShareObserver::ChangeInfo &changeInfo));
    MOCK_METHOD1(NormalizeUri, Uri(Uri &uri));
    MOCK_METHOD1(DenormalizeUri, Uri(Uri &uri));
    MOCK_METHOD3(AddQueryTemplate, int(const std::string &uri, int64_t subscriberId, Template &tpl));
    MOCK_METHOD2(DelQueryTemplate, int(const std::string &uri, int64_t subscriberId));
    MOCK_METHOD2(Publish, std::vector<OperationResult>(const Data &data, const std::string &bundleName));
    MOCK_METHOD2(GetPublishedData, Data(const std::string &bundleName, int &resultCode));
    MOCK_METHOD3(SubscribeRdbData,
                 std::vector<OperationResult>(const std::vector<std::string> &uris,
                                              const TemplateId &templateId,
                                              const std::function<void(const RdbChangeNode &changeNode)> &callback));
    MOCK_METHOD2(UnsubscribeRdbData,
                 std::vector<OperationResult>(const std::vector<std::string> &uris, const TemplateId &templateId));
    MOCK_METHOD2(EnableRdbSubs,
                 std::vector<OperationResult>(const std::vector<std::string> &uris, const TemplateId &templateId));
    MOCK_METHOD2(DisableRdbSubs,
                 std::vector<OperationResult>(const std::vector<std::string> &uris, const TemplateId &templateId));
    MOCK_METHOD3(
        SubscribePublishedData,
        std::vector<OperationResult>(const std::vector<std::string> &uris,
                                     int64_t subscriberId,
                                     const std::function<void(const PublishedDataChangeNode &changeNode)> &callback));
    MOCK_METHOD2(UnsubscribePublishedData,
                 std::vector<OperationResult>(const std::vector<std::string> &uris, int64_t subscriberId));
    MOCK_METHOD2(EnablePubSubs,
                 std::vector<OperationResult>(const std::vector<std::string> &uris, int64_t subscriberId));
    MOCK_METHOD2(DisablePubSubs,
                 std::vector<OperationResult>(const std::vector<std::string> &uris, int64_t subscriberId));
    MOCK_METHOD2(InsertEx, std::pair<int32_t, int32_t>(Uri &uri, const DataShareValuesBucket &value));
    MOCK_METHOD3(UpdateEx,
                 std::pair<int32_t, int32_t>(Uri &uri,
                                             const DataSharePredicates &predicates,
                                             const DataShareValuesBucket &value));
    MOCK_METHOD2(DeleteEx, std::pair<int32_t, int32_t>(Uri &uri, const DataSharePredicates &predicates));
    MOCK_METHOD3(UserDefineFunc, int32_t(MessageParcel &data, MessageParcel &reply, MessageOption &option));
};

class IDataHelper {
public:
    virtual ~IDataHelper() = default;
    virtual std::pair<int, std::shared_ptr<DataShareHelper>> Create() = 0;
    virtual SwitchStatus GetSwitchStatus() = 0;
    virtual int32_t QuerySwitchStatus() = 0;

public:
    static inline std::shared_ptr<IDataHelper> ins = nullptr;
};

class DataHelperMock : public IDataHelper {
public:
    MOCK_METHOD0(Create, std::pair<int, std::shared_ptr<DataShareHelper>>());
    MOCK_METHOD0(GetSwitchStatus, SwitchStatus());
    MOCK_METHOD0(QuerySwitchStatus, int32_t());
};

class IOsAccountManager {
public:
    IOsAccountManager() = default;
    virtual ~IOsAccountManager() = default;
    virtual ErrCode GetForegroundOsAccountLocalId(int32_t &localId) = 0;

public:
    static inline std::shared_ptr<IOsAccountManager> ins = nullptr;
};

class OsAccountManagerMock : public IOsAccountManager {
public:
    MOCK_METHOD1(GetForegroundOsAccountLocalId, ErrCode(int32_t &));
};
} // namespace OHOS::FileManagement::CloudFile
#endif