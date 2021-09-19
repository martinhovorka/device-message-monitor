#include "fnv.hpp"

namespace fnv
{
    fnv64_t Fnv64a(const std::string& str)
    {
        fnv64_t hash((fnv64_t)(0xcbf29ce484222325ULL));

        for (auto& c : str)
        {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
            hash ^= ((uint64_t)(c));
#pragma GCC diagnostic pop
            hash += (hash << 1) + (hash << 4) + (hash << 5) + (hash << 7) + (hash << 8) + (hash << 40);
        }

        return hash;
    }
}
        
