#include "parse_xattr_int.h"
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>

using OHOS::FileManagement::CloudDisk::ParseXattrInt32;
using OHOS::FileManagement::CloudDisk::ParseXattrUint32;

static void Expect(bool cond, const char *msg)
{
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s\n", msg);
        std::exit(1);
    }
}

static bool LeftoverStoiAborts(const std::string &s)
{
    try {
        (void)std::stoi(s);
        return false;
    } catch (const std::invalid_argument &) {
        return true;
    } catch (const std::out_of_range &) {
        return true;
    }
}

int main()
{
    int32_t v = -1;
    Expect(ParseXattrInt32("0", v) && v == 0, "i0");
    Expect(ParseXattrInt32("1", v) && v == 1, "i1");
    Expect(ParseXattrInt32("2147483647", v) && v == INT_MAX, "iMax");
    Expect(!ParseXattrInt32("", v), "iEmpty");
    Expect(!ParseXattrInt32("abc", v), "iJunk");
    Expect(!ParseXattrInt32("12a", v), "iTrailing");
    Expect(!ParseXattrInt32("2147483648", v), "iOverflow");
    Expect(!ParseXattrInt32("9999999999999999999", v), "iHuge");
    Expect(!ParseXattrInt32(" 1", v), "iSpace");
    // leftover: empty passes all_of(::isdigit) then stoi throws
    Expect(LeftoverStoiAborts(""), "leftover empty throws");
    Expect(LeftoverStoiAborts("9999999999999999999"), "leftover huge throws");
    Expect(LeftoverStoiAborts("2147483648"), "leftover overflow throws");

    uint32_t u = 0;
    Expect(ParseXattrUint32("0", u) && u == 0, "u0");
    Expect(ParseXattrUint32("100", u) && u == 100, "u100");
    Expect(ParseXattrUint32("4294967295", u) && u == 4294967295u, "uMax");
    Expect(!ParseXattrUint32("", u), "uEmpty");
    Expect(!ParseXattrUint32("abc", u), "uJunk");
    Expect(!ParseXattrUint32("4294967296", u), "uOverflow");
    Expect(!ParseXattrUint32("-1", u), "uNeg");

    // negative int32 is valid for from_chars int32
    Expect(ParseXattrInt32("-1", v) && v == -1, "iNeg");

    std::puts("All host tests passed");
    return 0;
}
