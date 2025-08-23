#include <llove/builder.hpp>
#include <llove/debug.hpp>
#include <llove/tree.hpp>
#include <llove/type.hpp>
#include <llove/value.hpp>

llove::DebugBuilder::DebugBuilder(
    const bool enable,
    llvm::Module &module,
    const std::filesystem::path &source_path,
    const bool optimized,
    const bool profiling,
    const std::string &command_line,
    const llvm::DICompileUnit::DebugEmissionKind emission)
    : m_Strip(!enable)
{
    if (m_Strip)
        return;

    m_DIBuilder = std::make_unique<llvm::DIBuilder>(module);
    m_CompileUnit = m_DIBuilder->createCompileUnit(
        /* TODO: submit on https://dwarfstd.org/ for official language code */
        0x8086,
        m_DIBuilder->createFile(source_path.filename().string(), source_path.parent_path().string()),
        "LLove",
        optimized,
        command_line,
        0u,
        {},
        emission,
        0,
        true,
        profiling);
}

llvm::DIScope *llove::DebugBuilder::GetScope() const
{
    Assert(!m_Strip, "no debug information");

    return m_Scopes.empty() ? m_CompileUnit : m_Scopes.back();
}

llvm::DIType *llove::DebugBuilder::GetVoidType() const
{
    Assert(!m_Strip, "no debug information");

    return m_DIBuilder->createUnspecifiedType("void");
}

llvm::DIType *llove::DebugBuilder::GetIntegerType(const bool sign, const unsigned bits) const
{
    Assert(!m_Strip, "no debug information");

    return m_DIBuilder->createBasicType(
        (sign ? 'i' : 'u') + std::to_string(bits),
        bits,
        sign ? llvm::dwarf::DW_ATE_signed : llvm::dwarf::DW_ATE_unsigned);
}

llvm::DIType *llove::DebugBuilder::GetFloatType(const unsigned bits) const
{
    Assert(!m_Strip, "no debug information");

    return m_DIBuilder->createBasicType('f' + std::to_string(bits), bits, llvm::dwarf::DW_ATE_float);
}

llvm::DIType *llove::DebugBuilder::GetPointerType() const
{
    Assert(!m_Strip, "no debug information");

    return m_DIBuilder->createPointerType(GetVoidType(), 64); // TODO: target dependent
}

llvm::DIType *llove::DebugBuilder::GetPointerType(llvm::DIType *base) const
{
    Assert(!m_Strip, "no debug information");

    return m_DIBuilder->createPointerType(base, 64); // TODO: target dependent
}

llvm::DIType *llove::DebugBuilder::GetArrayType(llvm::DIType *base, const unsigned size) const
{
    Assert(!m_Strip, "no debug information");

    const auto subrange = m_DIBuilder->getOrCreateSubrange(0, size);

    return m_DIBuilder->createArrayType(
        size,
        0,
        base,
        m_DIBuilder->getOrCreateArray({ subrange }));
}

llvm::DIType *llove::DebugBuilder::GetStructType(const std::vector<llvm::Metadata *> &fields, const unsigned size) const
{
    Assert(!m_Strip, "no debug information");

    return m_DIBuilder->createStructType(
        nullptr,
        {},
        nullptr,
        0u,
        size,
        0u,
        llvm::DINode::FlagZero,
        nullptr,
        m_DIBuilder->getOrCreateArray(fields));
}

llvm::DIType *llove::DebugBuilder::GetVariadicType() const
{
    return GetStructType(
        {
            GetFieldType("count", GetIntegerType(false, 32), 32, 0),
            GetFieldType("data", GetPointerType(), 64, 32),
        },
        32 + 64); // TODO: target dependent
}

llvm::DIType *llove::DebugBuilder::GetFieldType(
    const std::string &name,
    llvm::DIType *type,
    const unsigned size,
    const unsigned offset) const
{
    Assert(!m_Strip, "no debug information");

    return m_DIBuilder->createMemberType(
        nullptr,
        name,
        nullptr,
        0u,
        size,
        0u,
        offset,
        llvm::DINode::FlagZero,
        type);
}

llvm::DIType *llove::DebugBuilder::GetClassType(const std::string &name) const
{
    Assert(!m_Strip, "no debug information");

    return m_DIBuilder->createClassType(
        nullptr,
        name,
        nullptr,
        0u,
        0u,
        0u,
        0u,
        llvm::DINode::FlagZero,
        nullptr,
        {});
}

llvm::DIType *llove::DebugBuilder::GetClassType(
    const std::string &name,
    const std::vector<llvm::Metadata *> &fields,
    const unsigned size) const
{
    Assert(!m_Strip, "no debug information");

    return m_DIBuilder->createClassType(
        nullptr,
        name,
        nullptr,
        0u,
        size,
        0u,
        0u,
        llvm::DINode::FlagZero,
        nullptr,
        m_DIBuilder->getOrCreateArray(fields));
}

llvm::DISubroutineType *llove::DebugBuilder::GetFunctionType(
    llvm::DIType *self,
    const std::vector<llvm::Metadata *> &parameters,
    llvm::DIType *result) const
{
    Assert(!m_Strip, "no debug information");

    std::vector<llvm::Metadata *> elements;
    elements.emplace_back(result);

    if (self)
        elements.emplace_back(self);

    for (auto &parameter : parameters)
        elements.emplace_back(parameter);

    return m_DIBuilder->createSubroutineType(m_DIBuilder->getOrCreateTypeArray(elements));
}

void llove::DebugBuilder::CreateParameter(
    Builder &builder,
    const std::string &name,
    const unsigned index,
    const ValuePtr &value) const
{
    if (m_Strip)
        return;

    const auto local_variable = m_DIBuilder->createParameterVariable(
        GetScope(),
        name,
        index,
        GetScope()->getFile(),
        0u,
        value->GetType()->GenDI(builder),
        true);

    const auto expression = m_DIBuilder->createExpression();
    const auto location = llvm::DILocation::get(builder.GetLLVMContext(), 0u, 0u, GetScope());
    const auto block = builder.GetInsertBlock();

    if (value->IsReference())
        m_DIBuilder->insertDeclare(value->GetPointer(), local_variable, expression, location, block);
    else
        m_DIBuilder->insertDbgValueIntrinsic(value->Load(builder), local_variable, expression, location, block);
}

void llove::DebugBuilder::CreateVariable(Builder &builder, const std::string &name, const ValuePtr &value) const
{
    if (m_Strip)
        return;

    const auto local_variable = m_DIBuilder->createAutoVariable(
        GetScope(),
        name,
        GetScope()->getFile(),
        0u,
        value->GetType()->GenDI(builder),
        true);

    const auto expression = m_DIBuilder->createExpression();
    const auto location = llvm::DILocation::get(builder.GetLLVMContext(), 0u, 0u, GetScope());
    const auto block = builder.GetInsertBlock();

    if (value->IsReference())
        m_DIBuilder->insertDeclare(value->GetPointer(), local_variable, expression, location, block);
    else
        m_DIBuilder->insertDbgValueIntrinsic(value->Load(builder), local_variable, expression, location, block);
}

void llove::DebugBuilder::EmitLoc(Builder &builder) const
{
    if (m_Strip)
        return;

    builder.SetCurrentDebugLocation({});
}

void llove::DebugBuilder::EmitLoc(Builder &builder, const Location &loc) const
{
    if (m_Strip)
        return;

    builder.SetCurrentDebugLocation(
        llvm::DILocation::get(
            builder.GetLLVMContext(),
            loc.Row,
            loc.Col,
            GetScope()));
}

void llove::DebugBuilder::EmitLoc(Builder &builder, const GlobalPtr &ptr) const
{
    if (m_Strip)
        return;

    builder.SetCurrentDebugLocation(
        llvm::DILocation::get(
            builder.GetLLVMContext(),
            ptr->Loc().Row,
            ptr->Loc().Col,
            GetScope()));
}

void llove::DebugBuilder::EmitLoc(Builder &builder, const StatementPtr &ptr) const
{
    if (m_Strip)
        return;

    builder.SetCurrentDebugLocation(
        llvm::DILocation::get(
            builder.GetLLVMContext(),
            ptr->Loc().Row,
            ptr->Loc().Col,
            GetScope()));
}

void llove::DebugBuilder::EndModule() const
{
    if (m_Strip)
        return;

    m_DIBuilder->finalize();
}

void llove::DebugBuilder::BeginFunction(
    Builder &builder,
    const std::string &name,
    const Location &loc,
    const FunctionType::Ptr &function_type,
    const std::string &mangled_name,
    llvm::Function *function)
{
    if (m_Strip)
        return;

    auto subprogram = m_DIBuilder->createFunction(
        GetScope(),
        mangled_name,
        name,
        GetScope()->getFile(),
        loc.Row,
        function_type->GenDIFunction(builder),
        loc.Row,
        llvm::DINode::FlagPrototyped,
        llvm::DISubprogram::SPFlagDefinition);
    m_Subprograms.emplace_back(subprogram);
    m_Scopes.emplace_back(subprogram);

    function->setSubprogram(subprogram);
}

void llove::DebugBuilder::EndFunction()
{
    if (m_Strip)
        return;

    m_DIBuilder->finalizeSubprogram(m_Subprograms.back());
    m_Subprograms.pop_back();
    m_Scopes.pop_back();
}

void llove::DebugBuilder::PushFrame(const std::optional<Location> &loc)
{
    if (m_Strip)
        return;

    auto scope = loc.has_value()
                     ? m_DIBuilder->createLexicalBlock(GetScope(), GetScope()->getFile(), loc->Row, loc->Col)
                     : GetScope();
    m_Scopes.emplace_back(scope);
}

void llove::DebugBuilder::PopFrame()
{
    if (m_Strip)
        return;

    m_Scopes.pop_back();
}
