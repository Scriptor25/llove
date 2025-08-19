#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/type.hpp>

llove::FunctionType::FunctionType(
    std::vector<Field> parameters,
    const bool variadic,
    Field result,
    std::optional<Field> self)
    : m_Parameters(std::move(parameters)),
      m_Variadic(variadic),
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

bool llove::FunctionType::HasVariadic() const
{
    return m_Variadic;
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
    return 64; // TODO: target dependent
}

llvm::PointerType *llove::FunctionType::GenIR(Builder &builder)
{
    if (m_IRType)
        return llvm::dyn_cast<llvm::PointerType>(m_IRType);

    const auto type = builder.GetPointerType();
    m_IRType = type;
    return type;
}

llvm::DIType *llove::FunctionType::GenDI(Builder &builder)
{
    if (m_DIType)
        return m_DIType;

    const auto function = GenDIFunction(builder);
    return m_DIType = builder.GetDebug().GetPointerType(function);
}

llvm::FunctionType *llove::FunctionType::GenFunction(Builder &builder)
{
    if (m_IRFunction)
        return m_IRFunction;

    std::vector<llvm::Type *> parameters;

    if (m_Self)
    {
        parameters.emplace_back(m_Self->GenIRType(builder));
    }

    for (auto &parameter : m_Parameters)
    {
        parameters.emplace_back(parameter.GenIRType(builder));
    }

    if (m_Variadic)
    {
        parameters.emplace_back(builder.GetVariadicType());
    }

    const auto result = m_Result.GenIRType(builder);

    return m_IRFunction = builder.GetFunctionType(result, parameters);
}

llvm::DISubroutineType *llove::FunctionType::GenDIFunction(Builder &builder)
{
    if (m_DIFunction)
        return m_DIFunction;

    std::vector<llvm::Metadata *> parameters;
    for (auto &parameter : m_Parameters)
        parameters.emplace_back(parameter.GenDIType(builder));

    const auto self = m_Self ? m_Self->GenDIType(builder) : nullptr;
    const auto result = m_Result.GenDIType(builder);

    return m_DIFunction = builder.GetDebug().GetFunctionType(self, parameters, result);
}

llove::TypePtr llove::FunctionType::Reflect(Context &context) const
{
    std::vector<Field> parameters;
    Field result, self;

    for (auto &parameter : m_Parameters)
        parameter.Reflect(context, parameters.emplace_back());

    m_Result.Reflect(context, result);

    if (!m_Self)
        return context.GetFunction(std::move(parameters), m_Variadic, std::move(result));

    m_Self->Reflect(context, self);

    return context.GetFunction(std::move(parameters), m_Variadic, std::move(result), std::move(self));
}

std::string llove::FunctionType::Mangle() const
{
    std::string parameters;
    for (auto &parameter : m_Parameters)
        parameters += parameter.Mangle();
    return 'x'
           + std::string(m_Variadic ? "v" : "")
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
    if (m_Variadic)
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
