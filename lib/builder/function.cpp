#include <llove/builder.hpp>
#include <llove/error.hpp>
#include <llove/tree.hpp>

llvm::Function* llove::Builder::GetOrCreateFunction(
    const std::string& name,
    const FunctionType::Ptr& type,
    const bool external)
{
    if (const auto function = m_LLVMModule.getFunction(name))
    {
        return function;
    }
    return llvm::Function::Create(type->GenFunction(*this), external ? llvm::Function::ExternalLinkage : llvm::Function::InternalLinkage, name, m_LLVMModule);
}

llove::FunctionReference& llove::Builder::PushFunction(
    const bool expose,
    const bool implicit,
    std::string name,
    FunctionType::Ptr type,
    llvm::Function* callee)
{
    for (auto& function : m_Functions)
    {
        if (function.Name != name)
        {
            continue;
        }
        if (function.Type != type)
        {
            continue;
        }
        Assert(
            expose == function.IsExposed && implicit == function.IsImplicit
                && callee == function.Callee,
            "function prototype mismatch");
        return function;
    }

    return m_Functions.emplace_back(expose, implicit, std::move(name), std::move(type), callee);
}

std::vector<llove::FunctionReference> llove::Builder::GetFunctions(
    const std::string& name,
    const std::optional<Field>& self)
{
    std::vector<FunctionReference> functions;

    for (auto& function : m_Functions)
    {
        if (function.Name != name)
        {
            continue;
        }

        const auto& function_type = function.Type;
        const auto& function_self = function_type->GetSelf();

        if (self)
        {
            if (!function_self)
            {
                continue;
            }
            if (!Field::IsCastable(*this, *function_self, *self, false))
            {
                continue;
            }
            if (!function.IsExposed && function_self->GetType() != m_Class
                && !m_Class->InheritsFrom(function_self->GetType()))
            {
                continue;
            }
        }
        else if (function_self && !function.IsExposed && function_self->GetType() != m_Class)
        {
            continue;
        }

        functions.emplace_back(function);
    }

    if (self && self->HasType() && self->GetType()->IsClass())
    {
        const auto class_type = As<ClassType>(self->GetType());
        for (auto class_functions = class_type->GetFunctions(class_type, name); auto& [parent, function] : class_functions)
        {
            if (!function.IsExposed && parent != m_Class && !m_Class->InheritsFrom(parent))
            {
                continue;
            }

            std::vector<Parameter> parameters;
            for (auto& parameter : function.Parameters)
            {
                parameters.emplace_back(parameter);
            }

            Function agg;
            agg.IsExport = function.IsExport;
            agg.IsExposed = function.IsExposed;
            agg.IsVirtual = function.IsVirtual;
            agg.IsOverride = function.IsOverride;
            agg.IsImplicit = function.IsImplicit;
            agg.IsMutable = function.IsMutable;
            agg.Class = parent;
            agg.Name = function.Name;
            agg.Parameters = std::move(parameters);
            agg.Variadic = { function.HasVariadic, {} };
            agg.Result = function.Result;

            auto reference = GenFunction(agg);
            functions.emplace_back(std::move(reference));
        }
    }

    return functions;
}

std::optional<llove::FunctionReference> llove::Builder::FindFunction(
    const std::vector<FunctionReference>& functions,
    const std::vector<Field>& arguments,
    const std::optional<Field>& self) const
{
    auto lowest_error = ~0u;
    std::vector<FunctionReference> candidates;

    for (auto& function : functions)
    {
        const auto& function_type = function.Type;
        const auto& function_self = function_type->GetSelf();
        const auto parameter_count = function_type->GetParameterCount();

        if (function_self.has_value() != self.has_value())
        {
            continue;
        }

        auto error = 0u;

        if (function_self && self && Field::GetCastError(*this, *function_self, *self, error, true))
        {
            continue;
        }

        if (parameter_count > arguments.size())
        {
            continue;
        }
        if (!function_type->HasVariadic() && parameter_count < arguments.size())
        {
            continue;
        }

        if (parameter_count != arguments.size())
        {
            error += 2u;
        }

        unsigned i;
        for (i = 0; i < parameter_count; ++i)
        {
            if (Field::GetCastError(
                    *this,
                    function_type->GetParameter(i),
                    arguments.at(i),
                    error,
                    false))
            {
                break;
            }
        }
        if (i < parameter_count)
        {
            continue;
        }

        if (error > lowest_error)
        {
            continue;
        }

        if (error < lowest_error)
        {
            candidates.clear();
        }

        lowest_error = error;
        candidates.emplace_back(function);
    }

    if (candidates.empty())
    {
        return std::nullopt;
    }

    if (candidates.size() == 1)
    {
        return candidates.front();
    }

    Error("ambiguous candidates {} for {}, self '{}'", candidates, arguments, self);
}

std::optional<llove::FunctionReference> llove::Builder::FindFunction(
    const ClassType::VecRef<ClassFunctionReference>& functions,
    const std::vector<Field>& arguments,
    const Field& self,
    const bool implicit)
{
    auto lowest_error = ~0u;
    ClassType::VecRef<ClassFunctionReference> candidates;

    for (auto& [parent, function] : functions)
    {
        const Field function_self(function.IsMutable, true, parent);

        if (implicit && !function.IsImplicit)
        {
            continue;
        }

        auto error = 0u;

        if (Field::GetCastError(*this, function_self, self, error, true))
        {
            continue;
        }

        const auto parameter_count = function.Parameters.size();
        const auto argument_count = arguments.size();

        if (parameter_count > argument_count)
        {
            continue;
        }
        if (!function.HasVariadic && parameter_count < argument_count)
        {
            continue;
        }

        if (parameter_count != argument_count)
        {
            error += 2u;
        }

        unsigned i;
        for (i = 0; i < parameter_count; ++i)
        {
            if (Field::GetCastError(*this, function.Parameters.at(i), arguments.at(i), error, false))
            {
                break;
            }
        }
        if (i < parameter_count)
        {
            continue;
        }

        if (error > lowest_error)
        {
            continue;
        }

        if (error < lowest_error)
        {
            candidates.clear();
        }

        lowest_error = error;
        candidates.emplace_back(parent, function);
    }

    if (candidates.empty())
    {
        return std::nullopt;
    }

    if (candidates.size() == 1)
    {
        auto& [parent, candidate] = candidates.front();

        std::vector<Parameter> parameters;
        for (auto& parameter : candidate.Parameters)
        {
            parameters.emplace_back(parameter);
        }

        Function agg;
        agg.IsExport = candidate.IsExport;
        agg.IsExposed = candidate.IsExposed;
        agg.IsVirtual = candidate.IsVirtual;
        agg.IsOverride = candidate.IsOverride;
        agg.IsImplicit = candidate.IsImplicit;
        agg.IsMutable = candidate.IsMutable;
        agg.Class = parent;
        agg.Name = candidate.Name;
        agg.Parameters = std::move(parameters);
        agg.Variadic = { candidate.HasVariadic, {} };
        agg.Result = candidate.Result;

        return GenFunction(agg);
    }

    Error("ambiguous candidates {} for {}, self '{}'", candidates, arguments, self);
}
