#include <fstream>
#include <llove/builder.hpp>
#include <llove/context.hpp>
#include <llove/parser.hpp>
#include <llove/tree.hpp>
#include <llove/value.hpp>

llove::ImportGlobal::ImportGlobal(
    Location loc,
    std::string as,
    std::map<std::string, std::string> symbols,
    std::filesystem::path filepath)
    : Global(std::move(loc)),
      m_As(std::move(as)),
      m_Symbols(std::move(symbols)),
      m_Filepath(std::move(filepath))
{
}

void llove::ImportGlobal::Gen(Builder &builder) const try
{
    std::ifstream stream(m_Filepath);
    Assert(stream.is_open(), "failed to open import file '{}'", m_Filepath.string());

    Context context(&builder.GetContext());
    Parser parser(context, stream, m_Filepath);

    std::vector<std::pair<std::string, ValuePtr>> values;
    while (parser.Ok())
    {
        auto ptr = parser.Parse();
        if (auto [name, value] = ptr->GenImport(context, builder, m_As, m_Symbols); value)
            values.emplace_back(std::move(name), std::move(value));
    }

    stream.close();

    if (m_As.empty())
        return;

    std::vector<Parameter> fields;
    for (auto &[name, value] : values)
        fields.emplace_back(value->AsField(), name);
    auto type = builder.GetContext().GetStruct(std::move(fields));

    llvm::Value *aggregate = llvm::Constant::getNullValue(type->GenIR(builder));

    for (unsigned i = 0; i < values.size(); ++i)
    {
        auto &[name, value] = values.at(i);
        aggregate = builder.CreateInsertValue(aggregate, value->Load(builder), i);
    }

    auto value = Value::CreateR(type, aggregate);
    builder.SetValue(m_As, std::move(value));
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::pair<std::string, llove::ValuePtr> llove::ImportGlobal::GenImport(
    Context &parent,
    Builder &builder,
    const std::string &as,
    const std::map<std::string, std::string> &symbols) const try
{
    // TODO: check recursion

    std::ifstream stream(m_Filepath);
    Assert(stream.is_open(), "failed to open import file '{}'", m_Filepath.string());

    Context context(&parent);
    Parser parser(context, stream, m_Filepath);

    while (parser.Ok())
    {
        auto ptr = parser.Parse();
        (void) ptr->GenImport(context, builder, m_As, m_Symbols);
    }

    stream.close();

    return {};
}
catch (ref_exception<ErrorStack> &cause)
{
    throw ref_exception<ErrorStack>(std::move(cause), m_Loc, std::nullopt);
}

std::ostream &llove::ImportGlobal::Print(std::ostream &stream) const
{
    if (m_Symbols.empty())
        return stream << "import " << (m_As.empty() ? "*" : m_As) << " from \"" << m_Filepath.string() << "\";";

    stream << "import { ";
    for (auto i = m_Symbols.begin(); i != m_Symbols.end(); ++i)
    {
        if (i != m_Symbols.begin())
            stream << ", ";
        stream << i->first;
        if (i->second != i->first)
            stream << ": " << i->second;
    }
    if (!m_As.empty())
        stream << ", ..." << m_As;
    return stream << " } from \"" << m_Filepath.string() << "\";";
}
