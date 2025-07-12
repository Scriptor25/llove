#include <vector>
#include <llove/parameter.hpp>

llove::ParameterHash llove::GetFieldHash(const std::vector<Field> &fields)
{
    ParameterHash hash;
    hash += std::to_string(fields.size());
    for (const auto &[mutable_, reference_, type_] : fields)
    {
        if (mutable_)
            hash += 'm';
        if (reference_)
            hash += 'r';
        hash += std::to_string(reinterpret_cast<unsigned long long>(type_.get()));
    }
    return hash;
}
