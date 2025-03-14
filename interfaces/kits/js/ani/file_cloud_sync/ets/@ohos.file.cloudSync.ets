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
        code: number;
        data?: T;
    }

    type AsyncCallback<T, E = void> = (err: BusinessError<E> | null, data: T) => void;

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
            return new Promise<void>(() => {
                let promise = taskpool.execute(GallerySync.StartInner);
                promise.then<void>((): void => {});
            });
        }

        start(callback: AsyncCallback<void>): void {
            let promise = taskpool.execute(GallerySync.StartInner);
            promise.then<void>((): void => {
                callback(null, undefined);
            });
        }

        stop(): Promise<void> {
            return new Promise<void>(() => {
                let promise = taskpool.execute(GallerySync.StopInner);
                promise.then<void>((): void => {});
            });
        }

        stop(callback: AsyncCallback<void>): void {
            let promise = taskpool.execute(GallerySync.StopInner);
            promise.then((): void => {
                callback(null, undefined);
            });
        }

        static native GallerySyncStart(): void;
        static native GallerySyncStop(): void;

        static StartInner(): undefined {
            GallerySync.GallerySyncStart();
            return undefined;
        }
        static StopInner(): undefined {
            GallerySync.GallerySyncStop();
            return undefined;
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
            return new Promise<void>(() => {
                let promise = taskpool.execute(Download.StartInner, uri);
                promise.then<void>((): void => {});
            });
        }

        start(uri: string, callback: AsyncCallback<void>): void {
            let promise = taskpool.execute(Download.StartInner, uri);
            promise.then<void>((): void => {
                callback(null, undefined);
            });
        }

        stop(uri: string): Promise<void> {
            return new Promise<void>(() => {
                let promise = taskpool.execute(Download.StopInner, uri);
                promise.then<void>((): void => {});
            });
        }

        stop(uri: string, callback: AsyncCallback<void>): void {
            let promise = taskpool.execute(Download.StopInner, uri);
            promise.then<void>((): void => {
                callback(null, undefined);
            });
        }

        static native DownloadStart(uri: string): void;
        static native DownloadStop(uri: string): void;

        static StartInner(uri: string): undefined {
            Download.DownloadStart(uri);
            return undefined;
        }
        static StopInner(uri: string): undefined {
            Download.DownloadStop(uri);
            return undefined;
        }
    };
};