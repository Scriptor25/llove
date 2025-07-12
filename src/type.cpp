#include <utility>
#include <llove/type.hpp>

llove::IntType::IntType(const bool sign, const unsigned bits)
    : Sign(sign),
      Bits(bits)
{
}

llove::FltType::FltType(const unsigned bits)
    : Bits(bits)
{
}

llove::ArrayType::ArrayType(TypePtr base, const int64_t size)
    : Base(std::move(base)),
      Size(size)
{
}

llove::StructType::StructType(const std::vector<Parameter> &fields)
    : Fields(fields)
{
}
