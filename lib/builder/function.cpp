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
    const bool implicit,
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
        Assert(
            expose == function.Expose
            && implicit == function.Implicit
            && callee == function.Callee,
            "function prototype mismatch");
        return function;
    }

    return m_Functions.emplace_back(expose, implicit, std::move(name), std::move(type), callee);
}

std::vector<llove::FunctionReference> llove::Builder::GetFunctions(
    const std::string &name,
    const std::optional<Field> &self) const
{
    std::vector<FunctionReference> functions;

    for (auto &function : m_Functions)
    {
        if (function.Name != name)
            continue;

        auto &function_type = function.Type;
        const auto function_self = function_type->GetSelf();

        if (self)
        {
            if (!function_self)
                continue;
            if (function_self->Type != self->Type)
                continue;
            if (function_self->Mutable && !self->Mutable)
                continue;
            if (function_self->Reference != self->Reference)
                continue;
            if (!function.Expose && function_self->Type != m_Class)
                continue;
        }
        else if (function_self && !function.Expose && function_self->Type != m_Class)
            continue;

        functions.emplace_back(function);
    }

    return functions;
}

bool llove::Builder::HasFunction(
    const std::vector<FunctionReference> &functions,
    const std::vector<Field> &arguments,
    const std::optional<Field> &self) const
{
    for (const auto &function : functions)
    {
        const auto &function_type = function.Type;
        const auto &function_self = function_type->GetSelf();
        const auto parameter_count = function_type->GetParameterCount();

        if (function_self.has_value() != self.has_value())
            continue;

        if (function_self && self && !Field::IsCastable(*this, *function_self, *self, true))
            continue;

        if (parameter_count > arguments.size())
            continue;
        if (!function_type->IsVarArg() && parameter_count < arguments.size())
            continue;

        unsigned i;
        for (i = 0; i < parameter_count; ++i)
            if (!Field::IsCastable(*this, function_type->GetParameter(i), arguments.at(i), false))
                break;
        if (i < parameter_count)
            continue;

        return true;
    }

    return false;
}

std::optional<llove::FunctionReference> llove::Builder::FindFunction(
    const std::vector<FunctionReference> &functions,
    const std::vector<Field> &arguments,
    const std::optional<Field> &self) const
{
    auto lowest_error = ~0u;
    std::vector<FunctionReference> candidates;

    for (auto &function : functions)
    {
        const auto &function_type = function.Type;
        const auto &function_self = function_type->GetSelf();
        const auto parameter_count = function_type->GetParameterCount();

        if (function_self.has_value() != self.has_value())
            continue;

        auto error = 0u;

        if (function_self && self && Field::GetCastError(*this, *function_self, *self, error, true))
            continue;

        if (parameter_count > arguments.size())
            continue;
        if (!function_type->IsVarArg() && parameter_count < arguments.size())
            continue;

        if (parameter_count != arguments.size())
            error += 2u;

        unsigned i;
        for (i = 0; i < parameter_count; ++i)
            if (Field::GetCastError(*this, function_type->GetParameter(i), arguments.at(i), error, false))
                break;
        if (i < parameter_count)
            continue;

        if (error > lowest_error)
            continue;

        if (error < lowest_error)
            candidates.clear();

        lowest_error = error;
        candidates.emplace_back(function);
    }

    if (candidates.empty())
        return std::nullopt;

    if (candidates.size() == 1)
        return candidates.front();

    Error("ambiguous candidates {} for {}, self '{}'", candidates, arguments, self);
}

std::optional<llove::FunctionReference> llove::Builder::FindFunction(
    const std::vector<ClassFunctionReference> &functions,
    const std::vector<Field> &arguments,
    const ClassType::Ptr &class_type,
    const Field &self,
    const bool implicit)
{
    auto lowest_error = ~0u;
    std::vector<ClassFunctionReference> candidates;

    for (auto &function : functions)
    {
        const Field function_self
        {
            .Mutable = function.Mutable,
            .Reference = true,
            .Type = class_type,
        };

        if (implicit && !function.Implicit)
            continue;

        auto error = 0u;

        if (Field::GetCastError(*this, function_self, self, error, true))
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

        if (error < lowest_error)
            candidates.clear();

        lowest_error = error;
        candidates.emplace_back(function);
    }

    if (candidates.empty())
        return std::nullopt;

    if (candidates.size() == 1)
    {
        auto &candidate = candidates.front();

        std::vector<Parameter> parameters;
        for (auto &parameter : candidate.Parameters)
            parameters.emplace_back(parameter);

        return GenFunction(
            {
                .Implicit = candidate.Implicit,
                .Class = class_type,
                .Mutable = candidate.Mutable,
                .Expose = candidate.Expose,
                .Name = candidate.Name,
                .Parameters = std::move(parameters),
                .VarArg = candidate.VarArg,
                .Result = candidate.Result,
            });
    }

    Error("ambiguous candidates {} for {}, self '{}'", candidates, arguments, self);
}
