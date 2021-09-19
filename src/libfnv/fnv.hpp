#ifndef FNV_HPP
#define FNV_HPP

#include <cinttypes>
#include <string>

namespace fnv
{
    typedef uint64_t fnv64_t;

    /**
     * @brief  calculate Fnv64a hash from given string
     * 
     * http://www.isthe.com/chongo/tech/comp/fnv/
     * 
     * @param str input string
     * @return fnv64_t computed hash value
     */
    fnv64_t Fnv64a(const std::string& str);
}

#endif