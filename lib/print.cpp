#include <ostream>
#include <llove/forward.hpp>
#include <llove/tree.hpp>
#include <llove/type.hpp>

std::ostream &llove::operator<<(std::ostream &stream, const Field &field)
{
    return field.Print(stream);
}

std::ostream &llove::operator<<(std::ostream &stream, const Parameter &parameter)
{
    return parameter.Print(stream);
}

std::ostream &llove::operator<<(std::ostream &stream, const TypePtr &ptr)
{
    return ptr->Print(stream);
}

std::ostream &llove::operator<<(std::ostream &stream, const GlobalPtr &ptr)
{
    return ptr->Print(stream);
}

std::ostream &llove::operator<<(std::ostream &stream, const StatementPtr &ptr)
{
    if (dynamic_cast<Expression *>(ptr.get()))
        return ptr->Print(stream) << ';';

    return ptr->Print(stream);
}

std::ostream &llove::operator<<(std::ostream &stream, const ExpressionPtr &ptr)
{
    return ptr->Print(stream);
}

std::ostream &llove::Field::Print(std::ostream &stream, const bool has_name, const std::string &name) const
{
    if (has_name)
    {
        stream << (Mutable ? "mut " : "") << (Reference ? "&" : "") << name;
        if (Type)
            stream << ": " << Type;
        return stream;
    }
    return stream << (Mutable ? "mut " : "") << (Reference ? "&" : "") << Type;
}

std::ostream &llove::Parameter::Print(std::ostream &stream) const
{
    return Info.Print(stream, true, Name);
}

std::ostream &llove::VoidType::Print(std::ostream &stream) const
{
    return stream << "void";
}

std::ostream &llove::IntType::Print(std::ostream &stream) const
{
    return stream << (m_Sign ? 'i' : 'u') << m_Bits;
}

std::ostream &llove::FltType::Print(std::ostream &stream) const
{
    return stream << 'f' << m_Bits;
}

std::ostream &llove::PtrType::Print(std::ostream &stream) const
{
    return stream << m_Base << '[' << (m_Mutable ? "mut" : "") << ']';
}

std::ostream &llove::ArrayType::Print(std::ostream &stream) const
{
    return stream << m_Base << '[' << m_Size << ']';
}

std::ostream &llove::StructType::Print(std::ostream &stream) const
{
    stream << "{ ";
    for (auto i = m_Fields.begin(); i != m_Fields.end(); ++i)
    {
        if (i != m_Fields.begin())
            stream << ", ";
        stream << *i;
    }
    return stream << " }";
}

std::ostream &llove::ClassType::Print(std::ostream &stream) const
{
    return stream << "class<" << m_Name << '>';
}

void llove::ClassType::Set(std::vector<Parameter> fields)
{
    m_Fields = std::move(fields);
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
    if (m_Self.Type)
        stream << '[' << m_Self << ']';
    return stream << " => " << m_Result;
}

std::ostream &llove::DefinitionGlobal::Print(std::ostream &stream) const
{
    stream
            << (m_Interface ? "interface " : "define ")
            << m_Name
            << '(';
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
    if (m_Result.Type)
        stream << ": " << m_Result;
    if (!m_Content)
        return stream << ';';
    return stream << ' ' << m_Content;
}

std::ostream &llove::ClassDefinitionGlobal::Print(std::ostream &stream) const
{
    stream
            << "define:"
            << m_ClassName
            << ' '
            << (m_Mutable ? "mut " : "")
            << m_Name
            << '(';
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
    if (m_Result.Type)
        stream << ": " << m_Result;
    if (!m_Content)
        return stream << ';';
    return stream << ' ' << m_Content;
}

static unsigned depth = 0;

std::ostream &llove::ClassGlobal::Print(std::ostream &stream) const
{
    stream << "class " << m_Name;
    if (m_Opaque)
        return stream << ";";

    const auto cur = std::string(depth += 2, ' ');

    stream << " {" << std::endl;
    for (auto &function : m_Functions)
        stream << cur << function << std::endl;
    if (!m_Functions.empty() && !m_Fields.empty())
        stream << std::endl;
    for (auto &field : m_Fields)
        stream << cur << field << std::endl;
    return stream << std::string(depth -= 2, ' ') << '}';
}

std::ostream &llove::ScopeStatement::Print(std::ostream &stream) const
{
    const auto cur = std::string(depth += 2, ' ');

    stream << '{' << std::endl;
    for (auto &ptr : m_Content)
        stream << cur << ptr << std::endl;
    return stream << std::string(depth -= 2, ' ') << '}';
}

std::ostream &llove::LetStatement::Print(std::ostream &stream) const
{
    m_Info.Print(stream << "let ", true, m_Name);
    if (m_Value)
        stream << " = " << m_Value;
    return stream << ';';
}

std::ostream &llove::ForEachStatement::Print(std::ostream &stream) const
{
    return stream
           << "foreach ("
           << (m_Mutable ? "mut " : "")
           << (m_Reference ? "&" : "")
           << m_Name
           << " : "
           << m_Range
           << ") "
           << m_Content;
}

std::ostream &llove::YieldStatement::Print(std::ostream &stream) const
{
    if (m_Value)
        return stream << "yield " << m_Value << ';';
    return stream << "yield;";
}

std::ostream &llove::NullExpression::Print(std::ostream &stream) const
{
    if (m_Type)
        return stream << "null:" << m_Type;
    return stream << "null";
}

std::ostream &llove::IntExpression::Print(std::ostream &stream) const
{
    if (m_Type)
        return stream << m_Value << ':' << m_Type;
    return stream << m_Value;
}

std::ostream &llove::StringExpression::Print(std::ostream &stream) const
{
    std::string value;
    for (auto &c : m_Value)
    {
        if (c >= 0x20)
        {
            value += c;
            continue;
        }

        value += '\\';
        value += std::to_string(c / 0100);
        value += std::to_string(c % 0100 / 010);
        value += std::to_string(c % 010);
    }

    return stream << '"' << value << '"';
}

std::ostream &llove::RangeExpression::Print(std::ostream &stream) const
{
    return stream << m_Begin << ".." << m_End;
}

std::ostream &llove::ArrayExpression::Print(std::ostream &stream) const
{
    stream << "[ ";
    for (auto i = m_Values.begin(); i != m_Values.end(); ++i)
    {
        if (i != m_Values.begin())
            stream << ", ";
        stream << *i;
    }
    stream << " ]";
    if (m_Type)
        stream << ':' << m_Type;
    return stream;
}

std::ostream &llove::StructExpression::Print(std::ostream &stream) const
{
    stream << "{ ";
    for (auto i = m_Values.begin(); i != m_Values.end(); ++i)
    {
        if (i != m_Values.begin())
            stream << ", ";
        stream << i->first << ": " << i->second;
    }
    stream << " }";
    if (m_Type)
        stream << ':' << m_Type;
    return stream;
}

std::ostream &llove::SymbolExpression::Print(std::ostream &stream) const
{
    return stream << m_Name;
}

std::ostream &llove::BinaryExpression::Print(std::ostream &stream) const
{
    return stream << m_Left << ' ' << m_Operator << ' ' << m_Right;
}

std::ostream &llove::UnaryExpression::Print(std::ostream &stream) const
{
    if (m_Suffix)
        return stream << m_Operand << m_Operator;
    return stream << m_Operator << m_Operand;
}

std::ostream &llove::CallExpression::Print(std::ostream &stream) const
{
    stream << m_Callee << '(';
    for (auto i = m_Arguments.begin(); i != m_Arguments.end(); ++i)
    {
        if (i != m_Arguments.begin())
            stream << ", ";
        stream << *i;
    }
    return stream << ')';
}

std::ostream &llove::MemberExpression::Print(std::ostream &stream) const
{
    return stream << m_Value << '.' << m_Member;
}

std::ostream &llove::SubscriptExpression::Print(std::ostream &stream) const
{
    return stream << m_Value << '[' << m_Index << ']';
}

std::ostream &llove::operator<<(std::ostream &stream, const ClassField &field)
{
    return field.Info.Print(stream << "let ", true, field.Name) << ';';
}

std::ostream &llove::operator<<(std::ostream &stream, const ClassFunction &function)
{
    stream
            << (function.Expose ? "expose " : "")
            << (function.Mutable ? "mut " : "")
            << function.Name
            << '(';
    for (auto i = function.Parameters.begin(); i != function.Parameters.end(); ++i)
    {
        if (i != function.Parameters.begin())
            stream << ", ";
        stream << *i;
    }
    if (function.VarArg)
    {
        if (!function.Parameters.empty())
            stream << ", ";
        stream << "...";
    }
    stream << ')';
    if (function.Result.Type)
        stream << ": " << function.Result;
    if (!function.Content)
        return stream << ';';
    return stream << ' ' << function.Content;
}
