#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/error.hpp>
#include <llove/type.hpp>

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

const std::optional<llove::Field> &llove::FunctionType::GetSelf() const
{
    return m_Self;
}

llove::TypeId llove::FunctionType::GetId() const
{
    return TypeId_Function;
}

bool llove::FunctionType::IsFunction() const
{
    return true;
}

unsigned llove::FunctionType::SizeBits(Builder &builder) const
{
    return 64;
}

llvm::PointerType *llove::FunctionType::Gen(Builder &builder)
{
    if (m_IRType)
        return llvm::dyn_cast<llvm::PointerType>(m_IRType);

    const auto function = GenFunction(builder);
    const auto type = builder.GetPointerType(function);
    m_IRType = type;
    return type;
}

llvm::DIType *llove::FunctionType::GenDbg(Builder &builder)
{
    if (m_DIType)
        return m_DIType;

    const auto function = GenDbgFunction(builder);
    return m_DIType = builder.GetDbgPointerType(function);
}

llvm::FunctionType *llove::FunctionType::GenFunction(Builder &builder)
{
    if (m_IRFunction)
        return m_IRFunction;

    std::vector<llvm::Type *> parameters;
    if (m_Self)
        parameters.emplace_back(m_Self->GenType(builder));
    for (auto &parameter : m_Parameters)
        parameters.emplace_back(parameter.GenType(builder));

    return m_IRFunction = builder.GetFunctionType(m_Result.GenType(builder), parameters, m_VarArg);
}

llvm::DISubroutineType *llove::FunctionType::GenDbgFunction(Builder &builder)
{
    if (m_DIFunction)
        return m_DIFunction;

    std::vector<llvm::Metadata *> parameters;
    for (auto &parameter : m_Parameters)
        parameters.emplace_back(parameter.GenDbgType(builder));

    const auto result = m_Result.GenDbgType(builder);

    return m_DIFunction = builder.GetDbgFunctionType(parameters, result);
}

llove::TypePtr llove::FunctionType::Reflect(Builder &builder) const
{
    std::vector<Field> parameters;
    Field result, self;

    for (auto &parameter : m_Parameters)
        parameter.Reflect(builder, parameters.emplace_back());

    m_Result.Reflect(builder, result);

    if (!m_Self)
        return builder.GetTypes().GetFunction(std::move(parameters), m_VarArg, std::move(result));

    m_Self->Reflect(builder, self);

    return builder.GetTypes().GetFunction(std::move(parameters), m_VarArg, std::move(result), std::move(self));
}

std::string llove::FunctionType::Mangle() const
{
    std::string parameters;
    for (auto &parameter : m_Parameters)
        parameters += parameter.Mangle();
    return 'x'
           + std::string(m_VarArg ? "v" : "")
           + std::string(m_Self ? "s" : "")
           + std::to_string(m_Parameters.size())
           + '_'
           + parameters
           + m_Result.Mangle()
           + (m_Self ? m_Self->Mangle() : std::string());
}

std::ostream &llove::FunctionType::Print(std::ostream &stream) const
{
    stream << '(';
    for (auto i = m_Parameters.begin(); i != m_Parameters.end(); ++i)
    {
        if (i != m_Parameters.begin())
            stream << ", ";
        stream << *i;
    }
    if (m_VarArg)
    {
        if (!m_Parameters.empty())
            stream << ", ";
        stream << "...";
    }
    stream << ')';
    if (m_Self)
        stream << '[' << *m_Self << ']';
    return stream << " => " << m_Result;
}
