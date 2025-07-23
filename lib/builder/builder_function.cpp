#include <llove/builder.hpp>
#include <llove/error.hpp>

llvm::Function *llove::Builder::GetOrCreateFunction(
    const std::string &name,
    const FunctionType::Ptr &type,
    const bool external)
{
    if (const auto function = m_Module.getFunction(name))
        return function;
    return llvm::Function::Create(
        type->GenFunction(*this),
        external ? llvm::Function::ExternalLinkage : llvm::Function::InternalLinkage,
        name,
        m_Module);
}

llove::FunctionReference &llove::Builder::PushFunction(
    const bool expose,
    std::string name,
    FunctionType::Ptr type,
    llvm::Function *callee)
{
    for (auto &function : m_Functions)
    {
        if (function.Name != name)
            continue;
        if (function.Type != type)
            continue;
        Assert(expose == function.Expose && callee == function.Callee, "function prototype mismatch");
        return function;
    }

    return m_Functions.emplace_back(
        FunctionReference
        {
            .Expose = expose,
            .Name = std::move(name),
            .Type = std::move(type),
            .Callee = callee,
        }
    );
}

std::vector<llove::FunctionReference> llove::Builder::GetFunctions(const std::string &name) const
{
    std::vector<FunctionReference> functions;
    for (auto &function : m_Functions)
    {
        if (function.Name != name)
            continue;
        if (function.Type->HasSelf() && !function.Expose && function.Type->GetSelf().Type != m_Class)
            continue;
        functions.emplace_back(function);
    }
    return functions;
}

std::vector<llove::FunctionReference> llove::Builder::GetFunctions(const std::string &name, const Field &self) const
{
    std::vector<FunctionReference> functions;
    for (auto &function : m_Functions)
    {
        if (function.Name != name)
            continue;
        if (!function.Expose && function.Type->GetSelf().Type != m_Class)
            continue;
        if (!function.Type->HasSelf())
            continue;
        auto &function_self = function.Type->GetSelf();
        if (function_self.Type != self.Type)
            continue;
        if (function_self.Mutable && !self.Mutable)
            continue;
        if (!function.Expose && m_Class != self.Type)
            continue;
        functions.emplace_back(function);
    }
    return functions;
}

bool llove::Builder::HasFunction(
    const std::vector<FunctionReference> &functions,
    const std::vector<Field> &arguments,
    const bool has_self,
    const Field &self) const
{
    for (const auto &function : functions)
    {
        const auto function_type = function.Type;
        if (function_type->HasSelf() != has_self)
            continue;

        if (has_self && !Field::IsCastable(*this, function_type->GetSelf(), self, true))
            continue;

        if (function_type->GetParameterCount() > arguments.size())
            continue;
        if (!function_type->IsVarArg() && function_type->GetParameterCount() < arguments.size())
            continue;

        unsigned i;
        for (i = 0; i < function_type->GetParameterCount(); ++i)
            if (!Field::IsCastable(*this, function_type->GetParameter(i), arguments.at(i), false))
                break;
        if (i < function_type->GetParameterCount())
            continue;

        return true;
    }

    return false;
}

std::optional<llove::FunctionReference> llove::Builder::FindFunction(
    const std::vector<FunctionReference> &functions,
    const std::vector<Field> &arguments,
    const Field &self) const
{
    auto lowest_error = ~0u;
    std::optional<FunctionReference> callee;

    const auto has_self = static_cast<bool>(self);

    for (auto &function : functions)
    {
        const auto function_type = function.Type;
        if (function_type->HasSelf() != has_self)
            continue;

        auto error = 0u;

        if (has_self && Field::GetCastError(*this, function_type->GetSelf(), self, error, true))
            continue;

        if (function_type->GetParameterCount() > arguments.size())
            continue;
        if (!function_type->IsVarArg() && function_type->GetParameterCount() < arguments.size())
            continue;

        if (function_type->GetParameterCount() != arguments.size())
            error += 2u;

        unsigned i;
        for (i = 0; i < function_type->GetParameterCount(); ++i)
            if (Field::GetCastError(*this, function_type->GetParameter(i), arguments.at(i), error, false))
                break;
        if (i < function_type->GetParameterCount())
            continue;

        if (error > lowest_error)
            continue;

        Assert(error != lowest_error, "ambiguous candidates");

        lowest_error = error;
        callee = function;
    }

    return callee;
}

std::optional<llove::FunctionReference> llove::Builder::FindFunction(
    const std::vector<ClassFunctionReference> &functions,
    const std::vector<Field> &arguments,
    const ClassType::Ptr &class_type,
    const Field &self)
{
    auto lowest_error = ~0u;
    std::optional<ClassFunctionReference> candidate;

    for (auto &function : functions)
    {
        const Field class_
        {
            .Mutable = function.Mutable,
            .Reference = true,
            .Type = class_type,
        };

        auto error = 0u;

        if (Field::GetCastError(*this, class_, self, error, true))
            continue;

        const auto parameter_count = function.Parameters.size();
        const auto argument_count = arguments.size();

        if (parameter_count > argument_count)
            continue;
        if (!function.VarArg && parameter_count < argument_count)
            continue;

        if (parameter_count != argument_count)
            error += 2u;

        unsigned i;
        for (i = 0; i < parameter_count; ++i)
            if (Field::GetCastError(*this, function.Parameters.at(i), arguments.at(i), error, false))
                break;
        if (i < parameter_count)
            continue;

        if (error > lowest_error)
            continue;

        Assert(error != lowest_error, "ambiguous candidates");

        lowest_error = error;
        candidate = function;
    }

    if (!candidate.has_value())
        return std::nullopt;

    std::vector<Parameter> parameters;
    for (auto &parameter : candidate->Parameters)
        parameters.emplace_back(parameter);

    return GenFunction(
        {
            .Class = class_type,
            .Mutable = candidate->Mutable,
            .Expose = candidate->Expose,
            .Name = candidate->Name,
            .Parameters = std::move(parameters),
            .VarArg = candidate->VarArg,
            .Result = candidate->Result,
        });
}
