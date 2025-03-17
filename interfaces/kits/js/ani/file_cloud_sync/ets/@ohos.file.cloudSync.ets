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

export default namespace cloudSync {

    export class BusinessError<T = void> {
        name: string;
        message: string;
        code: number;
        data?: T;
        constructor(code: number, msg: string, data?: T) {
            this.name = "BusinessError";
            this.code = code;
            this.message = msg;
            if (data !== undefined) {
                this.data = data;
            }
        }
    }

    export type AsyncCallback<T, E = void> = (err: BusinessError<E>, data: T) => void;

    export enum ErrorType {
        NO_ERROR = 0,
        NETWORK_UNAVAILABLE = 1,
        WIFI_UNAVAILABLE = 2,
        BATTERY_LEVEL_LOW = 3,
        BATTERY_LEVEL_WARNING = 4,
        CLOUD_STORAGE_FULL = 5,
        LOCAL_STORAGE_FULL = 6,
        DEVICE_TEMPERATURE_TOO_HIGH = 7,
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
                let e = new BusinessError(0, "");
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
                let e = new BusinessError(0, "");
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
                let e = new BusinessError(0, "");
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
                let e = new BusinessError(0, "");
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
};