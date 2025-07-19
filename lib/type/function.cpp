#include <llove/builder.hpp>
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

bool llove::FunctionType::HasSelf() const
{
    return static_cast<bool>(m_Self);
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
    if (m_Self)
        parameters.emplace_back(builder.GetPointerType(m_Self.Type->Gen(builder)));
    for (auto &parameter : m_Parameters)
        parameters.emplace_back(parameter.GenType(builder));

    return builder.GetFunctionType(m_Result.GenType(builder), parameters, m_VarArg);
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

std::ostream &llove::FunctionType::Print(std::ostream &stream) const
{
    stream << '(';
    for (auto i = m_Parameters.begin(); i != m_Parameters.end(); ++i)
    {
        if (i != m_Parameters.begin())
            stream << ", ";
        stream << *i;
    }
    stream << ')';
    if (m_Self)
        stream << '[' << m_Self << ']';
    return stream << " => " << m_Result;
}
