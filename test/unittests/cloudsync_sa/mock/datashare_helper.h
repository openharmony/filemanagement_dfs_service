/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef DATASHARE_HELPER_H
#define DATASHARE_HELPER_H

#include <cstdint>
#include <string>

#include "data_ability_observer_interface.h"
#include "datashare_result_set.h"
#include "uri.h"

namespace OHOS {
namespace DataShare {
constexpr int32_t E_OK = 0;

struct CreateOptions {
    /** Specifies whether the {@link DataShareHelper} in proxy mode. */
    bool isProxy_ = true;
    /** Specifies the System token. */
    sptr<IRemoteObject> token_;
    /** Specifies whether use options to create DataShareHelper. */
    bool enabled_ = false;
};

class DataSharePredicates {
public:
    DataSharePredicates *EqualTo(const std::string &field, const std::string &value);
};
class DatashareBusinessError {};

class DataShareHelper {
public:
    DataShareHelper() = default;
    ~DataShareHelper() = default;
    static std::shared_ptr<DataShareHelper> Creator(
    const std::string &strUri, const CreateOptions &options, const std::string &bundleName = "com.ohos.photos");
    std::shared_ptr<DataShareResultSet> Query(Uri &uri, const DataSharePredicates &predicates,
        std::vector<std::string> &columns, DatashareBusinessError *businessError = nullptr);

    static void RegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver);
    static void UnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver);
    static bool Release();

private:
    static std::shared_ptr<DataShareHelper> instance_;
    std::shared_ptr<DataShareResultSet> resultSet_;
};


} // namespace OHOS
} // namespace DataShare
#endif // DATASHARE_HELPER_H