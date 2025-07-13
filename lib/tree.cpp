#include <utility>
#include <vector>
#include <llove/tree.hpp>

llove::DefinitionGlobal::DefinitionGlobal(
    const bool interface,
    std::string name,
    std::vector<Parameter> parameters,
    const bool vararg,
    Field result,
    StatementPtr content)
    : m_Interface(interface),
      m_Name(std::move(name)),
      m_Parameters(std::move(parameters)),
      m_VarArg(vararg),
      m_Result(std::move(result)),
      m_Content(std::move(content))
{
}

llove::ClassDefinitionGlobal::ClassDefinitionGlobal(
    std::string class_name,
    const bool mutable_,
    std::string name,
    std::vector<Parameter> parameters,
    const bool vararg,
    Field result,
    StatementPtr content)
    : m_ClassName(std::move(class_name)),
      m_Mutable(mutable_),
      m_Name(std::move(name)),
      m_Parameters(std::move(parameters)),
      m_VarArg(vararg),
      m_Result(std::move(result)),
      m_Content(std::move(content))
{
}

llove::ClassGlobal::ClassGlobal(std::string name)
    : m_Name(std::move(name)),
      m_Opaque(true)
{
}

llove::ClassGlobal::ClassGlobal(std::string name, std::vector<ClassField> fields, std::vector<ClassFunction> functions)
    : m_Name(std::move(name)),
      m_Opaque(false),
      m_Fields(std::move(fields)),
      m_Functions(std::move(functions))
{
}

llove::ScopeStatement::ScopeStatement(std::vector<StatementPtr> content)
    : m_Content(std::move(content))
{
}

llove::ForStatement::ForStatement(
    StatementPtr prefix,
    StatementPtr suffix,
    ExpressionPtr condition,
    StatementPtr content)
    : m_Prefix(std::move(prefix)),
      m_Suffix(std::move(suffix)),
      m_Condition(std::move(condition)),
      m_Content(std::move(content))
{
}

llove::ForEachStatement::ForEachStatement(
    const bool mutable_,
    const bool reference,
    std::string name,
    ExpressionPtr range,
    StatementPtr content)
    : m_Mutable(mutable_),
      m_Reference(reference),
      m_Name(std::move(name)),
      m_Range(std::move(range)),
      m_Content(std::move(content))
{
}

llove::IfStatement::IfStatement(ExpressionPtr condition, StatementPtr then, StatementPtr else_)
    : m_Condition(std::move(condition)),
      m_Then(std::move(then)),
      m_Else(std::move(else_))
{
}

llove::LetStatement::LetStatement(Field info, std::string name, ExpressionPtr value)
    : m_Info(std::move(info)),
      m_Name(std::move(name)),
      m_Value(std::move(value))
{
}

llove::YieldStatement::YieldStatement(ExpressionPtr value)
    : m_Value(std::move(value))
{
}

llove::NullExpression::NullExpression(TypePtr type)
    : m_Type(std::move(type))
{
}

llove::IntExpression::IntExpression(const uint64_t value, IntegerType::Ptr type)
    : m_Value(value),
      m_Type(std::move(type))
{
}

llove::StringExpression::StringExpression(std::string value)
    : m_Value(std::move(value))
{
}

llove::RangeExpression::RangeExpression(ExpressionPtr begin, ExpressionPtr end)
    : m_Begin(std::move(begin)),
      m_End(std::move(end))
{
}

llove::ArrayExpression::ArrayExpression(std::vector<ExpressionPtr> values, ArrayType::Ptr type)
    : m_Values(std::move(values)),
      m_Type(std::move(type))
{
}

llove::StructExpression::StructExpression(std::map<std::string, ExpressionPtr> values, StructType::Ptr type)
    : m_Values(std::move(values)),
      m_Type(std::move(type))
{
}

llove::SymbolExpression::SymbolExpression(std::string name)
    : m_Name(std::move(name))
{
}

llove::BinaryExpression::BinaryExpression(std::string operator_, ExpressionPtr left, ExpressionPtr right)
    : m_Operator(std::move(operator_)),
      m_Left(std::move(left)),
      m_Right(std::move(right))
{
}

llove::UnaryExpression::UnaryExpression(std::string operator_, ExpressionPtr operand, const bool suffix)
    : m_Operator(std::move(operator_)),
      m_Operand(std::move(operand)),
      m_Suffix(suffix)
{
}

llove::CallExpression::CallExpression(ExpressionPtr callee, std::vector<ExpressionPtr> arguments)
    : m_Callee(std::move(callee)),
      m_Arguments(std::move(arguments))
{
}

llove::MemberExpression::MemberExpression(ExpressionPtr value, std::string member)
    : m_Value(std::move(value)),
      m_Member(std::move(member))
{
}

llove::SubscriptExpression::SubscriptExpression(ExpressionPtr value, ExpressionPtr index)
    : m_Value(std::move(value)),
      m_Index(std::move(index))
{
}

llove::CreateExpression::CreateExpression(ClassType::Ptr class_type, std::vector<ExpressionPtr> arguments)
    : m_ClassType(std::move(class_type)),
      m_Arguments(std::move(arguments))
{
}
