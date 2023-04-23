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

#ifndef DRIVE_KIT_ERROR_H
#define DRIVE_KIT_ERROR_H

#include <iostream>
#include <vector>

namespace DriveKit {
enum class DKLocalErrorCode {
    NO_ERROR = 0,
    IPC_CONNECT_FAILED,
    IPC_SEND_FAILED,
    DATA_TYPE_ERROR,
    ACCESS_DENIED,
    ATOMIC_ERROR,
    AUTHENTICATION_FAILED,
    AUTHENTICATION_REQUIRED,
    BAD_REQUEST,
    CONFLICT,
    EXISTS,
    INTERNAL_ERROR,
    NOT_FOUND,
    QUOTA_EXCEEDED,
    SIGN_IN_FAILED,
    THROTTLED,
    TRY_AGAIN_LATER,
    VALIDATING_REFERENCE_ERROR,
    UNIQUE_FIELD_ERROR,
    ZONE_NOT_FOUND,
    UNKNOWN_ERROR,
    NETWORK_ERROR,
    SERVICE_UNAVAILABLE,
    INVALID_ARGUMENTS,
    UNEXPECTED_SERVER_RESPONSE,
    CONFIGURATION_ERROR,
    SHARE_UI_TIMEOUT,
    BAD_ALLOC_MEMORY,
};

enum class DKServerErrorCode {
    ACCESS_DENIED = 403,
    ATOMIC_ERROR = 400,
    AUTHENTICATION_FAILED = 401,
    AUTHENTICATION_REQUIRED = 421,
    BAD_REQUEST = 400,
    CONFLICT = 409,
    EXISTS = 409,
    INTERNAL_ERROR = 500,
    NOT_FOUND = 404,
    QUOTA_EXCEEDED = 413,
    THROTTLED = 429,
    TRY_AGAIN_LATER = 503,
    VALIDATING_REFERENCE_ERROR = 412,
    ZONE_NOT_FOUND = 404,
};

struct DKErrorDetail {
    std::string domain;
    std::string reason;
    std::string errorCode;
    std::string description;
    std::string errorPos;
    std::string errorParam;
};

class DKError {
public:
    bool HasError() const;
    void SetLocalError(DKLocalErrorCode code);
    void SetServerError(int code);

    bool isLocalError = false;
    DKLocalErrorCode dkErrorCode;
    bool isServerError = false;
    int serverErrorCode;
    std::string reason;
    std::vector<DKErrorDetail> errorDetails;
    int retryAfter;
};
} // namespace DriveKit
#endif
