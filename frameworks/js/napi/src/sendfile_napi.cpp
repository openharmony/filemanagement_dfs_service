/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "sendfile_napi.h"

#include "filemgmt_libn.h"
#include "send_file.h"
#include "trans_event.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace FileManagement::LibN;

std::vector<std::string> GetJsPath(napi_env env, napi_value param)
{
    std::vector<std::string> jsPath;
    jsPath.clear();

    bool isArray = false;
    if (napi_is_array(env, param, &isArray) != napi_ok || isArray == false) {
        LOGE("JS sendFile filename must be a string array.");
        return jsPath;
    }

    uint32_t arraySize = 0;
    if (napi_get_array_length(env, param, &arraySize) != napi_ok) {
        LOGE("JS sendFile get filename array size failed.");
        return jsPath;
    }
    if (arraySize > 1) {
        return jsPath;
    }
    for (uint32_t i = 0; i < arraySize; ++i) {
        napi_value result = nullptr;
        napi_status status = napi_get_element(env, param, i, &result);
        if (status == napi_ok && result != nullptr) {
            bool succ = false;
            std::unique_ptr<char []> path;
            std::tie(succ, path, std::ignore) = NVal(env, result).ToUTF8String();
            if (succ) {
                jsPath.push_back(path.get());
            } else {
                LOGE("JS sendFile: only string array were accepted as filename para.");
            }
        }
    }
    return jsPath;
}

napi_value JsSendFile(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(DFS_ARG_CNT::FOUR, DFS_ARG_CNT::FIVE)) {
        NError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char []> deviceId;
    std::tie(succ, deviceId, std::ignore) = NVal(env, funcArg[DFS_ARG_POS::FIRST]).ToUTF8String();
    std::string deviceIdString = "";
    if (succ) {
        deviceIdString =  std::string(deviceId.get());
    }

    uint32_t fileCount = 0;
    napi_status status = napi_get_value_uint32(env, funcArg[DFS_ARG_POS::FOURTH], &fileCount);
    if (status != napi_ok && fileCount != 1) {
        LOGE("JS sendFile: param fileCounts only accecpt 1.");
        return nullptr;
    }

    std::vector<std::string> sourPath = GetJsPath(env, funcArg[DFS_ARG_POS::SECOND]);
    std::vector<std::string> destPath = GetJsPath(env, funcArg[DFS_ARG_POS::THIRD]);

    auto resultCode = std::make_shared<int32_t>();
    auto cbExec = [deviceIdString, sourPath, destPath, fileCount, resultCode]() -> NError {
        *resultCode = SendFile::ExecSendFile(deviceIdString.c_str(), sourPath, destPath, fileCount);
        LOGD("JsSendFile: resultCode1 [%{public}d]", *resultCode);
        return NError(*resultCode);
    };

    auto cbComplete = [resultCode](napi_env env, NError err) -> NVal {
        LOGD("cbComplete: resultCode1 [%{public}d]", *resultCode);
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        LOGD("JsSendFile: resultCode2 [%{public}d]", *resultCode);
        return { NVal::CreateInt64(env, *resultCode) };
    };

    std::string procedureName = "SendFile";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == DFS_ARG_CNT::FOUR) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else if (funcArg.GetArgc() == DFS_ARG_CNT::FIVE) {
        NVal cb(env, funcArg[DFS_ARG_POS::FIFTH]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value JsOn(napi_env env, napi_callback_info cbinfo)
{
    size_t requireArgc = 2;
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = 0;
    void* data = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    EventAgent* agent = nullptr;
    napi_unwrap(env, thisVar, (void**)&agent);

    NAPI_ASSERT(env, argc >= requireArgc, "requires 2 parameter");

    napi_valuetype eventValueType = napi_undefined;
    napi_typeof(env, argv[0], &eventValueType);
    NAPI_ASSERT(env, eventValueType == napi_string, "type mismatch for parameter 1");

    napi_valuetype eventHandleType = napi_undefined;
    napi_typeof(env, argv[1], &eventHandleType);
    NAPI_ASSERT(env, eventHandleType == napi_function, "type mismatch for parameter 2");

    char type[64] = { 0 };
    size_t typeLen = 0;

    napi_get_value_string_utf8(env, argv[0], type, sizeof(type), &typeLen);

    agent->On((const char*)type, argv[1]);

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value JsOff(napi_env env, napi_callback_info cbinfo)
{
    size_t requireArgc = 1;
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = 0;
    void* data = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    EventAgent* agent = nullptr;
    napi_unwrap(env, thisVar, (void**)&agent);

    NAPI_ASSERT(env, argc == requireArgc, "requires 1 parameter");

    napi_valuetype eventValueType = napi_undefined;
    napi_typeof(env, argv[0], &eventValueType);
    NAPI_ASSERT(env, eventValueType == napi_string, "type mismatch for parameter 1");

    char* type = nullptr;
    size_t typeLen = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &typeLen);

    NAPI_ASSERT(env, typeLen > 0, "typeLen == 0");
    type = new char[typeLen + 1];

    napi_get_value_string_utf8(env, argv[0], type, typeLen + 1, &typeLen);

    agent->Off(type);

    delete[] type;
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value JsConstructor(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 2;
    napi_value argv[2] = { nullptr };
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr);

    char bundleName[SendFile::SENDFILE_NAPI_BUF_LENGTH] = { 0 };
    size_t typeLen = 0;
    napi_get_value_string_utf8(env, argv[0], bundleName, sizeof(bundleName), &typeLen);
    LOGI("JsConstructor. [%{public}s]", bundleName);

    std::shared_ptr<EventAgent> agent;
    {
        std::unique_lock<std::mutex> lock(SendFile::g_uidMutex);
        if (SendFile::mapUidToEventAgent_.end() != SendFile::mapUidToEventAgent_.find(SendFile::BUNDLE_ID_)) {
            agent = SendFile::mapUidToEventAgent_[SendFile::BUNDLE_ID_];
            LOGI("JsConstructor: event agent for [%{public}s] is existed.", bundleName);
        } else if (SendFile::mapUidToEventAgent_.size() <= SendFile::MAX_SEND_FILE_HAP_NUMBER) {
            agent = std::make_shared<EventAgent>(env, thisVar);
            auto [ignored, inserted] = SendFile::mapUidToEventAgent_.insert(make_pair(SendFile::BUNDLE_ID_, agent));
            if (!inserted) {
                LOGE("map bundle to event agent error.");
                return nullptr;
            } else {
                LOGI("map size %{public}d", SendFile::mapUidToEventAgent_.size());
            }
        }
    }

    napi_wrap(env, thisVar, agent.get(),
        [](napi_env env, void* data, void* hint) {
            LOGI("SendFile Js Desconstructor1.");
            auto iter = SendFile::mapUidToEventAgent_.find(SendFile::BUNDLE_ID_);
            if (SendFile::mapUidToEventAgent_.end() != iter) {
                auto agent = (EventAgent*)data;
                if (agent != nullptr) {
                    LOGI("SendFile Js Desconstructor2.");
                    std::unique_lock<std::mutex> lock(SendFile::g_uidMutex);
                    agent->ClearDevice();
                    SendFile::mapUidToEventAgent_.erase(SendFile::BUNDLE_ID_);
                }
            }
        },
        nullptr, nullptr);

    return thisVar;
}

/***********************************************
 * Module export and register
 ***********************************************/
napi_value SendFileExport(napi_env env, napi_value exports)
{
    const char className[] = "SendFile";
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("sendFile", JsSendFile),
        DECLARE_NAPI_FUNCTION("on", JsOn),
        DECLARE_NAPI_FUNCTION("off", JsOff),
    };
    napi_value sendFileClass = nullptr;

    napi_define_class(env, className, sizeof(className), JsConstructor, nullptr,
        sizeof(desc) / sizeof(desc[0]), desc, &sendFileClass);

    napi_set_named_property(env, exports, "SendFile", sendFileClass);

    SendFile::RegisterCallback();
    return exports;
}

NAPI_MODULE(sendfile, SendFileExport)
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS