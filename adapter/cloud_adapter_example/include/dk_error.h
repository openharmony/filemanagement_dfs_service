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
namespace Json {
class Value;
}

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
    LOCAL_SPACE_FULL, // 本地空间不足
    TASK_CANCEL_FAIL,
    TASK_CANCEL,
    DECRYPT_FAIL,
    OPEN_FILE_FAIL,
    INIT_DECRYPT_FAIL,
    DOWNLOAD_REQUEST_ERROR,
    GET_DECRYPT_FAIL,
    GET_ASSET_FAIL,
    CREAT_TEMP_FILE_FAIL,
    ADD_SLICE_TASK_FAIL,
    SYNC_SWITCH_OFF,
    DOWNLOAD_PATH_ERROR,
    PREAD_MULTI_SLICE_NO_SUPPORT,
    RENAME_TEMPFILE_FAIL,
    SLTCE_NOT_DOWN_ALL,
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
    UID_EMPTY = 1003,         // 云空间未登录, userid为空
    RESPONSE_EMPTY = 1004,    // 服务器端返回为空
    RESPONSE_NOT_OK = 1005,   // 服务端返回结果错误
    NO_NETWORK = 1006,        // 云空间没有网络
    GRS_NULL = 1007,          // grs为空
    NETWORK_ERROR = 1008,     // 云空间网络异常
    ERROR_PARAM = 1009,       // 参数错误
    GET_AT_FAIL = 1010,       // 获取AT失效
    ASSET_NOT_EXIST = 1011,   // Asset不存在
    DELETE_ASSET_FAIL = 1012, // 删除Asset失败
    SWITCH_OFF = 1013,        // 同步开关关闭
};

enum class DKDetailErrorCode {
    LACK_OF_PARAM = 4001,                                       //缺少入参
    PARAM_INVALID = 4002,                                       //参数校验失败
    PARAM_VALUE_NOT_SUPPORT = 4003,                             //参数不支持
    PARAM_EXPIRED = 4004,                                       //入参已失效
    SITE_NOT_FOUND = 4005,                                      //站点信息未知
    NON_SUPPORT_CHARACTER_INCLUDED = 4006,                      //包含有不支持的字符
    ILLEGAL_CHARACTER_INCLUDED = 4007,                          //包含有非法的字符
    PARAMETER_LENGTH_BEYOND_LIMIT = 4008,                       //参数长度超过限制
    PARENTFOLDER_NOT_FOUND = 4009,                              //父目录不在
    TOKEN_CFG_INVALID = 4010,                                   //token配置项无效
    USER_NOT_AUTHORIZED = 4011,                                 //用户没有授权
    APP_NOT_AUTHORIZED = 4012,                                  //App没有授权
    TICKET_INVALID = 4013,                                      //ticket无效
    GRANT_CANCEL = 4014,                                        //取消授权
    LOGIN_FAILED = 4015,                                        //登录失败
    SESSION_TIMEOUT = 4016,                                     //会话过期
    FORCE_RELOGIN = 4017,                                       //强制重新登录账号
    FLOW_ID_NOT_MATCH = 4020,                                   //越权
    DATA_MIGRATING = 4031,                                      //数据割接未完成
    SERVICE_NOT_SUPPORT = 4032,                                 //服务未开放
    AGREEMENT_NOT_SIGNED = 4033,                                //协议未签署
    CROSS_SITE_NOT_SUPPORT = 4034,                              //跨站点功能限制
    INSUFFICIENT_SCOPE = 4035,                                  //Scope校验失败
    INSUFFICIENT_PERMISSION = 4036,                             //无此操作权限
    OPERATION_NOT_COMPLETE = 4037,                              //由于错误无法完成操作
    OUTER_SERVICE_ERROR = 4038,                                 //外部服务错误且无法恢复
    SPACE_FULL = 4039,                                          //用户空间不足
    CONTENT_NOT_FIND = 4041,                                    //内容未发现
    CONTENT_UNAVAILABLE = 4042,                                 //内容解析失败
    CHANNEL_NOT_FOUND = 4043,                                   //订阅未发现
    THUMBNAIL_NOT_FOUND = 4044,                                 //缩略图不存在
    SHARE_LINK_NOT_FOUND = 4045,                                //分享链接不存在
    TEMP_DATA_INVALID = 4046,                                   //临时数据无效
    FILE_NOT_FOUND = 4047,                                      //实体数据不存在
    APP_NOT_EXISTS = 4048,                                      //APP不存在
    CATEGORY_NOT_EXISTS = 4049,                                 //分类不存在
    SHARE_CONTENT_NOT_EXISTS = 4050,                            //分享内容不存在
    THUMBNAIL_GENERATE_FAILED = 4051,                           //缩略图生成失败
    VERSION_CONFLICT = 4090,                                    //版本冲突
    LOCK_FAILED = 4091,                                         //获取锁失败
    SILENCE_USER_FAILED = 4092,                                 //沉默用户拒绝失败
    FILE_USING_FORBIDDEN_OP = 4093,                             //文件正在使用禁止操作
    CURSOR_EXPIRED = 4100,                                      //游标过期
    TEMP_DATA_CLEARD = 4101,                                    //临时数据已经被清理
    CLOUD_DATA_UPDATED = 4121,                                  //云端数据已被更新
    USER_REQUEST_TOO_MANY = 4291,                               //用户请求太频繁
    APP_REQUEST_TOO_MANY = 4292,                                //应用请求太频繁
    FLOW_CONTROL = 4293,                                        //用户请求流控
    USER_REQUEST_ERROR_TOO_MANY = 4294,                         //用户请求错误次数超过限制
    PARTIAL_FILE_NOT_SUPPORT_SHARE = 4905,                      //部分文件不支持分享
    PARENTFOLDER_TRASHED = 4906,                                //父目录被删除到回收站
    ACCOUNT_NAME_WRONG = 4907,                                  //用户账号错误
    CIPHER_INVALID = 4908,                                      //用户秘钥已经失效
    DUPLICATED_ID = 4909,                                       //id已存在
    VUDID_IMEI_INVALID = 4910,                                  //VUDID转换IMEI失败
    FILE_NOT_SUPPORT_SHARE = 4911,                              //所有文件不支持分享
    EXTERNAL_LINK_NOT_AUTHORIZED = 4912,                        //外部链接没有授权
    ORIGINAL_NOT_EXSIT = 4913,                                  //源文件不存在
    ABNORMAL_DOWNLOAD = 4931,                                   //用户异常下载
    SAME_FILENAME_NOT_ALLOWED = 4932,                           //禁止重复文件名
    CANNOT_USE_SERVICE = 4933,                                  //无法为此用户提供服务
    FILES_NUM_BEYOND_LIMIT = 4934,                              //下载文件数超过限制
    FILES_SIZE_BEYOND_LIMIT = 4935,                             //下载文件大小超过限制
    TIER_BEYOND_LIMIT = 4936,                                   //层级超过限制
    HISTORYVERSIONS_BEYOND_LIMIT = 4937,                        //历史版本数超过限制
    COPY_FORBIDDEN = 4938,                                      //此文件禁止拷贝
    USER_SUSPEND_SERVICE = 4939,                                //用户停用服务
    FILE_VERSION_CONFLICT = 4940,                               //文件版本冲突
    REAL_NAME_AUTHENTICATION_FAILED = 4941,                      //实名认证失败
    SHARE_LINK_EXPIRED = 4942,                                  //分享链接已过期
    RECEIVER_BEYOND_LIMIT = 4943,                               //分享人数达到限制
    CONTENT_SHARE_NOT_ALLOWED = 4944,                           //内容禁止分享
    DATA_NUMBER_BEYOND_LIMIT = 4945,                            //云端数据超过限制
    FORBIDDEN_USER = 4946,                                      //禁止用户操作
    PARAM_CAN_NOT_UPDATE = 4947,                                //参数禁止更新
    HORIZONTAL_PRIVILEGE_ESCALATION = 4948,                     //存在横向越权
    OPERATION_FORBIDDEN_IN_RECYCLE = 4949,                      //禁止在回收站进行此操作
    FILENAME_LENGTH_BEYOND_LIMIT = 4950,                        //文件名超过上限
    OPERATION_FORBIDDEN_DELETE_BACKUP_EXIST = 4951,             //禁止删除操作，存在clearTime内的备份记录
    TEMP_KEY_EXPIRED = 4952,                                    //临时秘钥已过期
    USER_SHARE_NOT_ALLOWED = 4953,                              //用户被封禁，禁止分享
    USER_SHARE_PRIVILEGE_LIMITED = 4954,                        //用户没有权益
    CONTENT_COPYRIGHT_LIMIT = 4955,                             //版权文件禁止下载
    RISK_MANAGE_FAILED = 4956,                                  //风控失败
    TASK_RUNNING = 4959,                                        //任务正在执行
    SERVER_VERSION_UNAVAILABLE = 4960,                          //服务版本号不可用
    APP_STATUS_ABNORMAL = 4961,                                 //APP状态不正常
    CATEGORY_STATUS_ABNORMAL = 4962,                            //分类状态不正常
    CATEGORY_APP_ASSOCIATIONS_EXISTS = 4963,                    //存在应用和分类关系
    FUNCTION_NOT_SUPPORT = 4964,                                //功能暂不支持
    MEMBER_KINSHIP_EXISTS = 4965,                               //成员关系已存在
    MEMBER_NUMBER_LIMIT = 4966,                                 //成员人数已达上限
    USER_IS_JOINED = 4967,                                      //用户已经加入群组
    USER_BE_FROZEN = 4968,                                      //用户被冻结
    INVITER_LINK_EXPIRED = 4969,                                //邀请码或链接已过期
    RISK_SCANNING = 4970,                                       //分享内容审核中禁止访问
    ALBUM_APPLICATION_NUMBER_LIMIT = 4971,                      //共享相册申请加入次数超过上限
    INVITE_CODE_ERROR = 4972,                                   //邀请码错误
    INVITER_LINK_USED = 4973,                                   //邀请码或链接已使用过
    APPLICANT_IS_EXIST = 4974,                                  //记录已存在
    APPLICANT_NOT_FOUND = 4975,                                 //记录不存在
    APPLICANT_IS_EXPIRED = 4976,                                //记录已失效
    DATA_CLEARED_FORBIDDEN = 4977,                              //云侧数据清理禁止访问
    APPEAL_ACCOUNT_FORBIDDEN = 4978,                            //人工封禁不支持申诉
    APPLICANT_IS_PROCESSED = 4979,                              //申请已经处理过
    RESOURCE_NOT_MATCH = 4980,                                  //备份记录资源归属不匹配
    RISKFILE_FORBIDDEN_DOWN = 4981,                             //文件违规下载
    CONTENT_IS_EXPIRED = 4982,                                  //内容已过期
    BATCH_IS_EXPIRED = 4983,                                    //批次失效
    USER_NOT_REALNAME = 4984,                                   //用户未实名认证
    SERVER_IS_BUSY = 5001,                                      //服务器资源不够
    OUTER_SERVICE_UNAVAILABLE = 5002,                           //外部服务不可用
    OUTER_SERVICE_BUSY = 5003,                                  //外部服务忙
    DATABASE_UNAVAILABLE = 5004,                                //数据库操作失败
    RESOURCE_LOCKED = 5005,                                     //资源被锁住，暂时禁止操作
    SERVER_TEMP_ERROR = 5006,                                   //服务器临时错误
    SERVER_UPDATING = 5007,                                     //服务器升级中
    RESOURCE_TRASHING = 5008,                                   //资源GC中，暂时禁止操作
    SERVICE_UNAVAILABLE = 5030,                                 //服务不可用
    NSP_FLOW_CONTROL = 5031,                                    //NSP_FLOW_CONTROL
    TEMP_ERROR_RETRY = 5040,                                    //触发端侧重试错误码
};

struct DKErrorDetail {
    std::string domain;
    std::string reason;
    std::string errorCode;
    std::string description;
    std::string errorPos;
    std::string errorParam;
    int detailCode;
};

enum class DKErrorType {
    TYPE_UNKNOWN = 0,
    TYPE_NOT_NEED_RETRY,
    TYPE_MAX,
};

class DKError {
public:
    bool HasError() const;
    DKErrorType GetErrorType() const;
    void SetLocalError(DKLocalErrorCode code);
    void SetServerError(int code);
    void ParseErrorFromJson(const Json::Value &jvError);

    bool isLocalError = false;
    DKLocalErrorCode dkErrorCode;
    bool isServerError = false;
    int serverErrorCode;
    std::string reason;
    std::vector<DKErrorDetail> errorDetails;
    int retryAfter;
    DKErrorType errorType = DKErrorType::TYPE_UNKNOWN;
};
} // namespace DriveKit
#endif
