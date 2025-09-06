/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef BIT_OPS_H
#define BIT_OPS_H

namespace OHOS::FileManagement::CloudDiskService {

struct BitOps {
    static const uint8_t BIT_PER_BYTE = 8;
    static int TestBit(uint32_t nr, const uint8_t addr[])
    {
        return 1 & (addr[nr / BIT_PER_BYTE] >> (nr & (BIT_PER_BYTE - 1)));
    }

    static void ClearBit(uint32_t nr, uint8_t addr[])
    {
        addr[nr / BIT_PER_BYTE] &= ~(1UL << ((nr) % BIT_PER_BYTE));
    }

    static void SetBit(uint32_t nr, uint8_t addr[])
    {
        addr[nr / BIT_PER_BYTE] |= (1UL << ((nr) % BIT_PER_BYTE));
    }

    static uint32_t FindNextBit(const uint8_t addr[], uint32_t maxSlots, uint32_t start)
    {
        while (start < maxSlots) {
            if (BitOps::TestBit(start, addr)) {
                return start;
            }
            start++;
        }
        return maxSlots;
    }

    static uint32_t FindNextZeroBit(const uint8_t addr[], uint32_t maxSlots, uint32_t start)
    {
        while (start < maxSlots) {
            if (!BitOps::TestBit(start, addr)) {
                return start;
            }
            start++;
        }
        return maxSlots;
    }
};

} // namespace OHOS::FileManagement::CloudDiskService

#endif // BIT_OPS_H
