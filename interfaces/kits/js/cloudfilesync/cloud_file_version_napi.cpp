/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "cloud_file_version_napi.h"

#include <sys/types.h>

#include "async_work.h"
#include "cloud_sync_manager.h"
#include "dfs_error.h"
#include "utils_log.h"
#include "uv.h"

namespace OHOS::FileManagement::CloudSync {
using namespace FileManagement::LibN;
using namespace std;

static const int64_t PERCENT = 100;
static const int32_t TOP_NUM = 10000;
static const int DEC = 10;
const int32_t ARGS_ONE = 1;

struct VersionParams {
    vector<HistoryVersion> versionList;
    string versionUri;
    bool isConflict = false;
    int64_t downloadId = 0;
};

napi_value FileVersionNapi::Constructor(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        LOGE("Start Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto fileVersionEntity = make_unique<FileVersionEntity>();
    if (!NClass::SetEntityFor<FileVersionEntity>(env, funcArg.GetThisVar(), move(fileVersionEntity))) {
        LOGE("Failed to set file cache entity.");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    return funcArg.GetThisVar();
}

static NVal AsyncComplete(const napi_env &env, const vector<HistoryVersion> &list)
{
    napi_value results = nullptr;

    napi_status status = napi_create_array(env, &results);
    if (status != napi_ok) {
        LOGE("Failed to create array");
        return {env, NError(LibN::STORAGE_SERVICE_SYS_CAP_TAG + LibN::E_IPCSS).GetNapiErr(env)};
    }

    int32_t index = 0;
    for (auto version : list) {
        NVal obj = NVal::CreateObject(env);
        obj.AddProp("editedTime", NVal::CreateInt64(env, version.editedTime).val_);
        obj.AddProp("fileSize", NVal::CreateBigIntUint64(env, version.fileSize).val_);
        obj.AddProp("versionId", NVal::CreateUTF8String(env, to_string(version.versionId)).val_);
        obj.AddProp("originalFileName", NVal::CreateUTF8String(env, version.originalFileName).val_);
        obj.AddProp("sha256", NVal::CreateUTF8String(env, version.sha256).val_);
        obj.AddProp("autoResolved", NVal::CreateBool(env, version.resolved).val_);
        status = napi_set_element(env, results, index, obj.val_);
        if (status != napi_ok) {
            LOGE("Failed to set element on data");
            return {env, NError(LibN::STORAGE_SERVICE_SYS_CAP_TAG + LibN::E_IPCSS).GetNapiErr(env)};
        }
        index++;
    }
    return {env, results};
}

static tuple<bool, string, int32_t> GetHistoryVersionListParam(const napi_env &env, const NFuncArg &funcArg)
{
    auto [succ, uriNVal, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        LOGE("unavailable uri.");
        return {false, "", 0};
    }
    string uri = uriNVal.get();
    if (uri.empty()) {
        LOGE("unavailable uri.");
        return {false, "", 0};
    }
    auto [succLimit, numLimit] = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
    if (!succLimit || numLimit <= 0) {
        LOGE("unavailable version num limit.");
        return {false, "", 0};
    }

    return make_tuple(true, move(uri), numLimit >= TOP_NUM ? TOP_NUM : numLimit);
}

napi_value FileVersionNapi::GetHistoryVersionList(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO)) {
        LOGE("Start Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succ, uriNVal, numNVal] = GetHistoryVersionListParam(env, funcArg);
    if (!succ) {
        LOGE("param is unavailable.");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto param = make_shared<VersionParams>();
    auto cbExec = [uri{uriNVal}, num{numNVal}, param]() -> NError {
        int32_t ret = CloudSyncManager::GetInstance().GetHistoryVersionList(uri, num, param->versionList);
        if (ret != E_OK) {
            LOGE("get history version list faild, ret = %{public}d", ret);
            return NError(Convert2JsErrNum(ret));
        }
        return NError(NO_ERROR);
    };

    auto cbCompl = [param](napi_env env, NError err) -> NVal {
        if (err) {
            return {env, err.GetNapiErr(env)};
        }
        return AsyncComplete(env, param->versionList);
    };

    string procedureName = "fileVersion";
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, static_cast<size_t>(NARG_CNT::THREE));
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbCompl).val_;
}

static bool SafeStringToUInt(const string &str, uint64_t &value)
{
    char *endPtr = nullptr;
    errno = 0;

    value = strtoull(str.c_str(), &endPtr, DEC);
    if (str.c_str() == endPtr || *endPtr != '\0' || errno == ERANGE) {
        LOGE("string to uint64 failed.");
        return false;
    }

    return true;
}

static tuple<bool, string, uint64_t, NVal> GetDownloadVersionParam(const napi_env &env, const NFuncArg &funcArg)
{
    NVal res;
    auto [succ, uriNVal, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        LOGE("unavailable uri.");
        return {false, "", 0, res};
    }
    string uri = uriNVal.get();
    if (uri.empty()) {
        LOGE("unavailable uri.");
        return {false, "", 0, res};
    }
    auto [succId, versionNVal, ignoreId] = NVal(env, funcArg[NARG_POS::SECOND]).ToUTF8String();
    if (!succId) {
        LOGE("unavailable version id.");
        return {false, "", 0, res};
    }
    string versionId = versionNVal.get();
    if (versionId.empty()) {
        LOGE("unavailable version id.");
        return {false, "", 0, res};
    }
    bool isDigit = std::all_of(versionId.begin(), versionId.end(), ::isdigit);
    if (!isDigit) {
        LOGE("unavailable version id.");
        return {false, "", 0, res};
    }
    uint64_t value = 0;
    if (!SafeStringToUInt(versionId, value)) {
        LOGE("unavailable version id.");
        return {false, "", 0, res};
    }

    NVal callbackVal(env, funcArg[NARG_POS::THIRD]);
    if (!callbackVal.TypeIs(napi_function)) {
        LOGE("download version argument type mismatch");
        return {false, "", 0, res};
    }

    return make_tuple(true, move(uri), value, move(callbackVal));
}

napi_value FileVersionNapi::DownloadHistoryVersion(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::THREE)) {
        LOGE("Start Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succ, uriNVal, idNVal, callbackRef] = GetDownloadVersionParam(env, funcArg);
    if (!succ) {
        LOGE("param is unavailable.");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto fileVersionEntity = NClass::GetEntityOf<FileVersionEntity>(env, funcArg.GetThisVar());
    if (!fileVersionEntity) {
        LOGE("Failed to get file version entity.");
        NError(Convert2JsErrNum(E_SERVICE_INNER_ERROR)).ThrowErr(env);
        return nullptr;
    }
    {
        lock_guard<mutex> lock(mtx_);
        if (fileVersionEntity->callbackImpl == nullptr) {
            fileVersionEntity->callbackImpl = make_shared<VersionDownloadCallbackImpl>(env, callbackRef.val_);
        }
    }
    auto param = make_shared<VersionParams>();
    auto cbExec = [uri{uriNVal}, id{idNVal}, callback{fileVersionEntity->callbackImpl}, param]() -> NError {
        if (!callback) {
            LOGE("download history version callback is null.");
            return NError(Convert2JsErrNum(E_SERVICE_INNER_ERROR));
        }
        int32_t ret = CloudSyncManager::GetInstance()
            .DownloadHistoryVersion(uri, param->downloadId, id, callback, param->versionUri);
        if (ret != E_OK) {
            LOGE("Start download history version failed! ret = %{public}d", ret);
            return NError(Convert2JsErrNum(ret));
        }
        return NError(NO_ERROR);
    };
    auto cbCompl = [param](napi_env env, NError err) -> NVal {
        if (err) {
            return {env, err.GetNapiErr(env)};
        }
        return NVal::CreateUTF8String(env, param->versionUri);
    };
    string procedureName = "fileVersion";
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, static_cast<size_t>(NARG_CNT::FOUR));
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbCompl).val_;
}

static tuple<bool, string, string> GetReplaceVersionListParam(const napi_env &env, const NFuncArg &funcArg)
{
    auto [succOri, uriOriNVal, ignoreOri] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succOri) {
        LOGE("unavailable origin uri.");
        return {false, "", ""};
    }
    string uriOri = uriOriNVal.get();
    if (uriOri.empty()) {
        LOGE("unavailable origin uri.");
        return {false, "", ""};
    }
    auto [succUri, uriNVal, ignore] = NVal(env, funcArg[NARG_POS::SECOND]).ToUTF8String();
    if (!succUri) {
        LOGE("unavailable uri.");
        return {false, "", ""};
    }
    string uri = uriNVal.get();
    if (uri.empty()) {
        LOGE("unavailable uri.");
        return {false, "", ""};
    }

    return make_tuple(true, move(uriOri), move(uri));
}

napi_value FileVersionNapi::ReplaceFileWithHistoryVersion(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO)) {
        LOGE("Start Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succ, oriUriNVal, uriNVal] = GetReplaceVersionListParam(env, funcArg);
    if (!succ) {
        LOGE("param is unavailable.");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto cbExec = [oriUri{oriUriNVal}, uri{uriNVal}]() -> NError {
        int32_t ret = CloudSyncManager::GetInstance().ReplaceFileWithHistoryVersion(oriUri, uri);
        if (ret != E_OK) {
            LOGE("replace history version failed! ret = %{public}d", ret);
            return NError(Convert2JsErrNum(ret));
        }
        return NError(NO_ERROR);
    };

    auto cbCompl = [](napi_env env, NError err) -> NVal {
        if (err) {
            return {env, err.GetNapiErr(env)};
        }
        return NVal::CreateUndefined(env);
    };

    string procedureName = "fileVersion";
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, static_cast<size_t>(NARG_CNT::THREE));
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbCompl).val_;
}

napi_value FileVersionNapi::IsConflict(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        LOGE("Start Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succ, uriNVal, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        LOGE("unavailable uri.");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    string uri = uriNVal.get();
    if (uri.empty()) {
        LOGE("unavailable uri.");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto param = make_shared<VersionParams>();
    auto cbExec = [uriIn{uri}, param]() -> NError {
        int32_t ret = CloudSyncManager::GetInstance().IsFileConflict(uriIn, param->isConflict);
        if (ret != E_OK) {
            LOGE("replace history version failed! ret = %{public}d", ret);
            return NError(Convert2JsErrNum(ret));
        }
        return NError(NO_ERROR);
    };

    auto cbCompl = [param](napi_env env, NError err) -> NVal {
        if (err) {
            return {env, err.GetNapiErr(env)};
        }
        return NVal::CreateBool(env, param->isConflict);
    };

    string procedureName = "fileVersion";
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, static_cast<size_t>(NARG_CNT::TWO));
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbCompl).val_;
}

napi_value FileVersionNapi::ClearFileConflict(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        LOGE("Start Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succ, uriNVal, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        LOGE("unavailable uri.");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    string uri = uriNVal.get();
    if (uri.empty()) {
        LOGE("unavailable uri.");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto cbExec = [uriIn{uri}]() -> NError {
        int32_t ret = CloudSyncManager::GetInstance().ClearFileConflict(uriIn);
        if (ret != E_OK) {
            LOGE("replace history version failed! ret = %{public}d", ret);
            return NError(Convert2JsErrNum(ret));
        }
        return NError(NO_ERROR);
    };

    auto cbCompl = [](napi_env env, NError err) -> NVal {
        if (err) {
            return {env, err.GetNapiErr(env)};
        }
        return NVal::CreateUndefined(env);
    };

    string procedureName = "fileVersion";
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, static_cast<size_t>(NARG_CNT::TWO));
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbCompl).val_;
}

bool FileVersionNapi::Export()
{
    std::vector<napi_property_descriptor> props = {
        NVal::DeclareNapiFunction("getHistoryVersionList", GetHistoryVersionList),
        NVal::DeclareNapiFunction("downloadHistoryVersion", DownloadHistoryVersion),
        NVal::DeclareNapiFunction("replaceFileWithHistoryVersion", ReplaceFileWithHistoryVersion),
        NVal::DeclareNapiFunction("isFileConflict", IsConflict),
        NVal::DeclareNapiFunction("clearFileConflict", ClearFileConflict),
    };
    std::string className = GetClassName();
    auto [succ, classValue] =
        NClass::DefineClass(exports_.env_, className, FileVersionNapi::Constructor, std::move(props));
    if (!succ) {
        NError(E_GETRESULT).ThrowErr(exports_.env_);
        LOGE("Failed to define FileVersion class");
        return false;
    }

    succ = NClass::SaveClass(exports_.env_, className, classValue);
    if (!succ) {
        NError(E_GETRESULT).ThrowErr(exports_.env_);
        LOGE("Failed to save FileVersion class");
        return false;
    }

    return exports_.AddProp(className, classValue);
}

string FileVersionNapi::GetClassName()
{
    return className_;
}

VersionDownloadCallbackImpl::VersionDownloadCallbackImpl(napi_env env, napi_value fun) : env_(env)
{
    if (fun != nullptr) {
        napi_create_reference(env_, fun, 1, &cbOnRef_);
    }
}

void VersionDownloadCallbackImpl::OnComplete(UvChangeMsg *msg)
{
    auto downloadcCallback = msg->downloadCallback_.lock();
    if (downloadcCallback == nullptr || downloadcCallback->cbOnRef_ == nullptr) {
        LOGE("downloadcCallback->cbOnRef_ is nullptr");
        return;
    }
    auto env = downloadcCallback->env_;
    auto ref = downloadcCallback->cbOnRef_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    napi_value jsCallback = nullptr;
    napi_status status = napi_get_reference_value(env, ref, &jsCallback);
    if (status != napi_ok) {
        LOGE("Create reference failed, status: %{public}d", status);
        napi_close_handle_scope(env, scope);
        return;
    }
    NVal obj = NVal::CreateObject(env);
    auto process = (msg->downloadProgress_.downloadedSize * PERCENT) / msg->downloadProgress_.totalSize;
    obj.AddProp("state", NVal::CreateInt32(env, (int32_t)msg->downloadProgress_.state).val_);
    obj.AddProp("progress", NVal::CreateInt64(env, process).val_);
    obj.AddProp("taskId", NVal::CreateInt64(env, msg->downloadProgress_.downloadId).val_);
    obj.AddProp("errType", NVal::CreateInt32(env, (int32_t)msg->downloadProgress_.downloadErrorType).val_);

    LOGI("OnComplete callback start for taskId: %{public}lld",
         static_cast<long long>(msg->downloadProgress_.downloadId));
    napi_value retVal = nullptr;
    napi_value global = nullptr;
    napi_get_global(env, &global);
    status = napi_call_function(env, global, jsCallback, ARGS_ONE, &(obj.val_), &retVal);
    if (status != napi_ok) {
        LOGE("napi call function failed, status: %{public}d", status);
    }
    napi_close_handle_scope(env, scope);
}

void VersionDownloadCallbackImpl::OnDownloadProcess(const DownloadProgressObj &progress)
{
    UvChangeMsg *msg = new (std::nothrow) UvChangeMsg(shared_from_this(), progress);
    if (msg == nullptr) {
        LOGE("Failed to create uv message object");
        return;
    }
    auto task = [msg]() {
        if (msg->downloadCallback_.expired()) {
            LOGE("downloadCallback_ is expired");
            delete msg;
            return;
        }
        msg->downloadCallback_.lock()->OnComplete(msg);
        delete msg;
    };
    napi_status ret = napi_send_event(env_, task, napi_event_priority::napi_eprio_immediate);
    if (ret != napi_ok) {
        LOGE("Failed to execute libuv work queue, ret: %{public}d", ret);
        delete msg;
        return;
    }
}

VersionDownloadCallbackImpl::~VersionDownloadCallbackImpl()
{
    if (cbOnRef_ != nullptr) {
        napi_status status = napi_delete_reference(env_, cbOnRef_);
        if (status != napi_ok) {
            LOGE("Failed to delete napi ref, %{public}d", status);
        }
        cbOnRef_ = nullptr;
    }
}
} // namespace OHOS::FileManagement::CloudSync