#ifndef TOOLS_H
#define TOOLS_H

namespace GLRhi
{
#define SAFE_DELETE(ptr)     \
do {                         \
    if (nullptr != (ptr)) {  \
        delete (ptr);        \
        (ptr) = nullptr;     \
    }                        \
} while (0)

#define SAFE_DELETE_ARRAY(ptr) \
do {                           \
    if (nullptr != (ptr)) {    \
        delete[] (ptr);        \
        (ptr) = nullptr;       \
    }                          \
} while (0)
}
#endif // TOOLS_H