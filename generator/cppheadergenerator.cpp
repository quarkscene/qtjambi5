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

#include "cppheadergenerator.h"
#include "metainfogenerator.h"

#include <QtCore/QDir>

#include <qdebug.h>

static Indentor INDENT;

QString CppHeaderGenerator::fileNameForClass(const AbstractMetaClass *java_class) const {
    if(java_class->typeEntry()->designatedInterface()){
        return fileNameForClass(java_class->enclosingClass());
    }
    return QString("%1_shell.h").arg(java_class->name().replace("$", "_"));
}

QString CppHeaderGenerator::fileNameForFunctional(const AbstractMetaFunctional *java_class) const {
    if(java_class->enclosingClass() && !java_class->enclosingClass()->isFake())
        return QString("%1_%2_shell.h").arg(java_class->enclosingClass()->name().replace("$", "_")).arg(java_class->name().replace("$", "_"));
    else
        return QString("%1_shell.h").arg(java_class->name().replace("$", "_"));
}

void CppHeaderGenerator::writeFieldAccessors(QTextStream &s, const AbstractMetaField *java_field) {
    Q_ASSERT(java_field->isProtected());

    const AbstractMetaFunction *setter = java_field->setter();
    const AbstractMetaFunction *getter = java_field->getter();

    if (!java_field->type()->isConstant())
        writeFunction(s, setter);

    writeFunction(s, getter);
}

void CppHeaderGenerator::write(QTextStream &s, const AbstractMetaFunctional *java_class, int) {
#if 0
    {
        QString fileName("written_classes_h.log");
        QFile file(fileName);
        QFile::OpenMode flags(QFile::WriteOnly);
        if(file.exists()){
            flags.setFlag(QFile::Append);
        }
        if (file.open(flags)) {
            QTextStream s(&file);
            s << java_class->typeEntry()->qualifiedCppName() << endl;
            file.close();
        }
    }
#endif
    QString include_block = java_class->name().replace("$", "_").toUpper() + "_SHELL_H";
    s << "#ifndef " << include_block << endl
      << "#define " << include_block << endl << endl;

    s << "#include <qtjambi/qtjambi_core.h>" << endl
      << "#include <QtCore/QHash>" << endl
      << "#include <QtCore/QWeakPointer>" << endl;
    QSet<QString> included;
    included << "<qtjambi/qtjambi_core.h>"
             << "<QtCore/QHash>"
             << "<QtCore/QWeakPointer>";

    if(java_class->enclosingClass()){
        writeInclude(s, java_class->enclosingClass()->typeEntry()->include(), included);
    }
    writeInclude(s, java_class->typeEntry()->include(), included);
    s << endl;

    IncludeList list = java_class->typeEntry()->extraIncludes();
    std::sort(list.begin(), list.end());
    for(const Include &inc : list) {
        if (inc.type != Include::TargetLangImport)
            writeInclude(s, inc, included);
    }

    if(java_class->enclosingClass() && !java_class->enclosingClass()->typeEntry()->ppCondition().isEmpty()){
        s << endl << "#if " << java_class->enclosingClass()->typeEntry()->ppCondition() << endl;
    }
    writeForwardDeclareSection(s, java_class);

    s << endl
      << "class " << shellClassName(java_class) << " final : public FunctionalBase" << endl
      << "{" << endl
      << "public:" << endl
      << "    " << shellClassName(java_class) << "();" << endl
      << "    ~" << shellClassName(java_class) << "() override;" << endl
      << "    void getFunctional(void*) override;" << endl
      << "    static void operator delete(void * ptr) noexcept;" << endl << endl
      << "    class Functor final : private FunctorBase{" << endl
      << "    public:" << endl
      << "        Functor(const Functor& functor);" << endl
      << "        ~Functor() override;" << endl
      << "        ";
    if(java_class->type()){
        writeTypeInfo(s, java_class->type(), Option());
    }else{
        s << "void";
    }
    s << " operator()(";
    int counter = 0;
    for(AbstractMetaArgument * arg : java_class->arguments()){
        if(counter>0)
            s << ", ";
        writeTypeInfo(s, arg->type(), Option());
        s << arg->indexedName();
        ++counter;
    }
    s << ");" << endl
      << "    private:" << endl
      << "        Functor(" << shellClassName(java_class) << "& functional);" << endl
      << "        friend class " << shellClassName(java_class) << ";" << endl
      << "    };" << endl << endl
      << "    static jobject resolveFunctional(JNIEnv *, const " << java_class->typeEntry()->qualifiedCppName() << "*);" << endl << endl
      << "protected:" << endl
      << "    QtJambiShell* __shell() const override;" << endl
      << "private:" << endl
      << "    friend class " << shellClassName(java_class) << "::Functor;" << endl;
    if(java_class->isFunctionPointer()){
        s << "    std::function<void()> m_functionPointerDeleter;" << endl
          << "    static std::function<const Functor*(" << java_class->typeEntry()->qualifiedCppName() << ")> reverseFunctionGetter;" << endl;
    }

    s  << "};" << endl << endl;



    if(java_class->enclosingClass() && !java_class->enclosingClass()->typeEntry()->ppCondition().isEmpty()){
        s << "#endif // " << java_class->enclosingClass()->typeEntry()->ppCondition() << endl;
    }
    s  << "#endif // " << include_block << endl;

    QString pro_file_name = MetaInfoGenerator::subDirectoryForClass(java_class, MetaInfoGenerator::CppDirectory) + "/generated.pri";

    priGenerator->addHeader(pro_file_name, fileNameForFunctional(java_class));
}

void CppHeaderGenerator::write(QTextStream &s, const AbstractMetaClass *java_class, int) {
#if 0
    {
        QString fileName("written_classes_h.log");
        QFile file(fileName);
        QFile::OpenMode flags(QFile::WriteOnly);
        if(file.exists()){
            flags.setFlag(QFile::Append);
        }
        if (file.open(flags)) {
            QTextStream s(&file);
            s << java_class->typeEntry()->qualifiedCppName() << endl;
            file.close();
        }
    }
#endif
    QString include_block = java_class->name().replace("$", "_").toUpper() + "_SHELL_H";

    s << "#ifndef " << include_block << endl
      << "#define " << include_block << endl << endl;

    s << "#include <qtjambi/qtjambi_core.h>" << endl
      << "#include <QtCore/QHash>" << endl
      << "#include <QtCore/QWeakPointer>" << endl;

    QSet<QString> included;
    included << "<qtjambi/qtjambi_core.h>"
             << "<QtCore/QHash>"
             << "<QtCore/QWeakPointer>";

    if(java_class->enclosingClass()){
        writeInclude(s, java_class->enclosingClass()->typeEntry()->include(), included);
    }
    writeInclude(s, java_class->typeEntry()->include(), included);

    IncludeList list = java_class->typeEntry()->extraIncludes();
    std::sort(list.begin(), list.end());
    for(const Include &inc : list) {
        if (inc.type == Include::TargetLangImport)
            continue;
        writeInclude(s, inc, included);
    }
    s << endl;

    if(!java_class->typeEntry()->ppCondition().isEmpty()){
        s << endl << "#if " << java_class->typeEntry()->ppCondition() << endl;
    }

    writeForwardDeclareSection(s, java_class);

    if (java_class->generateShellClass()) {
        s << endl
          << "class " << shellClassName(java_class)
          << " : public " << java_class->qualifiedCppName() << endl
          << "{" << endl;

        s << "public:" << endl;
        if(!java_class->hasPrivateDestructor()){
            for(const AbstractMetaFunction *function : java_class->functions()) {
                if (function->isConstructor() && !function->isPrivate()){
                    QStringList ppConditions;
                    for(const AbstractMetaArgument *argument : function->arguments()) {
                        if(function->argumentRemoved(argument->argumentIndex()+1)!=ArgumentRemove_No){
                            if(!argument->type()->typeEntry()->ppCondition().isEmpty() && !ppConditions.contains(argument->type()->typeEntry()->ppCondition())){
                                ppConditions << argument->type()->typeEntry()->ppCondition();
                            }
                            for(const AbstractMetaType* inst : argument->type()->instantiations()){
                                if(!inst->typeEntry()->ppCondition().isEmpty() && !ppConditions.contains(inst->typeEntry()->ppCondition())){
                                    ppConditions << inst->typeEntry()->ppCondition();
                                }
                            }
                        }
                    }
                    if(!java_class->typeEntry()->ppCondition().isEmpty()){
                        ppConditions.removeAll(java_class->typeEntry()->ppCondition());
                    }
                    if(!ppConditions.isEmpty()){
                        for (int i=0; i<ppConditions.size(); ++i) {
                            if(ppConditions[i].contains("||")){
                                ppConditions[i] = "(" + ppConditions[i] + ")";
                            }
                        }
                        s << endl << "#if " << ppConditions.join(" && ") << endl << endl;
                    }
                    Option option = Option(SkipRemovedArguments);
                    //if(java_class->isQObject()){
                        option = Option(option | IncludeDefaultExpression);
                    //}
                    if(!function->isPublic())
                        option = Option(option | EnumAsInts);
                    writeFunction(s, function, option);
                    if(!ppConditions.isEmpty()){
                        s << endl << "#endif //" << ppConditions.join(" && ") << endl << endl;
                    }
                }
            }

            s << "    ~" << shellClassName(java_class) << "()";
            if(java_class->hasVirtualDestructor()){
                s << " override";
            }
            s << ";" << endl;
        }
        s << endl;

        QList<const AbstractMetaFunction *> functionsInTargetLang;
        QList<const AbstractMetaFunction *> signalsInTargetLang;

        for(AbstractMetaFunction* signal : java_class->cppSignalFunctions()){
            if (signal->declaringClass() == java_class && !signal->hasTemplateArgumentTypes())
                signalsInTargetLang << signal;
        }

        for(AbstractMetaFunction *function : java_class->functionsInTargetLang()) {
            if (
                (!function->isConstructor() ||
                 !java_class->hasUnimplmentablePureVirtualFunction()) &&
                 !function->isEmptyFunction() &&
                 !function->hasTemplateArgumentTypes()
                )
                functionsInTargetLang << function;
        }
        for(AbstractMetaFunction *function : java_class->queryFunctions(AbstractMetaClass::NormalFunctions
                                                                           | AbstractMetaClass::AbstractFunctions
                                                                           | AbstractMetaClass::NotRemovedFromTargetLang)) {
            if (function->implementingClass() != java_class &&
                    !function->hasTemplateArgumentTypes() ) {
                functionsInTargetLang << function;
            }
        }

        // All functions in original class that should be reimplemented in shell class
        QList<const AbstractMetaFunction *> privatePureVirtuals;
        for(const AbstractMetaFunction *function : java_class->functionsInShellClass()) {
            if(function->isFinalInCpp() && !function->isVirtualSlot())
                continue;
            if(function->wasPrivate()){
                if(function->isAbstract())
                    privatePureVirtuals << function;
            }else{
                writeFunction(s, function);
            }
        }

        // Public call throughs for protected functions
        for(const AbstractMetaFunction *function : java_class->publicOverrideFunctions()) {
            if(functionsInTargetLang.contains(function) || signalsInTargetLang.contains(function))
                writePublicFunctionOverride(s, function);
        }

        // Override all virtual functions to get the decision on static/virtual call
        for(const AbstractMetaFunction *function : java_class->virtualOverrideFunctions()) {
            if(!function->hasTemplateArgumentTypes()
                    && !function->isRemovedFrom(java_class, TypeSystem::TargetLangCode)
                    && !function->isRemovedFrom(function->declaringClass(), TypeSystem::TargetLangCode)
                    && !function->isModifiedRemoved(TypeSystem::NativeCode)
                    && !function->hasRReferences()
                    && !function->wasPrivate()
            ){
                writeVirtualFunctionOverride(s, function);
            }
        }

        // Field accessors
        for(const AbstractMetaField *field : java_class->fields()) {
            if (field->isProtected())
                writeFieldAccessors(s, field);
        }

        const AbstractMetaClass* interfaceClass = nullptr;
        for(const AbstractMetaClass* iface : java_class->interfaces()){
            ImplementorTypeEntry * origin = static_cast<const InterfaceTypeEntry *>(iface->typeEntry())->origin();
            if(origin==java_class->typeEntry()){
                interfaceClass = iface;
                break;
            }
        }
        s << "    static void operator delete(void * ptr) noexcept;" << endl;
        if (java_class->isQObject() && java_class->qualifiedCppName()!="QDBusInterface") {
            s << "    const QMetaObject *metaObject() const override final;" << endl
              << "    void *qt_metacast(const char *) override final;" << endl
              << "    int qt_metacall(QMetaObject::Call, int, void **) override final;" << endl;
        }
        QList<AbstractMetaEnum *> protetedEnums;
        for(AbstractMetaEnum *cpp_enum : java_class->enums()){
            if(cpp_enum->isProtected() || !java_class->isPublic()){
                protetedEnums << cpp_enum;
            }
        }
        if(interfaceClass){
            for(AbstractMetaEnum *cpp_enum : interfaceClass->enums()){
                if(cpp_enum->isProtected() || !interfaceClass->isPublic()){
                    protetedEnums << cpp_enum;
                }
            }
        }
        {
            INDENTATION(INDENT)
            for(AbstractMetaEnum * cpp_enum : protetedEnums){
                s << INDENT << "static inline const std::type_info& __registerEnumTypeInfo_" << cpp_enum->name().replace("::", "_") << "() {" << endl;
                {
                    INDENTATION(INDENT)
                    const EnumTypeEntry *entry = cpp_enum->typeEntry();
                    const QString qtEnumName = entry->qualifiedCppName();
                    const QString javaEnumName = [java_class,entry]()->QString{
                        if(java_class){
                            if(java_class->typeEntry()->targetLangName()=="package_global"){
                                if(java_class->typeEntry()->javaPackage().isEmpty()){
                                    return entry->targetLangName();
                                }else{
                                    return java_class->typeEntry()->javaPackage().replace(".", "/") + "/" + entry->targetLangName();
                                }
                            }else{
                                if(java_class->typeEntry()->javaPackage().isEmpty()){
                                    return java_class->typeEntry()->targetLangName() + "$" + entry->targetLangName();
                                }else{
                                    return java_class->typeEntry()->javaPackage().replace(".", "/") + "/" + java_class->typeEntry()->targetLangName() + "$" + entry->targetLangName();
                                }
                            }
                        }else{
                            return entry->targetLangName();
                        }
                    }();
                    if(const FlagsTypeEntry * fentry = cpp_enum->typeEntry()->flags()){
                        const QString qtFlagName = fentry->qualifiedCppName();
                        const QString javaFlagName = [java_class,fentry]()->QString{
                            if(java_class){
                                if(java_class->typeEntry()->javaPackage().isEmpty()){
                                    return java_class->typeEntry()->targetLangName() + "$" + fentry->targetLangName();
                                }else{
                                    return java_class->typeEntry()->javaPackage().replace(".", "/") + "/" + java_class->typeEntry()->targetLangName() + "$" + fentry->targetLangName();
                                }
                            }else{
                                return fentry->targetLangName();
                            }
                        }();
                        QString qtFlagsAliasName = fentry->originalName();
                        s << INDENT << "return registerEnumTypeInfo" << (entry->isHiddenMetaObject() ? "NoMetaObject" : "") << "<" << qtEnumName << ">(\"" << qtEnumName << "\", \"" << javaEnumName << "\", \"" << qtFlagName << "\", \"" << qtFlagsAliasName << "\", \"" << javaFlagName << "\");" << endl;
                    }else{
                        s << INDENT << "return registerEnumTypeInfo" << (entry->isHiddenMetaObject() ? "NoMetaObject" : "") << "<" << qtEnumName << ">(\"" << qtEnumName << "\", \"" << javaEnumName << "\");" << endl;
                    }
                    s << INDENT << "}" << endl;
                }
            }
        }

        s << "private:" << endl;
        for(const AbstractMetaFunction *function : privatePureVirtuals) {
            writeFunction(s, function);
        }
        s << "    QtJambiShell* __shell() const;" << endl;
        s << "    jmethodID __shell_javaMethod(int pos) const;" << endl;

        //writeVariablesSection(s, java_class, interfaceClass!=nullptr);
        writeInjectedCode(s, java_class);

        s  << "};" << endl << endl;
    }

    if(!java_class->typeEntry()->ppCondition().isEmpty()){
        s << "#endif // " << java_class->typeEntry()->ppCondition() << endl;
    }

    s  << "#endif // " << include_block << endl;

    QString pro_file_name = MetaInfoGenerator::subDirectoryForClass(java_class, MetaInfoGenerator::CppDirectory) + "/generated.pri";

    priGenerator->addHeader(pro_file_name, fileNameForClass(java_class));
}


/*!
    Writes out declarations of virtual C++ functions so that they
    can be reimplemented from the java side.
*/
void CppHeaderGenerator::writeFunction(QTextStream &s, const AbstractMetaFunction *java_function, Option options) {
    if (java_function->hasRReferences())
        return ;
    if (java_function->isModifiedRemoved(TypeSystem::ShellCode))
        return;
    if (java_function->hasTemplateArgumentTypes())
        return;
    QStringList pps = getFunctionPPConditions(java_function);
    if(!pps.isEmpty()){
        s << endl << "#if " << pps.join(" && ") << endl;
    }
    s << "    ";
    writeFunctionSignature(s, java_function, nullptr, QString(), Option(OriginalName | ShowStatic | ShowOverride | options));
    s << ";" << endl;
    if(!pps.isEmpty()){
        s << "#endif //" << pps.join(" && ") << endl << endl;
    }
}

void CppHeaderGenerator::writePublicFunctionOverride(QTextStream &s,
        const AbstractMetaFunction *java_function) {
    QStringList pps = getFunctionPPConditions(java_function);
    if(!pps.isEmpty()){
        s << endl << "#if " << pps.join(" && ") << endl;
    }
    s << "    ";
    writeFunctionSignature(s, java_function, nullptr, "__public_", Option(EnumAsInts |
                           ShowStatic | UnderscoreSpaces));
    s << ";" << endl;
    if(!pps.isEmpty()){
        s << "#endif //" << pps.join(" && ") << endl << endl;
    }
}


void CppHeaderGenerator::writeVirtualFunctionOverride(QTextStream &s,
        const AbstractMetaFunction *java_function) {
    QStringList pps = getFunctionPPConditions(java_function);
    if(!pps.isEmpty()){
        s << endl << "#if " << pps.join(" && ") << endl;
    }
    s << "    ";
    writeFunctionSignature(s, java_function, nullptr, "__override_", Option(EnumAsInts | ShowStatic |
                           UnderscoreSpaces), QString(), QStringList() << ( java_function->isAbstract() ? "JNIEnv *__jni_env, QtJambiNativeID __this_nativeId" : "QtJambiNativeID __this_nativeId") );
    s << ";" << endl;
    if(!pps.isEmpty()){
        s << "#endif //" << pps.join(" && ") << endl << endl;
    }
}


void CppHeaderGenerator::writeForwardDeclareSection(QTextStream &, const AbstractMetaClass *) {
}

void CppHeaderGenerator::writeForwardDeclareSection(QTextStream &, const AbstractMetaFunctional *) {
}


void CppHeaderGenerator::writeVariablesSection(QTextStream &, const AbstractMetaClass *, bool) {
}

void CppHeaderGenerator::writeInjectedCode(QTextStream &s, const AbstractMetaClass *java_class) {
    CodeSnipList code_snips = java_class->typeEntry()->codeSnips();
    for(const CodeSnip &cs : code_snips) {
        if (cs.language == TypeSystem::ShellDeclaration) {
            s << cs.code() << endl;
        }
    }
}
