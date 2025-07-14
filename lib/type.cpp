#include <utility>
#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llove/type.hpp>

llove::TypeId llove::VoidType::GetId() const
{
    return TypeId_Void;
}

llvm::Type *llove::VoidType::Gen(Builder &builder) const
{
    return builder.GetVoidType();
}

std::string llove::VoidType::Mangle() const
{
    return "v";
}

llove::IntegerType::IntegerType(const bool sign, const unsigned bits)
    : m_Sign(sign),
      m_Bits(bits)
{
}

bool llove::IntegerType::IsSigned() const
{
    return m_Sign;
}

unsigned llove::IntegerType::GetBits() const
{
    return m_Bits;
}

llove::TypeId llove::IntegerType::GetId() const
{
    return TypeId_Integer;
}

llvm::IntegerType *llove::IntegerType::Gen(Builder &builder) const
{
    return builder.GetIntType(m_Bits);
}

std::string llove::IntegerType::Mangle() const
{
    return (m_Sign ? 'i' : 'u') + std::to_string(m_Bits) + '_';
}

llove::FloatType::FloatType(const unsigned bits)
    : m_Bits(bits)
{
}

unsigned llove::FloatType::GetBits() const
{
    return m_Bits;
}

llove::TypeId llove::FloatType::GetId() const
{
    return TypeId_Float;
}

llvm::Type *llove::FloatType::Gen(Builder &builder) const
{
    return builder.GetFltType(m_Bits);
}

std::string llove::FloatType::Mangle() const
{
    return 'f' + std::to_string(m_Bits) + '_';
}

llove::PointerType::PointerType(TypePtr base, const bool mutable_)
    : m_Base(std::move(base)),
      m_Mutable(mutable_)
{
}

llove::TypePtr llove::PointerType::GetBase() const
{
    Assert(m_Base != nullptr, "pointer type is opaque");
    return m_Base;
}

bool llove::PointerType::IsMutable() const
{
    return m_Mutable;
}

bool llove::PointerType::IsOpaque() const
{
    return !m_Base;
}

llove::TypeId llove::PointerType::GetId() const
{
    return TypeId_Pointer;
}

llvm::PointerType *llove::PointerType::Gen(Builder &builder) const
{
    if (m_Base)
        return builder.GetPointerType(m_Base->Gen(builder));
    return builder.GetPointerType();
}

std::string llove::PointerType::Mangle() const
{
    return 'p' + std::string(m_Mutable ? "m" : "i") + m_Base->Mangle();
}

llove::ArrayType::ArrayType(TypePtr base, const unsigned size)
    : m_Base(std::move(base)),
      m_Size(size)
{
}

llove::TypePtr llove::ArrayType::GetBase() const
{
    return m_Base;
}

unsigned llove::ArrayType::GetSize() const
{
    return m_Size;
}

llove::TypeId llove::ArrayType::GetId() const
{
    return TypeId_Array;
}

llvm::ArrayType *llove::ArrayType::Gen(Builder &builder) const
{
    return builder.GetArrayType(m_Base->Gen(builder), m_Size);
}

std::string llove::ArrayType::Mangle() const
{
    return 'a' + std::to_string(m_Size) + '_' + m_Base->Mangle();
}

llove::StructType::StructType(std::vector<ClassField> fields)
    : m_Fields(std::move(fields))
{
}

unsigned llove::StructType::GetFieldIndex(const std::string &name) const
{
    for (unsigned i = 0; i < m_Fields.size(); ++i)
        if (m_Fields.at(i).Name == name)
            return i;
    Error("no field with name '{}'", name);
}

unsigned llove::StructType::GetFieldCount() const
{
    return m_Fields.size();
}

const llove::Field &llove::StructType::GetField(const unsigned index) const
{
    return m_Fields.at(index).Info;
}

llove::TypeId llove::StructType::GetId() const
{
    return TypeId_Struct;
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

llove::ClassType::ClassType(std::string name, std::vector<ClassField> fields, std::vector<ClassFunctionInfo> functions)
    : m_Name(std::move(name)),
      m_Opaque(false),
      m_Fields(std::move(fields)),
      m_Functions(std::move(functions))
{
}

const std::string &llove::ClassType::GetName() const
{
    return m_Name;
}

bool llove::ClassType::IsOpaque() const
{
    return m_Opaque;
}

unsigned llove::ClassType::GetFieldIndex(const std::string &name) const
{
    for (unsigned i = 0; i < m_Fields.size(); ++i)
        if (m_Fields.at(i).Name == name)
            return i;
    Error("no field with name '{}'", name);
}

unsigned llove::ClassType::GetFieldCount() const
{
    return m_Fields.size();
}

const llove::Field &llove::ClassType::GetField(const unsigned index) const
{
    return m_Fields.at(index).Info;
}

const llove::ClassFunctionInfo *llove::ClassType::GetFunction(
    const std::string &name,
    const bool mutable_,
    const std::vector<Field> &parameters,
    const bool vararg,
    const Field &result) const
{
    for (auto &function : m_Functions)
    {
        if (function.Name != name)
            continue;
        if (function.Mutable != mutable_)
            continue;
        if (function.VarArg != vararg)
            continue;
        if (function.Parameters.size() != parameters.size())
            continue;
        if (function.Result != result)
            continue;
        unsigned i;
        for (i = 0; i < function.Parameters.size(); ++i)
            if (function.Parameters.at(i) != parameters.at(i))
                break;
        if (i < function.Parameters.size())
            continue;
        return &function;
    }

    return nullptr;
}

std::vector<const llove::ClassFunctionInfo *> llove::ClassType::GetCreates() const
{
    std::vector<const ClassFunctionInfo *> creates;
    for (auto &function : m_Functions)
        if (function.Name == "create")
            creates.emplace_back(&function);
    return creates;
}

void llove::ClassType::SetFields(Builder &builder, std::vector<ClassField> fields)
{
    m_Opaque = fields.empty();
    m_Fields = std::move(fields);

    if (m_Opaque)
    {
        builder.GetOrCreateNamedStructType(m_Name);
        return;
    }

    std::vector<llvm::Type *> elements;
    for (auto &[info_, name_] : m_Fields)
        elements.emplace_back(info_.Gen(builder));

    // TODO: packed struct
    builder.GetOrCreateNamedStructType(m_Name, elements, true);
}

void llove::ClassType::SetFunctions(std::vector<ClassFunctionInfo> functions)
{
    m_Functions = std::move(functions);
}

llove::TypeId llove::ClassType::GetId() const
{
    return TypeId_Class;
}

llvm::StructType *llove::ClassType::Gen(Builder &builder) const
{
    if (m_Opaque)
        return builder.GetOrCreateNamedStructType(m_Name);

    std::vector<llvm::Type *> elements;
    for (auto &[info_, name_] : m_Fields)
        elements.emplace_back(info_.Gen(builder));

    // TODO: packed struct
    return builder.GetOrCreateNamedStructType(m_Name, elements, true);
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

unsigned llove::FunctionType::GetParameterCount() const
{
    return m_Parameters.size();
}

const llove::Field &llove::FunctionType::GetParameter(const unsigned index) const
{
    return m_Parameters.at(index);
}

bool llove::FunctionType::IsVarArg() const
{
    return m_VarArg;
}

const llove::Field &llove::FunctionType::GetResult() const
{
    return m_Result;
}

bool llove::FunctionType::HasSelf() const
{
    return m_Self.Type != nullptr;
}

const llove::Field &llove::FunctionType::GetSelf() const
{
    return m_Self;
}

llove::TypeId llove::FunctionType::GetId() const
{
    return TypeId_Function;
}

llvm::FunctionType *llove::FunctionType::Gen(Builder &builder) const
{
    std::vector<llvm::Type *> parameters;
    if (m_Self.Type)
        parameters.emplace_back(builder.GetPointerType(m_Self.Type->Gen(builder)));
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
