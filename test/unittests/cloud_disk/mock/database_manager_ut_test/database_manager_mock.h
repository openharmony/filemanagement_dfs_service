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

#ifndef MOCK_DBM_UT_DATABASE_MANAGER_MOCK_H
#define MOCK_DBM_UT_DATABASE_MANAGER_MOCK_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "abs_shared_result_set.h"
#include "data_syncer_rdb_store.h"

namespace OHOS::FileManagement::CloudSync {
/*
 * DataSyncerRdbStore is not declared `final`, but it has a private constructor, so this mock is
 * standalone (does not inherit). The real member methods are rewritten in database_manager_mock.cpp
 * to delegate to proxy_ when it is set.
 */
class DataSyncerRdbStoreMock {
public:
    MOCK_METHOD3(QueryCloudSync,
                 int32_t(int32_t, const std::string &, std::shared_ptr<NativeRdb::ResultSet> &));
    static inline std::shared_ptr<DataSyncerRdbStoreMock> proxy_ = nullptr;
};
} // namespace OHOS::FileManagement::CloudSync

namespace OHOS::NativeRdb {
/*
 * AbsSharedResultSet is concrete (AbsResultSet overrides all pure virtuals of ResultSet/RemoteResultSet,
 * and AbsSharedResultSet overrides GetBlock/OnGo from SharedResultSet). So ResultSetMock only needs to
 * override GetRowCount, and the SUT's resultSet->GetRowCount dispatches here via virtual.
 */
class ResultSetMock : public AbsSharedResultSet {
public:
    MOCK_METHOD1(GetRowCount, int(int &));
};
} // namespace OHOS::NativeRdb
#endif // MOCK_DBM_UT_DATABASE_MANAGER_MOCK_H
