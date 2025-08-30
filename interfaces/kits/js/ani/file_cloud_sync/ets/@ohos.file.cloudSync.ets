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

import { BusinessError, AsyncCallback, Callback } from "@ohos.base";

export default namespace cloudSync {

    export enum ErrorType {
        NO_ERROR = 0,
        NETWORK_UNAVAILABLE = 1,
        WIFI_UNAVAILABLE = 2,
        BATTERY_LEVEL_LOW = 3,
        BATTERY_LEVEL_WARNING = 4,
        CLOUD_STORAGE_FULL = 5,
        LOCAL_STORAGE_FULL = 6,
        DEVICE_TEMPERATURE_TOO_HIGH = 7,
        REMOTE_SERVER_ABNORMAL = 8,
    };

    export enum SyncState {
        UPLOADING = 0,
        UPLOAD_FAILED = 1,
        DOWNLOADING = 2,
        DOWNLOAD_FAILED = 3,
        COMPLETED = 4,
        STOPPED = 5,
    };

    export interface SyncProgress {
        state: SyncState;
        error: ErrorType;
    }

    export class SyncProgressInner implements SyncProgress {
        constructor(
            syncState: SyncState,
            errorType: ErrorType
        ) {
            this.state = syncState;
            this.error = errorType;
        }
        public state: SyncState;
        public error: ErrorType;
    }

    export type GallerySyncCallback = (pg: SyncProgress) => void;

    export class GallerySync {
        static {
            loadLibrary("cloud_sync_ani");
        }
        private nativePtr: long = 0;
        native constructor();

        bindNativePtr(ptr: long): void {
            if (this.nativePtr == 0) {
                this.nativePtr = ptr;
            }
        }

        native on(evt: string, callback: GallerySyncCallback): void;

        native off(evt: string, callback: GallerySyncCallback): void;

        native off(evt: string): void;

        start(): Promise<void> {
            return new Promise<void>((resolve: (result: undefined) => void,
                reject: (e: BusinessError<void>) => void): void => {
                let promise = taskpool.execute((): int => this.StartInner());
                promise.then<void>((): void => {
                    resolve(undefined);
                }, (e: BusinessError<void>): void => {
                    reject(e);
                });
            });
        }

        start(callback: AsyncCallback<void>): void {
            let promise = taskpool.execute((): int => this.StartInner());
            promise.then<void>((): void => {
                let e = new BusinessError<void>();
                e.code = 0;
                callback(e, undefined);
            }, (e: BusinessError<void>): void => {
                callback(e, undefined);
            });
        }

        stop(): Promise<void> {
            return new Promise<void>((resolve: (result: undefined) => void,
                reject: (e: BusinessError<void>) => void): void => {
                let promise = taskpool.execute((): int => this.StopInner());
                promise.then<void>((): void => {
                    resolve(undefined);
                }, (e: BusinessError<void>): void => {
                    reject(e);
                });
            });
        }

        stop(callback: AsyncCallback<void>): void {
            let promise = taskpool.execute((): int => this.StopInner());
            promise.then((): void => {
                let e = new BusinessError<void>();
                e.code = 0;
                callback(e, undefined);
            }, (e: BusinessError<void>): void => {
                callback(e, undefined);
            });
        }

        native GallerySyncStart(): void;
        native GallerySyncStop(): void;

        StartInner(): int {
            this.GallerySyncStart();
            return 0;
        }
        StopInner(): int {
            this.GallerySyncStop();
            return 0;
        }
    };

    export enum State {
        RUNNING = 0,
        COMPLETED = 1,
        FAILED = 2,
        STOPPED = 3
    };

    export enum DownloadErrorType {
        NO_ERROR = 0,
        UNKNOWN_ERROR = 1,
        NETWORK_UNAVAILABLE = 2,
        LOCAL_STORAGE_FULL = 3,
        CONTENT_NOT_FOUND = 4,
        FREQUENT_USER_REQUESTS = 5,
    };

    export interface DownloadProgress {
        taskId: number;
        state: State;
        processed: number;
        size: number;
        uri: string;
        error: DownloadErrorType;
    }

    export class DownloadProgressInner implements DownloadProgress {
        constructor(
            taskId: number,
            state: State,
            processed: number,
            size: number,
            uri: string,
            error: DownloadErrorType
        ) {
            this.taskId = taskId
            this.state = state;
            this.processed = processed;
            this.size = size;
            this.uri = uri;
            this.error = error;
        }
        taskId: number;
        public state: State;
        processed: number;
        size: number;
        uri: string;
        error: DownloadErrorType;
    };

    export class MultiDownloadProgress {
        static {
            loadLibrary("cloud_sync_ani");
        }
        state: State;
        taskId: number;
        successfulCount: number;
        failedCount: number;
        totalCount: number;
        downloadedSize: number;
        totalSize: number;
        errType: DownloadErrorType;

        private nativePtr: long = 0;

        constructor(ptr: long) {
            if (this.nativePtr === 0) {
                this.nativePtr = ptr;
            }
        }
        native getFailedFiles(): Array<FailedFileInfo>;
        native getSuccessfulFiles(): Array<string>;
    };

    export type DownloadcCallback = (pg: DownloadProgress) => void;

    export class Download {
        static {
            loadLibrary("cloud_sync_ani");
        }
        private nativePtr: long = 0;
        native constructor();

        bindNativePtr(ptr: long): void {
            if (this.nativePtr == 0) {
                this.nativePtr = ptr;
            }
        }

        native on(evt: string, callback: DownloadcCallback): void;

        native off(evt: string, callback: DownloadcCallback): void;

        native off(evt: string): void;

        start(uri: string): Promise<void> {
            return new Promise<void>((resolve: (result: undefined) => void,
                reject: (e: BusinessError) => void): void => {
                let promise = taskpool.execute((uri: string): int => this.StartInner(uri), uri);
                promise.then<void>((): void => {
                    resolve(undefined);
                }, (e: BusinessError<void>): void => {
                    reject(e);
                });
            });
        }

        start(uri: string, callback: AsyncCallback<void>): void {
            let promise = taskpool.execute((uri: string): int => this.StartInner(uri), uri);
            promise.then<void>((): void => {
                let e = new BusinessError<void>();
                e.code = 0;
                callback(e, undefined);
            }, (e: BusinessError<void>): void => {
                callback(e, undefined);
            });
        }

        stop(uri: string): Promise<void> {
            return new Promise<void>((resolve: (result: undefined) => void,
                reject: (e: BusinessError<void>) => void): void => {
                let promise = taskpool.execute((uri: string): int => this.StopInner(uri), uri);
                promise.then<void>((): void => {
                    resolve(undefined);
                }, (e: BusinessError<void>): void => {
                    reject(e);
                });
            });
        }

        stop(uri: string, callback: AsyncCallback<void>): void {
            let promise = taskpool.execute((uri: string): int => this.StopInner(uri), uri);
            promise.then<void>((): void => {
                let e = new BusinessError<void>();
                e.code = 0;
                callback(e, undefined);
            }, (e: BusinessError<void>): void => {
                callback(e, undefined);
            });
        }

        native DownloadStart(uri: string): void;
        native DownloadStop(uri: string): void;

        StartInner(uri: string): int {
            this.DownloadStart(uri);
            return 0;
        }
        StopInner(uri: string): int {
            this.DownloadStop(uri);
            return 0;
        }
    };

    export enum NotifyType {
        NOTIFY_ADDED = 0,
        NOTIFY_MODIFIED = 1,
        NOTIFY_DELETED = 2,
        NOTIFY_RENAMED = 3
    }

    export interface ChangeData {
        type: NotifyType;
        isDirectory: Array<boolean>;
        uris: Array<string>;
    }

    export enum FileState {
        INITIAL_AFTER_DOWNLOAD = 0,
        UPLOADING = 1,
        STOPPED = 2,
        TO_BE_UPLOADED = 3,
        UPLOAD_SUCCESS = 4,
        UPLOAD_FAILURE = 5,
    }

    export enum FileSyncState {
        UPLOADING = 0,
        DOWNLOADING = 1,
        COMPLETED = 2,
        STOPPED = 3,
        TO_BE_UPLOADED = 4,
        UPLOAD_SUCCESS = 5,
        UPLOAD_FAILURE = 6,
    }

    export enum DownloadFileType {
        CONTENT = 0,
        THUMBNAIL = 1,
        LCD = 2
    }

    export interface FailedFileInfo {
        uri: string;
        error: DownloadErrorType;
    }

    export class FailedFileInfoInner implements FailedFileInfo {
        constructor(
            uri: string,
            error: DownloadErrorType
        ) {
            this.uri = uri;
            this.error = error;
        }
        uri: string;
        error: DownloadErrorType;
    };

    export class ChangeDataInner implements ChangeData {
        constructor(
            type: NotifyType,
            isDirectory: Array<boolean>,
            uris: Array<string>
        ) {
            this.type = type
            this.isDirectory = isDirectory;
            this.uris = uris;
        }
        type: NotifyType;
        isDirectory: Array<boolean>;
        uris: Array<string>;
    };

    export class FileSync {
        static {
            loadLibrary("cloud_sync_ani");
        }
        private nativePtr: long = 0;
        native constructor();
        native constructor(bundleName: string);

        bindNativePtr(ptr: long): void {
            if (this.nativePtr == 0) {
                this.nativePtr = ptr;
            }
        }

        native on(evt: string, callback: Callback<SyncProgress>): void;

        native off(evt: string, callback: Callback<SyncProgress>): void;

        native off(evt: string): void;

        start(): Promise<void> {
            return new Promise<void>((resolve: (result: undefined) => void,
                reject: (e: BusinessError<void>) => void): void => {
                let promise = taskpool.execute((): int => this.StartInner());
                promise.then<void>((): void => {
                    resolve(undefined);
                }, (e: BusinessError<void>): void => {
                    reject(e);
                });
            });
        }

        start(callback: AsyncCallback<void>): void {
            let promise = taskpool.execute((): int => this.StartInner());
            promise.then<void>((): void => {
                let e = new BusinessError<void>();
                e.code = 0;
                callback(e, undefined);
            }, (e: BusinessError<void>): void => {
                callback(e, undefined);
            });
        }

        stop(): Promise<void> {
            return new Promise<void>((resolve: (result: undefined) => void,
                reject: (e: BusinessError<void>) => void): void => {
                let promise = taskpool.execute((): int => this.StopInner());
                promise.then<void>((): void => {
                    resolve(undefined);
                }, (e: BusinessError<void>): void => {
                    reject(e);
                });
            });
        }

        stop(callback: AsyncCallback<void>): void {
            let promise = taskpool.execute((): int => this.StopInner());
            promise.then((): void => {
                let e = new BusinessError<void>();
                e.code = 0;
                callback(e, undefined);
            }, (e: BusinessError<void>): void => {
                callback(e, undefined);
            });
        }

        getLastSyncTime(): Promise<number> {
            return new Promise<number>((resolve: (result: number) => void,
                reject: (e: BusinessError<void>) => void): number => {
                let promise = taskpool.execute((): number => this.GallerySyncGetLastSyncTime());
                promise.then<number>((ret: number): void => {
                    resolve(ret);
                }, (e: BusinessError<void>): void => {
                    reject(e);
                });
            });
        }

        getLastSyncTime(callback: AsyncCallback<number>): void {
            let promise = taskpool.execute((): number => this.GallerySyncGetLastSyncTime());
            promise.then<number>((ret: number): void => {
                let e = new BusinessError<void>();
                e.code = 0;
                callback(e, ret);
            }, (e: BusinessError<void>): void => {
                let num: number = 0;
                callback(e, num);
            });
        }

        native FileSyncStart(): void;
        native FileSyncStop(): void;
        native GallerySyncGetLastSyncTime(): number;

        StartInner(): int {
            this.FileSyncStart();
            return 0;
        }
        StopInner(): int {
            this.FileSyncStop();
            return 0;
        }
    };

    export class CloudFileCache {
        static {
            loadLibrary("cloud_sync_ani");
        }
        private nativePtr: long = 0;
        native constructor();

        bindNativePtr(ptr: long): void {
            if (this.nativePtr == 0) {
                this.nativePtr = ptr;
            }
        }

        native on(evt: string, callback: Callback<DownloadProgress>): void;

        native off(evt: string, callback: Callback<DownloadProgress>): void;

        native off(evt: string): void;

        start(uri: string): Promise<void> {
            return new Promise<void>((resolve: (result: undefined) => void,
                reject: (e: BusinessError) => void): void => {
                let promise = taskpool.execute((uri: string): int => this.StartInner(uri), uri);
                promise.then<void>((): void => {
                    resolve(undefined);
                }, (e: BusinessError<void>): void => {
                    reject(e);
                });
            });
        }

        start(uri: string, callback: AsyncCallback<void>): void {
            let promise = taskpool.execute((uri: string): int => this.StartInner(uri), uri);
            promise.then<void>((): void => {
                let e = new BusinessError<void>();
                e.code = 0;
                callback(e, undefined);
            }, (e: BusinessError<void>): void => {
                callback(e, undefined);
            });
        }

        startBatch(uris: Array<string>, fileType?: DownloadFileType): Promise<number> {
            let fieldKey: DownloadFileType = DownloadFileType.CONTENT;
            if (fileType !== undefined) {
                fieldKey = fileType;
            }
            return new Promise<number>((resolve: (result: number) => void,
                reject: (e: BusinessError) => void): number => {
                let promise = taskpool.execute((uris: Array<string>, fieldKey: DownloadFileType): number =>
                    this.CloudFileCacheStartBatch(uris, fieldKey), uris, fieldKey);
                promise.then<number>((ret: number): void => {
                    resolve(ret);
                }, (e: BusinessError<void>): void => {
                    reject(e);
                });
            });
        }

        stop(uri: string, needClean?: boolean): Promise<void> {
            let clean: boolean = false;
            if (needClean !== undefined) {
                clean = needClean;
            }
            return new Promise<void>((resolve: (result: undefined) => void,
                reject: (e: BusinessError<void>) => void): void => {
                let promise = taskpool.execute((uri: string, clean: boolean): int => {
                    return this.StopInner(uri, clean);
                }, uri, clean);
                promise.then<void>((): void => {
                    resolve(undefined);
                }, (e: BusinessError<void>): void => {
                    reject(e);
                });
            });
        }

        stop(uri: string, callback: AsyncCallback<void>): void {
            let clean: boolean = false;
            let promise = taskpool.execute((uri: string, clean: boolean): int => {
                return this.StopInner(uri, clean);
            }, uri, clean);
            promise.then<void>((): void => {
                let e = new BusinessError<void>();
                e.code = 0;
                callback(e, undefined);
            }, (e: BusinessError<void>): void => {
                callback(e, undefined);
            });
        }

        stopBatch(downloadId: number, needClean?: boolean): Promise<void> {
            let clean: boolean = false;
            if (needClean !== undefined) {
                clean = needClean;
            }
            return new Promise<void>((resolve: (result: undefined) => void,
                reject: (e: BusinessError<void>) => void): void => {
                let promise = taskpool.execute((downloadId: number, clean: boolean): number => {
                    return this.StopBatchInner(downloadId, clean);
                }, downloadId, clean);
                promise.then<void>((): void => {
                    resolve(undefined);
                }, (e: BusinessError<void>): void => {
                    reject(e);
                });
            });
        }

        native CloudFileCacheStart(uri: string): void;
        native CloudFileCacheStop(uri: string, clean: boolean): void;
        native CloudFileCacheStartBatch(uris: Array<string>, fieldKey: DownloadFileType): number;
        native CloudFileCacheStopBatch(downloadId: number, clean: boolean): void;
        native cleanCache(uri: string): void;
        native cleanFileCache(uri: string): void;

        StartInner(uri: string): int {
            this.CloudFileCacheStart(uri);
            return 0;
        }
        StopInner(uri: string, clean: boolean): int {
            this.CloudFileCacheStop(uri, clean);
            return 0;
        }
        StopBatchInner(downloadId: number, clean: boolean): int {
            this.CloudFileCacheStopBatch(downloadId, clean);
            return 0;
        }
    };

    export enum OptimizeState {
        RUNNING = 0,
        COMPLETED = 1,
        FAILED = 2,
        STOPPED = 3
    }

    export interface OptimizeSpaceProgress {
        state: OptimizeState;
        progress: number;
    }

    export class OptimizeSpaceProgressInner implements OptimizeSpaceProgress {
        constructor(
            state: OptimizeState,
            progress: number
        ) {
            this.state = state;
            this.progress = progress;
        }
        state: OptimizeState;
        progress: number;
    }

    export interface OptimizeSpaceParam {
        totalSize: number;
        agingDays: number;
    }

    export class OptimizeSpaceParamInner implements OptimizeSpaceParam {
        constructor(
            totalSize: number,
            agingDays: number
        ) {
            this.totalSize = totalSize;
            this.agingDays = agingDays;
        }
        totalSize: number;
        agingDays: number;
    }

    class StaticFunction {
        static {
            loadLibrary("cloud_sync_ani");
        }
        static native getFileSyncStateInner(uri: string): int;
        static native getCoreFileSyncStateInner(uri: string): int;
        static native registerChangeInner(uri: string, recursion: boolean, callback: Callback<ChangeData>): void;
        static native unregisterChangeInner(uri: string): void;
        static native optimizeStorageInner(): void;
        static native startOptimizeSpaceInner(para: OptimizeSpaceParam, cbk: Callback<OptimizeSpaceProgress>): void;
        static native stopOptimizeSpaceInner(): void;
    };

    function getPromiseFileSyncState(uri: string): Promise<int> {
        let promise = new Promise<int>((resolve, reject) => {
            let taskPool = taskpool.execute((uri: string): int => {
                return StaticFunction.getFileSyncStateInner(uri);
            }, uri);

            taskPool.then((result: NullishType): void => {
                resolve(result as int);
            }, (error: BusinessError<void>): void => {
                reject(error);
            });
        });
        return promise;
    }

    export function getFileSyncState(uri: Array<string>): Promise<Array<FileSyncState>> {
        let promise = new Promise<Array<FileSyncState>>((resolve, reject) => {
            let promises = uri.map((v: string): Promise<int> => { return getPromiseFileSyncState(v) });
            Promise.all(promises).then<void, void>((re: Array<int>): void => {
                let res: Array<FileSyncState> = re.map((result: int) => {
                    return result as FileSyncState;
                });
                resolve(res);
            }).catch((e: BusinessError<void>): void => {
                reject(e);
            });
        });
        return promise;
    }

    export function getFileSyncState(uri: Array<string>, callback: AsyncCallback<Array<FileSyncState>>): void {
        let promises = uri.map((v: string): Promise<int> => { return getPromiseFileSyncState(v) });
        Promise.all(promises).then<void, void>((re: Array<int>): void => {
            let res: Array<FileSyncState> = re.map((result: int) => {
                return result as FileSyncState;
            });
            let e = new BusinessError<void>();
            e.code = 0;
            callback(e, res);
        }).catch((e: BusinessError<void>): void => {
            let res: Array<FileSyncState> = new Array<FileSyncState>();
            callback(e, res);
        });
    }

    export function getFileSyncState(uri: string): FileSyncState {
        let res: int = StaticFunction.getFileSyncStateInner(uri);
        return res as FileSyncState;
    }

    export function getCoreFileSyncState(uri: string): FileState {
        let res: int = StaticFunction.getCoreFileSyncStateInner(uri);
        return res as FileState;
    }

    export function registerChange(uri: string, recursion: boolean, callback: Callback<ChangeData>): void {
        StaticFunction.registerChangeInner(uri, recursion, callback);
    }

    export function unregisterChange(uri: string): void {
        StaticFunction.unregisterChangeInner(uri);
    }

    export function optimizeStorage(): Promise<void> {
        return new Promise<void>((resolve: (result: undefined) => void,
            reject: (e: BusinessError<void>) => void): void => {
            let promise = taskpool.execute((): int => {
                StaticFunction.optimizeStorageInner();
                return 0;
            });
            promise.then<void>((): void => {
                resolve(undefined);
            }, (e: BusinessError<void>): void => {
                reject(e);
            });
        });
    }

    export function startOptimizeSpace(para: OptimizeSpaceParam, cb?: Callback<OptimizeSpaceProgress>): Promise<void> {
        let callback: Callback<OptimizeSpaceProgress> = (data: OptimizeSpaceProgress) => { };
        if (cb !== undefined) {
            callback = cb;
        }
        return new Promise<void>((resolve: (result: undefined) => void,
            reject: (e: BusinessError<void>) => void): void => {
            let promise = taskpool.execute((
                param: OptimizeSpaceParam,
                cbk: Callback<OptimizeSpaceProgress>
            ): int => {
                StaticFunction.startOptimizeSpaceInner(param, cbk);
                return 0;
            }, para, callback);
            promise.then<void>((): void => {
                resolve(undefined);
            }, (e: BusinessError<void>): void => {
                reject(e);
            });
        });
    }

    export function stopOptimizeSpace(): void {
        StaticFunction.stopOptimizeSpaceInner();
    }

    export interface HistoryVersion {
        editedTime: number;
        fileSize: number;
        versionId: string;
        originalFileName: string;
        sha256: string;
        autoResolved: boolean;
    }

    class HistoryVersionInner implements HistoryVersion {
        constructor(
            editedTime: number,
            fileSize: number,
            versionId: string,
            originalFileName: string,
            sha256: string,
            autoResolved: boolean
        ) {
            this.editedTime = editedTime;
            this.fileSize = fileSize;
            this.versionId = versionId;
            this.originalFileName = originalFileName;
            this.sha256 = sha256;
            this.autoResolved = autoResolved;
        }
        public editedTime: number;
        public fileSize: number;
        public versionId: string;
        public originalFileName: string;
        public sha256: string;
        public autoResolved: boolean;
    }

    export interface VersionDownloadProgress {
        state: State;
        progress: number;
        errType: DownloadErrorType;
    }

    class VersionDownloadProgressInner implements VersionDownloadProgress {
        constructor(
            state: State,
            progress: number,
            errType: DownloadErrorType
        ) {
            this.state = state;
            this.progress = progress;
            this.errType = errType;
        }
        public state: State;
        public progress: number;
        public errType: DownloadErrorType;
    }

    export class FileVersion {
        static {
            loadLibrary("cloud_sync_ani");
        }
        private nativePtr: long = 0;
        native constructor();

        bindNativePtr(ptr: long): void {
            if (this.nativePtr == 0) {
                this.nativePtr = ptr;
            }
        }

        getHistoryVersionList(uri: string, versionNumLimit: number): Promise<Array<HistoryVersion>> {
            return new Promise<Array<HistoryVersion>>((resolve: (result: Array<HistoryVersion>) => void,
                reject: (e: BusinessError<void>) => void): void => {
                let promise = taskpool.execute((
                    uriIn: string,
                    versionNumLimitIn: number
                ): Array<HistoryVersion> => {
                    return this.getHistoryVersionListInner(uriIn, versionNumLimitIn);
                }, uri, versionNumLimit);
                promise.then<Array<HistoryVersion>>((ret: Array<HistoryVersion>): void => {
                    resolve(ret);
                }, (e: BusinessError<void>): void => {
                    reject(e);
                });
            });
        }

        downloadHistoryVersion(uri: string, versionId: string, callback: Callback<VersionDownloadProgress>): Promise<string> {
            return new Promise<string>((resolve: (result: string) => void,
                reject: (e: BusinessError<void>) => void): void => {
                let promise = taskpool.execute((
                    uriIn: string,
                    versionIdIn: string,
                    callbackIn: Callback<VersionDownloadProgress>
                ): string => {
                    return this.downloadHistoryVersionInner(uriIn, versionIdIn, callbackIn);
                }, uri, versionId, callback);
                promise.then<string>((ret: string): void => {
                    resolve(ret);
                }, (e: BusinessError<void>): void => {
                    reject(e);
                });
            });
        }
        replaceFileWithHistoryVersion(originalUri: string, versionUri: string): Promise<void> {
            return new Promise<void>((resolve: (result: undefined) => void,
                reject: (e: BusinessError<void>) => void): void => {
                let promise = taskpool.execute((
                    originalUriIn: string,
                    versionUriIn: string
                ): int => {
                    this.replaceFileWithHistoryVersionInner(originalUriIn, versionUriIn);
                    return 0;
                }, originalUri, versionUri);
                promise.then<void>((ret: undefined): void => {
                    resolve(ret);
                }, (e: BusinessError<void>): void => {
                    reject(e);
                });
            });
        }

        isFileConflict(uri: string): Promise<boolean> {
            return new Promise<boolean>((resolve: (result: boolean) => void,
                reject: (e: BusinessError<void>) => void): void => {
                let promise = taskpool.execute((
                    uriIn: string
                ): boolean => {
                    return this.isFileConflictInner(uriIn);
                }, uri);
                promise.then<boolean>((ret: boolean): void => {
                    resolve(ret);
                }, (e: BusinessError<void>): void => {
                    reject(e);
                });
            });
        }

        clearFileConflict(uri: string): Promise<void> {
            return new Promise<void>((resolve: (result: undefined) => void,
                reject: (e: BusinessError<void>) => void): void => {
                let promise = taskpool.execute((
                    uriIn: string
                ): int => {
                    this.clearFileConflictInner(uriIn);
                    return 0;
                }, uri);
                promise.then<void>((ret: undefined): void => {
                    resolve(ret);
                }, (e: BusinessError<void>): void => {
                    reject(e);
                });
            });
        }

        native getHistoryVersionListInner(uri: string, versionNumLimit: number): Array<HistoryVersion>;
        native downloadHistoryVersionInner(uri: string, versionId: string, callback: Callback<VersionDownloadProgress>): string;
        native replaceFileWithHistoryVersionInner(originalUri: string, versionUri: string): void;
        native isFileConflictInner(uri: string): boolean;
        native clearFileConflictInner(uri: string): void;
    }
};