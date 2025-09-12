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

#ifndef FILEMANAGEMENT_KIT_OH_CLOUD_DISK_MANAGER_NDK_H
#define FILEMANAGEMENT_KIT_OH_CLOUD_DISK_MANAGER_NDK_H

#include <cstddef>
#include <cstdint>

#include "oh_cloud_disk_error_code.h"

/**
 * @addtogroup CloudDiskManager
 * @{
 *
 * @brief Provides APIs and error code for managing cloud disks.
 *
 * @since 21
 * @version 1.0
 */

/**
 * @file oh_cloud_disk_manager.h
 *
 * @brief Provides APIs for managing cloud disks.
 * @library libohclouddiskmanager.so
 * @syscap SystemCapability.FileManagement.CloudDiskManager
 * @since 21
 * @version 1.0
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumerates the synchronization states of a cloud disk.
 * @since 21
 *
 * This enumeration defines the possible states during cloud disk synchronization.
 */
typedef enum CloudDisk_SyncState {
    /**
     * The cloud disk is idle and not performing any synchronization.
     */
    IDLE = 0,
    /**
     * The cloud disk is currently synchronizing.
     */
    SYNCING = 1,
    /**
     * The cloud disk synchronization completed successfully.
     */
    SYNC_SUCCESSED = 2,
    /**
     * The cloud disk synchronization failed.
     */
    SYNC_FAILED = 3,
    /**
     * The cloud disk synchronization was canceled.
     */
    SYNC_CANCELED = 4,
    /**
     * The cloud disk synchronization encountered a conflict.
     */
    SYNC_CONFLICTED = 5,
} CloudDisk_SyncState;

/**
 * @brief Enumerates the operation types for file changes in the cloud disk.
 * @since 21
 */
typedef enum CloudDisk_OperationType {
    /**
     * Create a file or folder.
     */
    CREATE = 0,
    /**
     * Delete a file or folder.
     */
    DELETE = 1,
    /**
     * Move a file or folder from a source location.
     */
    MOVE_FROM = 2,
    /**
     * Move a file or folder to a target location.
     */
    MOVE_TO = 3,
    /**
     * Close a file after writing to operations.
     */
    CLOSE_WRITE = 4,
    /**
     * The sync folder path is invalid.
     */
    SYNC_FOLDER_INVALID = 5,
} CloudDisk_OperationType;

/**
 * @brief Enumerates the error reasons for file synchronization failures.
 * @since 21
 */
typedef enum CloudDisk_ErrorReason {
    /**
     * The input parameter is invalid.
     */
    INVALID_ARGUMENT = 0,
    /**
     * The specified file does not exist.
     */
    NO_SUCH_FILE = 1,
    /**
     * There is no space left on the device.
     */
    NO_SPACE_LEFT = 2,
    /**
     * The operation is out of the valid range.
     */
    OUT_OF_RANGE = 3,
    /**
     * The sync state is not set for the cloud disk.
     */
    NO_SYNC_STATE = 4,
} CloudDisk_ErrorReason;

/**
 * @brief Represents file path information.
 * @since 21
 */
typedef struct CloudDisk_PathInfo {
    /**
     * The file path as a null-terminated string.
     */
    char *value;
    /**
     * The length of the file path, excluding the null terminator.
     */
    size_t length;
} CloudDisk_PathInfo;

/**
 * @brief Defines the file ID information.
 * @since 21
 */
typedef CloudDisk_PathInfo CloudDisk_FileIdInfo;

/**
 * @brief Defines the sync folder path information.
 * @since 21
 */
typedef CloudDisk_PathInfo CloudDisk_SyncFolderPath;

/**
 * @brief Defines file sync states.
 * @since 21
 */
typedef struct CloudDisk_FileSyncState {
    /**
     * Indicates the file path information.
     */
    CloudDisk_PathInfo filePathInfo;
    /**
     * Indicates the synchronization state of the file.
     */
    CloudDisk_SyncState syncState;
} CloudDisk_FileSyncState;

/**
 * @brief Defines the data structure for a single file change event in the sync folder.
 * @since 21
 *
 * This structure contains detailed information about a file change, including its unique ID,
 * parent ID, relative path, change type, file size, and timestamps.
 */
typedef struct CloudDisk_ChangeData {
    /**
     * The update sequence number for the change event.
     * Increases by 1 each time the file is changed, and is monotonically increasing.
     * Used for incremental change queries.
     */
    uint64_t updateSequenceNumber{0};
    /**
     * The globally unique file ID. Remains unchanged during the lifecycle of the file.
     */
    CloudDisk_FileIdInfo fileId;
    /**
     * The parent file ID information.
     */
    CloudDisk_FileIdInfo parentFileId;
    /**
     * The relative path information of the file.
     */
    CloudDisk_PathInfo relativePathInfo;
    /**
     * The type of operation for this file change (such as create, delete, move, etc.).
     */
    CloudDisk_OperationType operationType;
    /**
     * The size of the file in bytes.
     */
    uint64_t size{0};
    /**
     * The last modified time of the file, in milliseconds.
     */
    uint64_t mtime{0};
    /**
     * The timestamp of the change event, in milliseconds.
     */
    uint64_t timeStamp{0};
} CloudDisk_ChangeData;

/**
 * @brief Represents the result of querying file changes in a sync folder.
 * @since 21
 *
 * This structure contains the incremental change data for files in a sync folder,
 * including the next update sequence number, end-of-log flag, and an array of change data items.
 */
typedef struct CloudDisk_ChangesResult {
    /**
     * The next update sequence number for incremental queries.
     */
    uint64_t nextUsn{0};
    /**
     * Indicates whether the end of the change log is reached.
     * If true, all file changes have been returned.
     */
    bool isEof{false};
    /**
     * The number of change data items in the array.
     */
    size_t length{0};
    /**
     * The array of file change data items.
     */
    CloudDisk_ChangeData dataDatas[];
} CloudDisk_ChangesResult;

/**
 * @brief Represents a failed file in a synchronization operation.
 * @since 21
 *
 * This structure contains the file path information and the specific error reason for the failure.
 */
typedef struct CloudDisk_FailedList {
    /**
     * The file path information of the failed file.
     */
    CloudDisk_PathInfo pathInfo;
    /**
     * The error reason for the file synchronization failure.
     */
    CloudDisk_ErrorReason errorReason;
} CloudDisk_FailedList;

/**
 * @brief Represents the result of a file synchronization operation.
 * @since 21
 *
 * This structure contains the absolute path of the file, the synchronization result,
 * and either the sync state or the error reason depending on the operation outcome.
 */
typedef struct CloudDisk_ResultList {
    /**
     * The absolute path information of the file.
     */
    CloudDisk_PathInfo pathInfo;
    /**
     * Indicates whether the synchronization operation was successful.
     * If true, syncState is valid; if false, errorReason is valid.
     */
    bool isSuccess;
    /**
     * The synchronization state of the file. Valid only if isSuccess is true.
     */
    CloudDisk_SyncState syncState;
    /**
     * The error reason for the file synchronization failure. Valid only if isSuccess is false.
     */
    CloudDisk_ErrorReason errorReason;
} CloudDisk_ResultList;

/**
 * @brief Enumerates the state of sync folder.
 * @since 21
 */
typedef enum CloudDisk_SyncFolderState {
    /**
     * Indicates that the state of sync folder is inactive.
     */
    INACTIVE = 0,
    /**
     * Indicates that the state of sync folder is active.
     */
    ACTIVE = 1,
} CloudDisk_SyncFolderState;

/**
 * @brief Defines the sync folder displayName info of cloud disk.
 * @since 21
 */
typedef struct CloudDisk_DisplayNameInfo {
    /**
     * Indicates the displayNameResId of sync folder.
     */
    uint32_t displayNameResId;
    /**
     * Indicates the user-defined alias.
     */
    char *customAlias;
    /**
     * Indicates the length of the customAlias.
     */
    size_t customAliasLength;
} CloudDisk_DisplayNameInfo;

/**
 * @brief Defines the sync folder of cloud disk.
 * @since 21
 */
typedef struct CloudDisk_SyncFolder {
    /**
     * Indicates the path of sync folder.
     */
    CloudDisk_SyncFolderPath path;
    /**
     * Indicates the state of sync folder.
     */
    CloudDisk_SyncFolderState state;
    /**
     * Indicates the displayName info of sync folder.
     */
    CloudDisk_DisplayNameInfo displayNameInfo;
} CloudDisk_SyncFolder;

/**
 * @brief Registers a callback function to notify the application of sync folder changes.
 *
 * @param syncFolderPath Indicates the sync folder path information.
 * @param callback Indicates the callback function to be registered.
 * @return Returns {@link CLOUD_DISK_ERR_OK} if the callback function is registered successfully;
 * <br> returns an error code defined in {@link oh_cloud_disk_error_code.h} otherwise.
 * @since 21
 */
CloudDisk_ErrorCode
    OH_CloudDisk_RegisterSyncFolderChanges(const CloudDisk_SyncFolderPath syncFolderPath,
                                          void (*callback)(const CloudDisk_SyncFolderPath syncFolderPath,
                                                           const CloudDisk_ChangeData changeDatas[], size_t length));

/**
 * @brief Unregisters a callback function for sync folder changes.
 *
 * @param syncFolderPath Indicates the sync folder path information.
 * @return Returns {@link CLOUD_DISK_ERR_OK} if the callback function is unregistered successfully;
 * <br> returns an error code defined in {@link oh_cloud_disk_error_code.h} otherwise.
 * @since 21
 */
CloudDisk_ErrorCode OH_CloudDisk_UnregisterSyncFolderChanges(const CloudDisk_SyncFolderPath syncFolderPath);

/**
 * @brief Gets the change datas of specified folder.
 *
 * @param syncFolderPath Indicates the sync folder path information.
 * @param startUsn Indicates the start update sequence number.
 * @param count Indicates the number of files.
 * @param changesResult Indicates the result of querying file changes. For details, see {@link CloudDisk_ChangesResult}.
 * @return Returns {@link CLOUD_DISK_ERR_OK} if the callback function is unregistered successfully;
 * <br> returns an error code defined in {@link oh_cloud_disk_error_code.h} otherwise.
 * @since 21
 */
CloudDisk_ErrorCode OH_CloudDisk_GetSyncFolderChanges(const CloudDisk_SyncFolderPath syncFolderPath,
                                                      uint64_t startUsn,
                                                      size_t count,
                                                      CloudDisk_ChangesResult **changesResult);

/**
 * @brief Sets the synchronization state for specified files in a sync folder.
 *
 * @param syncFolderPath Indicates the sync folder path information.
 * @param fileSyncStates The array of {@link CloudDisk_FileSyncState} specifying the file paths and their target sync
 * <br> states.
 * @param length The number of files to set.
 * @param failedLists Output parameter. Returns a pointer to an array of {@link CloudDisk_FailedList} for files that
 * <br> failed to set.
 * @param failedCount Output parameter. Returns the number of failed files in {@link failedLists}.
 * @return Returns {@link CLOUD_DISK_ERR_OK} if the operation is successful;
 * <br> otherwise, returns an error code defined in {@link oh_cloud_disk_error_code.h}.
 * @since 21
 */
CloudDisk_ErrorCode OH_CloudDisk_SetFileSyncStates(const CloudDisk_SyncFolderPath syncFolderPath,
                                                   const CloudDisk_FileSyncState fileSyncStates[],
                                                   size_t length,
                                                   CloudDisk_FailedList **failedLists,
                                                   size_t *failedCount);

/**
 * @brief Queries the synchronization state of specified files in a sync folder.
 *
 * @param syncFolderPath Indicates the sync folder path information.
 * @param paths The array of file paths to query.
 * @param length The number of file paths in the array.
 * @param resultLists Output parameter. Returns a pointer to an array of {@link CloudDisk_ResultList} containing the
 * <br> query results.
 * @param resultCount Output parameter. Returns the number of results in {@link resultLists}.
 * @return Returns {@link CLOUD_DISK_ERR_OK} if the query is successful;
 * <br> otherwise, returns an error code defined in {@link oh_cloud_disk_error_code.h}.
 * @since 21
 */
CloudDisk_ErrorCode OH_CloudDisk_GetFileSyncStates(const CloudDisk_SyncFolderPath syncFolderPath,
                                                   const CloudDisk_PathInfo paths[],
                                                   size_t length,
                                                   CloudDisk_ResultList **resultLists,
                                                   size_t *resultCount);

/**
 * @brief Registers a sync folder.
 *
 * @param syncFolder Indicates the sync folder.
 * @return Returns {@link CLOUD_DISK_ERR_OK} if the operation is successful;
 * <br> returns an error code defined in {@link oh_cloud_disk_error_code.h} otherwise.
 * @since 21
 */
CloudDisk_ErrorCode OH_CloudDisk_RegisterSyncFolder(const CloudDisk_SyncFolder *syncFolder);

/**
 * @brief Unregisters a sync folder.
 *
 * @param syncFolderPath Indicates the sync folder path.
 * @return Returns {@link CLOUD_DISK_ERR_OK} if the operation is successful;
 * <br> returns an error code defined in {@link oh_cloud_disk_error_code.h} otherwise.
 * @since 21
 */
CloudDisk_ErrorCode OH_CloudDisk_UnregisterSyncFolder(const CloudDisk_SyncFolderPath syncFolderPath);

/**
 * @brief Activates a sync folder.
 *
 * @param syncFolderPath Indicates the sync folder path.
 * @return Returns {@link CLOUD_DISK_ERR_OK} if the operation is successful;
 * <br> returns an error code defined in {@link oh_cloud_disk_error_code.h} otherwise.
 * @since 21
 */
CloudDisk_ErrorCode OH_CloudDisk_ActiveSyncFolder(const CloudDisk_SyncFolderPath syncFolderPath);

/**
 * @brief Deactivates a sync folder.
 *
 * @param syncFolderPath Indicates the sync folder path.
 * @return Returns {@link CLOUD_DISK_ERR_OK} if the operation is successful;
 * <br> returns an error code defined in {@link oh_cloud_disk_error_code.h} otherwise.
 * @since 21
 */
CloudDisk_ErrorCode OH_CloudDisk_DeactiveSyncFolder(const CloudDisk_SyncFolderPath syncFolderPath);

/**
 * @brief Gets the sync folders.
 *
 * @param syncFolders Output parameter. Returns the array of {@link CloudDisk_SyncFolder} to store the sync folders.
 * @param count Output parameter. Returns the number of sync folders.
 * @return Returns {@link CLOUD_DISK_ERR_OK} if the operation is successful;
 * <br> returns an error code defined in {@link oh_cloud_disk_error_code.h} otherwise.
 * @since 21
 */
CloudDisk_ErrorCode OH_CloudDisk_GetSyncFolders(CloudDisk_SyncFolder **syncFolders,
                                                size_t *count);

/**
 * @brief Updates the display name of a sync folder.
 *
 * @param syncFolderPath Indicates the sync folder path.
 * @param customAlias Indicates the user-defined alias.
 * @param customAliasLength Indicates the length of the customAlias.
 * @return Returns {@link CLOUD_DISK_ERR_OK} if the operation is successful;
 * <br> returns an error code defined in {@link oh_cloud_disk_error_code.h} otherwise.
 * @since 21
 */
CloudDisk_ErrorCode OH_CloudDisk_UpdateCustomAlias(const CloudDisk_SyncFolderPath syncFolderPath,
                                                   const char *customAlias,
                                                   size_t customAliasLength);
#ifdef __cplusplus
}
#endif
#endif // FILEMANAGEMENT_KIT_OH_CLOUD_DISK_MANAGER_NDK_H