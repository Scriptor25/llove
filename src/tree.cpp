#include <utility>
#include <vector>
#include <llove/tree.hpp>

llove::DefinitionGlobal::DefinitionGlobal(
    const bool interface,
    const bool demangle,
    std::string name,
    const std::vector<Parameter> &parameters,
    const bool vararg,
    Field result,
    StatementPtr content)
    : Interface(interface),
      Demangle(demangle),
      Name(std::move(name)),
      Parameters(parameters),
      Vararg(vararg),
      Result(std::move(result)),
      Content(std::move(content))
{
}

llove::ScopeStatement::ScopeStatement(std::vector<StatementPtr> content)
    : Content(std::move(content))
{
}

llove::LetStatement::LetStatement(Field info, std::string name, ExpressionPtr value)
    : Info(std::move(info)),
      Name(std::move(name)),
      Value(std::move(value))
{
}

llove::ForEachStatement::ForEachStatement(
    const bool mutable_,
    const bool reference,
    std::string name,
    ExpressionPtr range,
    StatementPtr content)
    : Mutable(mutable_),
      Reference(reference),
      Name(std::move(name)),
      Range(std::move(range)),
      Content(std::move(content))
{
}

llove::YieldStatement::YieldStatement(ExpressionPtr value)
    : Value(std::move(value))
{
}

llove::IntExpression::IntExpression(const uint64_t value, TypePtr type)
    : Value(value),
      Type(std::move(type))
{
}

llove::StringExpression::StringExpression(std::string value)
    : Value(std::move(value))
{
}

llove::RangeExpression::RangeExpression(
    const bool include_begin,
    ExpressionPtr begin,
    ExpressionPtr end,
    const bool include_end)
    : IncludeBegin(include_begin),
      Begin(std::move(begin)),
      End(std::move(end)),
      IncludeEnd(include_end)
{
}

llove::SymbolExpression::SymbolExpression(std::string name)
    : Name(std::move(name))
{
}

llove::BinaryExpression::BinaryExpression(std::string operator_, ExpressionPtr left, ExpressionPtr right)
    : Operator(std::move(operator_)),
      Left(std::move(left)),
      Right(std::move(right))
{
}

llove::UnaryExpression::UnaryExpression(std::string operator_, ExpressionPtr operand, const bool suffix)
    : Operator(std::move(operator_)),
      Operand(std::move(operand)),
      Suffix(suffix)
{
}

llove::CallExpression::CallExpression(ExpressionPtr callee, std::vector<ExpressionPtr> arguments)
    : Callee(std::move(callee)),
      Arguments(std::move(arguments))
{
}
