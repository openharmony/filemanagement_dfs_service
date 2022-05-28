#include <fstream>
#include <iostream>
#include <memory>
#include <system_error>

using namespace std;

// 测试 system_error
void __attribute__((noinline)) Foo1()
{
    string test(std::to_string(15151515));
    auto tmp = make_unique<char[]>(66);
    tmp[0] = 'a';
    throw system_error(EINVAL, generic_category());
}

// 测试 iso_base::failure
void __attribute__((noinline)) Foo2()
{
    ifstream f("doesn't exist");
    f.exceptions(f.failbit);
}

// 测试 runtime_error
void __attribute__((noinline)) Foo3()
{
    throw runtime_error(to_string(123));
}

int main(int argc, char const *argv[])
{
    try {
        switch (atoi(argv[1])) {
            case 1:
                Foo1();
                break;
            case 2:
                Foo2();
                break;
            case 3:
                Foo3();
                break;
            default:
                break;
        }
    } catch (const ios_base::failure &e) {
        printf("%d %s", __LINE__, e.what());
    } catch (const runtime_error &e) {
        printf("%d %s", __LINE__, e.what());
    } catch (const exception &e) {
        printf("%d %s", __LINE__, e.what());
    } catch (...) {
        printf("%d", __LINE__);
    }

    printf("\nWe survived\n");
    return 0;
}
