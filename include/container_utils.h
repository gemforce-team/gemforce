#ifndef _CONTAINER_UTILS_H
#define _CONTAINER_UTILS_H

#include <memory>
#include <vector>

using std::vector;
template<class T>
using pool_t = std::unique_ptr<T[]>;

template<class T>
inline T* operator+ (vector<T>& v, std::integral auto i)
{
    return v.data() + i;
}

template<class T>
inline const T* operator+ (const vector<T>& v, std::integral auto i)
{
    return v.data() + i;
}

template<class T>
inline T* operator+ (pool_t<T>& p, std::integral auto i)
{
    return p.get() + i;
}

template<class T>
inline const T* operator+ (const pool_t<T>& p, std::integral auto i)
{
    return p.get() + i;
}

template<class T>
inline pool_t<T> make_uninitialized_pool(size_t len) {
    return std::make_unique_for_overwrite<T[]>(len);
}

#endif // _CONTAINER_UTILS_H
