#include <llove/builder.hpp>
#include <llove/forward.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>
#include <utility>

llove::FunctionGlobal::FunctionGlobal(
    Location loc,
    const bool is_export,
    const bool is_interface,
    const bool is_implicit,
    const bool is_operator,
    std::string name,
    std::vector<Parameter> parameters,
    std::pair<
        bool,
        std::string> variadic,
    Field result,
    StatementPtr content)
    : Global(std::move(loc)),
      m_IsExport(is_export),
      m_IsInterface(is_interface),
      m_IsImplicit(is_implicit),
      m_IsOperator(is_operator),
      m_Name(std::move(name)),
      m_Parameters(std::move(parameters)),
      m_Variadic(std::move(variadic)),
      m_Result(std::move(result)),
      m_Content(std::move(content))
{
}

std::string llove::FunctionGlobal::GetName() const
{
    return m_Name;
}

llove::GlobalPtr llove::FunctionGlobal::Reflect(Context &context) const
{
    std::vector<Parameter> parameters;
    for (auto &parameter : m_Parameters)
        parameter.Reflect(context, parameters.emplace_back());

    Field result;
    m_Result.Reflect(context, result);

    StatementPtr content;
    m_Content->Reflect(context, content);

    return std::make_unique<FunctionGlobal>(
        m_Loc,
        m_IsExport,
        m_IsInterface,
        m_IsImplicit,
        m_IsOperator,
        m_Name,
        std::move(parameters),
        m_Variadic,
        std::move(result),
        std::move(content));
}

void llove::FunctionGlobal::Gen(Builder &builder) const try
{
    StatementPtr content;
    if (m_Content)
        m_Content->Reflect(builder.GetContext(), content);

    Function agg;
    agg.Loc = m_Loc;
    agg.IsExport = m_IsExport;
    agg.IsInterface = m_IsInterface;
    agg.IsImplicit = m_IsImplicit;
    agg.Name = m_Name;
    agg.Parameters = m_Parameters;
    agg.Variadic = m_Variadic;
    agg.Result = m_Result;
    agg.Content = std::move(content);

    builder.GenFunction(agg, true);
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

llove::TemplateInstancePtr llove::FunctionGlobal::GenTemplate(
    Builder *builder,
    Context &context,
    std::string /* name */) const
{
    Assert(!!builder, m_Loc, "builder must not be null");

    StatementPtr content;
    if (m_Content)
        m_Content->Reflect(context, content);

    std::vector<Parameter> parameters;
    for (auto &parameter : m_Parameters)
        parameter.Reflect(context, parameters.emplace_back());

    Function agg;
    agg.Loc = m_Loc;
    agg.IsExport = m_IsExport;
    agg.IsInterface = m_IsInterface;
    agg.IsImplicit = m_IsImplicit;
    agg.Name = m_Name;
    agg.Parameters = std::move(parameters);
    agg.Variadic = m_Variadic;
    agg.Result = m_Result;
    agg.Content = std::move(content);

    auto callee = builder->GenFunction(agg, false);

    return std::make_unique<FunctionTemplateInstance>(std::move(callee));
}

llove::Import llove::FunctionGlobal::GenImport(
    Context & /* context */,
    Builder &builder,
    const std::string &as,
    const ImportSymbols &symbols) const
{
    if (!m_IsExport)
        return {};

    if (!m_IsOperator && as.empty() && !symbols.empty() && !symbols.contains(m_Name))
        return {};

    const auto register_function = m_IsOperator
                                   || (as.empty() && symbols.empty())
                                   || (as.empty() && symbols.contains(m_Name) && symbols.at(m_Name) == m_Name);

    Function agg;
    agg.Loc = m_Loc;
    agg.IsExport = m_IsExport;
    agg.IsInterface = m_IsInterface;
    agg.IsImplicit = m_IsImplicit;
    agg.Name = m_Name;
    agg.Parameters = m_Parameters;
    agg.Variadic = m_Variadic;
    agg.Result = m_Result;

    const auto function = builder.GenFunction(agg, register_function);

    if (register_function)
        return { m_Name, nullptr };

    auto value = Value::CreateR(function.Type, function.Callee);

    if (symbols.contains(m_Name))
    {
        builder.SetValue(symbols.at(m_Name), std::move(value));
        return { m_Name, nullptr };
    }

    return { m_Name, std::move(value) };
}

std::ostream &llove::FunctionGlobal::Print(std::ostream &stream) const
{
    stream << (m_IsExport ? "export " : "") << (m_IsInterface ? "interface " : "function ") << (
        m_IsImplicit ? "implicit " : "") << m_Name << '(';

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
