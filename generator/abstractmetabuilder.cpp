/****************************************************************************
**
** Copyright (C) 1992-2009 Nokia. All rights reserved.
** Copyright (C) 2009-2020 Dr. Peter Droste, Omix Visualization GmbH & Co. KG. All rights reserved.
**
** This file is part of Qt Jambi.
**
** ** $BEGIN_LICENSE$
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** $END_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "abstractmetabuilder.h"
#include "reporthandler.h"

#include "ast.h"
#include "binder.h"
#include "control.h"
#include "default_visitor.h"
#include "dumptree.h"
#include "lexer.h"
#include "parser.h"
#include "tokens.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextCodec>
#include <QTextStream>
#include <QVariant>

static QString strip_template_args(const QString &name) {
    int pos = name.indexOf('<');
    return pos < 0 ? name : name.left(pos);
}

/**
 * Checks if operator is legit and returns returns
 * corresponding operator with prefix.
 *
 * For example for "+" return "operator_add"
 */
QString AbstractMetaBuilder::rename_operator(const QString &oper) {
    QString op = oper.trimmed();
    static QHash<QString, QString> *operator_names = nullptr;
    if (!operator_names) {
        operator_names = new QHash<QString, QString>();

        operator_names->insert("+", "add");
        operator_names->insert("-", "subtract");
        operator_names->insert("*", "multiply");
        operator_names->insert("/", "divide");
        operator_names->insert("%", "modulo");
        operator_names->insert("&", "and");
        operator_names->insert("|", "or");
        operator_names->insert("^", "xor");
        operator_names->insert("~", "negate");
        operator_names->insert("<<", "shift_left");
        operator_names->insert(">>", "shift_right");

        // assigments
        operator_names->insert("=", "assign");
        operator_names->insert("+=", "add_assign");
        operator_names->insert("-=", "subtract_assign");
        operator_names->insert("*=", "multiply_assign");
        operator_names->insert("/=", "divide_assign");
        operator_names->insert("%=", "modulo_assign");
        operator_names->insert("&=", "and_assign");
        operator_names->insert("|=", "or_assign");
        operator_names->insert("^=", "xor_assign");
        operator_names->insert("<<=", "shift_left_assign");
        operator_names->insert(">>=", "shift_right_assign");

        // Logical
        operator_names->insert("&&", "logical_and");
        operator_names->insert("||", "logical_or");
        operator_names->insert("!", "not");

        // incr/decr
        operator_names->insert("++", "increment");
        operator_names->insert("--", "decrement");

        // compare
        operator_names->insert("<", "less");
        operator_names->insert(">", "greater");
        operator_names->insert("<=", "less_or_equal");
        operator_names->insert(">=", "greater_or_equal");
        operator_names->insert("<<=", "shift_left_assign");
        operator_names->insert(">>=", "shift_right_assign");
        operator_names->insert("!=", "not_equal");
        operator_names->insert("==", "equal");

        // other
        operator_names->insert("[]", "subscript");
        operator_names->insert("->", "pointer");
        operator_names->insert("()", "call");
        operator_names->insert("new", "cunstruct");
        operator_names->insert("delete", "destruct");
    }

    if (!operator_names->contains(op)) {
        TypeDatabase *tb = TypeDatabase::instance();

        TypeParser::Info typeInfo = TypeParser::parse(op);
        QString cast_to_name = typeInfo.qualified_name.join("::");
        TypeEntry *te = tb->findType(cast_to_name);
        if ((te && te->codeGeneration() == TypeEntry::GenerateNothing)
                || tb->isClassRejected(cast_to_name)) {
            return QString();
        } else if (te) {
            return "operator_cast_" + typeInfo.qualified_name.join("::");
        } else {
            TypeAliasModelItem typeAlias = currentScope()->findTypeAlias(cast_to_name);
            if(typeAlias && typeAlias->type().qualifiedName().join("::")!=cast_to_name){
                return rename_operator(typeAlias->type().qualifiedName().join("::"));
            }else{
                ReportHandler::warning(QString("unknown operator '%1'").arg(op));
                return "operator " + op;
            }
        }
    }

    return "operator_" + operator_names->value(op);
}

AbstractMetaBuilder::AbstractMetaBuilder()
        : m_current_class(nullptr) {
}

/**
 * This function is here, simply to print out warnings about
 * modifications not having the proper signature...
 */
void AbstractMetaBuilder::checkFunctionModifications() {
    TypeDatabase *types = TypeDatabase::instance();
    SingleTypeEntryHash entryHash = types->entries();
    QList<TypeEntry *> entries = entryHash.values();
    for(TypeEntry *entry : entries) {
        if (!entry)
            continue;
        if (!entry->isComplex() || entry->codeGeneration() == TypeEntry::GenerateNothing)
            continue;

        ComplexTypeEntry *centry = static_cast<ComplexTypeEntry *>(entry);

        const FunctionModificationList& functionModifications = centry->functionModifications();
        AbstractMetaClass *clazz = nullptr;
        for(const FunctionModification& modification : functionModifications) {
            const QString& signature = modification.signature;

            QString name = signature.trimmed();
            if(!name.isEmpty()){
                name = name.mid(0, signature.indexOf("("));

                if (!clazz){
                    clazz = m_meta_classes.findClass(centry->qualifiedCppName(), AbstractMetaClassList::QualifiedCppName);
                    if (!clazz)
                        break;
                }

                const AbstractMetaFunctionList& functions = clazz->functions();
                bool found = false;
                QStringList possibleSignatures;
                for(AbstractMetaFunction *function : functions) {
                    if (function->minimalSignature() == signature && function->implementingClass() == clazz) {
                        found = true;
                        break;
                    }

                    if (function->originalName() == name)
                        possibleSignatures.append(function->minimalSignature() + " in " + function->implementingClass()->name());
                }

                if (!found) {
                    QString warning
                    = QString("signature '%1' for function modification in '%2' not found. Possible candidates: %3")
                      .arg(signature)
                      .arg(clazz->qualifiedCppName())
                      .arg(possibleSignatures.join(",\n    "));

                    ReportHandler::warning(warning);
                }
            }
        }
    }
}

AbstractMetaClass *AbstractMetaBuilder::argumentToClass(ArgumentModelItem argument, const QString &contextString) {
    AbstractMetaClass *returned = nullptr;
    bool ok = false;
    AbstractMetaType *type = translateType(argument->type(), &ok, contextString);
    if (ok && type && type->typeEntry() && type->typeEntry()->isComplex()) {
        const TypeEntry *entry = type->typeEntry();
        returned = m_meta_classes.findClass(entry->name());
        if(!returned)
            returned = m_meta_classes.findClass(entry->qualifiedCppName());
    }
    delete type;
    return returned;
}

/**
 * Checks the argument of a hash function and flags the type if it is a complex type
 */
void AbstractMetaBuilder::registerHashFunction(FunctionModelItem function_item) {
    ArgumentList arguments = function_item->arguments();
    bool hasDefaultArgs = arguments.size() > 0;
    for(int i=1; i<arguments.size(); i++){
        hasDefaultArgs &= arguments.at(i)->defaultValue();
    }
    if (arguments.size() == 1 || hasDefaultArgs) {
        if (AbstractMetaClass *cls = argumentToClass(arguments.at(0), "AbstractMetaBuilder::registerHashFunction")) {
            QFileInfo info(function_item->fileName());
            cls->typeEntry()->addExtraInclude(Include(Include::IncludePath, info.fileName()));

            cls->setHasHashFunction(true);
        }
    }
}

/**
 * Check if a class has a debug stream operator that can be used as toString
 */
void AbstractMetaBuilder::registerToStringCapability(FunctionModelItem function_item) {
    ArgumentList arguments = function_item->arguments();
    if (arguments.size() == 2) {
        if (arguments.at(0)->type().toString() == "QDebug") {
            ArgumentModelItem arg = arguments.at(1);
            if (AbstractMetaClass *cls = argumentToClass(arg, "AbstractMetaBuilder::registerToStringCapability")) {
                if (arg->type().indirections().size() < 2) {
                    cls->setToStringCapability(function_item);
                }
            }
        }
    }
}

void AbstractMetaBuilder::traverseCompareOperator(FunctionModelItem item) {
    ArgumentList arguments = item->arguments();
    if (arguments.size() == 2 && item->accessPolicy() == CodeModel::Public) {
        AbstractMetaClass *comparer_class = argumentToClass(arguments.at(0), "AbstractMetaBuilder::traverseCompareOperator comparer_class");
        AbstractMetaClass *compared_class = argumentToClass(arguments.at(1), "AbstractMetaBuilder::traverseCompareOperator compared_class");
        if (comparer_class && compared_class) {
            AbstractMetaClass *old_current_class = m_current_class;
            m_current_class = comparer_class;

            AbstractMetaFunction *meta_function = traverseFunction(item);
            if (meta_function && !meta_function->isInvalid()) {
                // Strip away first argument, since that is the containing object
                AbstractMetaArgumentList arguments = meta_function->arguments();
                arguments.pop_front();
                meta_function->setArguments(arguments);

                meta_function->setFunctionType(AbstractMetaFunction::GlobalScopeFunction);

                meta_function->setOriginalAttributes(meta_function->attributes());
                setupFunctionDefaults(meta_function, comparer_class);

                FunctionModificationList mods = meta_function->modifications(comparer_class);
                for(const FunctionModification& mod : mods) {
                    if (mod.isRenameModifier())
                        meta_function->setName(mod.renamedTo());
                    if (mod.isAccessModifier()) {
                        if (mod.isPrivate())
                            meta_function->setVisibility(AbstractMetaAttributes::Private);
                        else if (mod.isProtected())
                            meta_function->setVisibility(AbstractMetaAttributes::Protected);
                        else if (mod.isPublic())
                            meta_function->setVisibility(AbstractMetaAttributes::Public);
                        else if (mod.isFriendly())
                            meta_function->setVisibility(AbstractMetaAttributes::Friendly);
                    }

                }

                comparer_class->addFunction(meta_function);
            } else if (meta_function) {
                delete meta_function;
            }

            m_current_class = old_current_class;
        }
    }
}

void AbstractMetaBuilder::traverseStreamOperator(FunctionModelItem item) {
    ArgumentList arguments = item->arguments();
    if (arguments.size() == 2 && item->accessPolicy() == CodeModel::Public) {
        AbstractMetaClass *streamClass = argumentToClass(arguments.at(0), "AbstractMetaBuilder::traverseStreamOperator streamClass");
        AbstractMetaClass *streamedClass = argumentToClass(arguments.at(1), "AbstractMetaBuilder::traverseStreamOperator streamedClass");

        if (streamClass && streamedClass && !streamedClass->isNamespace()
                && (streamClass->name() == "QDataStream" || streamClass->name() == "QTextStream")) {
            AbstractMetaClass *old_current_class = m_current_class;
            m_current_class = streamedClass;
            AbstractMetaFunction *streamFunction = traverseFunction(item);

            if (streamFunction && !streamFunction->isInvalid()) {
                QString name = item->name();
                streamFunction->setFunctionType(AbstractMetaFunction::GlobalScopeFunction);

                if (name.endsWith("<<"))
                    streamFunction->setName("writeTo");
                else
                    streamFunction->setName("readFrom");

                // Strip away last argument, since that is the containing object
                AbstractMetaArgumentList arguments = streamFunction->arguments();
                arguments.removeLast();
                streamFunction->setArguments(arguments);
                streamFunction->setOriginalAttributes(streamFunction->attributes());

                *streamFunction += AbstractMetaAttributes::Final;
                *streamFunction += AbstractMetaAttributes::Public;
                streamFunction->setType(nullptr);

                setupFunctionDefaults(streamFunction, streamedClass);

                streamedClass->addFunction(streamFunction);
                streamedClass->typeEntry()->addExtraInclude(streamClass->typeEntry()->include());

                m_current_class = old_current_class;
            }
        }
    }
}

void AbstractMetaBuilder::fixQObjectForScope(TypeDatabase *types,
        NamespaceModelItem scope) {
    for(QHash<QString, ClassModelItem>::const_iterator i = scope->classMap().begin(); i!=scope->classMap().end(); i++){
        QString qualified_name = i.value()->qualifiedName().join("::");
        TypeEntry *entry = types->findType(qualified_name);
        if (entry) {
            if (isQObject(qualified_name) && entry->isComplex()) {
                static_cast<ComplexTypeEntry *>(entry)->setQObject(true);
            }
        }
    }

    for(const NamespaceModelItem& item : scope->namespaceMap().values()) {
        if (scope != item)
            fixQObjectForScope(types, item);
    }
}

static bool class_less_than(AbstractMetaClass *a, AbstractMetaClass *b) {
    return a->name() < b->name();
}


void AbstractMetaBuilder::sortLists() {
    std::sort(m_meta_classes.begin(), m_meta_classes.end(), class_less_than);
    for(AbstractMetaClass *cls : m_meta_classes) {
        cls->sortFunctions();
    }
}

void writeContent(QTextStream &stream,
                  QString indent,
                  const QHash<QString, ClassModelItem>& classMap,
                  const QHash<QString, EnumModelItem>& enumMap,
                  const QHash<QString, TypeAliasModelItem>& typeAliasMap,
                  const QHash<QString, VariableModelItem>& variableMap,
                  const QMultiHash<QString, FunctionDefinitionModelItem>& functionDefinitionMap,
                  const QMultiHash<QString, FunctionModelItem>& functionMap,
                  const QHash<QString, NamespaceModelItem>& namespaceMap){
    {
        QMap<QString, ClassModelItem> _map;
        for(QHash<QString, ClassModelItem>::const_iterator i = classMap.begin(); i!=classMap.end(); i++){
            _map.insert(i.key(), i.value());
        }
        if(!_map.isEmpty()){
            stream << endl;
            stream << indent << "Classes:" << endl;
            for(QMap<QString, ClassModelItem>::const_iterator i = _map.begin(); i!=_map.end(); i++){
                stream << indent << "class " << i.key() << "{" << endl;
                writeContent(stream, indent+"    ", i.value()->classMap(), i.value()->enumMap(), i.value()->typeAliasMap(), i.value()->variableMap(), i.value()->functionDefinitionMap(), i.value()->functionMap(), QHash<QString, NamespaceModelItem>());
                stream << indent << "}" << endl;
            }
        }
    }
    {
        QMap<QString, EnumModelItem> _map;
        for(QHash<QString, EnumModelItem>::const_iterator i = enumMap.begin(); i!=enumMap.end(); i++){
            _map.insert(i.key(), i.value());
        }
        if(!_map.isEmpty()){
            stream << endl;
            stream << indent << "Enums:" << endl;
            for(QMap<QString, EnumModelItem>::const_iterator i = _map.begin(); i!=_map.end(); i++){
                stream << indent << i.key() << endl;
            }
        }
    }
    {
        QMap<QString, TypeAliasModelItem> _map;
        for(QHash<QString, TypeAliasModelItem>::const_iterator i = typeAliasMap.begin(); i!=typeAliasMap.end(); i++){
            _map.insert(i.key(), i.value());
        }
        if(!_map.isEmpty()){
            stream << endl;
            stream << indent << "Type Aliases:" << endl;
            for(QMap<QString, TypeAliasModelItem>::const_iterator i = _map.begin(); i!=_map.end(); i++){
                stream << indent << i.key() << endl;
            }
        }
    }
    {
        QMap<QString, VariableModelItem> _map;
        for(QHash<QString, VariableModelItem>::const_iterator i = variableMap.begin(); i!=variableMap.end(); i++){
            _map.insert(i.key(), i.value());
        }
        if(!_map.isEmpty()){
            stream << endl;
            stream << indent << "Variables:" << endl;
            for(QMap<QString, VariableModelItem>::const_iterator i = _map.begin(); i!=_map.end(); i++){
                stream << indent << i.key() << endl;
            }
        }
    }
    {
        QMap<QString, FunctionDefinitionModelItem> _map;
        for(QHash<QString, FunctionDefinitionModelItem>::const_iterator i = functionDefinitionMap.begin(); i!=functionDefinitionMap.end(); i++){
            _map.insert(i.key(), i.value());
        }
        if(!_map.isEmpty()){
            stream << endl;
            stream << indent << "Function Definitions:" << endl;
            for(QMap<QString, FunctionDefinitionModelItem>::const_iterator i = _map.begin(); i!=_map.end(); i++){
                stream << indent << i.key() << endl;
            }
        }
    }
    {
        QMap<QString, FunctionModelItem> _map;
        for(QHash<QString, FunctionModelItem>::const_iterator i = functionMap.begin(); i!=functionMap.end(); i++){
            _map.insert(i.key(), i.value());
        }
        if(!_map.isEmpty()){
            stream << endl;
            stream << indent << "Functions:" << endl;
            for(QMap<QString, FunctionModelItem>::const_iterator i = _map.begin(); i!=_map.end(); i++){
                stream << indent << i.key() << endl;
            }
        }
    }
    {
        QMap<QString, NamespaceModelItem> _map;
        for(QHash<QString, NamespaceModelItem>::const_iterator i = namespaceMap.begin(); i!=namespaceMap.end(); i++){
            _map.insert(i.key(), i.value());
        }
        if(!_map.isEmpty()){
            stream << endl;
            stream << indent << "Namespaces:" << endl;
            for(QMap<QString, NamespaceModelItem>::const_iterator i = _map.begin(); i!=_map.end(); i++){
                stream << indent << "namespace " << i.key() << "{" << endl;
                writeContent(stream, indent+"    ", i.value()->classMap(), i.value()->enumMap(), i.value()->typeAliasMap(), i.value()->variableMap(), i.value()->functionDefinitionMap(), i.value()->functionMap(), i.value()->namespaceMap());
                stream << indent << "}" << endl;
            }
        }
    }
}

bool AbstractMetaBuilder::build() {
    Q_ASSERT(!m_file_name.isEmpty());
    ReportHandler::setContext("Parser");

    QFile file(m_file_name);

    if (!file.open(QFile::ReadOnly))
        return false;

    QByteArray contents;
    {
        QTextStream stream(&file);
        stream.setCodec(QTextCodec::codecForName("UTF-8"));
        contents = stream.readAll().toUtf8();
    }
    file.close();

    Control control;
    Parser p(&control);
    pool __pool;

    TranslationUnitAST *ast = p.parse(contents, std::size_t(contents.size()), &__pool);
    for(const Control::ErrorMessage& message : control.errorMessages()){
        ReportHandler::warning(QString("%4 (in %1, line %2, column %3)").arg(message.fileName()).arg(QString::number(message.line())).arg(QString::number(message.column())).arg(message.message()));
    }

    CodeModel model;
    Binder binder(&model, p.location());
    m_dom = binder.run(ast);

    ReportHandler::setContext("MetaJavaBuilder");

    pushScope(model_dynamic_cast<ScopeModelItem>(m_dom));

#if 0
    {
        QFile file2("found-classes.log");
        if (file2.open(QFile::WriteOnly)){
            QTextStream stream(&file2);
            stream.setCodec(QTextCodec::codecForName("UTF-8"));
            writeContent(stream, "", m_dom->classMap(), m_dom->enumMap(), m_dom->typeAliasMap(), m_dom->variableMap(), m_dom->functionDefinitionMap(), m_dom->functionMap(), m_dom->namespaceMap());
            file2.close();
        }
    }
#endif

    // fix up QObject's in the type system..
    TypeDatabase *types = TypeDatabase::instance();
    fixQObjectForScope(types, model_dynamic_cast<NamespaceModelItem>(m_dom));
    {
        ClassModelItem const_iterator = m_dom->classMap().value("QVector__const_iterator");
        if(const_iterator){
            m_dom->removeClass(const_iterator);
            ClassModelItem qvec = m_dom->classMap().value("QVector");
            if(qvec){
                const_iterator->setName("const_iterator");
                qvec->addClass(const_iterator);
            }
        }
    }

    {
        QStringList classNames;
        classNames << "QMap<Key,T>"
                   << "QMap"
                   << "QMultiMap<Key,T>"
                   << "QMultiMap"
                   << "QHash<Key,T>"
                   << "QHash"
                   << "QMultiHash<Key,T>"
                   << "QMultiHash"
                   << "QSet<T>"
                   << "QSet";
        for(const QString& className : classNames) {
            if(ClassModelItem mapClass = m_dom->classMap().value(className)){
                for(const FunctionModelItem& fct : mapClass->functionMap().values("insert")){
                    TypeInfo type = fct->type();
                    if(type.qualifiedName().join("::")=="iterator"){
                        type.setQualifiedName(QStringList()<<"void");
                        fct->setType(type);
                    }
                }
            }
        }
    }

    // Start the generation...
    for(QHash<QString, ClassModelItem>::const_iterator i = m_dom->classMap().begin(); i!=m_dom->classMap().end(); i++){
        addAbstractMetaClass(traverseClass(i.value()));
    }

    for(QHash<QString, NamespaceModelItem>::const_iterator i = m_dom->namespaceMap().begin(); i!=m_dom->namespaceMap().end(); i++){
        AbstractMetaClass *meta_class = traverseNamespace(i.value());
        if (meta_class)
            m_meta_classes << meta_class;
    }

    // Some trickery to support global-namespace enums...
    m_current_class = nullptr;
    for(QHash<QString, EnumModelItem>::const_iterator i = m_dom->enumMap().begin(); i!=m_dom->enumMap().end(); i++){
        AbstractMetaEnum *meta_enum = traverseEnum(i.value(), nullptr, QSet<QString>());

        if (meta_enum) {
            QString package = meta_enum->typeEntry()->javaPackage();
            QString globalName = TypeDatabase::globalNamespaceClassName();

            AbstractMetaClass *global = m_meta_classes.findClass(package.isEmpty() ? globalName : package + "." + globalName);
            if (!global) {
                ComplexTypeEntry *gte = new ObjectTypeEntry(globalName);
                gte->setTargetLangPackage(meta_enum->typeEntry()->javaPackage());
                gte->setTargetTypeSystem(meta_enum->typeEntry()->targetTypeSystem());
                gte->setCodeGeneration(TypeEntry::GenerateNothing);
                global = createMetaClass();
                global->setTypeEntry(gte);
                *global += AbstractMetaAttributes::Final;
                *global += AbstractMetaAttributes::Public;
                *global += AbstractMetaAttributes::Fake;

                m_meta_classes << global;
            }

            global->addEnum(meta_enum);
            meta_enum->setEnclosingClass(global);
//            meta_enum->typeEntry()->setQualifier(globalName);
        }
    }

    // Go through all typedefs to see if we have defined any
    // specific typedefs to be used as classes.
    for(QHash<QString, TypeAliasModelItem>::const_iterator i = m_dom->typeAliasMap().begin(); i!=m_dom->typeAliasMap().end(); i++){
        if(i.value()->type().qualifiedName().join("::").startsWith("std::function<") || i.value()->type().isFunctionPointer()){
            AbstractMetaFunctional *fnl = traverseFunctional(i.value());
            if(fnl)
                addAbstractMetaFunctional(fnl);
        }else{
            AbstractMetaClass *cls = traverseTypeAlias(i.value());
            if(cls){
                addAbstractMetaClass(cls);
            }
        }
    }

    for(AbstractMetaClass *cls : m_meta_classes) {
        if (!cls->isInterface() && !cls->isNamespace()) {
            setupInheritance(cls);
        }else if(cls->isInterface()){
            setupInheritance(cls->extractInterfaceImpl());
        }
        setupTemplateInstantiations(cls);
    }

    fixMissingIterator();

    for(AbstractMetaClass *cls : m_meta_classes) {
        cls->fixFunctions([this](TypeEntry * etype) -> AbstractMetaArgument*{
            AbstractMetaType* type = this->createMetaType();
            type->setTypeEntry(etype);
            type->setIndirections(QList<bool>() << false);
            type->setTypeUsagePattern(AbstractMetaType::ObjectPattern);
            AbstractMetaArgument* argument = this->createMetaArgument();
            argument->setName("_this");
            argument->setType(type);
            return argument;
        });

        if (!cls->typeEntry()) {
            ReportHandler::warning(QString("class '%1' does not have an entry in the type system")
                                   .arg(cls->name()));
        } else {
            if (!cls->hasConstructors() && !cls->isFinalInCpp() && !cls->isInterface() && !cls->isNamespace())
                cls->addDefaultConstructor();
        }

        if (cls->isAbstract() && !cls->isInterface()) {
            cls->typeEntry()->setLookupName(cls->typeEntry()->targetLangName() + "$ConcreteWrapper");
        }
    }

    for(AbstractMetaClass *cls : m_meta_classes) {
        cls->fixUnimplmentablePureVirtualFunctions();
    }

    QList<TypeEntry *> entries = TypeDatabase::instance()->entries().values();
    for(const TypeEntry *entry : entries) {
        if (entry->isPrimitive())
            continue;

        if ((entry->isValue() || entry->isObject())
                && !entry->isString()
                && !entry->isStringRef()
                && !entry->isChar()
                && !entry->isContainer()
//                && !entry->isIterator()
                && !entry->isPointerContainer()
                && !entry->isInitializerList()
                && !entry->isQModelIndexType()
                && !entry->isQMetaEnumType()
                && !entry->isQMetaMethodType()
                && !entry->isQMetaObjectType()
                && !entry->isQMetaObjectConnectionType()
                && !entry->isQMetaPropertyType()
                && !entry->isVariant()
                && !m_meta_classes.findClass(entry->qualifiedCppName(), AbstractMetaClassList::QualifiedCppName)) {
            ReportHandler::warning(QString("type '%1' is specified in typesystem, "
                                           "but not defined. This could potentially "
                                           "lead to compilation errors.")
                                   .arg(entry->qualifiedCppName()));
        }

        if (entry->isEnum()) {
            QString pkg = entry->javaPackage();
            QString name = (pkg.isEmpty() ? QString() : pkg + ".")
                           + static_cast<const EnumTypeEntry *>(entry)->javaQualifier();
            AbstractMetaClass *cls = m_meta_classes.findClass(name);

            if(!cls && static_cast<const EnumTypeEntry *>(entry)->javaQualifier().isEmpty()){
                QString globalName = TypeDatabase::globalNamespaceClassName();
                cls = m_meta_classes.findClass(pkg + "." + globalName);
                if (!cls) {
                    ComplexTypeEntry *gte = new ObjectTypeEntry(globalName);
                    gte->setTargetTypeSystem(entry->javaPackage());
                    gte->setTargetLangPackage(entry->targetTypeSystem());
                    gte->setCodeGeneration(TypeEntry::GenerateNothing);
                    cls = createMetaClass();
                    cls->setTypeEntry(gte);
                    *cls += AbstractMetaAttributes::Final;
                    *cls += AbstractMetaAttributes::Public;
                    *cls += AbstractMetaAttributes::Fake;
                    m_meta_classes << cls;
                }
            }

            if (!cls) {
                ReportHandler::warning(QString("namespace '%1' for enum '%2' is not declared")
                                       .arg(name).arg(entry->targetLangName()));
            } else {
                AbstractMetaEnum *e = cls->findEnum(entry->targetLangName());
                if (!e)
                    ReportHandler::warning(QString("enum '%1' is specified in typesystem, "
                                                   "but not declared")
                                           .arg(entry->qualifiedCppName()));
            }
        }

        if (entry->isFunctional()) {
            QString pkg = entry->javaPackage();
            QString name = (pkg.isEmpty() ? QString() : pkg + ".")
                           + static_cast<const FunctionalTypeEntry *>(entry)->javaQualifier();
            AbstractMetaClass *cls = m_meta_classes.findClass(name);

            if(!cls && static_cast<const FunctionalTypeEntry *>(entry)->javaQualifier().isEmpty()){
                QString globalName = TypeDatabase::globalNamespaceClassName();
                name += globalName;
                cls = m_meta_classes.findClass(name);
                if (!cls) {
                    ComplexTypeEntry *gte = new ObjectTypeEntry(globalName);
                    gte->setTargetTypeSystem(entry->javaPackage());
                    gte->setTargetLangPackage(entry->targetTypeSystem());
                    gte->setCodeGeneration(TypeEntry::GenerateNothing);
                    cls = createMetaClass();
                    cls->setTypeEntry(gte);
                    *cls += AbstractMetaAttributes::Final;
                    *cls += AbstractMetaAttributes::Public;
                    *cls += AbstractMetaAttributes::Fake;
                    m_meta_classes << cls;
                }
            }

            if (!cls) {
                ReportHandler::warning(QString("namespace '%1' for functional '%2' is not declared")
                                       .arg(name).arg(entry->targetLangName()));
            } else {
                AbstractMetaFunctional *e = cls->findFunctional(entry->targetLangName());
                if (!e){
                    e = findFunctional(cls, static_cast<const FunctionalTypeEntry *>(entry));
                }
                if (!e)
                    ReportHandler::warning(QString("function '%1' is specified in typesystem, "
                                                   "but not declared")
                                           .arg(entry->qualifiedCppName()));
            }
        }
    }

    for(const FunctionModelItem& item : m_dom->findFunctions("qHash")) {
        registerHashFunction(item);
    }

    for(const FunctionModelItem& item : m_dom->findFunctions("operator<<")) {
        registerToStringCapability(item);
    }

    {
        FunctionList list;
        list << m_dom->findFunctions("operator==")
             << m_dom->findFunctions("operator<=")
             << m_dom->findFunctions("operator>=")
             << m_dom->findFunctions("operator<")
             << m_dom->findFunctions("operator>");
        for(const FunctionModelItem& item : list) {
            traverseCompareOperator(item);
        }
    }

    {
        FunctionList list;
        list << m_dom->findFunctions("operator<<")
             << m_dom->findFunctions("operator>>");
        for(const FunctionModelItem& item : list) {
            traverseStreamOperator(item);
        }
    }

    for(QHash<QString, NamespaceModelItem>::const_iterator i = m_dom->namespaceMap().begin(); i!=m_dom->namespaceMap().end(); i++){
        figureOutFunctionsInNamespace(i.value());
    }

    figureOutEnumValues();
    figureOutDefaultEnumArguments();
    checkFunctionModifications();

    for(AbstractMetaClass *cls : m_meta_classes) {
        setupEquals(cls);
        setupComparable(cls);
        setupClonable(cls);
        setupBeginEnd(cls);
    }

    dumpLog();

    sortLists();

    return true;
}

AbstractMetaFunctional * AbstractMetaBuilder::findFunctional(AbstractMetaClass *cls, const FunctionalTypeEntry * fentry)
{
    AbstractMetaFunctional *e = nullptr;
    const QString& _using = fentry->getUsing();
    if(!_using.isEmpty()){
        FunctionalTypeEntry *ftype = const_cast<FunctionalTypeEntry *>(fentry);
        if(ftype){
            if(_using.startsWith("std::function<") && _using.endsWith(">")){
                QString normalizedSignature = "std::function<";
                QString newUsing = "std::function<";
                QScopedPointer<AbstractMetaFunctional> meta_functional;
                meta_functional.reset(createMetaFunctional());
                meta_functional->setTypeEntry(ftype);
                meta_functional->setBaseTypeName(fentry->targetLangName());
                bool ok = false;
                QString functionalReturnType = _using.mid(14).chopped(1);
                QString functionalArguments = functionalReturnType;
                int idx = functionalReturnType.indexOf("(");
                functionalReturnType = functionalReturnType.mid(0, idx).trimmed();
                functionalArguments = functionalArguments.mid(idx+1).chopped(1).trimmed();
                meta_functional->setFunctionPointer(false);
                AbstractMetaType * type = translateType(analyzeTypeInfo(cls, functionalReturnType), &ok, QString("traverseFunctional %1").arg(fentry->name()));
                if(ok){
                    newUsing += type ? type->minimalSignature() : "void";
                    normalizedSignature += type ? type->normalizedSignature() : "void";
                    meta_functional->setType(type);
                    int counter = 0;
                    newUsing += "(";
                    normalizedSignature += "(";
                    for(const QString& arg : functionalArguments.split(",")){
                        AbstractMetaType * atype = translateType(analyzeTypeInfo(cls, arg), &ok, QString("traverseFunctional %1").arg(fentry->name()));
                        if(ok){
                            AbstractMetaArgument* argument = createMetaArgument();
                            argument->setArgumentIndex(counter);
                            argument->setType(atype);
                            for(const ArgumentModification& mod : ftype->argumentModification()){
                                if(mod.index==counter+1 && !mod.modified_name.isEmpty()){
                                    argument->setName(mod.modified_name);
                                }
                            }
                            meta_functional->addArgument(argument);
                            if(counter>0){
                                newUsing += ",";
                                normalizedSignature += ",";
                            }
                            newUsing += atype->minimalSignature();
                            normalizedSignature += atype->normalizedSignature();
                            ++counter;
                        }
                    }
                    newUsing += ")";
                    normalizedSignature += ")";
                }
                newUsing += ">";
                normalizedSignature += ">";
                e = meta_functional.take();
                cls->addFunctional(e);
                cls->addFunctionalByUsing(_using, e);
                cls->addFunctionalByUsing(QMetaObject::normalizedSignature(qPrintable(_using)), e);
                cls->addFunctionalByUsing(newUsing, e);
                cls->addFunctionalByUsing(QMetaObject::normalizedSignature(qPrintable(newUsing)), e);
                cls->addFunctionalByUsing(normalizedSignature, e);
                ftype->setUsing(newUsing);
                ftype->setNormalizedSignature(normalizedSignature);
            }
        }
    }
    return e;
}

TypeInfo AbstractMetaBuilder::analyzeTypeInfo(AbstractMetaClass *cls, QString strg){
    TypeInfo result;
    if(strg.startsWith("const ")){
        result.setConstant(true);
        strg = strg.mid(6);
    }
    if(strg.endsWith("&&")){
        result.setReferenceType(TypeInfo::RReference);
        strg.chop(2);
    }
    if(strg.endsWith("&")){
        result.setReferenceType(TypeInfo::Reference);
        strg.chop(1);
    }
    strg = strg.trimmed();
    if(cls->findEnum(strg)){
        result.setQualifiedName(cls->qualifiedCppName().split("::") << strg.split("::"));
    }else if(m_meta_classes.findClass(cls->qualifiedCppName()+"::"+strg)){
        result.setQualifiedName(cls->qualifiedCppName().split("::") << strg.split("::"));
    }else if(m_meta_classes.findClass(strg)){
        result.setQualifiedName(strg.split("::"));
    }else{
        result.setQualifiedName(strg.split("::"));
    }
    return result;
}

void AbstractMetaBuilder::figureOutFunctionsInNamespace(const NamespaceModelItem &item){
    {
        FunctionList hash_functions = item->findFunctions("qHash");
        for(const FunctionModelItem& item : hash_functions) {
            registerHashFunction(item);
        }
    }

    {
        FunctionList hash_functions = item->findFunctions("operator<<");
        for(const FunctionModelItem& item : hash_functions) {
            registerToStringCapability(item);
        }
    }

    {
        FunctionList compare_operators = item->findFunctions("operator==")
                                         + item->findFunctions("operator<=")
                                         + item->findFunctions("operator>=")
                                         + item->findFunctions("operator<")
                                         + item->findFunctions("operator>");
        for(const FunctionModelItem& item : compare_operators) {
            traverseCompareOperator(item);
        }
    }

    {
        FunctionList stream_operators = item->findFunctions("operator<<") + item->findFunctions("operator>>");
        for(const FunctionModelItem& item : stream_operators) {
            traverseStreamOperator(item);
        }
    }

    for(QHash<QString, NamespaceModelItem>::const_iterator i = item->namespaceMap().begin(); i!=item->namespaceMap().end(); i++){
        figureOutFunctionsInNamespace(i.value());
    }
}


void AbstractMetaBuilder::addAbstractMetaFunctional(AbstractMetaFunctional *cls) {
    if (!cls)
        return;
    cls->setOriginalAttributes(cls->attributes());
    m_meta_functionals << cls;
    QString pkg = cls->typeEntry()->javaPackage();
    QString globalName = TypeDatabase::globalNamespaceClassName();
    AbstractMetaClass *_cls = m_meta_classes.findClass(pkg + "." + globalName);
    if(!_cls){
        ComplexTypeEntry *gte = new ObjectTypeEntry(globalName);
        gte->setTargetLangPackage(cls->typeEntry()->javaPackage());
        gte->setTargetTypeSystem(cls->typeEntry()->targetTypeSystem());
        gte->setCodeGeneration(TypeEntry::GenerateNothing);
        _cls = createMetaClass();
        _cls->setTypeEntry(gte);
        *_cls += AbstractMetaAttributes::Final;
        *_cls += AbstractMetaAttributes::Public;
        *_cls += AbstractMetaAttributes::Fake;
        m_meta_classes << _cls;
    }
    _cls->addFunctional(cls);
}


void AbstractMetaBuilder::addAbstractMetaClass(AbstractMetaClass *cls) {
    if (!cls)
        return;

    cls->setOriginalAttributes(cls->attributes());
    if (cls->typeEntry()->isContainer()) {
        m_templates << cls;
        ReportHandler::debugSparse(" -> container "+cls->qualifiedCppName());
    } else {
        if(cls->typeEntry()->isIterator()){
            const IteratorTypeEntry* entry = static_cast<const IteratorTypeEntry*>(cls->typeEntry());
            if(entry->containerType() && entry->containerType()->isContainer()){
                m_template_iterators << cls;
                ReportHandler::debugSparse(" -> iterator "+cls->qualifiedCppName());
                return;
            }
        }
        m_meta_classes << cls;
        if(cls->isInterface()){
            AbstractMetaClass *interfaceImpl = cls->extractInterfaceImpl();
            m_meta_classes << interfaceImpl;
            ReportHandler::debugSparse(QString(" -> interface implementation class '%1'").arg(interfaceImpl->name()));
        }
        /*
        if (cls->typeEntry()->designatedInterface()) {
            AbstractMetaClass *interface = cls->extractInterfaceImpl();
            m_meta_classes << interface;
            ReportHandler::debugSparse(QString(" -> interface '%1'").arg(interface->name()));
        }
         */
    }
}


AbstractMetaClass *AbstractMetaBuilder::traverseNamespace(NamespaceModelItem namespace_item) {
    QString namespace_name = (!m_namespace_prefix.isEmpty() ? m_namespace_prefix + "::" : QString()) + namespace_item->name();

    NamespaceTypeEntry *type = TypeDatabase::instance()->findNamespaceType(namespace_name);

    if (TypeDatabase::instance()->isClassRejected(namespace_name)) {
        m_rejected_classes.insert({namespace_name, namespace_item->fileName()}, GenerationDisabled);
        return nullptr;
    }

    if (!type) {
        ReportHandler::warning(QString("namespace '%1' does not have a type entry")
                               .arg(namespace_name));
        return nullptr;
    }

    AbstractMetaClass *meta_class = createMetaClass();
    meta_class->setTypeEntry(type);

    *meta_class += AbstractMetaAttributes::Public;

    m_current_class = meta_class;

    ReportHandler::debugSparse(QString("namespace '%1.%2'")
                               .arg(meta_class->package())
                               .arg(namespace_item->name()));

    traverseEnums(model_dynamic_cast<ScopeModelItem>(namespace_item), meta_class, namespace_item->enumsDeclarations());
    traverseFunctions(model_dynamic_cast<ScopeModelItem>(namespace_item), meta_class);
//     traverseClasses(model_dynamic_cast<ScopeModelItem>(namespace_item));

    pushScope(model_dynamic_cast<ScopeModelItem>(namespace_item));
    m_namespace_prefix = currentScope()->qualifiedName().join("::");

    for(QHash<QString, ClassModelItem>::const_iterator i = namespace_item->classMap().begin(); i!=namespace_item->classMap().end(); i++){
        AbstractMetaClass *mjc = traverseClass(i.value());
        // the classes inside of a namespace are realized as static member classes
        // of the namespace representing java interface.
        if (mjc) {
            bool isEnclosedClass = mjc->typeEntry()->targetLangName().startsWith(meta_class->typeEntry()->targetLangName()+"$");
            if(isEnclosedClass){
                meta_class->addEnclosedClass(mjc);
            }
            m_meta_classes << mjc;
            if(mjc->isInterface()){
                AbstractMetaClass *interfaceImpl = mjc->extractInterfaceImpl();
                m_meta_classes << interfaceImpl;
                ReportHandler::debugSparse(QString(" -> interface implementation class '%1'").arg(interfaceImpl->name()));
            }
            /*
            if (mjc->typeEntry()->designatedInterface()) {
                AbstractMetaClass *interface = mjc->extractInterface();
                if(isEnclosedClass){
                    meta_class->addEnclosedClass(interface);
                }
                m_meta_classes << interface;
                ReportHandler::debugSparse(QString(" -> interface '%1'").arg(interface->name()));
            }
            */
        }
    }

    // Go through all typedefs to see if we have defined any
    // specific typedefs to be used as classes.
    for(QHash<QString, TypeAliasModelItem>::const_iterator i = namespace_item->typeAliasMap().begin(); i!=namespace_item->typeAliasMap().end(); i++){
        if(i.value()->type().qualifiedName().join("::").startsWith("std::function<") || i.value()->type().isFunctionPointer()){
            AbstractMetaFunctional *fnl = traverseFunctional(i.value());
            if(fnl)
                meta_class->addFunctional(fnl);
        }else{
            AbstractMetaClass *cls = traverseTypeAlias(i.value());
            // the classes inside of a namespace are realized as static member classes
            // of the namespace representing java interface.
            if (cls) {
                bool isEnclosedClass = cls->typeEntry()->targetLangName().startsWith(meta_class->typeEntry()->targetLangName()+"$");
                if(isEnclosedClass){
                    meta_class->addEnclosedClass(cls);
                }
                m_meta_classes << cls;
                if(cls->isInterface()){
                    AbstractMetaClass *interfaceImpl = cls->extractInterfaceImpl();
                    m_meta_classes << interfaceImpl;
                    ReportHandler::debugSparse(QString(" -> interface implementation class '%1'").arg(interfaceImpl->name()));
                }
            }
        }
    }


    // Traverse namespaces recursively
    for(QHash<QString, NamespaceModelItem>::const_iterator i = namespace_item->namespaceMap().begin(); i!=namespace_item->namespaceMap().end(); i++){
        AbstractMetaClass *mjc = traverseNamespace(i.value());
        // the namespace inside of a namespace are realized as static member interfaces
        // of the namespace representing java interface.
        if (mjc) {
            bool isEnclosedClass = mjc->typeEntry()->targetLangName().startsWith(meta_class->typeEntry()->targetLangName()+"$");
            if(isEnclosedClass){
                meta_class->addEnclosedClass(mjc);
            }
            m_meta_classes << mjc;
        }
    }

    m_current_class = nullptr;


    popScope();
    m_namespace_prefix = currentScope()->qualifiedName().join("::");

    if (!type->include().isValid()) {
        QFileInfo info(namespace_item->fileName());
        QMap<QString,QString> requiredFeatures;
        for(const QString& feature : namespace_item->requiredFeatures()){
            if(m_features && m_features->contains(feature)){
                QFileInfo _info(m_features->value(feature));
                requiredFeatures[feature] = _info.fileName();
            }else{
                requiredFeatures[feature] = "";
            }
        }
        type->setInclude(Include(Include::IncludePath, info.fileName(), requiredFeatures));
    }
    if(!namespace_item->requiredFeatures().isEmpty()){
        QStringList ppConditions;
        if(!type->ppCondition().isEmpty()){
            ppConditions << type->ppCondition();
        }
        for(const QString& feature : namespace_item->requiredFeatures()){
            ppConditions << QString("QT_CONFIG(%1)").arg(feature);
        }
        type->setPPCondition(ppConditions.join(" && "));
    }

    return meta_class;
}

QVariant Operator::calculate(QVariant x) {
    QVariant result;
    switch (type) {
        case Plus:
        if(value.isValid()){
            switch(size){
            case 8:
                result.setValue<qint8>(x.value<qint8>() + value.value<qint8>());
                break;
            case 16:
                result.setValue<qint16>(x.value<qint16>() + value.value<qint16>());
                break;
            case 32:
                result.setValue<qint32>(x.value<qint32>() + value.value<qint32>());
                break;
            case 64:
                result.setValue<qint64>(x.value<qint64>() + value.value<qint64>());
                break;
            default:
                result.setValue<qint32>(x.value<qint32>() + value.value<qint32>());
                break;
            }
        }else{
            result = x;
        }
        break;
        case Minus:
        if(value.isValid()){
            switch(size){
            case 8:
                result.setValue<qint8>(x.value<qint8>() - value.value<qint8>());
                break;
            case 16:
                result.setValue<qint16>(x.value<qint16>() - value.value<qint16>());
                break;
            case 32:
                result.setValue<qint32>(x.value<qint32>() - value.value<qint32>());
                break;
            case 64:
                result.setValue<qint64>(x.value<qint64>() - value.value<qint64>());
                break;
            default:
                result.setValue<qint32>(x.value<qint32>() - value.value<qint32>());
                break;
            }
        }else{
            switch(size){
            case 8:
                result.setValue<qint8>(-x.value<qint8>());
                break;
            case 16:
                result.setValue<qint16>(-x.value<qint16>());
                break;
            case 32:
                result.setValue<qint32>(-x.value<qint32>());
                break;
            case 64:
                result.setValue<qint64>(-x.value<qint64>());
                break;
            default:
                result.setValue<qint32>(-x.value<qint32>());
                break;
            }
        }
        break;
        case ShiftLeft:
        switch(size){
        case 8:
            result.setValue<qint8>(qint8(x.value<qint8>() << value.value<qint8>()));
            break;
        case 16:
            result.setValue<qint16>(qint16(x.value<qint16>() << value.value<qint16>()));
            break;
        case 32:
            result.setValue<qint32>(x.value<qint32>() << value.value<qint32>());
            break;
        case 64:
            result.setValue<qint64>(x.value<qint64>() << value.value<qint64>());
            break;
        default:
            result.setValue<qint32>(x.value<qint32>() << value.value<qint32>());
            break;
        }
        break;
        case Not:
        switch(size){
        case 8:
            result.setValue<qint8>(~x.value<qint8>());
            break;
        case 16:
            result.setValue<qint16>(~x.value<qint16>());
            break;
        case 32:
            result.setValue<qint32>(~x.value<qint32>());
            break;
        case 64:
            result.setValue<qint64>(~x.value<qint64>());
            break;
        default:
            result.setValue<qint32>(~x.value<qint32>());
            break;
        }
        break;
        case None: result = x;
        break;
    }
    return result;
}

QString Operator::toString(QString x) {
    QString result;
    switch (type) {
        case Plus:
        if(value.type()==QVariant::String){
            result = "(" + x + "+" + value.toString() + ")";
        }else if(value.isValid()){
            switch(size){
            case 8:
                result = "(" + x + "+" + QString::number(value.value<qint8>()) + ")";
                break;
            case 16:
                result = "(" + x + "+" + QString::number(value.value<qint16>()) + ")";
                break;
            case 32:
                result = "(" + x + "+" + QString::number(value.value<qint32>()) + ")";
                break;
            case 64:
                result = "(" + x + "+" + QString::number(value.value<qint64>()) + ")";
                break;
            default:
                result = "(" + x + "+" + QString::number(value.value<qint32>()) + ")";
                break;
            }
        }else{
            result = x;
        }
        break;
        case Minus:
        if(value.type()==QVariant::String){
            result = "(" + x + "-" + value.toString() + ")";
        }else if(value.isValid()){
            switch(size){
            case 8:
                result = "(" + x + "-" + QString::number(value.value<qint8>()) + ")";
                break;
            case 16:
                result = "(" + x + "-" + QString::number(value.value<qint16>()) + ")";
                break;
            case 32:
                result = "(" + x + "-" + QString::number(value.value<qint32>()) + ")";
                break;
            case 64:
                result = "(" + x + "-" + QString::number(value.value<qint64>()) + ")";
                break;
            default:
                result = "(" + x + "-" + QString::number(value.value<qint32>()) + ")";
                break;
            }
        }else{
            result = "-" + x;
        }
        break;
        case ShiftLeft:
        if(value.type()==QVariant::String){
            result = "(" + x + " << " + value.toString() + ")";
        }else{
            switch(size){
            case 8:
                result = "(" + x + " << " + QString::number(value.value<qint8>()) + ")";
                break;
            case 16:
                result = "(" + x + " << " + QString::number(value.value<qint16>()) + ")";
                break;
            case 32:
                result = "(" + x + " << " + QString::number(value.value<qint32>()) + ")";
                break;
            case 64:
                result = "(" + x + " << " + QString::number(value.value<qint64>()) + ")";
                break;
            default:
                result = "(" + x + " << " + QString::number(value.value<qint32>()) + ")";
                break;
            }
        }
        break;
        case Not:
            result = "~" + x;
        break;
        case None: result = x;
        break;
    }
    return result;
}


Operator AbstractMetaBuilder::findOperator(const uint size, QString *s,
                                           AbstractMetaClass *global,
                                           AbstractMetaEnum *meta_enum,
                                           AbstractMetaFunction *meta_function,
                                           QSet<QString> *warnings) {
    const char *names[] = {
        "+",
        "-",
        "<<",
        "~"
    };

    for (int i = 0; i < Operator::None; ++i) {
        QString name = QLatin1String(names[i]);
        QString str = *s;
        int splitPoint = str.indexOf(name);
        if (splitPoint > 0) {
            QString right = str.mid(splitPoint + name.length());
            Operator op(size);
            op.value = figureOutEnumValue(size,
                                right,
                                QVariant::fromValue<QString>(right),
                                global, meta_enum, meta_function, warnings);
            if (op.value.isValid()) {
                op.type = Operator::Type(i);
                *s = str.left(splitPoint).trimmed();
                return op;
            }
        }else if (splitPoint == 0 && name!="<<") {
            QString number = str.mid(splitPoint + name.length()).trimmed();
            bool ok = false;
            if(name!="-"){
                // don't apply unary minus to numbers. They are parsed as negative numbers.
                if(number.endsWith("L")){
                    number = number.left(number.length()-1);
                }
                if(number.size()>1 && number.startsWith("0")){
                    number.toLongLong(&ok, 0);
                }else{
                    number.toLongLong(&ok);
                }
            }
            if(!ok){
                Operator op(size);
                op.type = Operator::Type(i);
                *s = str.mid(splitPoint + name.length()).trimmed();
                return op;
            }else{
                break;
            }
        }
    }
    return Operator(size);
}

QVariant AbstractMetaBuilder::figureOutEnumValue(
        const uint size,
        const QString &origStringValue,
        QVariant oldValuevalue,
        AbstractMetaClass *global,
        AbstractMetaEnum *meta_enum,
        AbstractMetaFunction *meta_function,
        QSet<QString> *warnings) {
    if (origStringValue.isEmpty())
        return oldValuevalue;

    QString stringValue(origStringValue.trimmed());
    bool negate = false;
    bool _not = false;
    if(stringValue.startsWith("~")){
        stringValue = stringValue.mid(1);
        _not = true;
    }else if(stringValue.startsWith("-")){
        stringValue = stringValue.mid(1);
        negate = true;
    }
    while(stringValue.startsWith("(") && stringValue.endsWith(")")){
        stringValue = stringValue.mid(1, stringValue.length()-2);
    }

    // This block deals with "static_cast<FooBar::Type>" prefix on cpp defaulted values
    const QString keyword_static_cast("static_cast");
    if (stringValue.startsWith(keyword_static_cast)) {
        stringValue = stringValue.remove(0, keyword_static_cast.length()).trimmed();
        if (stringValue.length() > 0 && stringValue.at(0) == QChar('<')) {
            int end_pos = stringValue.indexOf(QChar('>'));
            if (end_pos >= 0)	// remove the whole "<FooBar::Type>"
                stringValue = stringValue.remove(0, end_pos).trimmed();
        }
    }
    // This block deals with "FooBar::Type(.....)" around the part we really want "....."
    // or with "(type)....."
    {
        int beg_pos = stringValue.indexOf(QChar('('));
        if (beg_pos >= 0) {
            QString tmp_stringValue = stringValue;
            tmp_stringValue = tmp_stringValue.remove(0, beg_pos+1).trimmed();	// remove "FooBar::Type("
            if (beg_pos == 0) { // was "(type)....."
                int nextClose_pos = tmp_stringValue.indexOf(QChar(')'));
                if(nextClose_pos>0){
                    tmp_stringValue = tmp_stringValue.remove(0, nextClose_pos+1).trimmed();	// remove "type)"
                }
            }

            int nextClose_pos = tmp_stringValue.indexOf(QChar(')'));
            int nextOpen_pos = tmp_stringValue.indexOf(QChar('('));
            if(nextOpen_pos<0 || nextOpen_pos<nextClose_pos){
                int end_pos = tmp_stringValue.lastIndexOf(QChar(')'));
                if (end_pos >= 0)
                    tmp_stringValue = tmp_stringValue.remove(end_pos, 1).trimmed();	// remove ")"
                stringValue = tmp_stringValue;
                if(stringValue.trimmed().isEmpty()){
                    return 0;
                }
            }
        }
    }

    QStringList stringValues = stringValue.split("|");

    QVariantList disjointReturnValues;
    QStringList disjointReturnValueStrings;
    bool hasValidNumberVariants = true;

    for (int i = 0; i < stringValues.size(); ++i) {
        QString s = stringValues.at(i).trimmed();

        while(s.startsWith("(") && s.endsWith(")")){
            s = s.mid(1, s.length()-2);
        }

        // This block deals with "static_cast<FooBar::Type>" prefix on cpp defaulted values
        if (s.startsWith(keyword_static_cast)) {
            s = s.remove(0, keyword_static_cast.length()).trimmed();
            if (s.length() > 0 && s.at(0) == QChar('<')) {
                int end_pos = s.indexOf(QChar('>'));
                if (end_pos >= 0)	// remove the whole "<FooBar::Type>"
                    s = s.remove(0, end_pos).trimmed();
            }
        }
        // This block deals with "FooBar::Type(.....)" around the part we really want "....."
        // or with "(type)....."
        {
            int beg_pos = s.indexOf(QChar('('));
            if (beg_pos >= 0) {
                QString tmp_string = s;
                tmp_string = tmp_string.remove(0, beg_pos+1).trimmed();	// remove "FooBar::Type("
                if (beg_pos == 0) { // was "(type)....."
                    int nextClose_pos = tmp_string.indexOf(QChar(')'));
                    if(nextClose_pos>0){
                        tmp_string = tmp_string.remove(0, nextClose_pos+1).trimmed();	// remove "type)"
                    }
                }

                int nextClose_pos = tmp_string.indexOf(QChar(')'));
                int nextOpen_pos = tmp_string.indexOf(QChar('('));
                if(nextOpen_pos<0 || nextOpen_pos<nextClose_pos){
                    int end_pos = tmp_string.lastIndexOf(QChar(')'));
                    if (end_pos >= 0)
                        tmp_string = tmp_string.remove(end_pos, 1).trimmed();	// remove ")"
                    s = tmp_string;
                }
            }
        }

        {
            int beg_pos = s.indexOf(QChar('('));
            if (beg_pos >= 0) {
                QString tmp_stringValue = s;
                tmp_stringValue = tmp_stringValue.remove(0, beg_pos+1).trimmed();	// remove "("

                int nectClose_pos = tmp_stringValue.indexOf(QChar(')'));
                int nextOpen_pos = tmp_stringValue.indexOf(QChar('('));
                if(nextOpen_pos<0 || nextOpen_pos<nectClose_pos){
                    int end_pos = tmp_stringValue.lastIndexOf(QChar(')'));
                    if (end_pos >= 0)
                        tmp_stringValue = tmp_stringValue.remove(end_pos, 1).trimmed();	// remove ")"
                    s = tmp_stringValue;
                }
            }
        }

        bool ok = false;
        QVariant v;

        Operator op = findOperator(size, &s, global, meta_enum, meta_function, warnings);

        if(s.length()==3 && s.startsWith("'") && s.endsWith("'")){
            QChar chr = s[1];
            switch(size){
            case 8:
                v.setValue<qint8>(qint8(quint8(chr.toLatin1())));
                break;
            case 16:
                v.setValue<qint16>(qint16(quint16(chr.toLatin1())));
                break;
            case 32:
                v.setValue<qint32>(qint32(quint32(chr.toLatin1())));
                break;
            case 64:
                v.setValue<qint64>(qint64(quint64(chr.toLatin1())));
                break;
            default:
                v.setValue<qint32>(qint32(quint32(chr.toLatin1())));
                break;
            }
            ok = true;
        }else if(s.length()>3 && s.startsWith("'\\u") && s.endsWith("'")){
            QString unicode = "0x"+s.mid(3, s.length()-2);
            switch(size){
            case 8:
                v.setValue<qint8>(qint8(quint8(unicode.toULongLong(&ok, 16))));
                break;
            case 16:
                v.setValue<qint16>(qint16(quint16(unicode.toULongLong(&ok, 16))));
                break;
            case 32:
                v.setValue<qint32>(qint32(quint32(unicode.toULongLong(&ok, 16))));
                break;
            case 64:
                v.setValue<qint64>(qint64(quint64(unicode.toULongLong(&ok, 16))));
                break;
            default:
                v.setValue<qint32>(qint32(quint32(unicode.toULongLong(&ok, 16))));
                break;
            }
        }else if(s.length()==4 && s.startsWith("'\\") && s.endsWith("'")){
            QChar chr = s[2];
            switch(size){
            case 8:
                v.setValue<qint8>(qint8(quint8(chr.toLatin1())));
                break;
            case 16:
                v.setValue<qint16>(qint16(quint16(chr.toLatin1())));
                break;
            case 32:
                v.setValue<qint32>(qint32(quint32(chr.toLatin1())));
                break;
            case 64:
                v.setValue<qint64>(qint64(quint64(chr.toLatin1())));
                break;
            default:
                v.setValue<qint32>(qint32(quint32(chr.toLatin1())));
                break;
            }
            ok = true;
        }else if (s.length() > 0 && s.at(0) == QLatin1Char('0')
                  && s.at(s.length()-1).toUpper() != QLatin1Char('U')
                  && s.at(s.length()-1).toUpper() != QLatin1Char('L')){
            switch(size){
            case 8:
                v.setValue<qint8>(qint8(quint8(s.toUShort(&ok, 0))));
                break;
            case 16:
                v.setValue<qint16>(qint16(s.toUShort(&ok, 0)));
                break;
            case 32:
                v.setValue<qint32>(qint32(s.toUInt(&ok, 0)));
                break;
            case 64:
                v.setValue<qint64>(qint64(s.toULongLong(&ok, 0)));
                break;
            default:
                v.setValue<qint32>(qint32(s.toUInt(&ok, 0)));
                break;
            }
        }else if (s.length() > 0 && s.at(s.length()-1).toUpper() == QLatin1Char('U')){
            QString unsignedNumber = s.left(s.length()-1);
            if(unsignedNumber.startsWith("0x")){
                switch(size){
                case 8:
                    v.setValue<qint8>(qint8(unsignedNumber.toULongLong(&ok, 16)));
                    break;
                case 16:
                    v.setValue<qint16>(qint16(unsignedNumber.toULongLong(&ok, 16)));
                    break;
                case 32:
                    v.setValue<qint32>(qint32(unsignedNumber.toULongLong(&ok, 16)));
                    break;
                case 64:
                    v.setValue<qint64>(qint64(unsignedNumber.toULongLong(&ok, 16)));
                    break;
                default:
                    v.setValue<qint32>(qint32(unsignedNumber.toULongLong(&ok, 16)));
                    break;
                }
            }else{
                switch(size){
                case 8:
                    v.setValue<qint8>(qint8(unsignedNumber.toULongLong(&ok)));
                    break;
                case 16:
                    v.setValue<qint16>(qint16(unsignedNumber.toULongLong(&ok)));
                    break;
                case 32:
                    v.setValue<qint32>(qint32(unsignedNumber.toULongLong(&ok)));
                    break;
                case 64:
                    v.setValue<qint64>(qint64(unsignedNumber.toULongLong(&ok)));
                    break;
                default:
                    v.setValue<qint32>(qint32(unsignedNumber.toULongLong(&ok)));
                    break;
                }
            }
        }else if (s.length() > 1
                  && s.at(s.length()-2).toUpper() == QLatin1Char('u')
                  && s.at(s.length()-1).toUpper() == QLatin1Char('L')){
            QString longNumber = s.left(s.length()-2);
            switch(size){
            case 8:
                v.setValue<qint8>(qint8(longNumber.toULongLong(&ok)));
                break;
            case 16:
                v.setValue<qint16>(qint16(longNumber.toULongLong(&ok)));
                break;
            case 32:
                v.setValue<qint32>(qint32(longNumber.toULongLong(&ok)));
                break;
            case 64:
                v.setValue<qint64>(qint64(longNumber.toULongLong(&ok)));
                break;
            default:
                v.setValue<qint32>(qint32(longNumber.toULongLong(&ok)));
                break;
            }
        }else if (s.length() > 0 && s.at(s.length()-1).toUpper() == QLatin1Char('L')){
            QString longNumber = s.left(s.length()-1);
            switch(size){
            case 8:
                v.setValue<qint8>(qint8(longNumber.toLongLong(&ok)));
                break;
            case 16:
                v.setValue<qint16>(qint16(longNumber.toLongLong(&ok)));
                break;
            case 32:
                v.setValue<qint32>(qint32(longNumber.toLongLong(&ok)));
                break;
            case 64:
                v.setValue<qint64>(qint64(longNumber.toLongLong(&ok)));
                break;
            default:
                v.setValue<qint32>(qint32(longNumber.toLongLong(&ok)));
                break;
            }
        } else {
            switch(size){
            case 8:
                v.setValue<qint8>(qint8(s.toShort(&ok)));
                break;
            case 16:
                v.setValue<qint16>(qint16(s.toShort(&ok)));
                break;
            case 32:
                v.setValue<qint32>(qint32(s.toInt(&ok)));
                break;
            case 64:
                v.setValue<qint64>(qint64(s.toLongLong(&ok)));
                break;
            default:
                v.setValue<qint32>(qint32(s.toInt(&ok)));
                break;
            }
        }

        AbstractMetaEnumValue *ev = nullptr;
        if (!ok) {
            v = QVariant();
            if (m_enum_values.contains(s)) {
                ev = m_enum_values[s];
                // if target value has not yet been computed do it
                if(!ev->value().isValid()){
                    AbstractMetaClass * ownerType = nullptr;
                    QString ownerTypeName;
                    bool found = false;
                    if (s.contains("::")) {
                        ownerTypeName = s.left(s.lastIndexOf("::"));
                        AbstractMetaClass * ownerType = classes().findClass(ownerTypeName);
                        if(!ownerType){
                            if(ownerTypeName.contains("::")){
                                QString _ownerTypeName = ownerTypeName.left(ownerTypeName.lastIndexOf("::"));
                                ownerType = classes().findClass(_ownerTypeName);
                            }else if(global){
                                if(AbstractMetaEnum* target_meta_enum = global->findEnum(ownerTypeName)){
                                    QVariant variant = figureOutEnumValue(target_meta_enum->typeEntry()->size(), ev->stringValue(), target_meta_enum->values().indexOf(ev), global, target_meta_enum, nullptr, warnings);
                                    if(variant.isValid()){
                                        ev->setValue(variant);
                                        v = variant;
                                        found = true;
                                    }
                                }
                            }else{
                                QString globalName = TypeDatabase::globalNamespaceClassName();
                                for(AbstractMetaClass *cls : m_meta_classes){
                                    if(cls->name()==globalName){
                                        if(AbstractMetaEnum* target_meta_enum = cls->findEnum(ownerTypeName)){
                                            QVariant variant = figureOutEnumValue(target_meta_enum->typeEntry()->size(), ev->stringValue(), target_meta_enum->values().indexOf(ev), cls, target_meta_enum, nullptr, warnings);
                                            if(variant.isValid()){
                                                ev->setValue(variant);
                                                v = variant;
                                                found = true;
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }else{
                        ownerType = global;
                    }
                    if(!found){
                        if(ownerType){
                            if(AbstractMetaEnum* target_meta_enum = ownerType->findEnumForValue(ev->cppName())){
                                QVariant variant = figureOutEnumValue(target_meta_enum->typeEntry()->size(), ev->stringValue(), target_meta_enum->values().indexOf(ev), global, target_meta_enum, nullptr, warnings);
                                if(variant.isValid()){
                                    ev->setValue(variant);
                                    v = variant;
                                }
                            }
                        }else if(ownerTypeName.isEmpty()){
                            if(warnings){
                                warnings->insert("Not able to find enum constant '"+s+"'");
                            }else{
                                ReportHandler::warning("Not able to find enum constant '"+s+"'");
                            }
                        }else{
                            if(warnings){
                                warnings->insert("Enum constant '"+s+"' belongs to unknown type "+ownerTypeName);
                            }else{
                                ReportHandler::warning("Enum constant '"+s+"' belongs to unknown type "+ownerTypeName);
                            }
                        }
                    }
                }else{
                    v = ev->value();
                }
            } else {
                AbstractMetaEnum* target_meta_enum = nullptr;
                QString enumEntry = s;
                if(meta_enum && meta_enum->typeEntry()->isScopedEnum() && enumEntry.startsWith(meta_enum->typeEntry()->targetLangName()+"::")){
                    enumEntry = enumEntry.right(enumEntry.length() - meta_enum->typeEntry()->targetLangName().length() - 2);
                }else if(meta_enum && meta_enum->typeEntry()->isScopedEnum() && enumEntry.startsWith(meta_enum->typeEntry()->qualifiedCppName()+"::")){
                    enumEntry = enumEntry.right(enumEntry.length() - meta_enum->typeEntry()->qualifiedCppName().length() - 2);
                }
                if (meta_enum && (ev = meta_enum->values().find(enumEntry))) {
                    // if target value has not yet been computed do it
                    if(!ev->value().isValid()){
                        QVariant variant = figureOutEnumValue(meta_enum->typeEntry()->size(), ev->stringValue(), QVariant(), global, meta_enum, nullptr, warnings);
                        if(variant.isValid()){
                            ev->setValue(variant);
                            v = variant;
                        }
                    }else{
                        v = ev->value();
                    }
                } else if (meta_enum && (ev = meta_enum->enclosingClass()->findEnumValue(s, meta_enum))) {
                    // if target value has not yet been computed do it
                    if(!ev->value().isValid()){
                        target_meta_enum = meta_enum->enclosingClass()->findEnumForValue(ev->cppName());
                        if(target_meta_enum){
                            QVariant variant = figureOutEnumValue(meta_enum->typeEntry()->size(), ev->stringValue(), QVariant(), global, target_meta_enum, nullptr, warnings);
                            if(variant.isValid()){
                                ev->setValue(variant);
                                v = variant;
                            }
                        }
                    }else{
                        v = ev->value();
                    }
                } else if (global
                           && !s.contains("::")
                           && (target_meta_enum = global->findEnumForValue(s))
                           && (ev = global->findEnumValue(s, target_meta_enum))) {
                    // if target value has not yet been computed do it
                    if(!ev->value().isValid()){
                        QVariant variant = figureOutEnumValue(target_meta_enum->typeEntry()->size(), ev->stringValue(), QVariant(), global, target_meta_enum, nullptr, warnings);
                        if(variant.isValid()){
                            ev->setValue(variant);
                            v = variant;
                        }
                    }else{
                        v = ev->value();
                    }
                } else if (global
                           && s!=enumEntry
                           && !enumEntry.contains("::")
                           && (target_meta_enum = global->findEnumForValue(enumEntry))
                           && (ev = global->findEnumValue(enumEntry, target_meta_enum))) {
                    // if target value has not yet been computed do it
                    if(!ev->value().isValid()){
                        QVariant variant = figureOutEnumValue(target_meta_enum->typeEntry()->size(), ev->stringValue(), QVariant(), global, target_meta_enum, nullptr, warnings);
                        if(variant.isValid()){
                            ev->setValue(variant);
                            v = variant;
                        }
                    }else{
                        v = ev->value();
                    }
                } else {
                    AbstractMetaClass * ownerType = nullptr;
                    QString ownerTypeName;
                    QString enumValueName;
                    if (s.contains("::")) {
                        ownerTypeName = s.left(s.lastIndexOf("::"));
                        enumValueName = s.right(s.length()-s.lastIndexOf("::")-2);
                        AbstractMetaClass * ownerType = classes().findClass(ownerTypeName);
                        if(!ownerType && ownerTypeName.contains("::")){
                            QString _ownerTypeName = ownerTypeName.left(ownerTypeName.lastIndexOf("::"));
                            enumValueName = ownerTypeName.right(ownerTypeName.length()-ownerTypeName.lastIndexOf("::")-2) + "::" + enumValueName;
                            ownerType = classes().findClass(_ownerTypeName);
                        }
                    }else{
                        ownerType = global;
                    }
                    if(ownerType && (target_meta_enum = ownerType->findEnumForValue(s))
                            && (ev = ownerType->findEnumValue(s, target_meta_enum))){
                        if(!ev->value().isValid()){
                            QVariant variant = figureOutEnumValue(target_meta_enum->typeEntry()->size(), ev->stringValue(), QVariant(), global, target_meta_enum, nullptr, warnings);
                            if(variant.isValid()){
                                ev->setValue(variant);
                                v = variant;
                            }
                        }else{
                            v = ev->value();
                        }
                    }
                    if(ownerType && (target_meta_enum = ownerType->findEnumForValue(enumValueName))
                            && (ev = ownerType->findEnumValue(enumValueName, target_meta_enum))){
                        if(!ev->value().isValid()){
                            QVariant variant = figureOutEnumValue(target_meta_enum->typeEntry()->size(), ev->stringValue(), QVariant(), global, target_meta_enum, nullptr, warnings);
                            if(variant.isValid()){
                                ev->setValue(variant);
                                v = variant;
                            }
                        }else{
                            v = ev->value();
                        }
                    }
                    if(!v.isValid()){
                        if (meta_enum)
                            ReportHandler::warning("unhandled enum value: " + s + " in "
                                                   + meta_enum->enclosingClass()->name() + "::"
                                                   + meta_enum->name());
                        else
                            ReportHandler::warning("unhandled enum value: Unknown enum");
                    }
                }
            }
        }

        if(v.type()==QVariant::String){
            disjointReturnValueStrings << op.toString(v.toString());
            disjointReturnValues << QVariant();
            hasValidNumberVariants = false;
        }else if (v.isValid()){
            QString number;
            switch(size){
            case 8:
                number = "((byte)"+QString::number(v.value<qint8>())+")";
                break;
            case 16:
                number = "((short)"+QString::number(v.value<qint16>())+")";
                break;
            case 32:
                number = QString::number(v.value<qint32>());
                break;
            case 64:
                number = QString::number(v.value<qint64>())+"L";
                break;
            default:
                number = QString::number(v.value<qint32>());
                break;
            }
            disjointReturnValueStrings << op.toString(number);
            QVariant result = op.calculate(v);
            if(!result.isValid() || result.type()==QVariant::String){
                hasValidNumberVariants = false;
                disjointReturnValues << QVariant();
            }else{
                disjointReturnValues << result;
            }
        }else{
            if(ev){
                if(ev->getEnum()->typeEntry()->forceInteger()){
                    disjointReturnValueStrings << op.toString(ev->getEnum()->typeEntry()->qualifiedTargetLangName()+"."+ev->name());
                }else{
                    disjointReturnValueStrings << op.toString(ev->getEnum()->typeEntry()->qualifiedTargetLangName()+"."+ev->name()+".value()");
                }
            }else{
                disjointReturnValueStrings << op.toString(s);
            }
            disjointReturnValues << QVariant();
            hasValidNumberVariants = false;
        }
    }

    QVariant returnValue;
    if(hasValidNumberVariants){
        for(int i=0; i<disjointReturnValues.size(); i++){
            if(i==0){
                returnValue = disjointReturnValues.at(i);
            }else{
                switch(size){
                case 8:
                    returnValue.setValue<qint8>(returnValue.value<qint8>() | disjointReturnValues.at(i).value<qint8>());
                    break;
                case 16:
                    returnValue.setValue<qint16>(returnValue.value<qint16>() | disjointReturnValues.at(i).value<qint16>());
                    break;
                case 32:
                    returnValue.setValue<qint32>(returnValue.value<qint32>() | disjointReturnValues.at(i).value<qint32>());
                    break;
                case 64:
                    returnValue.setValue<qint64>(returnValue.value<qint64>() | disjointReturnValues.at(i).value<qint64>());
                    break;
                default:
                    returnValue.setValue<qint32>(returnValue.value<qint32>() | disjointReturnValues.at(i).value<qint32>());
                    break;
                }
            }
        }
    }else{
        QString stringResult;
        for(int i=0; i<disjointReturnValueStrings.size(); i++){
            if(i==0){
                stringResult = disjointReturnValueStrings.at(i);
            }else{
                stringResult += "|" + disjointReturnValueStrings.at(i);
            }
        }
        returnValue.setValue<QString>(stringResult);
    }

    if(negate){
        switch(size){
        case 8:
            returnValue.setValue<qint8>(-returnValue.value<qint8>());
            break;
        case 16:
            returnValue.setValue<qint16>(-returnValue.value<qint16>());
            break;
        case 32:
            returnValue.setValue<qint32>(-returnValue.value<qint32>());
            break;
        case 64:
            returnValue.setValue<qint64>(-returnValue.value<qint64>());
            break;
        default:
            returnValue.setValue<qint32>(-returnValue.value<qint32>());
            break;
        }
    }else if(_not){
        switch(size){
        case 8:
            returnValue.setValue<qint8>(~returnValue.value<qint8>());
            break;
        case 16:
            returnValue.setValue<qint16>(~returnValue.value<qint16>());
            break;
        case 32:
            returnValue.setValue<qint32>(~returnValue.value<qint32>());
            break;
        case 64:
            returnValue.setValue<qint64>(~returnValue.value<qint64>());
            break;
        default:
            returnValue.setValue<qint32>(~returnValue.value<qint32>());
            break;
        }
    }

    return returnValue;
}

void AbstractMetaBuilder::figureOutEnumValuesForClass(AbstractMetaClass *meta_class,
        QSet<AbstractMetaClass *> *classes, QSet<AbstractMetaClass *> *repeatClasses, QSet<QString> *warnings) {
    AbstractMetaClass *base = meta_class->baseClass();

    if (base && !classes->contains(base))
        figureOutEnumValuesForClass(base, classes, repeatClasses, warnings);

    if (classes->contains(meta_class))
        return;

    bool hasStringEnumValues = false;

    AbstractMetaEnumList enums = meta_class->enums();
    for(AbstractMetaEnum *e : enums) {
        if (!e)
            ReportHandler::warning("bad enum in class " + meta_class->name());
        AbstractMetaEnumValueList lst = e->values();
        QVariant value;
        switch(e->typeEntry()->size()){
        case 8:
            value = QVariant::fromValue<qint8>(0); break;
        case 16:
            value = QVariant::fromValue<qint16>(0); break;
        case 32:
            value = QVariant::fromValue<qint32>(0); break;
        case 64:
            value = QVariant::fromValue<qint64>(0); break;
        default:
            value = QVariant::fromValue<qint32>(0); break;
        }
        for (int i = 0; i < lst.size(); ++i) {
            // value could have been computed previously
            if(!lst.at(i)->value().isValid() || lst.at(i)->value().type()==QVariant::String){
                QVariant _value = figureOutEnumValue(e->typeEntry()->size(), lst.at(i)->stringValue(), value, nullptr, e, nullptr, warnings);
                if(_value.isValid()){
                    lst.at(i)->setValue(_value);
                    value = _value;
                }
            }else{
                value = lst.at(i)->value();
            }
            if(value.type()==QVariant::String){
                value.setValue<QString>(value.toString()+"+1");
                hasStringEnumValues = true;
            }else{
                switch(e->typeEntry()->size()){
                case 8:
                    value.setValue<qint8>(value.value<qint8>()+1); break;
                case 16:
                    value.setValue<qint16>(value.value<qint16>()+1); break;
                case 32:
                    value.setValue<qint32>(value.value<qint32>()+1); break;
                case 64:
                    value.setValue<qint64>(value.value<qint64>()+1); break;
                default:
                    value.setValue<qint32>(value.value<qint32>()+1); break;
                }
            }
        }

        // Check for duplicate values...
        EnumTypeEntry *ete = e->typeEntry();
        if (!ete->forceInteger()) {
            QHash<qint64, AbstractMetaEnumValue *> entries;
            for(AbstractMetaEnumValue *v : lst) {

                bool vRejected = ete->isEnumValueRejected(v->name());

                if(v->value().isValid() && v->value().type()!=QVariant::String){
                    AbstractMetaEnumValue *current = entries.value(v->value().value<qint64>());
                    if (current) {
                        bool currentRejected = ete->isEnumValueRejected(current->name());
                        if (!currentRejected && !vRejected) {
                            ReportHandler::warning(
                                QString("duplicate enum values: %1::%2, %3 and %4 are %5, already rejected: (%6)")
                                .arg(meta_class->name())
                                .arg(e->name())
                                .arg(v->name())
                                .arg(entries[v->value().value<qint64>()]->name())
                                .arg(v->value().value<qint64>())
                                .arg(ete->enumValueRejections().join(", ")));
                            continue;
                        }
                    }

                    if (!vRejected)
                        entries[v->value().value<qint64>()] = v;
                }
            }

            // Entries now contain all the original entries, no
            // rejected ones... Use this to generate the enumValueRedirection table.
            for(AbstractMetaEnumValue *reject : lst) {
                if (!ete->isEnumValueRejected(reject->name()))
                    continue;

                AbstractMetaEnumValue *used = entries.value(reject->value().value<qint64>());
                if (!used) {
                    ReportHandler::warning(
                        QString::fromLatin1("Rejected enum has no alternative...: %1::%2\n")
                        .arg(meta_class->name())
                        .arg(reject->name()));
                    continue;
                }
                ete->addEnumValueRedirection(reject->name(), used->name());
            }

        }
    }


    if(!hasStringEnumValues) // otherwise compute enum values again
        *classes += meta_class;
    else
        *repeatClasses += meta_class;
}


void AbstractMetaBuilder::figureOutEnumValues() {
    // Keep a set of classes that we already traversed. We use this to
    // enforce that we traverse base classes prior to subclasses.
    QSet<AbstractMetaClass *> classes;
    QSet<AbstractMetaClass *> repeatClasses;
    QSet<QString> warnings;
    for(AbstractMetaClass *c : m_meta_classes) {
        figureOutEnumValuesForClass(c, &classes, &repeatClasses, &warnings);
    }
    if(!repeatClasses.isEmpty()){
        int i=0;
        QSet<AbstractMetaClass *> _repeatClasses;
        while(i<3){
            warnings.clear();
            for(AbstractMetaClass *c : repeatClasses) {
                figureOutEnumValuesForClass(c, &classes, &_repeatClasses, &warnings);
            }
            if(_repeatClasses.isEmpty() || _repeatClasses==repeatClasses){
                break;
            }
            if(_repeatClasses.size()==repeatClasses.size()){
                i++;
            }
            repeatClasses.swap(_repeatClasses);
            _repeatClasses.clear();
        }
        for(const QString& w : warnings){
            ReportHandler::warning(w);
        }

        for(AbstractMetaClass *c : repeatClasses) {
            for(const AbstractMetaEnum* enm : c->enums()){
                for(const AbstractMetaEnumValue* value : enm->values()) {
                    if(value->value().type()==QVariant::String){
                        QString warn = QString("possible compilation error in enum value %1").arg(value->value().toString());
                        ReportHandler::warning(warn);
                    }
                }
            }
        }
    }
}

void AbstractMetaBuilder::figureOutDefaultEnumArguments() {
    for(AbstractMetaClass *meta_class : m_meta_classes) {
        QString package = meta_class->typeEntry()->javaPackage();
        QString globalName = TypeDatabase::globalNamespaceClassName();
        AbstractMetaClass *global = m_meta_classes.findClass(package.isEmpty() ? globalName : package + "." + globalName);

        for(AbstractMetaFunction *meta_function : meta_class->functions()) {
            for(AbstractMetaArgument *arg : meta_function->arguments()) {

                QString expr = arg->defaultValueExpression();
                if (expr.isEmpty())
                    continue;

                if (!meta_function->replacedDefaultExpression(meta_function->implementingClass(),
                        arg->argumentIndex() + 1).isEmpty()) {
                    continue;
                }

                QString new_expr = expr;
                if (arg->type()->isEnum()) {
                    QStringList lst = expr.split(QLatin1String("::"));
                    if (lst.size() == 1
                            || (arg->type()->typeEntry()->isScopedEnum() && lst.size() == 2)) {
                        QVector<AbstractMetaClass *> classes(1, meta_class);
                        AbstractMetaEnum *e = nullptr;
                        AbstractMetaEnumValue *ev = nullptr;
                        while (!classes.isEmpty() && !e) {
                            if (classes.front()) {
                                classes << classes.front()->baseClass();

                                AbstractMetaClassList interfaces = classes.front()->interfaces();
                                for(AbstractMetaClass *interface : interfaces){
                                    AbstractMetaClass * primaryInterfaceImplementor = interface->primaryInterfaceImplementor();
                                    if(!classes.contains(primaryInterfaceImplementor)){
                                        classes << primaryInterfaceImplementor;
                                    }else{
                                        qDebug("The interface of %s is %s.", qPrintable(classes.front()->name()), qPrintable(primaryInterfaceImplementor->name()));
                                    }
                                }

                                ev = classes.front()->findEnumValue(expr, nullptr);
                                if(ev && !ev->getEnum())
                                    ev = nullptr;
                                if(!ev && !expr.startsWith(classes.front()->qualifiedCppName()+"::")){
                                    ev = classes.front()->findEnumValue(classes.front()->qualifiedCppName()+"::"+expr, nullptr);
                                    if(ev && !ev->getEnum())
                                        ev = nullptr;
                                }
                                if(!ev){
                                    e = classes.front()->findEnumForValue(expr);
                                    if(!e && !expr.startsWith(classes.front()->qualifiedCppName()+"::")){
                                        e = classes.front()->findEnumForValue(classes.front()->qualifiedCppName()+"::"+expr);
                                    }
                                }
                                if(e || ev)
                                    break;
                            }

                            classes.pop_front();
                        }

                        if (!e && !ev && global) {
                            ev = global->findEnumValue(expr, nullptr);
                            if(!ev){
                                e = global->findEnumForValue(expr);
                            }
                        }

                        if (ev) {
                            new_expr = QString("%1.%2")
                                       .arg(ev->getEnum()->typeEntry()->qualifiedTargetLangName())
                                       .arg(ev->name());
                        } else if (e) {
                            new_expr = QString("%1.%2")
                                       .arg(e->typeEntry()->qualifiedTargetLangName())
                                       .arg(lst.last());
                        } else {
                            ReportHandler::warning("Cannot find enum constant for value '" + expr + "' in '" + meta_class->name() + "' or any of its super classes");
                        }
                    } else {
                        AbstractMetaEnum *e = nullptr;
                        QString enumValueName;
                        QString className;
                        if(arg->type()->typeEntry()->isScopedEnum() && lst.length()>1){
                            enumValueName = lst.at(lst.length()-2) + "::" + lst.at(lst.length()-1);
                            for(int i=0; i<lst.size()-2; i++){
                                if(!className.isEmpty()){
                                    className += "::";
                                }
                                className += lst.at(i);
                            }
                        }else{
                            enumValueName = lst.at(lst.length()-1);
                            for(int i=0; i<lst.size()-1; i++){
                                if(!className.isEmpty()){
                                    className += "::";
                                }
                                className += lst.at(i);
                            }
                        }
                        AbstractMetaClass *cl = m_meta_classes.findClass(className);
                        if (!cl) {
                            if(arg->type()->typeEntry()->isScopedEnum()){
                                cl = global;
                            }else{
                                if(lst.size()==2 && global){
                                    enumValueName = lst.join("::");
                                    cl = global;
                                } else if(lst.size()>2){
                                    className = "";
                                    enumValueName = lst.at(lst.length()-2)+"::"+lst.at(lst.length()-1);
                                    for(int i=0; i<lst.size()-2; i++){
                                        if(!className.isEmpty()){
                                            className += "::";
                                        }
                                        className += lst.at(i);
                                    }
                                    cl = m_meta_classes.findClass(className);
                                }
                            }
                        }

                        if (!cl) {
                            ReportHandler::warning("missing required class for enums: " + className);
                            continue;
                        }
                        e = cl->findEnumForValue(enumValueName);
                        if(e){
                            new_expr = QString("%1.%2")
                                   .arg(e->typeEntry()->qualifiedTargetLangName())
                                   .arg(lst.last());
                        }else if(cl->isFake()){
                            new_expr = QString("%1.%2")
                                   .arg(arg->type()->name())
                                   .arg(lst.last());
                        }else{
                            new_expr = QString("%1.%2.%3")
                                   .arg(cl->typeEntry()->qualifiedTargetLangName())
                                   .arg(arg->type()->name())
                                   .arg(lst.last());
                        }
                    }

                } else if (arg->type()->isFlags()) {
                    const FlagsTypeEntry *flagsEntry =
                        static_cast<const FlagsTypeEntry *>(arg->type()->typeEntry());
                    const EnumTypeEntry *enumEntry = flagsEntry->originator();
                    AbstractMetaEnum *meta_enum = m_meta_classes.findEnum(enumEntry);
                    if (!meta_enum) {
                        ReportHandler::warning("unknown required enum " + enumEntry->qualifiedCppName());
                        continue;
                    }
                    AbstractMetaEnumValue * v = meta_enum->values().find(expr);
                    if(v){
                        if(v->value().isValid()){
                            if(v->value().type()==QVariant::String){
                                new_expr = v->value().toString();
                                ReportHandler::warning(QString("possible compilation error in enum value %1 when parsing default value of '%2' in class '%3'")
                                                       .arg(new_expr)
                                                       .arg(meta_function->name())
                                                       .arg(meta_function->implementingClass()->name()));
                            }else{
                                new_expr = QString::number(v->value().value<qint32>());
                            }
                        }else{
                            QVariant value = figureOutEnumValue(32, expr, meta_enum->values().indexOf(v), global, meta_enum, meta_function);
                            if(value.isValid()){
                                v->setValue(value);
                                if(value.type()==QVariant::String){
                                    new_expr = value.toString();
                                    ReportHandler::warning(QString("possible compilation error in enum value %1 when parsing default value of '%2' in class '%3'")
                                                           .arg(new_expr)
                                                           .arg(meta_function->name())
                                                           .arg(meta_function->implementingClass()->name()));
                                }else{
                                    new_expr = QString::number(value.value<qint32>());
                                }
                            }
                        }
                    }else{
                        QVariant value = figureOutEnumValue(32, expr, QVariant::fromValue<QString>(expr), global, meta_enum, meta_function);
                        if(value.isValid()){
                            if(value.type()==QVariant::String){
                                new_expr = value.toString();
                            }else{
                                new_expr = QString::number(value.value<qint32>());
                            }
                        }
                    }
                } else if (arg->type()->isPrimitive()) {
                    int baseTypeId = QMetaType::type(qPrintable(arg->type()->typeEntry()->qualifiedCppName()));
                    uint size = 32;
                    if(baseTypeId!=QMetaType::UnknownType){
                        size = uint(QMetaType::sizeOf(baseTypeId))*8;
                        if(size!=8 && size!=16 && size!=32 && size!=64){
                            size = 32;
                        }
                    }
                    AbstractMetaEnumValue *value = nullptr;
                    if (expr.contains("::"))
                        value = m_meta_classes.findEnumValue(expr);
                    if (!value)
                        value = meta_class->findEnumValue(expr, nullptr);
                    if (!value && global)
                        value = global->findEnumValue(expr, nullptr);

                    QVariant vvalue;
                    if (value) {
                        vvalue = value->value();
                    } else if (expr.contains(QLatin1Char('+'))) {
                        vvalue = figureOutEnumValue(size, expr, 0, global, nullptr);
                    }
                    if(vvalue.isValid()){
                        if(vvalue.type()==QVariant::String){
                            new_expr = vvalue.toString();
                        }else{
                            switch(size){
                            case 8:
                                new_expr = QString::number(vvalue.value<qint8>());
                                break;
                            case 16:
                                new_expr = QString::number(vvalue.value<qint16>());
                                break;
                            case 32:
                                new_expr = QString::number(vvalue.value<qint32>());
                                break;
                            case 64:
                                new_expr = QString::number(vvalue.value<qint64>())+"L";
                                break;
                            default:
                                new_expr = QString::number(vvalue.value<qint32>());
                                break;
                            }
                        }
                    }
                }

                arg->setDefaultValueExpression(new_expr);
            }
        }
    }
}


AbstractMetaEnum *AbstractMetaBuilder::traverseEnum(EnumModelItem enum_item, AbstractMetaClass *enclosing, const QSet<QString> &enumsDeclarations) {
    // Skipping private enums.
    if (enum_item->accessPolicy() == CodeModel::Private) {
        return nullptr;
    }

    QString qualified_name = enum_item->qualifiedName().join("::");

    TypeEntry *type_entry = TypeDatabase::instance()->findType(qualified_name);
    QString enum_name = enum_item->name();

    QString class_name;
    if (m_current_class)
        class_name = m_current_class->typeEntry()->qualifiedCppName();

    if (TypeDatabase::instance()->isEnumRejected(class_name, enum_name)) {
        m_rejected_enums.insert({qualified_name, enum_item->fileName()}, GenerationDisabled);
        return nullptr;
    }

    if (!type_entry || !type_entry->isEnum()) {
        //QString context = m_current_class ? m_current_class->name() : QLatin1String("");
        ReportHandler::warning(QString("enum '%1' does not have a type entry or is not an enum")
                               .arg(qualified_name));
        m_rejected_enums.insert({qualified_name, enum_item->fileName()}, NotInTypeSystem);
        return nullptr;
    }

    AbstractMetaEnum *meta_enum = createMetaEnum();
    EnumTypeEntry * enumType = static_cast<EnumTypeEntry *>(type_entry);
    meta_enum->setTypeEntry(enumType);

    if (enumsDeclarations.contains(qualified_name)
            || enumsDeclarations.contains(enum_name)) {
        meta_enum->setHasQEnumsDeclaration(true);
    }else if(enumType->flags()){
        if (enumsDeclarations.contains(enumType->flags()->qualifiedCppName())
                || enumsDeclarations.contains(enumType->flags()->name())
                || enumsDeclarations.contains(enumType->flags()->originalName())
                || enumsDeclarations.contains(enumType->flags()->originalName().split("::").last())) {
            meta_enum->setHasQEnumsDeclaration(true);
        }
    }
    switch (enum_item->accessPolicy()) {
        case CodeModel::Public: *meta_enum += AbstractMetaAttributes::Public; break;
        case CodeModel::Protected:
        *meta_enum += AbstractMetaAttributes::Protected;
        meta_enum->typeEntry()->setPublic(false);
        break;
        case CodeModel::Private:
        meta_enum->typeEntry()->setPublic(false);
        break;
    }
    if(enum_item->isAnonymous()){
        *meta_enum += AbstractMetaAttributes::Annonymous;
    }
    if(enum_item->isDeclDeprecated()){
        *meta_enum += AbstractMetaAttributes::Deprecated;
        meta_enum->setDeprecatedComment(enum_item->getDeclDeprecatedComment());
    }
    if(enum_item->isScopedEnum()){
        meta_enum->typeEntry()->setEnumClass(true);
    }
    meta_enum->setBaseTypeName(enum_item->baseType().qualifiedName().join("::"));
    uint size = 32;
    if(!meta_enum->baseTypeName().isEmpty()){
        QString baseType = meta_enum->baseTypeName();
        TypeDatabase *types = TypeDatabase::instance();
        TypeEntry *base_class_entry = types->findType(qPrintable(baseType));
        if(base_class_entry)
            baseType = base_class_entry->qualifiedCppName();
        int baseTypeId = QMetaType::type(qPrintable(baseType));
        if(baseTypeId!=QMetaType::UnknownType){
            size = uint(QMetaType::sizeOf(baseTypeId)*8);
            if(size!=8 && size!=16 && size!=32 && size!=64){
                size = 32;
            }
        }
    }
    meta_enum->typeEntry()->setSize(size);

    ReportHandler::debugMedium(QString(" - traversing enum %1").arg(meta_enum->fullName()));

    for(const EnumeratorModelItem& value : enum_item->enumerators()) {

        AbstractMetaEnumValue *meta_enum_value = createMetaEnumValue();
        meta_enum_value->setName(value->name());
        meta_enum_value->setEnum(meta_enum);
        // Deciding the enum value...

        meta_enum_value->setStringValue(value->value());
        meta_enum_value->setDeprecated(value->deprecated());
        meta_enum_value->setDeprecatedComment(value->deprecatedComment());
        meta_enum->addEnumValue(meta_enum_value);

        ReportHandler::debugFull("   - " + meta_enum_value->name() + " = "
                                 + QString::number(meta_enum_value->value().value<qint64>()));

        // Add into global register...
        if (enclosing)
            m_enum_values[enclosing->name() + "::" + meta_enum_value->cppName()] = meta_enum_value;
        else
            m_enum_values[meta_enum_value->cppName()] = meta_enum_value;
    }

    m_enums << meta_enum;

    if (!meta_enum->typeEntry()->include().isValid()) {
        QFileInfo info(enum_item->fileName());
        QMap<QString,QString> requiredFeatures;
        for(const QString& feature : enum_item->requiredFeatures()){
            if(m_features && m_features->contains(feature)){
                QFileInfo _info(m_features->value(feature));
                requiredFeatures[feature] = _info.fileName();
            }else{
                requiredFeatures[feature] = "";
            }
        }
        meta_enum->typeEntry()->setInclude(Include(Include::IncludePath, info.fileName(), requiredFeatures));
    }
    if(!enum_item->requiredFeatures().isEmpty()){
        QStringList ppConditions;
        if(!meta_enum->typeEntry()->ppCondition().isEmpty()){
            ppConditions << meta_enum->typeEntry()->ppCondition();
        }
        for(const QString& feature : enum_item->requiredFeatures()){
            ppConditions << QString("QT_CONFIG(%1)").arg(feature);
        }
        static_cast<EnumTypeEntry *>(type_entry)->setPPCondition(ppConditions.join(" && "));
    }
    return meta_enum;
}

AbstractMetaFunctional *AbstractMetaBuilder::traverseFunctional(TypeAliasModelItem item){
    if (TypeDatabase::instance()->isClassRejected(item->qualifiedName().join("::"))) {
        m_rejected_functionals.insert({item->qualifiedName().join("::"), item->fileName()}, GenerationDisabled);
        return nullptr;
    }
    QScopedPointer<AbstractMetaFunctional> meta_functional;
    FunctionalTypeEntry *ftype = TypeDatabase::instance()->findFunctionalType(item->qualifiedName().join("::"));
    if(ftype){
        meta_functional.reset(createMetaFunctional());
        meta_functional->setTypeEntry(ftype);
        meta_functional->setBaseTypeName(item->name());
        meta_functional->setFunctionPointer(item->type().isFunctionPointer());
        bool ok = false;
        if(item->type().qualifiedName().join("::").startsWith("std::function<")){
            AbstractMetaType * type = translateType(item->type().functionalReturnType(), &ok, QString("traverseFunctional %1").arg(item->type().qualifiedName().join("::")));
            if(ok){
                meta_functional->setType(type);
                int counter = 0;
                for(const TypeInfo& arg : item->type().functionalArgumentTypes()){
                    AbstractMetaType * atype = translateType(arg, &ok, QString("traverseFunctional %1").arg(item->type().qualifiedName().join("::")));
                    if(ok){
                        AbstractMetaArgument* argument = createMetaArgument();
                        argument->setArgumentIndex(counter);
                        //argument->setName(arg.qualifiedName().join("::"));
                        argument->setType(atype);
                        for(const ArgumentModification& mod : ftype->argumentModification()){
                            if(mod.index==counter+1 && !mod.modified_name.isEmpty()){
                                argument->setName(mod.modified_name);
                            }
                        }
                        meta_functional->addArgument(argument);
                        ++counter;
                    }else {
                        m_rejected_functionals.insert({item->qualifiedName().join("::"), item->fileName()}, UnmatchedArgumentType);
                        return nullptr;
                    }
                }
            }else {
                m_rejected_functionals.insert({item->qualifiedName().join("::"), item->fileName()}, UnmatchedReturnType);
                return nullptr;
            }
        }else{
            TypeInfo info = item->type();
            info.setFunctionPointer(false);
            info.setArgumentNames(QList<QString>());
            info.setArguments(QList<TypeInfo>());
            AbstractMetaType * type = translateType(info, &ok, QString("traverseFunctional %1").arg(info.qualifiedName().join("::")));
            if(ok){
                meta_functional->setType(type);
                int counter = 0;
                for(const TypeInfo& arg : item->type().arguments()){
                    AbstractMetaType * atype = translateType(arg, &ok, QString("traverseFunctional %1").arg(item->type().qualifiedName().join("::")));
                    if(ok){
                        AbstractMetaArgument* argument = createMetaArgument();
                        argument->setArgumentIndex(counter);
                        argument->setType(atype);
                        argument->setName(item->type().argumentNames()[counter]);
                        for(const ArgumentModification& mod : ftype->argumentModification()){
                            if(mod.index==counter+1 && !mod.modified_name.isEmpty()){
                                argument->setName(mod.modified_name);
                            }
                        }
                        meta_functional->addArgument(argument);
                        ++counter;
                    }else {
                        m_rejected_functionals.insert({item->qualifiedName().join("::"), item->fileName()}, UnmatchedArgumentType);
                        return nullptr;
                    }
                }
            }else {
                m_rejected_functionals.insert({item->qualifiedName().join("::"), item->fileName()}, UnmatchedReturnType);
                return nullptr;
            }
        }
    }else{
        m_rejected_functionals.insert({item->qualifiedName().join("::"), item->fileName()}, NotInTypeSystem);
    }
    return meta_functional.take();
}

AbstractMetaClass *AbstractMetaBuilder::traverseTypeAlias(TypeAliasModelItem typeAlias) {
    QString class_name = strip_template_args(typeAlias->name());

    QString full_class_name = class_name;
    // we have an inner class
    if (m_current_class) {
        full_class_name = strip_template_args(m_current_class->typeEntry()->qualifiedCppName())
                          + "::" + full_class_name;
    }

    // If we haven't specified anything for the typedef, then we don't care
    ComplexTypeEntry *type = TypeDatabase::instance()->findComplexType(full_class_name);
    if (!type)
        return nullptr;

    if (type->isObject())
        static_cast<ObjectTypeEntry *>(type)->setQObject(isQObject(strip_template_args(typeAlias->type().qualifiedName().join("::"))));

    AbstractMetaClass *meta_class = createMetaClass();
    meta_class->setTypeAlias(true);
    meta_class->setTypeEntry(type);
    TypeEntry* typeAliasEntry = TypeDatabase::instance()->findType(typeAlias->type().qualifiedName().join("::"));
    if(typeAliasEntry){
        AbstractMetaType* typeAliasType = createMetaType();
        typeAliasType->setTypeEntry(typeAliasEntry);
        typeAliasType->setIndirections(typeAlias->type().indirections());
        typeAliasType->setConstant(typeAlias->type().isConstant());
        typeAliasType->setReferenceType(AbstractMetaType::ReferenceType(typeAlias->type().getReferenceType()));
        meta_class->setTypeAliasType(typeAliasType);
    }else{
        meta_class->setBaseClassNames(QStringList() << typeAlias->type().qualifiedName().join("::"));
    }
    *meta_class += AbstractMetaAttributes::Public;

    // Set the default include file name
    if (!type->include().isValid()) {
        QFileInfo info(typeAlias->fileName());
        QMap<QString,QString> requiredFeatures;
        for(const QString& feature : typeAlias->requiredFeatures()){
            if(m_features && m_features->contains(feature)){
                QFileInfo _info(m_features->value(feature));
                requiredFeatures[feature] = _info.fileName();
            }else{
                requiredFeatures[feature] = "";
            }
        }
        type->setInclude(Include(Include::IncludePath, info.fileName(), requiredFeatures));
    }
    if(!typeAlias->requiredFeatures().isEmpty()){
        QStringList ppifs;
        if(!type->ppCondition().isEmpty()){
            ppifs << type->ppCondition();
        }
        for(const QString& feature : typeAlias->requiredFeatures()){
            ppifs << QString("QT_CONFIG(%1)").arg(feature);
        }
        type->setPPCondition(ppifs.join(" && "));
    }

    return meta_class;
}

AbstractMetaClass *AbstractMetaBuilder::traverseClass(ClassModelItem class_item) {
    QString class_name = strip_template_args(class_item->name());
    QString full_class_name = class_name;

    // we have an inner class
    if (m_current_class) {
        full_class_name = strip_template_args(m_current_class->typeEntry()->qualifiedCppName())
                          + "::" + full_class_name;
    }

    ComplexTypeEntry *type = TypeDatabase::instance()->findComplexType(full_class_name);
    if(type && type->designatedInterface())
        type = type->designatedInterface();
    RejectReason reason = NoReason;

    if (TypeDatabase::instance()->isClassRejected(full_class_name)) {
        reason = GenerationDisabled;
    } else if (!type) {
        TypeEntry *te = TypeDatabase::instance()->findType(full_class_name);
        if (te && !te->isComplex())
            reason = RedefinedToNotClass;
        else
            reason = NotInTypeSystem;
    } else if (type->codeGeneration() == TypeEntry::GenerateNothing) {
        reason = GenerationDisabled;
    }

    if (reason != NoReason) {
        m_rejected_classes.insert({full_class_name, class_item->fileName()}, reason);
        return nullptr;
    }

    if (type->isObject()) {
        static_cast<ObjectTypeEntry *>(type)->setQObject(isQObject(full_class_name));
    }

    AbstractMetaClass *meta_class = createMetaClass();
    meta_class->setTypeEntry(type);
    meta_class->setBaseClassNames(class_item->baseClasses());
    meta_class->setHas_Q_GADGET(class_item->has_Q_GADGET());
    meta_class->setHas_Q_OBJECT(class_item->has_Q_OBJECT());
    *meta_class += AbstractMetaAttributes::Public;
    if(class_item->isDeclFinal()){
        *meta_class += AbstractMetaAttributes::Final;
        *meta_class += AbstractMetaAttributes::DeclaredFinalInCpp;
    }
    if(class_item->isDeclDeprecated()){
        *meta_class += AbstractMetaAttributes::Deprecated;
        meta_class->setDeprecatedComment(class_item->declDeprecatedComment());
    }

    AbstractMetaClass *old_current_class = m_current_class;
    m_current_class = meta_class;

    if (type->isContainer()) {
        ReportHandler::debugSparse(QString("container: '%1'").arg(full_class_name));
    } else {
        ReportHandler::debugSparse(QString("class: '%1'").arg(meta_class->fullName()));
    }

    const TemplateParameterList& template_parameters = class_item->templateParameters();
    QList<TypeEntry *> template_args;
    template_args.clear();
    for (int i = 0; i < template_parameters.size(); ++i) {
        const TemplateParameterModelItem &param = template_parameters.at(i);
        TemplateArgumentEntry *param_type = new TemplateArgumentEntry(param->name());
        param_type->setOrdinal(i);
        template_args.append(param_type);
    }
    meta_class->setTemplateArguments(template_args);

    // Inner classes
    {
        const QHash<QString, ClassModelItem>& inner_classes = class_item->classMap();
        for(const QString & key : inner_classes.keys()) {
            //if(!key.contains("<"))
            {
                ClassModelItem ci = inner_classes.value(key);
                AbstractMetaClass *cl = traverseClass(ci);
                if (cl) {
                    bool isEnclosedClass = cl->typeEntry()->targetLangName().startsWith(meta_class->typeEntry()->targetLangName()+"$");
                    if(isEnclosedClass){
                        meta_class->addEnclosedClass(cl);
                        if(cl->isInterface()){
                            AbstractMetaClass *interfaceImpl = cl->extractInterfaceImpl();
                            //m_meta_classes << interfaceImpl;
                            ReportHandler::debugSparse(QString(" -> interface implementation class '%1'").arg(interfaceImpl->name()));
                        }
                        /*
                        if (cl->typeEntry()->designatedInterface()) {
                            AbstractMetaClass *interface = cl->extractInterface();
                            meta_class->addEnclosedClass(interface);
                        }
                        */
                    }
                    addAbstractMetaClass(cl);

                }
            }
        }
    }

    // Go through all typedefs to see if we have defined any
    // specific typedefs to be used as classes.
    for(QHash<QString, TypeAliasModelItem>::const_iterator i = class_item->typeAliasMap().begin(); i!=class_item->typeAliasMap().end(); i++){
        TypeAliasModelItem tai = i.value();
        if(tai->type().qualifiedName().join("::").startsWith("std::function<") || tai->type().isFunctionPointer()){
            AbstractMetaFunctional *fnl = traverseFunctional(tai);
            if(fnl)
                meta_class->addFunctional(fnl);
        }else{
            AbstractMetaClass *cls = traverseTypeAlias(tai);
            if (cls) {
                meta_class->addEnclosedClass(cls);
                addAbstractMetaClass(cls);
            }
        }
    }

    parseQ_Property(meta_class, class_item->propertyDeclarations());

    traverseFunctions(model_dynamic_cast<ScopeModelItem>(class_item), meta_class);
    traverseEnums(model_dynamic_cast<ScopeModelItem>(class_item), meta_class, class_item->enumsDeclarations());
    traverseFields(model_dynamic_cast<ScopeModelItem>(class_item), meta_class);

    m_current_class = old_current_class;

    // Set the default include file name
    if (!type->include().isValid()) {
        QFileInfo info(class_item->fileName());
        QMap<QString,QString> requiredFeatures;
        for(const QString& feature : class_item->requiredFeatures()){
            if(m_features && m_features->contains(feature)){
                QFileInfo _info(m_features->value(feature));
                requiredFeatures[feature] = _info.fileName();
            }else{
                requiredFeatures[feature] = "";
            }
        }
        type->setInclude(Include(Include::IncludePath, info.fileName(), requiredFeatures));
    }

    if(!class_item->requiredFeatures().isEmpty()){
        QStringList ppifs;
        if(!type->ppCondition().isEmpty()){
            ppifs << type->ppCondition();
        }
        for(const QString& feature : class_item->requiredFeatures()){
            ppifs << QString("QT_CONFIG(%1)").arg(feature);
        }
        type->setPPCondition(ppifs.join(" && "));
    }

    // not possible to inherit from a union type, so declare final
    if(class_item.data()->classType()==CodeModel::Union){
        *meta_class += AbstractMetaAttributes::Final;
    }

    return meta_class;
}

AbstractMetaField *AbstractMetaBuilder::traverseField(VariableModelItem field, const AbstractMetaClass *cls) {
    const QString& field_name = field->name();
    QString class_name = m_current_class->typeEntry()->qualifiedCppName();

    //qDebug()<<"\n"<<field_name<<class_name;

    // Ignore friend decl.
    if (field->isFriend())
        return nullptr;

    if (field->accessPolicy() == CodeModel::Private)
        return nullptr;

    if (TypeDatabase::instance()->isFieldRejected(class_name, field_name)) {
        m_rejected_fields.insert({class_name + "::" + field_name, field->fileName()}, GenerationDisabled);
        return nullptr;
    }

    AbstractMetaField *meta_field = createMetaField();
    meta_field->setName(field_name);
    meta_field->setEnclosingClass(cls);

    bool ok;
    const TypeInfo& field_type = field->type();
    //qDebug()<<"\n\n\n"<<"Class in question:"<<cls->name()<<"\n\n\n";
    AbstractMetaType *meta_type = translateType(field_type, &ok, "traverseField " + class_name);

    if (!meta_type || !ok) {
        QString error = QString("skipping field '%1::%2' with unmatched type '%3'")
                        .arg(m_current_class->name())
                        .arg(field_name)
                        .arg(TypeInfo::resolveType(field_type, currentScope()->toItem()).qualifiedName().join("::"));
        ReportHandler::warning(error);
        delete meta_field;
        return nullptr;
    }

    meta_field->setType(meta_type);

    uint attr = 0;
    if (field->isStatic())
        attr |= AbstractMetaAttributes::Static;

    CodeModel::AccessPolicy policy = field->accessPolicy();
    if (policy == CodeModel::Public)
        attr |= AbstractMetaAttributes::Public;
    else if (policy == CodeModel::Protected)
        attr |= AbstractMetaAttributes::Protected;
    else
        attr |= AbstractMetaAttributes::Private;
    meta_field->setAttributes(attr);

    return meta_field;
}

void AbstractMetaBuilder::traverseFields(ScopeModelItem scope_item, AbstractMetaClass *meta_class) {
    for(const VariableModelItem& field : scope_item->variables()) {
        AbstractMetaField *meta_field = traverseField(field, meta_class);

        if (meta_field) {
            meta_field->setOriginalAttributes(meta_field->attributes());
            meta_class->addField(meta_field);
        }
    }
}

void AbstractMetaBuilder::setupFunctionDefaults(AbstractMetaFunction *meta_function, AbstractMetaClass *meta_class) {
    // Set the default value of the declaring class. This may be changed
    // in fixFunctions later on
    meta_function->setDeclaringClass(meta_class);

    // Some of the queries below depend on the implementing class being set
    // to function properly. Such as function modifications
    meta_function->setImplementingClass(meta_class);

    if (meta_function->name() == "operator_equal")
        meta_class->setHasEqualsOperator(true);
}

void AbstractMetaBuilder::traverseFunctions(ScopeModelItem scope_item, AbstractMetaClass *meta_class) {
    // if there are just private constructors avoid generating the shell class
    bool hasJustPrivateConstructors = false;
    // detect virtual destructor
    bool hasVirtualDestructor = false;
    // information about public descructor is required to detect the deletable supertype of a class
    bool hasPublicDestructor = true;
    // if there is a private descructor don't generate shell class
    bool hasPrivateDestructor = false;
    // if there is a pure virtual function which is private don't generate shell class
    QSet<QString> unimplementablePureVirtualFunctions;

    for(const FunctionModelItem& function : scope_item->functions()) {
        AbstractMetaFunction *meta_function = traverseFunction(function);

        if (meta_function) {

            QList<FunctionModification> mods = meta_function->modifications(meta_class);
            for (int i = 0; i < mods.size(); ++i) {
                if (mods.at(i).isPrivateSignal()) {
                    meta_function->setFunctionType(AbstractMetaFunction::PrivateSignalFunction);
                }
            }

            meta_function->setOriginalAttributes(meta_function->attributes());
            if (meta_class->isNamespace())
                *meta_function += AbstractMetaAttributes::Static;

            if (!meta_function->isInvalid()) {
                if (QPropertySpec *read = meta_class->propertySpecForRead(meta_function->name())) {
                    //if (meta_function->type() && read->type() == meta_function->type()->typeEntry()) {
                        *meta_function += AbstractMetaAttributes::PropertyReader;
                        meta_function->setPropertySpec(read);
                    //}
                } else if (QPropertySpec *write =
                               meta_class->propertySpecForWrite(meta_function->name())) {
                    //if (meta_function->arguments().at(0)->type() &&
                    //        write->type() == meta_function->arguments().at(0)->type()->typeEntry()) {
                        *meta_function += AbstractMetaAttributes::PropertyWriter;
                        meta_function->setPropertySpec(write);
                    //}
                } else if (QPropertySpec *reset =
                               meta_class->propertySpecForReset(meta_function->name())) {
                    *meta_function += AbstractMetaAttributes::PropertyResetter;
                    meta_function->setPropertySpec(reset);
                }
            }

            if(meta_function->isDestructor()){
                hasVirtualDestructor = function.data()->isVirtual();
                hasPrivateDestructor = meta_function->isPrivate();
                hasPublicDestructor = meta_function->isPublic();
            }else if(meta_function->isConstructor()){
                if(meta_function->isPrivate() || meta_function->isInvalid()){
                    AbstractMetaFunctionList functions = meta_class->queryFunctions(AbstractMetaClass::Constructors);
                    // do not generate derived class when only copy constructor is available
                    if(functions.isEmpty() || (functions.size()==1 && functions.at(0)->isCopyConstructor())){
                        hasJustPrivateConstructors = true;
                    }
                }else{
                    hasJustPrivateConstructors = false;
                }
            }else if((/*meta_function->isPrivate() ||*/ meta_function->isInvalid()) && meta_function->isAbstract()){
                unimplementablePureVirtualFunctions << meta_function->signature();
            }

            if (!meta_function->isDestructor()
                    && !meta_function->isInvalid()
                    && (!meta_function->isConstructor() || !meta_function->isPrivate())) {

                if (meta_class->typeEntry()->designatedInterface() && !meta_function->isPublic()
                        && !meta_function->isPrivate()) {
                    QString warn = QString("non-public function '%1' in interface '%2'")
                                   .arg(meta_function->name()).arg(meta_class->name());
                    ReportHandler::warning(warn);

//                  don't make public! just skip protected methods in the interface
                    //meta_function->setVisibility(AbstractMetaClass::Public);
                }

                setupFunctionDefaults(meta_function, meta_class);

                /*if (meta_function->isSignal() && meta_class->hasSignal(meta_function)) { // overloaded signals are possible now
                    QString warn = QString("signal '%1' in class '%2' is overloaded.")
                                   .arg(meta_function->name()).arg(meta_class->name());
                    ReportHandler::warning(warn);
                }*/

                if (meta_function->isSignal() && !meta_class->isQObject()) {
                    QString warn = QString("signal '%1' in non-QObject class '%2'")
                                   .arg(meta_function->name()).arg(meta_class->name());
                    ReportHandler::warning(warn);
                }

                meta_class->addFunction(meta_function);
            }else if(!meta_function->isDestructor() &&
                     !meta_function->isConstructor() &&
                     meta_function->isInvalid()){
                meta_class->addInvalidFunction(meta_function);
            }
        }
    }
    if(hasJustPrivateConstructors || hasPrivateDestructor){
        *meta_class += AbstractMetaAttributes::Final;
    }
    meta_class->setHasPublicDestructor(hasPublicDestructor);
    meta_class->setHasPrivateDestructor(hasPrivateDestructor);
    meta_class->setHasVirtualDestructor(hasVirtualDestructor);
    meta_class->setHasJustPrivateConstructors(hasJustPrivateConstructors);
    meta_class->setUnimplmentablePureVirtualFunctions(unimplementablePureVirtualFunctions);
}

bool AbstractMetaBuilder::setupTemplateInstantiations(AbstractMetaClass *meta_class){

    const AbstractMetaFunctionList& funcs = meta_class->functions();
    for(AbstractMetaFunction *func : funcs) {
        if(func->hasTemplateArgumentTypes()){
            QList<TemplateInstantiation> templateInstantiations = func->templateInstantiations(meta_class);
            if(!templateInstantiations.isEmpty()){
                for(const TemplateInstantiation& template_instantiation : templateInstantiations){
                    // duplicate
                    AbstractMetaFunction *func2 = func->copy();
                    // rename function
                    if(!template_instantiation.renamedTo().isEmpty()){
                        func2->setOriginalName(func2->name());
                        func2->setName(template_instantiation.renamedTo());
                    }

                    // find types of the template instantiations
                    AbstractMetaTemplateParameterList templateParameters;
                    QHash<QString,AbstractMetaType*> templateTypes;
                    for(int k=0; k<template_instantiation.arguments.size() && k<func->templateParameters().size(); k++){
                        TypeInfo info;
                        info.setQualifiedName(template_instantiation.arguments[k].split("::"));
                        bool ok = false;
                        AbstractMetaClass * tmp_current_class = m_current_class;
                        m_current_class = meta_class;
                        AbstractMetaType* ttype = translateType(info, &ok, QString("traverseTemplateInstantiation <%1>").arg(template_instantiation.arguments[k]), true, true, false);
                        m_current_class = tmp_current_class;
                        templateTypes[func->templateParameters()[k]->name()] = ttype;
                        AbstractMetaTemplateParameter* tparam = new AbstractMetaTemplateParameter();
                        tparam->setName(func->templateParameters()[k]->name());
                        tparam->setType(ttype);
                        tparam->setInstantiation(template_instantiation.arguments[k]);
                        templateParameters += tparam;
                    }

                    // set instantiations
                    func2->setTemplateParameters(templateParameters);

                    // exchange template types
                    if(func2->type()->typeUsagePattern()==AbstractMetaType::TemplateArgumentPattern){
                        AbstractMetaType* rtype = templateTypes[func2->type()->typeEntry()->name()];
                        if(rtype){
                            rtype = rtype->copy();
                            rtype->setConstant(func2->type()->isConstant());
                            rtype->setIndirections(QList<bool>() << rtype->indirections() << func2->type()->indirections());
                            rtype->setReferenceType(func2->type()->getReferenceType());
                            decideUsagePattern(rtype);
                            if(!rtype->indirections().isEmpty()){
                                rtype->setTypeUsagePattern(AbstractMetaType::ObjectPattern);
                            }
                            func2->setType(rtype);
                        }
                    }

                    for(AbstractMetaArgument* arg : func2->arguments()){
                        if(arg->type()->typeUsagePattern()==AbstractMetaType::TemplateArgumentPattern){
                            AbstractMetaType* rtype = templateTypes[arg->type()->typeEntry()->name()];
                            if(rtype){
                                rtype = rtype->copy();
                                rtype->setConstant(arg->type()->isConstant());
                                rtype->setIndirections(QList<bool>() << rtype->indirections() << arg->type()->indirections());
                                rtype->setReferenceType(arg->type()->getReferenceType());
                                decideUsagePattern(rtype);
                                arg->setType(rtype);
                            }
                        }
                    }

                    if(!func2->hasTemplateArgumentTypes()){
                        meta_class->addFunction(func2);
                    }else{
                        ReportHandler::warning(QString("template method has uninstantiated arguments %1 in class %2").arg(func->signature()).arg(func->implementingClass()->qualifiedCppName()));
                    }
                }
            }
        }
    }

    return true;
}

bool AbstractMetaBuilder::setupInheritance(AbstractMetaClass *meta_class) {
    Q_ASSERT(!meta_class->isInterface());

    if (m_setup_inheritance_done.contains(meta_class))
        return true;
    m_setup_inheritance_done.insert(meta_class);

    QStringList base_classes = meta_class->baseClassNames();

    TypeDatabase *types = TypeDatabase::instance();

    QStringList scope = meta_class->typeEntry()->qualifiedCppName().split("::");
    scope.removeLast();
    // we only support our own containers and ONLY if there is only one baseclass
    if (base_classes.size() == 1 && base_classes.first().count('<') == 1) {
        for (int i = scope.size(); i >= 0; --i) {
            QString prefix = i > 0 ? QStringList(scope.mid(0, i)).join("::") + "::" : QString();
            QString complete_name = prefix + base_classes.first();
            TypeParser::Info info = TypeParser::parse(complete_name);
            QString base_name = info.qualified_name.join("::");

            AbstractMetaClass *templ = nullptr;
            for(AbstractMetaClass *c : m_templates) {
                if (c->typeEntry()->name() == base_name) {
                    templ = c;
                    break;
                }
            }

            if (!templ)
                templ = m_meta_classes.findClass(base_name);

            if (templ) {
                setupInheritance(templ);
                inheritHiddenBaseType(meta_class, templ, info);
                return true;
            }
        }

        ReportHandler::warning(QString("template baseclass '%1' of '%2' is not known")
                               .arg(base_classes.first())
                               .arg(meta_class->name()));
        return false;
    }

    if(!scope.isEmpty()){
        for (int i = 0; i < base_classes.size(); ++i) {
            for (int l = scope.size(); l >= 0; --l) {
                QString prefix = l > 0 ? QStringList(scope.mid(0, l)).join("::") + "::" : QString();
                QString complete_name = prefix + base_classes.at(i);
                TypeParser::Info info = TypeParser::parse(complete_name);
                QString base_name = info.qualified_name.join("::");
                if(types->findType(base_name)){
                    base_classes.replace(i, base_name);
                    continue;
                }
            }
        }
    }

    int primary = -1;
    int primaries = 0;
    for (int i = 0; i < base_classes.size(); ++i) {
        if (types->isClassRejected(base_classes.at(i)))
            continue;

        TypeEntry *base_class_entry = types->findType(base_classes.at(i));
        if (!base_class_entry) {
            ReportHandler::warning(QString("class '%1' inherits from unknown base class '%2'")
                                   .arg(meta_class->name()).arg(base_classes.at(i)));
        }

        // true for primary base class
        else if (!base_class_entry->designatedInterface()) {
            if (primaries > 0) {
                ReportHandler::warning(QString("class '%1' has multiple primary base classes"
                                               " '%2' and '%3'")
                                       .arg(meta_class->name())
                                       .arg(base_classes.at(primary))
                                       .arg(base_class_entry->name()));
                return false;
            }
            primaries++;
            primary = i;
        }
    }

    if (primary >= 0) {
        AbstractMetaClass *base_class = m_meta_classes.findClass(base_classes.at(primary));
        if (!base_class) {
            if(!meta_class->isTypeAlias()){
                ReportHandler::warning(QString("unknown baseclass for '%1': '%2'")
                                       .arg(meta_class->name())
                                       .arg(base_classes.at(primary)));
            }
            return false;
        }
        if(base_class->isInterface()){
            setupInheritance(const_cast<AbstractMetaClass *>(base_class->extractInterfaceImpl()));
            if(!meta_class->interfaces().contains(base_class))
                meta_class->addInterface(base_class);
        }else{
            setupInheritance(base_class);
            meta_class->setBaseClass(base_class);
            if(meta_class->typeEntry()->designatedInterface()
                    && meta_class->extractInterface()
                    && base_class->typeEntry()->designatedInterface()
                    && base_class->extractInterface()){
                if(!meta_class->extractInterface()->interfaces().contains(base_class->extractInterface()))
                    meta_class->extractInterface()->addInterface(base_class->extractInterface());
            }
        }

        /*
        if(base_class->typeEntry()->codeGeneration()==TypeEntry::GenerateForSubclass){
            inheritHiddenBaseType(meta_class, base_class, TypeParser::Info());
        }*/

        if (meta_class->typeEntry()->designatedInterface() && meta_class->isQObject()) {
            ReportHandler::warning(QString("QObject extended by interface type '%1'. This is not supported and the generated Java code will not compile.")
                                   .arg(meta_class->name()));
        } else if (meta_class->typeEntry()->designatedInterface() && base_class && !base_class->isInterface()) {
            ReportHandler::warning(QString("object type '%1' extended by interface type '%2'. The resulting API will be less expressive than the original.")
                                   .arg(base_class->name())
                                   .arg(meta_class->name()));
        }

    }

    for (int i = 0; i < base_classes.size(); ++i) {
        if (types->isClassRejected(base_classes.at(i)))
            continue;

        if (i != primary) {
            AbstractMetaClass *base_class = m_meta_classes.findClass(base_classes.at(i), AbstractMetaClassList::QualifiedCppName);
            if (!base_class) {
                ReportHandler::warning(QString("class not found for setup inheritance '%1'").arg(base_classes.at(i)));
                continue;
            }

            if(base_class->isInterface()){
                setupInheritance(const_cast<AbstractMetaClass *>(base_class->extractInterfaceImpl()));
                if(!meta_class->interfaces().contains(base_class)){
                    meta_class->addInterface(base_class);
                }
                QString impl_name = InterfaceTypeEntry::implName(base_class->fullName()).replace("::", "$");
                AbstractMetaClass *ifaceImpl = m_meta_classes.findClass(impl_name, AbstractMetaClassList::FullName);
                if (!ifaceImpl) {
                    ReportHandler::warning(QString("unknown implementation for interface '%1': '%2'")
                                           .arg(meta_class->name())
                                           .arg(impl_name));
                    continue;
                }
                for(AbstractMetaClass *iface : ifaceImpl->interfaces()){
                    if(iface->isInterface()){
                        if(!meta_class->interfaces().contains(iface))
                            meta_class->addInterface(iface);
                    }else{
                        ReportHandler::warning(QString("inherited interface '%1' for class '%2' is not an interface type: '%3'")
                                               .arg(base_class->fullName())
                                               .arg(meta_class->name())
                                               .arg(iface->fullName()));
                    }
                }
            }else{
                setupInheritance(base_class);
                if(meta_class->typeEntry()->designatedInterface()
                        && meta_class->extractInterface()
                        && base_class->typeEntry()->designatedInterface()
                        && base_class->extractInterface()){
                    if(!meta_class->extractInterface()->interfaces().contains(base_class->extractInterface())){
                        meta_class->extractInterface()->addInterface(base_class->extractInterface());
                    }
                }
            }
        }
    }

    return true;
}

AbstractMetaClass * AbstractMetaBuilder::instantiateIterator(IteratorTypeEntry *iteratorTypeEntry, AbstractMetaClass *targetClass, const QList<const AbstractMetaType *>& template_types, const QMap<const TypeEntry *,const AbstractMetaType *>& template_types_by_name){
    Q_UNUSED(template_types_by_name)
    AbstractMetaClass * instantiationClass = nullptr;
    AbstractMetaClass *iteratorClass = nullptr;
    for(AbstractMetaClass *c : m_template_iterators) {
        if (c->typeEntry() == iteratorTypeEntry) {
            iteratorClass = c;
            break;
        }
    }
    if(iteratorClass){
        {
            instantiationClass = m_meta_classes.findClass(targetClass->qualifiedCppName()+"::"+iteratorTypeEntry->name());
            if(!instantiationClass){
                instantiationClass = m_templates.findClass(targetClass->qualifiedCppName()+"::"+iteratorTypeEntry->name());
            }
        }
        if(!instantiationClass){
            for(const Include& inc : iteratorTypeEntry->extraIncludes()){
                targetClass->typeEntry()->addExtraInclude(inc);
            }
            IteratorTypeEntry * newIteratorTypeEntry = iteratorTypeEntry->clone(targetClass->typeEntry(), targetClass->baseClassNames().at(0));
            TypeDatabase::instance()->addType(newIteratorTypeEntry);
            instantiationClass = createMetaClass();
            instantiationClass->setTypeEntry(newIteratorTypeEntry);
            instantiationClass->setAttributes(iteratorClass->attributes());
            instantiationClass->setOriginalAttributes(iteratorClass->originalAttributes());
            instantiationClass->setHasCloneOperator(iteratorClass->hasCloneOperator());
            instantiationClass->setHasEqualsOperator(iteratorClass->hasEqualsOperator());
            instantiationClass->setHasHashFunction(iteratorClass->hasHashFunction());
            instantiationClass->setHasJustPrivateConstructors(iteratorClass->hasJustPrivateConstructors());
            instantiationClass->setHasPrivateDestructor(iteratorClass->hasPrivateDestructor());
            instantiationClass->setHasVirtualDestructor(iteratorClass->hasVirtualDestructor());
            instantiationClass->setUnimplmentablePureVirtualFunctions(iteratorClass->unimplmentablePureVirtualFunctions());
            instantiationClass->setHasPublicDestructor(iteratorClass->hasPublicDestructor());
            instantiationClass->setToStringCapability(iteratorClass->toStringCapability());
            instantiationClass->setHasVirtuals(iteratorClass->hasVirtuals());
            instantiationClass->setHasVirtualSlots(iteratorClass->hasVirtualSlots());
            instantiationClass->setHas_Q_GADGET(iteratorClass->has_Q_GADGET());
            instantiationClass->setHas_Q_OBJECT(iteratorClass->has_Q_OBJECT());
            instantiationClass->setTemplateBaseClass(iteratorClass);
            instantiationClass->setTemplateBaseClassInstantiations(template_types);
            instantiationClass->setEnclosingClass(targetClass);
            targetClass->addEnclosedClass(instantiationClass);
            addAbstractMetaClass(instantiationClass);
        }
    }
    return instantiationClass;
}

void AbstractMetaBuilder::traverseEnums(ScopeModelItem scope_item, AbstractMetaClass *meta_class, const QStringList &enumsDeclarations) {
    EnumList enums = scope_item->enums();
    for(const EnumModelItem& enum_item : enums) {
        AbstractMetaEnum *meta_enum = traverseEnum(enum_item, meta_class, QSet<QString>(enumsDeclarations.begin(), enumsDeclarations.end()));
        if (meta_enum) {
            meta_enum->setOriginalAttributes(meta_enum->attributes());
            meta_class->addEnum(meta_enum);
            meta_enum->setEnclosingClass(meta_class);
        }
    }
}

AbstractMetaFunction *AbstractMetaBuilder::traverseFunction(FunctionModelItem function_item) {
    const ArgumentList& arguments = function_item->arguments();
    QString function_name = function_item->name();
    QString class_name = m_current_class->typeEntry()->qualifiedCppName();

    QString _function_name = function_name;
    _function_name += "(";
    int counter = 0;
    for(ArgumentModelItem arg : arguments){
        if(arg->type().getReferenceType()==TypeInfo::NoReference
            && arg->type().indirections().isEmpty()
            && arguments.size()==1
            && arg->type().toString()=="void"){
            continue;
        }
        if(counter!=0){
            _function_name += ", ";
        }
        if(arg->type().isConstant())
            _function_name += "const ";
        _function_name += arg->type().qualifiedName().join("::");
        for(bool i : arg->type().indirections()){
            if(i)
                _function_name += " const";
            _function_name += "*";
        }
        if(arg->type().getReferenceType()==TypeInfo::Reference)
            _function_name += "&";
        ++counter;
    }
    _function_name += ") -> ";
    if(function_item->type().isConstant())
        _function_name += "const ";
    _function_name += function_item->type().qualifiedName().join("::");
    for(bool i : function_item->type().indirections()){
        if(i)
            _function_name += " const";
        _function_name += "*";
    }
    if(function_item->type().getReferenceType()==TypeInfo::Reference)
        _function_name += "&";

    if (TypeDatabase::instance()->isFunctionRejected(class_name, function_name)) {
        m_rejected_functions.insert({class_name + "::" + _function_name, function_item->fileName()}, GenerationDisabled);
        return nullptr;
    }


    Q_ASSERT(function_item->functionType() == CodeModel::Normal
             || function_item->functionType() == CodeModel::Signal
             || function_item->functionType() == CodeModel::PrivateSignal
             || function_item->functionType() == CodeModel::Slot);

    if (function_item->isFriend())
        return nullptr;

    QString cast_type;

    if (function_name.startsWith("operator")) {
        function_name = rename_operator(function_name.mid(8));
        if (function_name.isEmpty()) {
            m_rejected_functions.insert({class_name + "::" + _function_name, function_item->fileName()},
                                        GenerationDisabled);
            return nullptr;
        }
        if (function_name.startsWith("operator_cast_")){
            cast_type = function_name.mid(14).trimmed();
            function_name = function_name.replace(" ", "_").replace("*", "_ast").replace("&", "_amp").replace("::", "_");
        }
    }

    AbstractMetaFunction *meta_function = createMetaFunction();
    meta_function->setConstant(function_item->isConstant());

    ReportHandler::debugMedium(QString(" - %2()").arg(function_name));
    meta_function->setName(function_name);
    meta_function->setOriginalName(function_item->name());

    if (function_item->isAbstract())
        *meta_function += AbstractMetaAttributes::Abstract;

    if (!meta_function->isAbstract())
        *meta_function += AbstractMetaAttributes::Native;

    if (!function_item->isVirtual())
        *meta_function += AbstractMetaAttributes::Final;

    if(function_item->isDeclFinal()){
        *meta_function += AbstractMetaAttributes::Final;
        *meta_function += AbstractMetaAttributes::DeclaredFinalInCpp;
    }

    if(function_item->isDeprecated()){
        *meta_function += AbstractMetaAttributes::Deprecated;
        meta_function->setDeprecatedComment(function_item->getDeprecatedComment());
    }

    if (function_item->isInvokable())
        *meta_function += AbstractMetaAttributes::Invokable;

    if (function_item->isStatic()) {
        *meta_function += AbstractMetaAttributes::Static;
        *meta_function += AbstractMetaAttributes::Final;
    }

    // Access rights
    if (function_item->accessPolicy() == CodeModel::Public)
        *meta_function += AbstractMetaAttributes::Public;
    else if (function_item->accessPolicy() == CodeModel::Private)
        *meta_function += AbstractMetaAttributes::Private;
    else
        *meta_function += AbstractMetaAttributes::Protected;
    QHash<QString,AbstractMetaTemplateParameter*> metaTemplateParameters;
    int k=0;
    for(const TemplateParameterModelItem& p : function_item->templateParameters()){
        if(!p->ownerClass()){
            TemplateArgumentEntry* tae = new TemplateArgumentEntry(p->name());
            tae->setOrdinal(k);
            AbstractMetaType *type = createMetaType();
            type->setTypeUsagePattern(AbstractMetaType::TemplateArgumentPattern);
            type->setTypeEntry(tae);
            k++;
            AbstractMetaTemplateParameter* tp = new AbstractMetaTemplateParameter();
            tp->setName(p->name());
            tp->setType(type);
            metaTemplateParameters[p->name()] = tp;
            meta_function->addTemplateParameter(tp);
        }
    }

    QString stripped_class_name = class_name;
    int cc_pos = stripped_class_name.lastIndexOf("::");
    if (cc_pos > 0)
        stripped_class_name = stripped_class_name.mid(cc_pos + 2);

    TypeInfo function_type = function_item->type();
    if (function_name.startsWith('~')) {
        meta_function->setFunctionType(AbstractMetaFunction::DestructorFunction);
        meta_function->setInvalid(true);
    } else if (strip_template_args(function_name) == stripped_class_name) {
        meta_function->setFunctionType(AbstractMetaFunction::ConstructorFunction);
        meta_function->setName(m_current_class->simpleName());
    } else {
        bool ok = false;
        AbstractMetaType *type = nullptr;

        if(metaTemplateParameters.contains(function_type.qualifiedName().join("::"))){
            type = metaTemplateParameters[function_type.qualifiedName().join("::")]->type()->copy();
            type->setIndirections(function_type.indirections());
            type->setConstant(function_type.isConstant());
            type->setReferenceType(AbstractMetaType::ReferenceType(function_type.getReferenceType()));
            ok = true;
        }

        if(!ok){
            if (!cast_type.isEmpty()) {
                TypeInfo info;
                info.setQualifiedName(QStringList(cast_type));
                type = translateType(info, &ok, QString("traverseField %1.%2").arg(class_name).arg(function_name));
            } else {
                type = translateType(function_type, &ok, QString("traverseField %1.%2").arg(class_name).arg(function_name));
            }
        }

        if (!ok) {
            ReportHandler::warning(QString("skipping function '%1::%2', unmatched return type '%3'")
                                   .arg(class_name)
                                   .arg(function_item->name())
                                   .arg(function_item->type().toString()));
            m_rejected_functions[{class_name + "::" + _function_name, function_item->fileName()}] =
                UnmatchedReturnType;
            meta_function->setInvalid(true);
            return meta_function;
        }
        meta_function->setType(type);

        if (function_item->functionType() == CodeModel::Signal)
            meta_function->setFunctionType(AbstractMetaFunction::SignalFunction);
        else if (function_item->functionType() == CodeModel::PrivateSignal)
            meta_function->setFunctionType(AbstractMetaFunction::PrivateSignalFunction);
        else if (function_item->functionType() == CodeModel::Slot)
            meta_function->setFunctionType(AbstractMetaFunction::SlotFunction);
    }

    AbstractMetaArgumentList meta_arguments;

    QString originalSignature = function_item->name();
    originalSignature += "(";

    int first_default_argument = 0;
    for (int i = 0; i < arguments.size(); ++i) {
        ArgumentModelItem arg = arguments.at(i);
        bool ok = false;
        AbstractMetaType *meta_type = nullptr;
        if(arg->type().getReferenceType()==TypeInfo::NoReference
                && arg->type().indirections().isEmpty()
                && arguments.size()==1
                && arg->type().toString()=="void"){
            //originalSignature += "void";
            continue;
        }
        if(i>0)
            originalSignature += ",";
        if(arg->type().isConstant())
            originalSignature += "const ";
        originalSignature += arg->type().qualifiedName().join("::");
        for(bool ind : arg->type().indirections()){
            if(ind)
                originalSignature += " const";
            originalSignature += "*";
        }
        if(arg->type().getReferenceType()==TypeInfo::Reference)
            originalSignature += "&";
        if(arg->type().getReferenceType()==TypeInfo::RReference)
            originalSignature += "&&";

        if(metaTemplateParameters.contains(arg->type().qualifiedName().join("::"))){
            meta_type = metaTemplateParameters[arg->type().qualifiedName().join("::")]->type()->copy();
            meta_type->setIndirections(arg->type().indirections());
            meta_type->setConstant(arg->type().isConstant());
            meta_type->setReferenceType(AbstractMetaType::ReferenceType(arg->type().getReferenceType()));
            ok = true;
        }

        if(!ok){
            meta_type = translateType(arg->type(), &ok, QString("traverseFunction %1.%2 arg#%3").arg(class_name).arg(function_name).arg(i));
        }
        if (!meta_type || !ok) {
            ReportHandler::warning(QString("skipping function '%1::%2', "
                                           "unmatched parameter type '%3'")
                                   .arg(class_name)
                                   .arg(function_item->name())
                                   .arg(arg->type().toString()));
            if(arg->type().isVolatile())
                meta_type = translateType(arg->type(), &ok, QString("traverseFunction %1.%2 arg#%3").arg(class_name).arg(function_name).arg(i));
            m_rejected_functions[{class_name + "::" + _function_name, function_item->fileName()}] =
                UnmatchedArgumentType;
            meta_function->setInvalid(true);
            return meta_function;
        }
        if(m_current_class->typeEntry()->isValue() &&
                arguments.size()==1 &&
                meta_type->getReferenceType()==AbstractMetaType::Reference &&
                !meta_type->isConstant() &&
                meta_type->typeEntry()==m_current_class->typeEntry() &&
                meta_function->name()=="swap"){
            meta_type->setTypeUsagePattern(AbstractMetaType::ValuePattern);
            FunctionModification mod;
            mod.signature = meta_function->minimalSignature();
            mod.argument_mods << ArgumentModification(1);
            mod.argument_mods[0].no_null_pointers = true;
            m_current_class->typeEntry()->addFunctionModification(mod);
        }
        AbstractMetaArgument *meta_argument = createMetaArgument();
        meta_argument->setType(meta_type);
        meta_argument->setName(arg->name());
        meta_argument->setArgumentIndex(i);
        meta_arguments << meta_argument;
    }

    originalSignature += ")";
    if(function_item->isConstant()){
        originalSignature += "const";
    }
    meta_function->setOriginalSignature(QLatin1String(QMetaObject::normalizedSignature(qPrintable(originalSignature)).data()));

    meta_function->setArguments(meta_arguments);

    for(ArgumentModification argumentModification : m_current_class->typeEntry()->functionArgumentModifications(meta_function->originalSignature())){
        if(!argumentModification.modified_name.isEmpty() && argumentModification.index>0 && argumentModification.index<=meta_arguments.size()){
            meta_arguments[argumentModification.index-1]->setModifiedName(argumentModification.modified_name);
        }
    }

    // Find the correct default values
    for (int i = 0; i < arguments.size(); ++i) {
        ArgumentModelItem arg = arguments.at(i);
        if(arg->type().toString()=="void"
                && arg->type().getReferenceType()==TypeInfo::NoReference
                && arg->type().indirections().isEmpty()
                && arguments.size()==1){
            continue;
        }
        AbstractMetaArgument *meta_arg = meta_arguments.at(i);
        if (arg->defaultValue()) {
            QString expr = arg->defaultValueExpression();
            if (!expr.isEmpty())
                meta_arg->setOriginalDefaultValueExpression(expr);

            expr = translateDefaultValue(arg->defaultValueExpression(), meta_arg->type(), meta_function, m_current_class, i);
            if (expr.isEmpty()) {
                first_default_argument = i;
            } else {
                meta_arg->setDefaultValueExpression(expr);
            }

            if (meta_arg->type()->isEnum() || meta_arg->type()->isFlags()) {
                m_enum_default_arguments << QPair<AbstractMetaArgument *, AbstractMetaFunction *>(meta_arg, meta_function);
            }

        /*}else{
            QString inserted_default_expression = meta_function->replacedDefaultExpression(m_current_class, meta_arg->argumentIndex() + 1);
            if(!inserted_default_expression.isEmpty()){
                meta_arg->setDefaultValueExpression(inserted_default_expression);
            }*/
        }
    }

    // If we where not able to translate the default argument make it
    // reset all default arguments before this one too.
    for (int i = 0; i < first_default_argument; ++i)
        meta_arguments[i]->setDefaultValueExpression(QString());

    if (ReportHandler::debugLevel() == ReportHandler::FullDebug) {
        for(AbstractMetaArgument *arg : meta_arguments)
            ReportHandler::debugFull("   - " + arg->toString());
    }

    return meta_function;
}


// uncomment the qDebug()s in order to inspect internals of the function
AbstractMetaType *AbstractMetaBuilder::translateType(const TypeInfo& type_info,
        bool *ok,
        const QString &contextString,
        bool resolveType,
        bool resolveScope,
        bool prependScope) {
    Q_ASSERT(ok);
    *ok = true;

    if(prependScope && !TypeDatabase::instance()->isUniqueClassName(type_info.qualifiedName().join("::"))){
        // the generator must distinguish between QTransform (QtCore) and QTransform (Qt3DCore)
        // the generator must distinguish between QCamera (QtMultimedia) and QCamera (Qt3DCore)
        QStringList scopes;
        if(m_current_class){
            scopes = m_current_class->qualifiedCppName().split("::");
        }else{
            scopes = currentScope()->qualifiedName();
        }
        while(!scopes.isEmpty()){
            TypeInfo tmp_type_info = type_info;
            QStringList qualifiedName = tmp_type_info.qualifiedName();
            if(!qualifiedName.join("::").startsWith(scopes[0])){
                for(int i=scopes.size()-1; i>=0; i--){
                    qualifiedName.insert(0, scopes[i]);
                }
                tmp_type_info.setQualifiedName(qualifiedName);

                bool _ok;
                AbstractMetaType * result = translateType(tmp_type_info, &_ok, contextString, resolveType, resolveScope, false);
                if(result && _ok){
                    return result;
                }
            }
            scopes.removeLast();
        }
    }
    //qDebug()<<"Start of translateType()"<<type_info.toString();

    // 1. Test the type info without resolving typedefs in case this is present in the
    //    type system

    TypeInfo typei;
    if (resolveType) {
        bool ok;
        AbstractMetaType *t = translateType(type_info, &ok, contextString, false, resolveScope, prependScope);
        if (t && ok) {
            //qDebug()<<"ResolveType ok (1. check)"<<t->fullName();
            return t;
        }
    }

    if (!resolveType)
        typei = type_info;
    else {
        // Go through all parts of the current scope (including global namespace)
        // to resolve typedefs. The parser does not properly resolve typedefs in
        // the global scope when they are referenced from inside a namespace.
        // This is a work around to fix this bug since fixing it in resolveType
        // seemed non-trivial
        int i = m_scopes.size() - 1;
        while (i >= 0) {
            typei = TypeInfo::resolveType(type_info, m_scopes.at(i--)->toItem());
            if (typei.qualifiedName().join("::") != type_info.qualifiedName().join("::"))
                break;
        }
        //qDebug()<<"Resolved:"<<typei.toString();
    }

    if (typei.isFunctionPointer()) {
        *ok = false;
        if(prependScope)
            ReportHandler::warning("isFunctionPointer: "+type_info.toString());
        return nullptr;
    }

    TypeParser::Info typeInfo = TypeParser::parse(typei.toString());

    if (typeInfo.is_busted) {
        if(m_current_class){
            QString qualifiedName = typei.qualifiedName().join("::");
            AbstractMetaFunctional* functional = m_current_class->functionalByUsing()[qualifiedName];
            if(!functional){
                if(FunctionalTypeEntry * entry = TypeDatabase::instance()->findFunctionalTypeByUsing(m_current_class->qualifiedCppName(), qualifiedName)){
                    functional = findFunctional(m_current_class, entry);
                }
            }
            if(functional){
                AbstractMetaType *meta_type = createMetaType();
                meta_type->setTypeEntry(functional->typeEntry());
                meta_type->setIndirections(typei.indirections());
                meta_type->setReferenceType(AbstractMetaType::ReferenceType(typei.getReferenceType()));
                meta_type->setConstant(typei.isConstant());
                meta_type->setOriginalTypeDescription(type_info.toString());
                decideUsagePattern(meta_type);
                return meta_type;
            }
        }
        *ok = false;
        if(prependScope)
            ReportHandler::warning("Type parser doesn't recognize the type " + typei.toString() + " (is_busted)");
        return nullptr;
    }

    //qDebug()<<"TypeInfo is after 1. method:"<<typeInfo.toString();

    // 2. Handle pointers specified as arrays with unspecified size

    bool array_of_unspecified_size = false;
    if (typeInfo.arrays.size() > 0) {
        array_of_unspecified_size = true;
        for (int i = 0; i < typeInfo.arrays.size(); ++i)
            array_of_unspecified_size = array_of_unspecified_size && typeInfo.arrays.at(i).isEmpty();

        if (!array_of_unspecified_size) {
            TypeInfo newInfo;
            //newInfo.setArguments(typei.arguments());
            newInfo.setIndirections(typei.indirections());
            newInfo.setConstant(typei.isConstant());
            newInfo.setFunctionPointer(typei.isFunctionPointer());
            newInfo.setQualifiedName(typei.qualifiedName());
            newInfo.setReferenceType(typei.getReferenceType());
            newInfo.setVolatile(typei.isVolatile());

            AbstractMetaType *elementType = translateType(newInfo, ok, contextString, true, true, prependScope);
            if (!elementType || !ok) {
                if(prependScope)
                    ReportHandler::warning("Something has happened when trying to resolve type " + newInfo.toString());
                return nullptr;
            }

            for (int i = typeInfo.arrays.size() - 1; i >= 0; --i) {
                QString s = typeInfo.arrays.at(i);
                bool ok;

                int elems = s.toInt(&ok);
                if (!ok) {
                    if(prependScope)
                        ReportHandler::warning("Something has happened when trying to read array initialization: " + s);
                    return nullptr;
                }

                AbstractMetaType *arrayType = createMetaType();
                arrayType->setArrayElementCount(elems);
                arrayType->setArrayElementType(elementType);
                arrayType->setTypeEntry(new ArrayTypeEntry(elementType->typeEntry(), elementType->indirections().size()));
                decideUsagePattern(arrayType);

                elementType = arrayType;
            }

            //qDebug()<<"Returning element type:"<<elementType->fullName();
            return elementType;
        }  else {
            for(int i=0; i<typeInfo.arrays.size(); i++){
                typeInfo.indirections << false;
            }
        }
    }

    QStringList qualifier_list = typeInfo.qualified_name;
    if (qualifier_list.isEmpty()) {
        if(prependScope)
            ReportHandler::warning(QString("horribly broken type '%1'").arg(type_info.toString()));
        *ok = false;
        return nullptr;
    }

    QString qualified_name = qualifier_list.join("::");
    QString name = qualifier_list.takeLast();

    // 3. Special case 'void' type
    if (name == "void" && typeInfo.indirections.size() == 0) {
        //if(prependScope) qDebug()<<"Returning from name == \"void\" && typeInfo.indirections == 0";
        return nullptr;
    }

    // 4. Special case QFlags (include instantiation in name)
    if (qualified_name == "QFlags")
        qualified_name = typeInfo.toString();

    // 5. Try to find the type
    const TypeEntry *type = TypeDatabase::instance()->findType(qualified_name);

    //if (type)
        //qDebug()<<"After 5. name: " << type->name();

        // 6. No? Try looking it up as a flags type
        if (!type)
            type = TypeDatabase::instance()->findFlagsType(qualified_name);

    // 7. No? Try looking it up as a container type
    if (!type)
        type = TypeDatabase::instance()->findContainerType(name);

    // 8. No? Try looking it up as a pointer container type
    if (!type)
        type = TypeDatabase::instance()->findPointerContainerType(name);

    // 8.1. No? Try looking it up as a initializer list type
    if (!type)
        type = TypeDatabase::instance()->findInitializerListType(name);

    // 8.2. No? Check if the current class is a template and this type is one
    //    of the parameters.
    if (!type && m_current_class) {
        QList<TypeEntry *> template_args = m_current_class->templateArguments();
        for(TypeEntry *te : template_args) {
            if (te->name() == qualified_name)
                type = te;
        }
    }

    ClassModelItem containing_class = m_dom->findClass(qualifier_list.join("::"));

    // 9. Try finding the type by prefixing it with
    //     all baseclasses of the containing class
    if (!type && !TypeDatabase::instance()->isClassRejected(qualified_name) && containing_class && resolveScope) {
        QStringList contexts;
        contexts.append(containing_class->baseClasses());

        // qDebug() << "9."<< contexts;

        TypeInfo info = typei;
        while (!contexts.isEmpty() && !type) {
            //type = TypeDatabase::instance()->findType(contexts.at(0) + "::" + qualified_name);

            bool ok;
            info.setQualifiedName(QStringList() << contexts.at(0) << name);
            //   qDebug()<< "whiling in 9. type," << info.toString();
            AbstractMetaType *t = translateType(info, &ok, contextString, true, false, prependScope);
            if (t && ok) {
                //     qDebug()<<"Returning ok from t != 0 && ok"<<t->fullName();
                return t;
            }

            //10. Try if the type is in a base class
            //e.g. MimeType is QWebPluginFactory::MimeType and called by QWebPluginFactory::Plugin
            QString base = contexts.at(0);
            QStringList parts = base.split("::");
            while (parts.size() > 1) {
                parts.removeLast();
                info.setQualifiedName(QStringList() << parts << name);
                AbstractMetaType *t = translateType(info, &ok, contextString, true, false, prependScope);
                if (t && ok) {
                    //       qDebug()<<"Returning ok from 11. method"<<t->fullName();
                    return t;
                }
            }

            ClassModelItem item = m_dom->findClass(contexts.at(0));
            if (item) {
                contexts += item->baseClasses();
            }else{
                AbstractMetaClass* cls = m_meta_classes.findClass(contexts.at(0));
                if(cls &&
                    !cls->baseClassNames().isEmpty() &&
                    !cls->baseClassNames()[0].isEmpty()){
                    contexts.insert(1, cls->baseClassNames()[0]);
                }
            }
            contexts.removeFirst();
        }
    }

    // 12. Try finding the type by prefixing it with the current
    //    context and all baseclasses of the current context
    if (!type && !TypeDatabase::instance()->isClassRejected(qualified_name) && m_current_class && resolveScope) {
        QStringList contexts;
        contexts.append(m_current_class->qualifiedCppName());
        if(!m_current_class->baseClassNames().isEmpty() &&
                !m_current_class->baseClassNames()[0].isEmpty()){
            contexts.append(m_current_class->baseClassNames()[0]);
        }
        contexts.append(currentScope()->qualifiedName().join("::"));

        // qDebug() << "9."<< contexts;

        AbstractMetaClass *current_class = m_current_class;
        TypeInfo info = typei;
        bool subclasses_done = false;
        while (!contexts.isEmpty() && !type) {
            //type = TypeDatabase::instance()->findType(contexts.at(0) + "::" + qualified_name);

            bool ok;
            info.setQualifiedName(QStringList() << contexts.at(0) << qualified_name);
            //   qDebug()<< "whiling in 9. type," << info.toString();
            AbstractMetaType *t = translateType(info, &ok, contextString, true, false, prependScope);
            if (t && ok) {
                //     qDebug()<<"Returning ok from t != 0 && ok"<<t->fullName();
                return t;
            }

            //13. Try if the type is in a base class
            //e.g. MimeType is QWebPluginFactory::MimeType and called by QWebPluginFactory::Plugin
            QString base = contexts.at(0);
            QStringList parts = base.split("::");
            while (parts.size() > 1) {
                parts.removeLast();
                info.setQualifiedName(QStringList() << parts << qualified_name);
                AbstractMetaType *t = translateType(info, &ok, contextString, true, false, prependScope);
                if (t && ok) {
                    //       qDebug()<<"Returning ok from 11. method"<<t->fullName();
                    return t;
                }
            }

            ClassModelItem item = m_dom->findClass(contexts.at(0));
            if (item) {
                contexts += item->baseClasses();
            }else if(current_class &&
                     !current_class->baseClassNames().isEmpty() &&
                     !current_class->baseClassNames()[0].isEmpty()){
                contexts.insert(1, current_class->baseClassNames()[0]);
                current_class = m_meta_classes.findClass(current_class->baseClassNames()[0]);
            }
            contexts.removeFirst();

            // 14. Last resort: special cased prefix of Qt namespace since the meta object implicitly inherits this, so
            //     enum types from there may be addressed without any scope resolution in properties.
            if (contexts.size() == 0 && !subclasses_done) {
                contexts << "Qt";
                subclasses_done = true;
            }
        }
    }

    /*{
        qualifier_list = typeInfo.qualified_name;
        qualified_name = qualifier_list.join("::");
        name = qualifier_list.takeLast();
        TypeAliasModelItem typeAliasModel = m_dom->findTypeAlias(qualified_name);
        if(!typeAliasModel){
            ScopeModelItem smi = m_dom->toScope();
            QStringList qualifierList = qualifier_list;
            while(smi && !qualifierList.isEmpty()){
                QString scope = qualifierList.takeFirst();
                if(ClassModelItem cls = smi->findClass(scope)){
                    smi = cls;
                }else if(NamespaceModelItem nms = smi->findNamespace(scope)){
                    smi = nms;
                }else{
                    smi = nullptr;
                }
            }
            if(smi){
                typeAliasModel = smi->findTypeAlias(name);
            }
        }
        if(typeAliasModel){
            bool ok;
            typei.setQualifiedName(typeAliasModel->type().qualifiedName());
            AbstractMetaType *t = translateType(typei, &ok, contextString, false, resolveScope, prependScope);
            if (t && ok) {
                //qDebug()<<"ResolveType ok (1. check)"<<t->fullName();
                return t;
            }
        }
    }*/

    // if error happened in type resolving
    if (!type) {
        *ok = false;
        //   qDebug()<<"Every type checking methods failed, typei:"<<typei.toString()<<", type_info:"<<type_info.toString();
        return nullptr;
    }

    //qDebug()<<"A type found:"<<type->name();

    // Used to for diagnostics later
    m_used_types << type;

    // These are only implicit and should not appear in code
    if(type->isInterface()){
        type = static_cast<const InterfaceTypeEntry*>(type)->origin();
    }
    Q_ASSERT(!type->isInterface());

    AbstractMetaType *meta_type = createMetaType();
    meta_type->setTypeEntry(type);
    meta_type->setIndirections(typeInfo.indirections);
    meta_type->setReferenceType(AbstractMetaType::ReferenceType(typeInfo.reference_type));
    meta_type->setConstant(typeInfo.is_constant);
    meta_type->setVolatile(typeInfo.is_volatile);
    meta_type->setOriginalTypeDescription(type_info.toString());
    decideUsagePattern(meta_type);

    if (meta_type->typeEntry()->isInitializerList()) {
        Q_ASSERT(typeInfo.template_instantiations.size() == 1);

        const TypeParser::Info &ta = typeInfo.template_instantiations[0];
        TypeInfo info;
        info.setConstant(ta.is_constant);
        info.setVolatile(ta.is_volatile);
        info.setReferenceType(TypeInfo::ReferenceType(ta.reference_type));
        info.setIndirections(ta.indirections);

        info.setFunctionPointer(false);
        QString qualifiedName = ta.instantiationName();
        info.setIndirections(info.indirections());
        info.setQualifiedName(qualifiedName.split("::"));

        AbstractMetaType *targ_type = translateType(info, ok, contextString);
        if (!(*ok)) {
            delete meta_type;
            return nullptr;
        }
        meta_type->addInstantiation(targ_type);

    }else if (meta_type->typeEntry()->isPointerContainer()) {
        PointerContainerTypeEntry::Type pointer_container_type = static_cast<const PointerContainerTypeEntry *>(type)->type();
        if (    pointer_container_type == PointerContainerTypeEntry::QSharedPointer
             || pointer_container_type == PointerContainerTypeEntry::QWeakPointer
             || pointer_container_type == PointerContainerTypeEntry::QScopedPointer
             || pointer_container_type == PointerContainerTypeEntry::unique_ptr
             || pointer_container_type == PointerContainerTypeEntry::shared_ptr
             || pointer_container_type == PointerContainerTypeEntry::weak_ptr ) {
            Q_ASSERT(typeInfo.template_instantiations.size() >= 1);
            const TypeParser::Info &ta = typeInfo.template_instantiations[0];
            TypeInfo info;
            info.setConstant(ta.is_constant);
            info.setVolatile(ta.is_volatile);
            info.setReferenceType(TypeInfo::ReferenceType(ta.reference_type));
            info.setIndirections(ta.indirections);

            info.setFunctionPointer(false);
            QString qualifiedName = ta.instantiationName();
            QList<bool> ic = info.indirections();
            if(!ic.isEmpty())ic.removeAt(0);
            info.setIndirections(ic);
            info.setQualifiedName(qualifiedName.split("::"));

            //    qDebug()<< "Foreaching in container thingy," << info.toString();
            AbstractMetaType *targ_type = translateType(info, ok, contextString);
            if (!(*ok)) {
                delete meta_type;
                return nullptr;
            }
            meta_type->addInstantiation(targ_type);
        }
    }else if (meta_type->typeEntry()->isContainer()) {
        //   qDebug()<<"The type is a container, descending...";
        ContainerTypeEntry::Type container_type = static_cast<const ContainerTypeEntry *>(type)->type();

        if (container_type == ContainerTypeEntry::StringListContainer) {
            TypeInfo info;
            info.setQualifiedName(QStringList() << "QString");
            AbstractMetaType *targ_type = translateType(info, ok, contextString);

            Q_ASSERT(*ok);
            Q_ASSERT(targ_type);

            meta_type->addInstantiation(targ_type);
            meta_type->setInstantiationInCpp(false);

        }else if (container_type == ContainerTypeEntry::ByteArrayListContainer) {
                TypeInfo info;
                info.setQualifiedName(QStringList() << "QByteArray");
                AbstractMetaType *targ_type = translateType(info, ok, contextString);

                Q_ASSERT(*ok);
                Q_ASSERT(targ_type);

                meta_type->addInstantiation(targ_type);
                meta_type->setInstantiationInCpp(false);

        } else if (container_type == ContainerTypeEntry::QVector2DArrayContainer) {
            TypeInfo info;
            info.setQualifiedName(QStringList() << "QVector2D");
            AbstractMetaType *targ_type = translateType(info, ok, "");

            Q_ASSERT(*ok);
            Q_ASSERT(targ_type);

            meta_type->addInstantiation(targ_type);
            meta_type->setInstantiationInCpp(false);

        } else if (container_type == ContainerTypeEntry::QVector3DArrayContainer) {
            TypeInfo info;
            info.setQualifiedName(QStringList() << "QVector3D");
            AbstractMetaType *targ_type = translateType(info, ok, "");

            Q_ASSERT(*ok);
            Q_ASSERT(targ_type);

            meta_type->addInstantiation(targ_type);
            meta_type->setInstantiationInCpp(false);

        } else if (container_type == ContainerTypeEntry::QVector4DArrayContainer) {
            TypeInfo info;
            info.setQualifiedName(QStringList() << "QVector4D");
            AbstractMetaType *targ_type = translateType(info, ok, "");

            Q_ASSERT(*ok);
            Q_ASSERT(targ_type);

            meta_type->addInstantiation(targ_type);
            meta_type->setInstantiationInCpp(false);

        } else {
            for(const TypeParser::Info &ta : typeInfo.template_instantiations) {
                TypeInfo info;
                info.setConstant(ta.is_constant);
                info.setVolatile(ta.is_volatile);
                info.setReferenceType(TypeInfo::ReferenceType(ta.reference_type));
                info.setIndirections(ta.indirections);

                info.setFunctionPointer(false);
                QString qualifiedName = ta.instantiationName();
                if (container_type == ContainerTypeEntry::QQmlListPropertyContainer || container_type == ContainerTypeEntry::QDeclarativeListPropertyContainer) {
                    QList<bool> ic = info.indirections();
                    ic << false;
                    info.setIndirections(ic);
                }
                info.setQualifiedName(qualifiedName.split("::"));

                //    qDebug()<< "Foreaching in container thingy," << info.toString();
                AbstractMetaType *targ_type = translateType(info, ok, contextString);
                if (!(*ok)) {
                    delete meta_type;
                    return nullptr;
                }
                if(!targ_type && info.toString()=="void"){
                    const TypeEntry *type = TypeDatabase::instance()->findType("void");
                    if(type){
                        targ_type = new AbstractMetaType();
                        targ_type->setTypeUsagePattern(AbstractMetaType::PrimitivePattern);
                        targ_type->setTypeEntry(type);
                    }
                }
                if (container_type == ContainerTypeEntry::QQmlListPropertyContainer || container_type == ContainerTypeEntry::QDeclarativeListPropertyContainer) {
                    QList<bool> ic = targ_type->indirections();
                    if(!ic.isEmpty())ic.removeAt(0);
                    targ_type->setIndirections(ic);
                }
                meta_type->addInstantiation(targ_type);
            }
        }

        if (container_type == ContainerTypeEntry::ListContainer
                || container_type == ContainerTypeEntry::InitializerListContainer
                || container_type == ContainerTypeEntry::VectorContainer
                || container_type == ContainerTypeEntry::QDBusReplyContainer
                || container_type == ContainerTypeEntry::StringListContainer
                || container_type == ContainerTypeEntry::ByteArrayListContainer
                || container_type == ContainerTypeEntry::QVector2DArrayContainer
                || container_type == ContainerTypeEntry::QVector3DArrayContainer
                || container_type == ContainerTypeEntry::QVector4DArrayContainer) {
            if(meta_type->instantiations().size() == 0){
//                meta_type->
            }
//            Q_ASSERT(meta_type->instantiations().size() == 1);
        }else
            // the QArray type can have two template arguments
            if (container_type == ContainerTypeEntry::QArrayContainer) {
                Q_ASSERT(meta_type->instantiations().size() == 1 || meta_type->instantiations().size() == 2);
        }
    }else if (meta_type->typeEntry()->isIterator()) {
        const IteratorTypeEntry* iteratorType = static_cast<const IteratorTypeEntry*>(meta_type->typeEntry());
        bool found = false;
        if(AbstractMetaClass * iteratorClass = classes().findClass(iteratorType->qualifiedCppName(), AbstractMetaClassList::QualifiedCppName)){
            if(iteratorClass->typeAliasType()){
                AbstractMetaType* typeAliasType = iteratorClass->typeAliasType()->copy();
                QList<bool> indirections = typeAliasType->indirections();
                if(!indirections.isEmpty()){
                    indirections.takeFirst();
                    typeAliasType->setIndirections(indirections);
                }
                typeAliasType->setReferenceType(AbstractMetaType::Reference);
                AbstractMetaBuilder::decideUsagePattern(typeAliasType);
                meta_type->addIteratorInstantiation(typeAliasType);
                found = true;
            }
        }
        if(!found && m_current_class){
            if(AbstractMetaClass * iteratorClass = m_current_class->findIterator(iteratorType)){
                meta_type->setIteratorInstantiations(iteratorClass->templateBaseClassInstantiations());
                found = true;
            }
        }
        if(!found){
            m_missing_iterators << MissingIterator(iteratorType, meta_type, m_current_class);
        }
    }
    return meta_type;
}

void AbstractMetaBuilder::fixMissingIterator(){
    QList<MissingIterator> missing_iterators;
    for(const MissingIterator& missingIterator : m_missing_iterators){
        bool found = false;
        if(AbstractMetaClass * iteratorClass = classes().findClass(missingIterator.iteratorType->qualifiedCppName(), AbstractMetaClassList::QualifiedCppName)){
            if(iteratorClass->typeAliasType()){
                AbstractMetaType* typeAliasType = iteratorClass->typeAliasType()->copy();
                QList<bool> indirections = typeAliasType->indirections();
                if(!indirections.isEmpty()){
                    indirections.takeFirst();
                    typeAliasType->setIndirections(indirections);
                }
                typeAliasType->setReferenceType(AbstractMetaType::Reference);
                AbstractMetaBuilder::decideUsagePattern(typeAliasType);
                missingIterator.meta_type->addIteratorInstantiation(typeAliasType);
                found = true;
            }
        }
        if(!found && missingIterator.current_class){
            if(AbstractMetaClass * iteratorClass = missingIterator.current_class->findIterator(missingIterator.iteratorType)){
                missingIterator.meta_type->setIteratorInstantiations(iteratorClass->templateBaseClassInstantiations());
                found = true;
            }
            if(missingIterator.current_class->templateBaseClass()){
                if(missingIterator.iteratorType->containerType()==missingIterator.current_class->templateBaseClass()->typeEntry()){
                    missingIterator.meta_type->setIteratorInstantiations(missingIterator.current_class->templateBaseClassInstantiations());
                    IteratorTypeEntry* newIteratorType = missingIterator.iteratorType->clone(missingIterator.iteratorType->containerType(), missingIterator.current_class->baseClassNames().join("::"));
                    missingIterator.meta_type->setTypeEntry(newIteratorType);
                    found = true;
                }
            }
        }
        if(!found)
            missing_iterators << missingIterator;
    }
    m_missing_iterators = missing_iterators;
}

void AbstractMetaBuilder::decideUsagePattern(AbstractMetaType *meta_type) {
    const TypeEntry *type = meta_type->typeEntry();

    if(meta_type->getReferenceType()==AbstractMetaType::RReference){
        meta_type->setTypeUsagePattern(AbstractMetaType::RValuePattern);

    } else if (type->isPrimitive() && (meta_type->actualIndirections() == 0
                                || (meta_type->isConstant() && meta_type->getReferenceType()==AbstractMetaType::Reference && meta_type->indirections().size() == 0))) {
        meta_type->setTypeUsagePattern(AbstractMetaType::PrimitivePattern);

    } else if (type->isVoid()) {
        meta_type->setTypeUsagePattern(AbstractMetaType::NativePointerPattern);

    } else if (type->isFunctional()) {
        meta_type->setTypeUsagePattern(AbstractMetaType::FunctionalPattern);

    } else if (type->isStringRef()
               && meta_type->indirections().size() == 0
               && (meta_type->isConstant() == (meta_type->getReferenceType()==AbstractMetaType::Reference)
                   || meta_type->isConstant())) {
        meta_type->setTypeUsagePattern(AbstractMetaType::StringRefPattern);

    } else if (type->isString()
               && meta_type->indirections().size() == 0
               && (meta_type->isConstant() == (meta_type->getReferenceType()==AbstractMetaType::Reference)
                   || meta_type->isConstant())) {
        meta_type->setTypeUsagePattern(AbstractMetaType::StringPattern);

    } else if (type->isLatin1String()
               && meta_type->indirections().size() == 0
               && (meta_type->isConstant() == (meta_type->getReferenceType()==AbstractMetaType::Reference)
                   || meta_type->isConstant())) {
        meta_type->setTypeUsagePattern(AbstractMetaType::Latin1StringPattern);

    } else if (type->isStringView()
               && meta_type->indirections().size() == 0
               && (meta_type->isConstant() == (meta_type->getReferenceType()==AbstractMetaType::Reference)
                   || meta_type->isConstant())) {
        meta_type->setTypeUsagePattern(AbstractMetaType::StringViewPattern);

    } else if (type->isChar()
               && meta_type->indirections().size() == 0
               && meta_type->isConstant() == (meta_type->getReferenceType()==AbstractMetaType::Reference)) {
        meta_type->setTypeUsagePattern(AbstractMetaType::CharPattern);

    } else if ((
                   type->isJObjectWrapper()
                   || type->isJMapWrapper()
                   || type->isJCollectionWrapper()
                   || type->isJEnumWrapper()
                   || type->isJIteratorWrapper()
                   || type->isJQFlagsWrapper()
                )
               && meta_type->indirections().size() == 0
               && meta_type->isConstant() == (meta_type->getReferenceType()==AbstractMetaType::Reference)) {
        if(type->isJObjectWrapper())
            meta_type->setTypeUsagePattern(AbstractMetaType::JObjectWrapperPattern);
        else if(type->isJMapWrapper())
            meta_type->setTypeUsagePattern(AbstractMetaType::JMapWrapperPattern);
        else if(type->isJCollectionWrapper())
            meta_type->setTypeUsagePattern(AbstractMetaType::JCollectionWrapperPattern);
        else if(type->isJEnumWrapper())
            meta_type->setTypeUsagePattern(AbstractMetaType::JEnumWrapperPattern);
        else if(type->isJIteratorWrapper())
            meta_type->setTypeUsagePattern(AbstractMetaType::JIteratorWrapperPattern);
        else if(type->isJQFlagsWrapper())
            meta_type->setTypeUsagePattern(AbstractMetaType::JQFlagsWrapperPattern);
    } else if (type->isVariant()
               && meta_type->indirections().size() == 0
               && meta_type->isConstant() == (meta_type->getReferenceType()==AbstractMetaType::Reference)) {
        meta_type->setTypeUsagePattern(AbstractMetaType::VariantPattern);

    } else if (type->isEnum() && (meta_type->actualIndirections() == 0
                    || (meta_type->isConstant() && meta_type->getReferenceType()==AbstractMetaType::Reference && meta_type->indirections().size() == 0))) {
        meta_type->setTypeUsagePattern(AbstractMetaType::EnumPattern);

    } else if (type->isObject()
               && meta_type->indirections().size() == 0
               && meta_type->getReferenceType()==AbstractMetaType::Reference) {
        if (static_cast<const ComplexTypeEntry *>(type)->isQObject())
            meta_type->setTypeUsagePattern(AbstractMetaType::QObjectPattern);
        else
            meta_type->setTypeUsagePattern(AbstractMetaType::ObjectPattern);

    } else if (type->isObject()
               && meta_type->indirections().size() == 1) {
        if (static_cast<const ComplexTypeEntry *>(type)->isQObject())
            meta_type->setTypeUsagePattern(AbstractMetaType::QObjectPattern);
        else
            meta_type->setTypeUsagePattern(AbstractMetaType::ObjectPattern);

        // const-references to pointers can be passed as pointers
        if (meta_type->getReferenceType()==AbstractMetaType::Reference && meta_type->isConstant()) {
            meta_type->setReferenceType(AbstractMetaType::NoReference);
            meta_type->setConstant(false);
        }

    } else if (type->isContainer() && meta_type->indirections().size() <= 1) {
        meta_type->setTypeUsagePattern(AbstractMetaType::ContainerPattern);

    } else if (type->isPointerContainer() && meta_type->indirections().size() == 0) {
        meta_type->setTypeUsagePattern(AbstractMetaType::PointerContainerPattern);

    } else if (type->isInitializerList() && meta_type->indirections().size() == 0) {
        meta_type->setTypeUsagePattern(AbstractMetaType::InitializerListPattern);

    } else if (type->isTemplateArgument()) {
        meta_type->setTypeUsagePattern(AbstractMetaType::TemplateArgumentPattern);

    } else if (type->isFlags() && (meta_type->actualIndirections() == 0
                    || (meta_type->isConstant() && meta_type->getReferenceType()==AbstractMetaType::Reference && meta_type->indirections().size() == 0))) {
        meta_type->setTypeUsagePattern(AbstractMetaType::FlagsPattern);

    } else if (type->isArray()) {
        meta_type->setTypeUsagePattern(AbstractMetaType::ArrayPattern);

    } else if (type->isIterator()) {
        meta_type->setTypeUsagePattern(AbstractMetaType::IteratorPattern);

    } else if (type->isQMetaObjectType()) {
        meta_type->setTypeUsagePattern(AbstractMetaType::ObjectPattern);

    } else if (type->isQMetaObjectConnectionType()) {
        meta_type->setTypeUsagePattern(AbstractMetaType::ValuePattern);

    } else if (type->isThread()) {
        Q_ASSERT(meta_type->indirections().size() == 1);
        meta_type->setTypeUsagePattern(AbstractMetaType::ThreadPattern);

    } else if (type->isValue()
               && meta_type->indirections().size() == 0
               && (meta_type->isConstant() == (meta_type->getReferenceType()==AbstractMetaType::Reference)
                   || meta_type->getReferenceType()==AbstractMetaType::NoReference)) {
        meta_type->setTypeUsagePattern(AbstractMetaType::ValuePattern);

    } else if (type->isObject() && meta_type->actualIndirections() == 0) {
        meta_type->setTypeUsagePattern(AbstractMetaType::NativePointerPattern);

    } else {
        meta_type->setTypeUsagePattern(AbstractMetaType::NativePointerPattern);
    }
}

QString AbstractMetaBuilder::translateDefaultValue(const QString& defaultValueExpression, AbstractMetaType *type,
        AbstractMetaFunction *fnc, AbstractMetaClass *implementing_class,
        int argument_index) {
    QString function_name = fnc->name();
    QString class_name = implementing_class->name();

    QString replaced_expression = fnc->replacedDefaultExpression(implementing_class, argument_index + 1);
    if (fnc->removedDefaultExpression(implementing_class, argument_index + 1)){
        return "";
    }
    if (!replaced_expression.isEmpty())
        return replaced_expression;

    QString expr = defaultValueExpression;
    if(type->isTemplateArgument()){
        return expr;
    }else if (type->isPrimitive()) {
        if (type->name() == "boolean") {
            if (expr == "false" || expr == "true") {
                return expr;
            } else if (expr == "boolean()") {
                return "false";
            } else {
                bool ok = false;
                int number = expr.toInt(&ok);
                if (ok && number)
                    return "true";
                else
                    return "false";
            }
        } else if (expr == "ULONG_MAX") {
            return "Long.MAX_VALUE";
        } else if (expr == "QVariant::Invalid") {
            return QString::number(QVariant::Invalid);
        } else {
            if(expr == type->name()+"()"){
                return "0";
            }
            // This can be an enum or flag so I need to delay the
            // translation untill all namespaces are completly
            // processed. This is done in figureOutEnumValues()
            return expr;
        }
    } else if (type && (type->isFlags() || type->isEnum())) {
        // Same as with enum explanation above...
        return expr;

    } else {
        // constructor or functioncall can be a bit tricky...
        if (expr == "QVariant()" || expr == "QModelIndex()") {
            return "null";
        } else if (expr == "QStringList()") {
            return "new java.util.ArrayList<>()";
        } else if (expr == "QVariantHash()") {
            return "new java.util.HashMap<>()";
        } else if (expr == "QVariantMap()") {
            return "new java.util.TreeMap<>()";
        } else if ((expr.startsWith("QVector<") || expr.startsWith("QSet<") || expr.startsWith("QList<") || expr.startsWith("QStack<") || expr.startsWith("QQueue<")) && expr.endsWith(">()")) {
            QString collectionType = expr.startsWith("QSet") ? "java.util.HashSet" : "java.util.ArrayList";
            QString typeParam = expr.remove(0, expr.indexOf("<")+1);
            typeParam = typeParam.left(typeParam.indexOf(">"));
            TypeEntry *typeEntry = TypeDatabase::instance()->findType(typeParam);
            if(typeEntry){
                return "new " + collectionType + "<>()";
            }
        } else if (expr.endsWith(")") && expr.contains("::")) {
            TypeEntry *typeEntry = TypeDatabase::instance()->findType(expr.left(expr.indexOf("::")));
            if (typeEntry)
                return typeEntry->qualifiedTargetLangName().replace("$", ".") + "." + expr.right(expr.length() - expr.indexOf("::") - 2);
        } else if (expr.endsWith(")") && type->isValue()) {
            int pos = expr.indexOf("(");

            TypeEntry *typeEntry = TypeDatabase::instance()->findType(expr.left(pos));
            if (typeEntry)
                return "new " + typeEntry->qualifiedTargetLangName().replace("$", ".") + expr.right(expr.length() - pos);
            else
                return expr;
        } else if (expr == "0" || expr == "Q_NULLPTR" || expr == "nullptr" || expr == "NULL") {
            return "null";
        } else if (type->isTargetLangString()) {
            if (expr == "{}" || expr == "QString()" || expr == "QStringRef()" || expr == "QStringView()") {
                return "\"\"";
            }
            if(expr.startsWith("QString(\"") && expr.endsWith("\")"))
                return expr.mid(8, expr.length()-9);
            return expr;
        } else if (type->isObject() || type->isValue() || expr.contains("::")) { // like Qt::black passed to a QColor
            if(type->isObject() && expr == type->name()+"()"){
                return "null";
            }
            if (expr == "{}"){
                return "new " + type->typeEntry()->qualifiedTargetLangName().replace("$", ".") + "()";
            }
            TypeEntry *typeEntry = TypeDatabase::instance()->findType(expr.left(expr.indexOf("::")));

            expr = expr.right(expr.length() - expr.indexOf("::") - 2);
            if (typeEntry) {
                return "new " + type->typeEntry()->qualifiedTargetLangName().replace("$", ".") +
                       "(" + typeEntry->qualifiedTargetLangName().replace("$", ".") + "." + expr + ")";
            }
        } else if (expr == "QString()" || expr == "QStringRef()" || expr == "QStringView()") {
            return "null";
        } else if (expr == "QChar()") {
            return "'\\0'";
        }
    }

    QString warn = QString("unsupported default value '%3' of argument in function '%1', class '%2'")
                   .arg(function_name).arg(class_name).arg(defaultValueExpression);
    ReportHandler::warning(warn);

    return QString();
}


bool AbstractMetaBuilder::isQObject(const QString &qualified_name) {
    if (qualified_name == "QObject")
        return true;

    ClassModelItem class_item = m_dom->findClass(qualified_name);

    if (!class_item) {
        QStringList names = qualified_name.split(QLatin1String("::"));
        NamespaceModelItem ns = model_dynamic_cast<NamespaceModelItem>(m_dom);
        for (int i = 0; i < names.size() - 1 && ns; ++i)
            ns = ns->namespaceMap().value(names.at(i));
        if (ns && names.size() >= 2)
            class_item = ns->findClass(names.at(names.size() - 1));
    }

    bool isqobject = class_item && class_item->extendsClass("QObject");

    if (class_item && !isqobject) {
        QStringList baseClasses = class_item->baseClasses();
        for (int i = 0; i < baseClasses.count(); ++i) {

            isqobject = isQObject(baseClasses.at(i));
            if (isqobject)
                break;
        }
    }

    return isqobject;
}


bool AbstractMetaBuilder::isEnum(const QStringList &qualified_name) {
    CodeModelItem item = m_dom->model()->findItem(qualified_name, m_dom->toItem());
    return item && item->kind() == _EnumModelItem::__node_kind;
}

AbstractMetaType *AbstractMetaBuilder::inheritTemplateType(const QList<const AbstractMetaType *> &template_types,
        const AbstractMetaType *meta_type, bool *ok) {
    if (ok)
        *ok = true;
    if (!meta_type || (!meta_type->typeEntry()->isTemplateArgument() && !meta_type->hasInstantiations()))
        return meta_type ? meta_type->copy() : nullptr;

    AbstractMetaType *returned = meta_type->copy();
    returned->setOriginalTemplateType(meta_type->copy());

    if (returned->typeEntry()->isTemplateArgument()) {
        const TemplateArgumentEntry *tae = static_cast<const TemplateArgumentEntry *>(returned->typeEntry());

        // If the template is intantiated with void we special case this as rejecting the functions that use this
        // parameter from the instantiation.
        if (template_types.size() <= tae->ordinal() || template_types.at(tae->ordinal())->typeEntry()->name() == "void") {
            if (ok)
                *ok = false;
            return nullptr;
        }

        AbstractMetaType *t = returned->copy();
        const AbstractMetaType * template_type = template_types.at(tae->ordinal());
        t->setTypeEntry(template_type->typeEntry());
        t->setForceBoxedPrimitives(template_type->forceBoxedPrimitives());
        if(template_type->indirections().size() + t->indirections().size()>0){
            t->setIndirections(QList<bool>() << false);
        }else{
            t->setIndirections(QList<bool>());
        }
        decideUsagePattern(t);

        delete returned;
        if(template_type->typeEntry()->isTemplateArgument()){
            return t;
        }
        returned = inheritTemplateType(template_types, t, ok);
        if (ok && !(*ok))
            return nullptr;
    }

    if (returned->hasInstantiations()) {
        QList<const AbstractMetaType *> instantiations = returned->instantiations();
        for (int i = 0; i < instantiations.count(); ++i) {
            instantiations[i] = inheritTemplateType(template_types, instantiations.at(i), ok);
            if (ok && !(*ok))
                return nullptr;
        }
        returned->setInstantiations(instantiations);
    }

    return returned;
}

bool AbstractMetaBuilder::inheritHiddenBaseType(AbstractMetaClass *subclass,
        const AbstractMetaClass *hidden_base_class,
        const TypeParser::Info &info) {
    QList<const AbstractMetaType *> template_types;
    for(const TypeParser::Info &i : info.template_instantiations) {
        TypeEntry *t = TypeDatabase::instance()->findType(i.qualified_name.join("::"));
        if (t) {
            AbstractMetaType *temporary_type = createMetaType();
            temporary_type->setTypeEntry(t);
            temporary_type->setConstant(i.is_constant);
            temporary_type->setReferenceType(AbstractMetaType::ReferenceType(i.reference_type));
            temporary_type->setIndirections(i.indirections);
            if( t->isPrimitive() && temporary_type->indirections().isEmpty() && (
                        hidden_base_class->typeEntry()->isContainer()
                        || hidden_base_class->typeEntry()->isIterator() ) ){
                temporary_type->setForceBoxedPrimitives(true);
            }
            decideUsagePattern(temporary_type);
            template_types << temporary_type;
        }else if(m_templates.contains(subclass) && template_types.size()<subclass->templateArguments().size()){
            AbstractMetaType *temporary_type = createMetaType();
            temporary_type->setTypeEntry(subclass->templateArguments().at(template_types.size()));
            temporary_type->setConstant(i.is_constant);
            temporary_type->setReferenceType(AbstractMetaType::ReferenceType(i.reference_type));
            temporary_type->setIndirections(i.indirections);
            decideUsagePattern(temporary_type);
            template_types << temporary_type;
        }
    }

    QMap<const TypeEntry *,const AbstractMetaType *> template_types_by_name;
    for (int i = 0; i < hidden_base_class->templateArguments().size() && i < template_types.size(); ++i) {
        template_types_by_name.insert(hidden_base_class->templateArguments().at(i), template_types.at(i));
    }

    IteratorTypeEntry * iteratorTypeEntry = TypeDatabase::instance()->findIteratorType(hidden_base_class->typeEntry());
    AbstractMetaClass * newIteratorClass = nullptr;
    if(iteratorTypeEntry){
        newIteratorClass = instantiateIterator(iteratorTypeEntry, subclass, template_types, template_types_by_name);
    }

    const AbstractMetaFunctionList& funcs = subclass->functions();
    for(const AbstractMetaFunction *function : hidden_base_class->functions()) {
        if (function->isModifiedRemoved(TypeSystem::All) || function->isStatic())
            continue;

        AbstractMetaFunction *f = function->copy();
        f->setArguments(AbstractMetaArgumentList());

        bool ok = true;
        AbstractMetaType *ftype = function->type();
        if(ftype){
            if(ftype->typeEntry()->isContainer()
                    && ftype->typeEntry()->qualifiedCppName()==hidden_base_class->typeEntry()->qualifiedCppName()
                    && ftype->instantiations().size()==0){
                ftype = ftype->copy();
                ftype->setInstantiations(template_types);
                f->setType(inheritTemplateType(template_types, ftype, &ok));
                delete ftype;
            }else if(iteratorTypeEntry
                     && newIteratorClass
                     && ftype->typeEntry()==iteratorTypeEntry){
                 ftype = ftype->copy();
                 ftype->setTypeEntry(newIteratorClass->typeEntry());
                 f->setType(inheritTemplateType(template_types, ftype, &ok));
                 delete ftype;
            }else{
                f->setType(inheritTemplateType(template_types, ftype, &ok));
            }
        }
        if (!ok) {
            delete f;
            continue;
        }

        for(AbstractMetaArgument *argument : function->arguments()) {
            AbstractMetaArgument *arg = argument->copy();
            AbstractMetaType *atype = argument->type();
            if(atype && atype->typeEntry()->isContainer()
                    && atype->typeEntry()->qualifiedCppName()==hidden_base_class->typeEntry()->qualifiedCppName()
                    && atype->instantiations().size()==0){
                atype = atype->copy();
                atype->setInstantiations(template_types);
                arg->setType(inheritTemplateType(template_types, atype, &ok));
                delete atype;
            }else if(iteratorTypeEntry
                     && newIteratorClass
                     && atype->typeEntry()==iteratorTypeEntry){
                atype = atype->copy();
                atype->setTypeEntry(newIteratorClass->typeEntry());
                arg->setType(inheritTemplateType(template_types, atype, &ok));
                delete atype;
            }else{
                arg->setType(inheritTemplateType(template_types, atype, &ok));
            }

            if (!ok)
                break;

            if(!arg->defaultValueExpression().isEmpty() && argument->type()->typeEntry()->isTemplateArgument()){
                QString exp = arg->defaultValueExpression();
                arg->setOriginalDefaultValueExpression(exp);
                exp = exp.replace(argument->type()->typeEntry()->qualifiedCppName(), arg->type()->typeEntry()->qualifiedCppName());
                exp = translateDefaultValue(exp, arg->type(), f, subclass, f->arguments().size());
                arg->setDefaultValueExpression(exp);
            }
            f->addArgument(arg);
        }

        if (!ok) {
            delete f;
            continue ;
        }

        // There is no base class in java to inherit from here, so the
        // template instantiation is the class that implements the function..
        f->setImplementingClass(subclass);

        // We also set it as the declaring class, since the superclass is
        // supposed to disappear. This allows us to make certain function modifications
        // on the inherited functions.
        f->setDeclaringClass(subclass);
        f->setDeclaringTemplate(subclass->baseClassNames().at(0));

        if (f->isConstructor() && subclass->isTypeAlias()) {
            f->setName(subclass->simpleName());
        } else if (f->isConstructor()) {
            delete f;
            continue;
        }

        // if the instantiation has a function named the same as an existing
        // function we have shadowing so we need to skip it.
        bool found = false;
        for (int i = 0; i < funcs.size(); ++i) {
            AbstractMetaFunction* f2 = funcs.at(i);
            if (f2->name() == f->name() && f2->arguments().size()==f->arguments().size()) {
                found = true;
                for(int j=0; j<f->arguments().size(); j++){
                    AbstractMetaArgument* arg = f->arguments().at(j);
                    AbstractMetaArgument* arg2 = f2->arguments().at(j);
                    if(
                            arg->type()->isConstant()!=arg2->type()->isConstant()
                            || arg->type()->indirections()!=arg2->type()->indirections()
                            || arg->type()->typeUsagePattern()!=arg2->type()->typeUsagePattern()
                            || arg->type()->typeEntry()->qualifiedCppName()!=arg2->type()->typeEntry()->qualifiedCppName()
                            ){
                        found = false;
                        break;
                    }
                }
                if (found) {
                    continue;
                }
            }
        }
        if (found) {
            delete f;
            continue;
        }

        ComplexTypeEntry *te = subclass->typeEntry();
        FunctionModificationList mods = function->modifications(hidden_base_class);
        for (int i = 0; i < mods.size(); ++i) {
            FunctionModification mod = mods.at(i);
            mod.signature = f->minimalSignature();

            // If we ever need it... Below is the code to do
            // substitution of the template instantation type inside
            // injected code..
            if (mod.modifiers & Modification::CodeInjection) {
                for (QMap<const TypeEntry *,const AbstractMetaType *>::const_iterator ki = template_types_by_name.begin();
                     ki != template_types_by_name.end(); ki++) {
                    CodeSnip &snip = mod.snips.last();
                    QString code = snip.code();
                    const TypeEntry * t_entry = ki.value()->typeEntry();
                    if(t_entry->isTemplateArgument())
                        code = code.replace("%"+ki.key()->name(),"%"+t_entry->qualifiedCppName());
                    else {
                        if(snip.language==TypeSystem::TargetLangCode){
                            if(t_entry->isPrimitive()){
                                const PrimitiveTypeEntry* ptype = reinterpret_cast<const PrimitiveTypeEntry*>(t_entry);
                                code = code.replace("%"+ki.key()->name(),ptype->javaObjectFullName());
                            }else{
                                code = code.replace("%"+ki.key()->name(),t_entry->qualifiedTargetLangName());
                            }
                        }else{
                            code = code.replace("%"+ki.key()->name(),t_entry->qualifiedCppName());
                        }
                    }
                    snip.codeList.clear();
                    snip.addCode(code);
                }
            }
            te->addFunctionModification(mod);
        }
        subclass->addFunction(f);
    }

    if(hidden_base_class->typeEntry()->qualifiedCppName()=="QStack"
            && template_types.size()>0
            && template_types.at(0)->isPrimitive()){
        FunctionModification mod2;
        mod2.signature = QMetaObject::normalizedSignature("pop()");
        mod2.modifiers = Modification::Rename | Modification::Private;
        mod2.renamedToName = "_pop";
        subclass->typeEntry()->addFunctionModification(mod2);
    }else if((hidden_base_class->typeEntry()->qualifiedCppName()=="QMap"
        || hidden_base_class->typeEntry()->qualifiedCppName()=="QMultiMap"
        || hidden_base_class->typeEntry()->qualifiedCppName()=="QHash"
        || hidden_base_class->typeEntry()->qualifiedCppName()=="QMultiHash")
            && template_types.size()>1){
        FunctionModification mod2;
        if(template_types.at(0)->isPrimitive()
                && (hidden_base_class->typeEntry()->qualifiedCppName()=="QMap"
                    || hidden_base_class->typeEntry()->qualifiedCppName()=="QMultiMap")){
            mod2.signature = QMetaObject::normalizedSignature("lastKey() const");
            mod2.modifiers = Modification::Rename | Modification::Private;
            mod2.renamedToName = "_lastKey";
            subclass->typeEntry()->addFunctionModification(mod2);
            mod2.signature = QMetaObject::normalizedSignature("firstKey() const");
            mod2.modifiers = Modification::Rename | Modification::Private;
            mod2.renamedToName = "_firstKey";
            subclass->typeEntry()->addFunctionModification(mod2);
        }
    }

    {
        if(!info.template_instantiations.isEmpty()){
            subclass->setTemplateBaseClass(hidden_base_class);
            subclass->setTemplateBaseClassInstantiations(template_types);

            subclass->setInterfaces(hidden_base_class->interfaces());
            subclass->setBaseClass(hidden_base_class->baseClass());
        }

        QString implements = hidden_base_class->typeEntry()->implements();
        if(!implements.isEmpty()){
            for (QMap<const TypeEntry *,const AbstractMetaType *>::const_iterator ki = template_types_by_name.begin();
                 ki != template_types_by_name.end(); ki++) {
                const TypeEntry * t_entry = ki.value()->typeEntry();
                if(t_entry->isTemplateArgument())
                    implements = implements.replace("%"+ki.key()->name(),"%"+t_entry->qualifiedCppName());
                else
                    implements = implements.replace("%"+ki.key()->name(),t_entry->qualifiedTargetLangName());
            }
            if(subclass->typeEntry()->implements().isEmpty())
                subclass->typeEntry()->setImplements(implements);
            else
                subclass->typeEntry()->setImplements(subclass->typeEntry()->implements()+", "+implements);
        }
    }
    subclass->setHasJustPrivateConstructors(hidden_base_class->hasJustPrivateConstructors());
    subclass->setHasPublicDestructor(hidden_base_class->hasPublicDestructor());
    subclass->setHasVirtualDestructor(hidden_base_class->hasVirtualDestructor());
    subclass->setHasPrivateDestructor(hidden_base_class->hasPrivateDestructor());

    return true;
}

void AbstractMetaBuilder::parseQ_Property(AbstractMetaClass *meta_class, const QStringList &declarations) {
    for (int i = 0; i < declarations.size(); ++i) {
        QString p = declarations.at(i);

        /*
        Pass 1: normalize all whitespace.
        Remove leading/trailing, convert contiguous whitespace sequences to a single space character.
        */
        {
            QString newP = QString();
            const int len = p.length();
            int state = 0;
            int j;
            for (j = 0; j < len; j++) {
                QChar c = p.at(j);
                if (state == 0) { // skip leading spaces
                    if(!c.isSpace()) {
                        newP += c;
                        state++;
                    }
                } else if(state == 1) { // last token was not whitespace
                    if(c.isSpace()) { // normalize to an actual space, from maybe \r\n\v\t etc.
                        newP += ' ';
                        state++;
                    } else {
                        newP += c;
                    }
                } else { // last token was whitespace
                    if(!c.isSpace()) {
                        newP += c;
                        state--;
                    }
                }
            }

            const int newplen = newP.length();

            // remove that last space we added
            if(state >= 2 && newplen > 1 && newP.at(newplen - 1) == QChar(' ')) {
                newP = newP.left(newplen - 2);
            }

            if(!newP.isNull()) {
                if (newP != p)
                    p = newP;
            }
        }

        /*
        Pass 2: Correct the first word to always be a type (this means removing
        all whitespace from the type declaration) by looking ahead at the data.

        Convert "Q_PROPERTY(QGraphicsObject * parent READ parentObject WRITE setParentItem NOTIFY parentChanged DESIGNABLE false)"
        into "Q_PROPERTY(QGraphicsObject* parent READ parentObject WRITE setParentItem NOTIFY parentChanged DESIGNABLE false)"

        moc.exe doesn't allow any comma in the property spec string, we do just in case
        that changes someday since it can be part of a type with multiple template info.
        */
        {
            QString newFirstWord = QString();
            const int len = p.length();

            /*
            0 = start (expect isalnum() || :)
            1 = last-char-was-space (expect <*&)
            2+ = inside-open-angle (expect isalnum() || : || <>*&, || ">")
            */
            int state = 0;

            int j;
            for (j = 0; j < len; j++) {
                QChar c = p.at(j);
                if (state == 0) {
                    if (c.isLetterOrNumber() || c == QChar(':') || c == QChar('*') || c == QChar('&')) {
                        newFirstWord += c;
                    } else if(c == QChar('<')) {
                        newFirstWord += c;
                        state = 2;
                    } else if (c.isSpace()) {
                        state = 1;
                    }
                } else if (state == 1) {
                    if (c == QChar('<')) {
                        newFirstWord += c;
                        state = 2;
                    } else if (c == QChar('*') || c == QChar('&')) {
                        newFirstWord += c;
                    } else if (c.isSpace()) {
                        /*
                        Use-case from state==0: strictly speaking 2 space chars in a row are not allowed
                        Use-case from state==2: we want to eat the space between 1st and 2nd word
                        */
                    } else { // if (c.isLetterOrNumber())
                        // this is to be the 2nd word, so we are done
                        break;
                    }
                } else if (state >= 2) {
                    if (c.isLetterOrNumber() || c == QChar(':') || c == QChar('*') || c == QChar('&') || c == QChar(',')) {
                        newFirstWord += c;
                    } else if (c == QChar('<')) {
                        newFirstWord += c;
                        state++;
                    } else if (c == QChar('>')) {
                        newFirstWord += c;
                        state--;
                    } else if (c.isSpace()) {
                        // nop - strictly speaking space is only allowed around non-isalnum() chars
                    } else {
                        qDebug() << "Q_PROPERTY() parse error p=" << p;
                        newFirstWord = QString(); // abort
                        break;
                    }
                }
            }
            // cut length i from left(), stitch newFirstWord
            if(!newFirstWord.isNull()) {
                QString newP = newFirstWord + ' ' + p.mid(j);
                if (newP != p)
                    p = newP;
            }
        }

        /*
        Pass 3: Split by space character then normalize parentesis by joining
        expression parts into single elements.

        "FooClass*" "foo" "READ" "(expr" "!=" "0)" "WRITE" "setFoo"

        becomes:

        "FooClass*" "foo" "READ" "(expr != 0)" "WRITE" "setFoo"

        This is known to allow correct parsing of some Q_PROPERTY in QToolBar.
        */
        QStringList l = p.split(QLatin1String(" "));
        {
            QStringList newL = QStringList();
            const int l_size = l.size();
            int nest = 0;
            QString newItem = QString(); // recombined item
            for (int j = 0; j < l_size; j++) {
                const QString item = l.at(j); // original item
                const int item_length = item.length();
                for (int k = 0; k < item_length; k++) {
                    const QChar ch = item.at(k);
                    if (ch == QChar('('))
                        nest++;
                    else if (ch == QChar(')'))
                        nest--;
                }
                if (newItem.isNull()) {
                    newItem = item;
                } else {
                    // only add spaces between parts we join
                    newItem += ' ';
                    newItem += item;
                }
                if(nest == 0) {
                    newL.append(newItem);
                    newItem = QString(); // set to null
                }
            }
            if (!newItem.isNull())
                newL.append(newItem);

            l = newL;
        }

        QString typeName = l.at(0);
        QPropertySpec *spec = new QPropertySpec();
        spec->setName(l.at(1));
        spec->setIndex(i);

        for (int pos = 2; pos < l.size(); pos ++) {
            // I have seen DESIGNABLE and SCRIPTABLE examples that do not have
            //  a true/false after but another keyword.
            if (l.at(pos) == QLatin1String("READ")){
                if(pos < l.size()-1)
                    spec->setRead(l.at(++pos));
                else
                    qDebug() << "WARNING: Q_PROPERTY(" << spec->name() << ", " << typeName << "): aspect " << l.at(pos) << " requires argument";
            }
            else if (l.at(pos) == QLatin1String("WRITE")){
                if(pos < l.size()-1)
                    spec->setWrite(l.at(++pos));
                else
                    qDebug() << "WARNING: Q_PROPERTY(" << spec->name() << ", " << typeName << "): aspect " << l.at(pos) << " requires argument";
            }
            else if (l.at(pos) == QLatin1String("DESIGNABLE")){
                if(pos < l.size()-1)
                    spec->setDesignable(l.at(++pos));
                else
                    qDebug() << "WARNING: Q_PROPERTY(" << spec->name() << ", " << typeName << "): aspect " << l.at(pos) << " requires argument";
            }
            else if (l.at(pos) == QLatin1String("SCRIPTABLE")){
                if(pos < l.size()-1)
                    spec->setScriptable(l.at(++pos));
                else
                    qDebug() << "WARNING: Q_PROPERTY(" << spec->name() << ", " << typeName << "): aspect " << l.at(pos) << " requires argument";
            }
            else if (l.at(pos) == QLatin1String("RESET")){
                if(pos < l.size()-1)
                    spec->setReset(l.at(++pos));
                else
                    qDebug() << "WARNING: Q_PROPERTY(" << spec->name() << ", " << typeName << "): aspect " << l.at(pos) << " requires argument";
            }
            else if (l.at(pos) == QLatin1String("NOTIFY")){
                if(pos < l.size()-1)
                    spec->setNotify(l.at(++pos));
                else
                    qDebug() << "WARNING: Q_PROPERTY(" << spec->name() << ", " << typeName << "): aspect " << l.at(pos) << " requires argument";
            }
            else if (l.at(pos) == QLatin1String("USER")){
                if(pos < l.size()-1)
                    spec->setUser(l.at(++pos));
                else
                    qDebug() << "WARNING: Q_PROPERTY(" << spec->name() << ", " << typeName << "): aspect " << l.at(pos) << " requires argument";
            }
            else if (l.at(pos) == QLatin1String("STORED")){
                if(pos < l.size()-1)
                    spec->setStored(l.at(++pos));
                else
                    qDebug() << "WARNING: Q_PROPERTY(" << spec->name() << ", " << typeName << "): aspect " << l.at(pos) << " requires argument";
            }
            else if (l.at(pos) == QLatin1String("REVISION")){
                if(pos < l.size()-1)
                    spec->setRevision(l.at(++pos));
                else
                    qDebug() << "WARNING: Q_PROPERTY(" << spec->name() << ", " << typeName << "): aspect " << l.at(pos) << " requires argument";
            }
            else if (l.at(pos) == QLatin1String("CONSTANT")){
                spec->setConstant(true);
            }
            else if (l.at(pos) == QLatin1String("FINAL")){
                spec->setFinal(true);
            }
            else
                qDebug() << "WARNING: Q_PROPERTY(" << spec->name() << ", " << typeName << "): unknown aspect " << l.at(pos);
        }

        meta_class->addPropertySpec(spec);
    }
}

static void hide_functions(const AbstractMetaFunctionList &l) {
    for(AbstractMetaFunction *f : l) {
        FunctionModification mod;
        mod.signature = f->minimalSignature();
        mod.modifiers = FunctionModification::Private;
        const_cast<ComplexTypeEntry *>(static_cast<const ComplexTypeEntry *>(f->implementingClass()->typeEntry()))->addFunctionModification(mod);
    }
}

static void remove_function(AbstractMetaFunction *f) {
    FunctionModification mod;
    mod.removal = TypeSystem::All;
    mod.signature = f->minimalSignature();
    const_cast<ComplexTypeEntry *>(static_cast<const ComplexTypeEntry *>(f->implementingClass()->typeEntry()))->addFunctionModification(mod);
}

static AbstractMetaFunctionList filter_functions(const AbstractMetaFunctionList &lst, QSet<QString> *signatures) {
    AbstractMetaFunctionList functions;
    for(AbstractMetaFunction *f : lst) {
        QString signature = f->minimalSignature();
        int start = signature.indexOf(QLatin1Char('(')) + 1;
        int end = signature.lastIndexOf(QLatin1Char(')'));
        signature = signature.mid(start, end - start);
        if (signatures->contains(signature)) {
            remove_function(f);
            continue;
        }
        (*signatures) << signature;
        functions << f;
    }
    return functions;
}

void AbstractMetaBuilder::setupBeginEnd(AbstractMetaClass *cls) {
    AbstractMetaFunctionList begins;
    AbstractMetaFunctionList ends;

    QString name_begin = QLatin1String("begin");
    QString name_end = QLatin1String("end");

    AbstractMetaFunctionList functions = cls->queryFunctions(AbstractMetaClass::ClassImplements
                                         | AbstractMetaClass::NotRemovedFromTargetLang);
    for(AbstractMetaFunction *f : functions) {
        if(f->type() && f->type()->typeEntry()->isIterator() && f->arguments().size()==0){
            if (f->name() == name_begin)
                begins << f;
            else if (f->name() == name_end)
                ends << f;
        }
    }

    if (begins.size() || ends.size()) {
        for(AbstractMetaFunction *f : begins) {
            FunctionModification mod;
            mod.signature = f->minimalSignature();
            mod.modifiers = FunctionModification::Protected;
            const_cast<ComplexTypeEntry *>(static_cast<const ComplexTypeEntry *>(f->implementingClass()->typeEntry()))->addFunctionModification(mod);
        }
        for(AbstractMetaFunction *f : ends) {
            FunctionModification mod;
            mod.signature = f->minimalSignature();
            mod.modifiers = FunctionModification::Protected;
            const_cast<ComplexTypeEntry *>(static_cast<const ComplexTypeEntry *>(f->implementingClass()->typeEntry()))->addFunctionModification(mod);
        }
        cls->setBeginFunctions(begins);
        cls->setEndFunctions(ends);
    }
}

void AbstractMetaBuilder::setupEquals(AbstractMetaClass *cls) {
    AbstractMetaFunctionList equals;
    AbstractMetaFunctionList nequals;

    QString op_equals = QLatin1String("operator_equal");
    QString op_nequals = QLatin1String("operator_not_equal");

    AbstractMetaFunctionList functions = cls->queryFunctions(AbstractMetaClass::ClassImplements
                                         | AbstractMetaClass::NotRemovedFromTargetLang);
    for(AbstractMetaFunction *f : functions) {
        if(f->arguments().size()==1){
            if (f->name() == op_equals)
                equals << f;
            else if (f->name() == op_nequals)
                nequals << f;
        }
    }

    if (equals.size() || nequals.size()) {
        if (!cls->hasHashFunction() && cls->typeEntry()->isValue()) {
            ReportHandler::warning(QString::fromLatin1("Class '%1' has equals operators but no qHash() function. Hashcode of objects will consistently be 0.")
                                   .arg(cls->name()));
        }

        hide_functions(equals);
        hide_functions(nequals);

        // We only need == if we have both == and !=, and one == for
        // each signature type, like QDateTime::==(QDate) and (QTime)
        // if such a thing exists...
        QSet<QString> func_signatures;
        cls->setEqualsFunctions(filter_functions(equals, &func_signatures));
        cls->setNotEqualsFunctions(filter_functions(nequals, &func_signatures));
    }
}

void AbstractMetaBuilder::setupComparable(AbstractMetaClass *cls) {
    AbstractMetaFunctionList greater;
    AbstractMetaFunctionList greaterEquals;
    AbstractMetaFunctionList less;
    AbstractMetaFunctionList lessEquals;

    QString op_greater = QLatin1String("operator_greater");
    QString op_greater_eq = QLatin1String("operator_greater_or_equal");
    QString op_less = QLatin1String("operator_less");
    QString op_less_eq = QLatin1String("operator_less_or_equal");

    AbstractMetaFunctionList functions = cls->queryFunctions(AbstractMetaClass::ClassImplements
                                         | AbstractMetaClass::NotRemovedFromTargetLang);
    for(AbstractMetaFunction *f : functions) {
        if (f->name() == op_greater)
            greater << f;
        else if (f->name() == op_greater_eq)
            greaterEquals << f;
        else if (f->name() == op_less)
            less << f;
        else if (f->name() == op_less_eq)
            lessEquals << f;
    }

    bool hasEquals = cls->equalsFunctions().size() || cls->notEqualsFunctions().size();

    // Conditions for comparable is:
    //     >, ==, <             - The basic case
    //     >, ==                - Less than becomes else case
    //     <, ==                - Greater than becomes else case
    //     >=, <=               - if (<= && >=) -> equal
    bool mightBeComparable = greater.size() || greaterEquals.size() || less.size() || lessEquals.size()
                             || greaterEquals.size() == 1 || lessEquals.size() == 1;

    if (mightBeComparable) {
        QSet<QString> signatures;

        // We only hide the original functions if we are able to make a compareTo() method
        bool wasComparable = false;

        // The three upper cases, prefer the <, == approach
        if (hasEquals && (greater.size() || less.size())) {
            cls->setLessThanFunctions(filter_functions(less, &signatures));
            cls->setGreaterThanFunctions(filter_functions(greater, &signatures));
            filter_functions(greaterEquals, &signatures);
            filter_functions(lessEquals, &signatures);
            wasComparable = true;
        } else if (hasEquals && (greaterEquals.size() || lessEquals.size())) {
            cls->setLessThanEqFunctions(filter_functions(lessEquals, &signatures));
            cls->setGreaterThanEqFunctions(filter_functions(greaterEquals, &signatures));
            filter_functions(less, &signatures);
            filter_functions(greater, &signatures);
            wasComparable = true;
        } else if (greaterEquals.size() == 1 || lessEquals.size() == 1) {
            cls->setGreaterThanEqFunctions(filter_functions(greaterEquals, &signatures));
            cls->setLessThanEqFunctions(filter_functions(lessEquals, &signatures));
            filter_functions(less, &signatures);
            filter_functions(greater, &signatures);
            wasComparable = true;
        } else if (greater.size() == 1 || less.size() == 1) {
            cls->setGreaterThanFunctions(filter_functions(greater, &signatures));
            cls->setLessThanFunctions(filter_functions(less, &signatures));
            filter_functions(greaterEquals, &signatures);
            filter_functions(lessEquals, &signatures);
            wasComparable = true;
        }

        if (wasComparable) {
            hide_functions(greater);
            hide_functions(greaterEquals);
            hide_functions(less);
            hide_functions(lessEquals);
        }
    }

}

void AbstractMetaBuilder::setupClonable(AbstractMetaClass *cls) {
    // All value types are required to have a copy constructor,
    // or they will not work as value types (it won't even compile,
    // because of calls to qRegisterMetaType(). Thus all value types
    // should be cloneable.
    if (cls->typeEntry()->isValue()) {
        AbstractMetaFunctionList functions = cls->queryFunctions(AbstractMetaClass::ClassImplements | AbstractMetaClass::Public | AbstractMetaClass::Constructors);
        for(AbstractMetaFunction *f : functions) {
            const AbstractMetaArgumentList& arguments = f->arguments();
            if (f->actualMinimumArgumentCount() == 1) {
                if (cls->typeEntry()->qualifiedCppName() == arguments.at(0)->type()->typeEntry()->qualifiedCppName()) {
                    remove_function(f);
                }
            }
        }
        cls->setHasCloneOperator(true);
        return;
    }
}

static void write_reject_log_file(QFile *f,
                                  const QMap<QPair<QString,QString>, AbstractMetaBuilder::RejectReason> &rejects) {
    if (!f->open(QIODevice::WriteOnly | QIODevice::Text)) {
        ReportHandler::warning(QString("failed to write log file: '%1'")
                               .arg(f->fileName()));
        return;
    }

    QTextStream s(f);

    int maxSize = 0;
    for (QMap<QPair<QString,QString>, AbstractMetaBuilder::RejectReason>::const_iterator it = rejects.constBegin();
            it != rejects.constEnd(); ++it) {
        maxSize = qMax(maxSize, it.key().first.size());
    }

    for (int reason = 0; reason < AbstractMetaBuilder::NoReason; ++reason) {
        s << QString(72, '*') << endl;
        switch (reason) {
            case AbstractMetaBuilder::NotInTypeSystem:
                s << "Not in type system";
                break;
            case AbstractMetaBuilder::GenerationDisabled:
                s << "Generation disabled by type system";
                break;
            case AbstractMetaBuilder::RedefinedToNotClass:
                s << "Type redefined to not be a class";
                break;

            case AbstractMetaBuilder::UnmatchedReturnType:
                s << "Unmatched return type";
                break;

            case AbstractMetaBuilder::UnmatchedArgumentType:
                s << "Unmatched argument type";
                break;

            default:
                s << "unknown reason";
                break;
        }

        s << endl;

        for (QMap<QPair<QString,QString>, AbstractMetaBuilder::RejectReason>::const_iterator it = rejects.constBegin();
                it != rejects.constEnd(); ++it) {
            if (it.value() != reason)
                continue;
            QString space;
            space.fill(QLatin1Char(' '), maxSize+5-it.key().first.size());
            s << " - " << it.key().first << space << it.key().second << endl;
        }

        s << QString(72, '*') << endl << endl;
    }

}


void AbstractMetaBuilder::dumpLog() {
    {
        QString fileName("mjb_rejected_classes.log");
        QFile file(fileName);
        if (!outputDirectory().isNull())
            file.setFileName(QDir(outputDirectory()).absoluteFilePath(fileName));
        write_reject_log_file(&file, m_rejected_classes);
    }

    {
        QString fileName("mjb_rejected_function_pointers.log");
        QFile file(fileName);
        if (!outputDirectory().isNull())
            file.setFileName(QDir(outputDirectory()).absoluteFilePath(fileName));
        write_reject_log_file(&file, m_rejected_functionals);
    }

    {
        QString fileName("mjb_rejected_enums.log");
        QFile file(fileName);
        if (!outputDirectory().isNull())
            file.setFileName(QDir(outputDirectory()).absoluteFilePath(fileName));
        write_reject_log_file(&file, m_rejected_enums);
    }

    {
        QString fileName("mjb_rejected_functions.log");
        QFile file(fileName);
        if (!outputDirectory().isNull())
            file.setFileName(QDir(outputDirectory()).absoluteFilePath(fileName));
        write_reject_log_file(&file, m_rejected_functions);
    }

    {
        QString fileName("mjb_rejected_fields.log");
        QFile file(fileName);
        if (!outputDirectory().isNull())
            file.setFileName(QDir(outputDirectory()).absoluteFilePath(fileName));
        write_reject_log_file(&file, m_rejected_fields);
    }

    {
        QString fileName("reported_warnings.log");
        QFile file(fileName);
        if (!outputDirectory().isNull())
            file.setFileName(QDir(outputDirectory()).absoluteFilePath(fileName));
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            {
                QTextStream s(&file);
                for(const QString& w : ReportHandler::reportedWarnings()){
                    if(!w.contains("Suppressed warning with no text specified"))
                        s << w << endl;
                }
            }
            file.close();
        }
    }
}

AbstractMetaClassList AbstractMetaBuilder::classesTopologicalSorted() const {
    AbstractMetaClassList res;

    AbstractMetaClassList classes = m_meta_classes;

    QSet<AbstractMetaClass*> noDependency;
    QHash<AbstractMetaClass*, QSet<AbstractMetaClass* >* > hash;
    for(AbstractMetaClass *cls : classes) {
        QSet<AbstractMetaClass* > *depends = new QSet<AbstractMetaClass* >();

        if (cls->baseClass())
            depends->insert(cls->baseClass());

        for(AbstractMetaClass *interface : cls->interfaces()) {
            depends->insert(interface);
        }

        if (depends->empty()) {
            noDependency.insert(cls);
        } else {
            hash.insert(cls, depends);
        }
    }

    while (!noDependency.empty()) {
        for(AbstractMetaClass *cls : noDependency.values()) {
            if (!cls->isInterface())
                res.append(cls);
            noDependency.remove(cls);
            QHashIterator<AbstractMetaClass*, QSet<AbstractMetaClass* >* > i(hash);
            while (i.hasNext()) {
                i.next();
                i.value()->remove(cls);
                if (i.value()->empty()) {
                    AbstractMetaClass *key = i.key();
                    noDependency.insert(key);
                    hash.remove(key);
                    delete(i.value());
                }
            }
        }
    }

    if (!noDependency.empty() || !hash.empty()) {
        qWarning("dependency graph was cyclic.");
    }

    return res;
}
