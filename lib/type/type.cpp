#include <llove/builder.hpp>
#include <llove/type.hpp>

bool llove::Type::IsTemplate() const
{
    return false;
}

bool llove::Type::IsVoid() const
{
    return false;
}

bool llove::Type::IsVariadic() const
{
    return false;
}

bool llove::Type::IsInteger() const
{
    return false;
}

bool llove::Type::IsFloat() const
{
    return false;
}

bool llove::Type::IsPointer() const
{
    return false;
}

bool llove::Type::IsArray() const
{
    return false;
}

bool llove::Type::IsStruct() const
{
    return false;
}

bool llove::Type::IsTuple() const
{
    return false;
}

bool llove::Type::IsRange() const
{
    return false;
}

bool llove::Type::IsClass() const
{
    return false;
}

bool llove::Type::IsFunction() const
{
    return false;
}

unsigned llove::Type::SizeBits(Builder& builder)
{
    return builder.GetDataLayout().getTypeSizeInBits(GenIR(builder));
}
