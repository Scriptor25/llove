#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llove/forward.hpp>
#include <llove/tree.hpp>
#include <utility>

llove::ClassFunctionGlobal::ClassFunctionGlobal(
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

std::string llove::ClassFunctionGlobal::GetName() const
{
    return m_Name;
}

llove::GlobalPtr llove::ClassFunctionGlobal::Reflect(Context& context) const
{
    ClassType::Ptr class_type;
    Type::Reflect(context, m_ClassType, class_type);

    std::vector<Parameter> parameters;
    for (auto& parameter : m_Parameters)
    {
        auto& p = parameters.emplace_back();
        p.Name = parameter.Name;
        parameter.Info.Reflect(context, p.Info);
    }

    Field result;
    m_Result.Reflect(context, result);

    std::vector<Initializer> initializers;
    for (auto& initializer : m_Initializers)
        initializer.Reflect(context, initializers.emplace_back());

    StatementPtr content;
    m_Content->Reflect(context, content);

    return std::make_unique<ClassFunctionGlobal>(m_Loc, std::move(class_type), m_IsMutable, m_Name, std::move(parameters), m_Variadic, std::move(result), std::move(initializers), std::move(content));
}

void llove::ClassFunctionGlobal::Gen(Builder& builder) const
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
    agg.IsPublic = function.IsPublic;
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

    builder.GenFunction(agg, false);
}
catch (ref_exception<ErrorStack>& cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::TemplateInstancePtr llove::ClassFunctionGlobal::GenTemplate(
    Builder* /* builder */,
    Context& /* context */,
    std::string /* name */) const
{
    Error(m_Loc, "class functions do not support templating");
}

llove::Import llove::ClassFunctionGlobal::GenImport(
    Context& /* context */,
    Builder& /* builder */,
    const std::string& /* as */,
    const ImportSymbols& /* symbols */) const
{
    return {};
}

std::ostream& llove::ClassFunctionGlobal::Print(std::ostream& stream) const
{
    stream << "function:" << m_ClassType->GetName() << ' ' << (m_IsMutable ? "mut " : "") << m_Name << '(';

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
