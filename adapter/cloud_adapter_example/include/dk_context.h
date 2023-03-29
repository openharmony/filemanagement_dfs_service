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

#ifndef DRIVE_KIT_CONTEXT_H
#define DRIVE_KIT_CONTEXT_H

namespace DriveKit {
class DKContext {
public:
    void SetData(void *data)
    {
        data_ = data;
    }
    void *GetData()
    {
        return data_;
    }

private:
    void *data_;
};
} // namespace DriveKit

#endif
