#include <utility>
#include <llove/builder.hpp>
#include <llove/type.hpp>

llvm::Type *llove::VoidType::Gen(Builder &builder) const
{
    return builder.GetVoidType();
}

std::string llove::VoidType::Mangle() const
{
    return "v";
}

llove::IntType::IntType(const bool sign, const unsigned bits)
    : m_Sign(sign),
      m_Bits(bits)
{
}

llvm::IntegerType *llove::IntType::Gen(Builder &builder) const
{
    return builder.GetIntType(m_Bits);
}

std::string llove::IntType::Mangle() const
{
    return (m_Sign ? 'i' : 'u') + std::to_string(m_Bits) + '_';
}

llove::FltType::FltType(const unsigned bits)
    : m_Bits(bits)
{
}

llvm::Type *llove::FltType::Gen(Builder &builder) const
{
    return builder.GetFltType(m_Bits);
}

std::string llove::FltType::Mangle() const
{
    return 'f' + std::to_string(m_Bits) + '_';
}

llove::PtrType::PtrType(TypePtr base, const bool mutable_)
    : m_Base(std::move(base)),
      m_Mutable(mutable_)
{
}

llvm::PointerType *llove::PtrType::Gen(Builder &builder) const
{
    return builder.GetPtrType(m_Base->Gen(builder));
}

std::string llove::PtrType::Mangle() const
{
    return 'p' + std::string(m_Mutable ? "m" : "i") + m_Base->Mangle();
}

llove::ArrayType::ArrayType(TypePtr base, const unsigned size)
    : m_Base(std::move(base)),
      m_Size(size)
{
}

llvm::ArrayType *llove::ArrayType::Gen(Builder &builder) const
{
    return builder.GetArrayType(m_Base->Gen(builder), m_Size);
}

std::string llove::ArrayType::Mangle() const
{
    return 'a' + std::to_string(m_Size) + '_' + m_Base->Mangle();
}

llove::StructType::StructType(std::vector<Parameter> fields)
    : m_Fields(std::move(fields))
{
}

llvm::StructType *llove::StructType::Gen(Builder &builder) const
{
    std::vector<llvm::Type *> fields;
    for (auto &[info_, name_] : m_Fields)
        fields.emplace_back(info_.Gen(builder));

    // TODO: packed struct
    return builder.GetStructType(fields, true);
}

std::string llove::StructType::Mangle() const
{
    std::string fields;
    for (auto &[info_, name_] : m_Fields)
        fields += info_.Mangle();
    return 's' + std::to_string(m_Fields.size()) + '_' + fields;
}

llove::ClassType::ClassType(std::string name)
    : m_Name(std::move(name)),
      m_Opaque(true)
{
}

llove::ClassType::ClassType(std::string name, std::vector<Parameter> fields)
    : m_Name(std::move(name)),
      m_Opaque(false),
      m_Fields(std::move(fields))
{
}

llvm::StructType *llove::ClassType::Gen(Builder &builder) const
{
    if (m_Opaque)
        return builder.GetOrCreateNamedStructType(m_Name);

    std::vector<llvm::Type *> fields;
    for (auto &[info_, name_] : m_Fields)
        fields.emplace_back(info_.Gen(builder));

    // TODO: packed struct
    return builder.GetOrCreateNamedStructType(m_Name, fields, true);
}

std::string llove::ClassType::Mangle() const
{
    return 'c' + std::to_string(m_Name.size()) + '_' + m_Name;
}

llove::FunctionType::FunctionType(std::vector<Field> parameters, const bool vararg, Field result)
    : m_Parameters(std::move(parameters)),
      m_VarArg(vararg),
      m_Result(std::move(result))
{
}

llove::FunctionType::FunctionType(std::vector<Field> parameters, const bool vararg, Field result, Field self)
    : m_Parameters(std::move(parameters)),
      m_VarArg(vararg),
      m_Result(std::move(result)),
      m_Self(std::move(self))
{
}

llvm::FunctionType *llove::FunctionType::Gen(Builder &builder) const
{
    std::vector<llvm::Type *> parameters;
    if (m_Self.Type)
        parameters.emplace_back(builder.GetPtrType(m_Self.Type->Gen(builder)));
    for (auto &parameter : m_Parameters)
        parameters.emplace_back(parameter.Gen(builder));

    return builder.GetFunctionType(m_Result.Gen(builder), parameters, m_VarArg);
}

std::string llove::FunctionType::Mangle() const
{
    std::string parameters;
    for (auto &parameter : m_Parameters)
        parameters += parameter.Mangle();
    return 'x'
           + std::string(m_VarArg ? "v" : "")
           + std::string(m_Self.Type ? "s" : "")
           + std::to_string(m_Parameters.size())
           + '_'
           + parameters
           + m_Result.Mangle()
           + (m_Self.Type ? m_Self.Mangle() : std::string());
}
