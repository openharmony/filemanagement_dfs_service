/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "dk_asset_read_session.h"

namespace DriveKit {
DKError DKAssetReadSession::InitSession()
{
    return DKError();
}
int64_t DKAssetReadSession::GetAssetSize() const
{
    return int64_t();
}
void DKAssetReadSession::GetAssetInfo() {}
void DKAssetReadSession::Seek(int64_t offset, DKSeekFromWhere fromWhere) {}
int64_t DKAssetReadSession::Read(int64_t size, char *buffer, DKError &error)
{
    return int64_t();
}
int64_t DKAssetReadSession::PRead(int64_t offset, int64_t size, char *buffer, DKError &error)
{
    return int64_t();
}
bool DKAssetReadSession::Close(bool needRemain) { return bool(); }
} // namespace DriveKit
