#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>
#include <utility>

llove::ClassDefinitionGlobal::ClassDefinitionGlobal(
    Location loc,
    ClassType::Ptr class_type,
    const bool is_mutable,
    std::string name,
    std::vector<Parameter> parameters,
    std::pair<
        bool,
        std::string> variadic,
    Field result,
    std::vector<Initializer> initializers,
    StatementPtr content)
    : Global(std::move(loc)),
      m_ClassType(std::move(class_type)),
      m_IsMutable(is_mutable),
      m_Name(std::move(name)),
      m_Parameters(std::move(parameters)),
      m_Variadic(std::move(variadic)),
      m_Result(std::move(result)),
      m_Initializers(std::move(initializers)),
      m_Content(std::move(content))
{
}

void llove::ClassDefinitionGlobal::Gen(Builder& builder) const
try
{
    std::vector<Field> parameters;
    for (auto& parameter : m_Parameters)
        parameters.emplace_back(parameter.Info);

    const auto reference = m_ClassType->GetFunction(
        m_ClassType,
        m_Name,
        m_IsMutable,
        parameters,
        m_Variadic.first,
        m_Result);
    Assert(reference.has_value(), "class function prototype mismatch");

    std::vector<Initializer> initializers;
    for (auto& initializer : m_Initializers)
        initializer.Reflect(builder.GetContext(), initializers.emplace_back());

    StatementPtr content;
    if (m_Content)
        m_Content->Reflect(builder.GetContext(), content);

    auto& [parent, function] = *reference;

    Function agg;
    agg.Loc = m_Loc;
    agg.IsExport = function.IsExport;
    agg.IsExposed = function.IsExposed;
    agg.IsVirtual = function.IsVirtual;
    agg.IsOverride = function.IsOverride;
    agg.IsImplicit = function.IsImplicit;
    agg.IsMutable = function.IsMutable;
    agg.Class = parent;
    agg.Name = function.Name;
    agg.Parameters = m_Parameters;
    agg.Variadic = m_Variadic;
    agg.Result = function.Result;
    agg.Initializers = std::move(initializers);
    agg.Content = std::move(content);

    builder.GenFunction(agg);
}
catch (ref_exception<ErrorStack>& cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::pair<
    std::string,
    llove::ValuePtr>
llove::ClassDefinitionGlobal::GenImport(
    Context& /* context */,
    Builder& /* builder */,
    const std::string& /* as */,
    const std::map<
        std::string,
        std::string>& /* symbols */) const
{
    return {};
}

std::ostream& llove::ClassDefinitionGlobal::Print(std::ostream& stream) const
{
    stream << "define:" << m_ClassType->GetName() << ' ' << (m_IsMutable ? "mut " : "") << m_Name << '(';

    for (auto i = m_Parameters.begin(); i != m_Parameters.end(); ++i)
    {
        if (i != m_Parameters.begin())
            stream << ", ";
        stream << *i;
    }
    if (m_Variadic.first)
    {
        if (!m_Parameters.empty())
            stream << ", ";
        stream << "...";
        if (!m_Variadic.second.empty())
            stream << m_Variadic.second;
    }

    stream << "): " << m_Result;

    if (!m_Content)
        return stream << ';';

    return stream << ' ' << m_Content;
}
