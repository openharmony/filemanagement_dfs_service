/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef DECOMPRESS_SYSCALL_MOCK_H
#define DECOMPRESS_SYSCALL_MOCK_H

#ifdef DECOMPRESS_UNIT_TEST

#include <cstdio>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

int MockOpen(const char *pathname, int flags, ...);
int MockClose(int fd);
int MockIoctl(int fd, unsigned long request, ...);
int MockStat(const char *pathname, struct stat *statbuf);
char *MockRealpath(const char *path, char *resolved);
void *MockDlopen(const char *filename, int flags);
void *MockDlsym(void *handle, const char *symbol);
FILE *MockFopen(const char *pathname, const char *mode);
int MockFclose(FILE *stream);
ssize_t MockGetline(char **lineptr, size_t *n, FILE *stream);

#define open(...) MockOpen(__VA_ARGS__)
#define close(...) MockClose(__VA_ARGS__)
#define ioctl(...) MockIoctl(__VA_ARGS__)
#define stat(...) MockStat(__VA_ARGS__)
#define realpath(...) MockRealpath(__VA_ARGS__)
#define dlopen(...) MockDlopen(__VA_ARGS__)
#define dlsym(...) MockDlsym(__VA_ARGS__)
#define fopen(...) MockFopen(__VA_ARGS__)
#define fclose(...) MockFclose(__VA_ARGS__)
#define getline(...) MockGetline(__VA_ARGS__)

#endif // DECOMPRESS_UNIT_TEST

#endif // DECOMPRESS_SYSCALL_MOCK_H
