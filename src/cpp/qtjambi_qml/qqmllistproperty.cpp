/****************************************************************************
**
** Copyright (C) 2009-2020 Dr. Peter Droste, Omix Visualization GmbH & Co. KG. All rights reserved.
**
** This file is part of Qt Jambi.
**
** ** $BEGIN_LICENSE$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
** $END_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtCore/QtCore>
#include <QtQml/QtQml>

#include "qqmllistproperty.h"
#include <qtjambi/qtjambi_core.h>
#include <qtjambi/qtjambi_repository.h>
#include <qtjambi/qtjambi_templates.h>
#include <qtjambi/qtjambi_registry.h>
#include <qtjambi/qtdynamicmetaobject_p.h>
#include <qtjambi/qtjambi_cast.h>

// emitting (writeExtraFunctions)
// emitting (writeToStringFunction)
// emitting (writeCloneFunction)

void deleter_QQmlListProperty(void* ptr){
    delete reinterpret_cast<QQmlListProperty<QObject> *>(ptr);
}

extern "C" Q_DECL_EXPORT jobject JNICALL QTJAMBI_FUNCTION_PREFIX(Java_io_qt_qml_QQmlListProperty__1_1qt_1clone)
(JNIEnv *__jni_env, jobject, QtJambiNativeID __this_nativeId)
{
    QTJAMBI_DEBUG_METHOD_PRINT("native", "QQmlListProperty::clone(QQmlListProperty)")
    try{
        QQmlListProperty<QObject> *__qt_this = qtjambi_object_from_nativeId<QQmlListProperty<QObject> >(__this_nativeId);
        qtjambi_check_resource(__jni_env, __qt_this, typeid(QQmlListProperty<QObject>));
        return qtjambi_cast<jobject>(__jni_env, *__qt_this);
    }catch(const JavaException& exn){
        exn.raiseInJava(__jni_env);
        return nullptr;
    }
}

// new QQmlListProperty()
void __qt_create_new_QQmlListProperty_0(void* __qtjambi_ptr, JNIEnv*, jobject, jvalue*)
{
    QTJAMBI_DEBUG_METHOD_PRINT("native", "new QQmlListProperty<QObject>()")
    QQmlListProperty<QObject> *__qt_this = new(__qtjambi_ptr) QQmlListProperty<QObject>();
    Q_UNUSED(__qt_this)
}

// new QQmlListProperty(QObject * o, QList<QObject > & list)
void __qt_create_new_QQmlListProperty_1(void* __qtjambi_ptr, JNIEnv* __jni_env, jobject, jvalue* __java_arguments)
{
    QTJAMBI_DEBUG_METHOD_PRINT("native", "new QQmlListProperty<QObject>(QObject * o, QList<QObject > & list)()")
    jobject o0 = __java_arguments[0].l;
    jobject list1 = __java_arguments[1].l;
    QQmlListProperty<QObject>* listProperty = new(__qtjambi_ptr) QQmlListProperty<QObject>();
    listProperty->object = qtjambi_to_qobject(__jni_env, o0);
    listProperty->data = __jni_env->NewWeakGlobalRef(list1);

    listProperty->append = [](QQmlListProperty<QObject> * list, QObject* object){
        if(JNIEnv * env = qtjambi_current_environment()){
            QTJAMBI_JNI_LOCAL_FRAME(env, 200)
            jobject javaList = jobject(list->data);
            if(!env->IsSameObject(javaList, nullptr)){
                try{
                    jobject java_object = qtjambi_from_QObject(env, object);
                    qtjambi_collection_add(env, javaList, java_object);
                } catch (const JavaException& exn) {
                    exn.raise( QTJAMBI_STACKTRACEINFO_ENV(env) );
                }
            }
        }
    };
    listProperty->count = [](QQmlListProperty<QObject> * list) -> int {
        if(JNIEnv * env = qtjambi_current_environment()){
            QTJAMBI_JNI_LOCAL_FRAME(env, 200)
            jobject javaList = jobject(list->data);
            if(!env->IsSameObject(javaList, nullptr)){
                try{
                    return qtjambi_collection_size(env, javaList);
                } catch (const JavaException& exn) {
                    exn.raise( QTJAMBI_STACKTRACEINFO_ENV(env) );
                }
            }
        }
        return 0;
    };
    listProperty->at = [](QQmlListProperty<QObject> * list, int index) -> QObject* {
        if(JNIEnv * env = qtjambi_current_environment()){
            QTJAMBI_JNI_LOCAL_FRAME(env, 200)
            jobject javaList = jobject(list->data);
            jobject result = qtjambi_list_get(env, javaList, index);
            if(!env->IsSameObject(javaList, nullptr)){
                try {
                    return qtjambi_to_qobject(env, result);
                } catch (const JavaException& exn) {
                    exn.raise( QTJAMBI_STACKTRACEINFO_ENV(env) );
                }
            }
        }
        return nullptr;
    };
    listProperty->clear = [](QQmlListProperty<QObject> * list){
        if(JNIEnv * env = qtjambi_current_environment()){
            QTJAMBI_JNI_LOCAL_FRAME(env, 200)
            jobject javaList = jobject(list->data);
            if(!env->IsSameObject(javaList, nullptr)){
                try {
                    qtjambi_collection_clear(env, javaList);
                } catch (const JavaException& exn) {
                    exn.raise( QTJAMBI_STACKTRACEINFO_ENV(env) );
                }
            }
        }
    };
}

// destruct QQmlListProperty
void __qt_destruct_QQmlListProperty(void* ptr)
{
    QTJAMBI_DEBUG_METHOD_PRINT("native", "destruct QQmlListProperty")
    reinterpret_cast<QQmlListProperty<QObject>*>(ptr)->~QQmlListProperty();
}

// emitting (writeSignalFunction)
// emitting  (functionsInTargetLang writeFinalFunction)
// QQmlListProperty::QQmlListProperty()
extern "C" Q_DECL_EXPORT void JNICALL QTJAMBI_FUNCTION_PREFIX(Java_io_qt_qml_QQmlListProperty__1_1qt_1QQmlListProperty_1new__Ljava_lang_Object_2)
(JNIEnv *__jni_env,
 jclass __jni_class, jobject __jni_object)
{
    QTJAMBI_DEBUG_METHOD_PRINT("native", "QQmlListProperty::QQmlListProperty()")
    try{
        qtjambi_initialize_native_object(__jni_env, __jni_class, __jni_object, &__qt_create_new_QQmlListProperty_0, sizeof(QQmlListProperty<QObject>), typeid(QQmlListProperty<QObject>), false, false, &deleter_QQmlListProperty, nullptr, nullptr);
    }catch(const JavaException& exn){
        exn.raiseInJava(__jni_env);
    }
}

// QQmlListProperty::QQmlListProperty(QObject * o, QList<QObject > & list)
extern "C" Q_DECL_EXPORT void JNICALL QTJAMBI_FUNCTION_PREFIX(Java_io_qt_qml_QQmlListProperty__1_1qt_1QQmlListProperty_1new__Ljava_lang_Object_2Lio_qt_core_QObject_2Ljava_util_List_2)
(JNIEnv *__jni_env,
 jclass __jni_class,
 jobject __jni_object,
 jobject o0,
 jobject list1)
{
    QTJAMBI_DEBUG_METHOD_PRINT("native", "QQmlListProperty::QQmlListProperty(QObject * o, QList<QObject > & list)")
    try{
        jvalue arguments[2];
        arguments[0].l = o0;
        arguments[1].l = list1;
        qtjambi_initialize_native_object(__jni_env, __jni_class, __jni_object, &__qt_create_new_QQmlListProperty_1, sizeof(QQmlListProperty<QObject>), typeid(QQmlListProperty<QObject>), false, false, &deleter_QQmlListProperty, nullptr, arguments);
    }catch(const JavaException& exn){
        exn.raiseInJava(__jni_env);
    }
}

// QQmlListProperty::append(QObject object)
extern "C" Q_DECL_EXPORT void JNICALL QTJAMBI_FUNCTION_PREFIX(Java_io_qt_qml_QQmlListProperty__1_1qt_1append_1QObject__JLio_qt_core_QObject_2)
(JNIEnv *__jni_env,
 jobject,
 QtJambiNativeID __this_nativeId,
 jobject object0)
{
    QTJAMBI_DEBUG_METHOD_PRINT("native", "QQmlListProperty::append(QObject object)")
    try{
        QQmlListProperty<QObject> *__qt_this = qtjambi_object_from_nativeId<QQmlListProperty<QObject> >(__this_nativeId);
        qtjambi_check_resource(__jni_env, __qt_this, typeid(QQmlListProperty<QObject>));
        if(__qt_this->append)
            __qt_this->append(__qt_this, qtjambi_cast<QObject*>(__jni_env, object0));
    }catch(const JavaException& exn){
        exn.raiseInJava(__jni_env);
    }
}

// QQmlListProperty::at(int index)
extern "C" Q_DECL_EXPORT jobject JNICALL QTJAMBI_FUNCTION_PREFIX(Java_io_qt_qml_QQmlListProperty__1_1qt_1at_1int__JI)
(JNIEnv *__jni_env,
 jobject,
 QtJambiNativeID __this_nativeId,
 jint index0)
{
    QTJAMBI_DEBUG_METHOD_PRINT("native", "QQmlListProperty::at(int index)")
    try{
        QQmlListProperty<QObject> *__qt_this = qtjambi_object_from_nativeId<QQmlListProperty<QObject> >(__this_nativeId);
        qtjambi_check_resource(__jni_env, __qt_this, typeid(QQmlListProperty<QObject>));
        jobject __qt_return_value = nullptr;
        if(__qt_this->at){
            QObject*  qt_return_value = __qt_this->at(__qt_this, int(index0));
            __qt_return_value = qtjambi_cast<jobject>(__jni_env, qt_return_value);
        }
        return __qt_return_value;
    }catch(const JavaException& exn){
        exn.raiseInJava(__jni_env);
        return nullptr;
    }
}

// QQmlListProperty::canAppend()
extern "C" Q_DECL_EXPORT jboolean JNICALL QTJAMBI_FUNCTION_PREFIX(Java_io_qt_qml_QQmlListProperty__1_1qt_1canAppend__J)
(JNIEnv * __jni_env,
 jobject,
 QtJambiNativeID __this_nativeId)
{
    QTJAMBI_DEBUG_METHOD_PRINT("native", "QQmlListProperty::canAppend()")
    try{
        QQmlListProperty<QObject> *__qt_this = qtjambi_object_from_nativeId<QQmlListProperty<QObject> >(__this_nativeId);
        qtjambi_check_resource(__jni_env, __qt_this, typeid(QQmlListProperty<QObject>));
        return __qt_this->append!=nullptr;
    }catch(const JavaException& exn){
        exn.raiseInJava(__jni_env);
        return false;
    }
}

// QQmlListProperty::canAt()
extern "C" Q_DECL_EXPORT jboolean JNICALL QTJAMBI_FUNCTION_PREFIX(Java_io_qt_qml_QQmlListProperty__1_1qt_1canAt__J)
(JNIEnv * __jni_env,
 jobject,
 QtJambiNativeID __this_nativeId)
{
    QTJAMBI_DEBUG_METHOD_PRINT("native", "QQmlListProperty::canAt()")
    try{
        QQmlListProperty<QObject> *__qt_this = qtjambi_object_from_nativeId<QQmlListProperty<QObject> >(__this_nativeId);
        qtjambi_check_resource(__jni_env, __qt_this, typeid(QQmlListProperty<QObject>));
        return __qt_this->at!=nullptr;
    }catch(const JavaException& exn){
        exn.raiseInJava(__jni_env);
        return false;
    }
}

// QQmlListProperty::canClear()
extern "C" Q_DECL_EXPORT jboolean JNICALL QTJAMBI_FUNCTION_PREFIX(Java_io_qt_qml_QQmlListProperty__1_1qt_1canClear__J)
(JNIEnv * __jni_env,
 jobject,
 QtJambiNativeID __this_nativeId)
{
    QTJAMBI_DEBUG_METHOD_PRINT("native", "QQmlListProperty::canClear()")
    try{
        QQmlListProperty<QObject> *__qt_this = qtjambi_object_from_nativeId<QQmlListProperty<QObject> >(__this_nativeId);
        qtjambi_check_resource(__jni_env, __qt_this, typeid(QQmlListProperty<QObject>));
        return __qt_this->clear!=nullptr;
    }catch(const JavaException& exn){
        exn.raiseInJava(__jni_env);
        return false;
    }
}

// QQmlListProperty::canCount()
extern "C" Q_DECL_EXPORT jboolean JNICALL QTJAMBI_FUNCTION_PREFIX(Java_io_qt_qml_QQmlListProperty__1_1qt_1canCount__J)
(JNIEnv * __jni_env,
 jobject,
 QtJambiNativeID __this_nativeId)
{
    QTJAMBI_DEBUG_METHOD_PRINT("native", "QQmlListProperty::canCount()")
    try{
        QQmlListProperty<QObject> *__qt_this = qtjambi_object_from_nativeId<QQmlListProperty<QObject> >(__this_nativeId);
        qtjambi_check_resource(__jni_env, __qt_this, typeid(QQmlListProperty<QObject>));
        return __qt_this->count!=nullptr;
    }catch(const JavaException& exn){
        exn.raiseInJava(__jni_env);
        return false;
    }
}

// QQmlListProperty::clear()
extern "C" Q_DECL_EXPORT void JNICALL QTJAMBI_FUNCTION_PREFIX(Java_io_qt_qml_QQmlListProperty__1_1qt_1clear__J)
(JNIEnv * __jni_env,
 jobject,
 QtJambiNativeID __this_nativeId)
{
    QTJAMBI_DEBUG_METHOD_PRINT("native", "QQmlListProperty::clear()")
    try{
        QQmlListProperty<QObject> *__qt_this = qtjambi_object_from_nativeId<QQmlListProperty<QObject> >(__this_nativeId);
        qtjambi_check_resource(__jni_env, __qt_this, typeid(QQmlListProperty<QObject>));
        if(__qt_this->clear)
            __qt_this->clear(__qt_this);
    }catch(const JavaException& exn){
        exn.raiseInJava(__jni_env);
    }
}

// QQmlListProperty::count()
extern "C" Q_DECL_EXPORT jint JNICALL QTJAMBI_FUNCTION_PREFIX(Java_io_qt_qml_QQmlListProperty__1_1qt_1count__J)
(JNIEnv * __jni_env,
 jobject,
 QtJambiNativeID __this_nativeId)
{
    QTJAMBI_DEBUG_METHOD_PRINT("native", "QQmlListProperty::count()")
    try{
        QQmlListProperty<QObject> *__qt_this = qtjambi_object_from_nativeId<QQmlListProperty<QObject> >(__this_nativeId);
        qtjambi_check_resource(__jni_env, __qt_this, typeid(QQmlListProperty<QObject>));
        if(__qt_this->count)
            return __qt_this->count(__qt_this);
    }catch(const JavaException& exn){
        exn.raiseInJava(__jni_env);
    }
    return 0;
}

// QQmlListProperty::object()
extern "C" Q_DECL_EXPORT jobject JNICALL QTJAMBI_FUNCTION_PREFIX(Java_io_qt_qml_QQmlListProperty__1_1qt_1object__J)
(JNIEnv *__jni_env,
 jobject,
 QtJambiNativeID __this_nativeId)
{
    QTJAMBI_DEBUG_METHOD_PRINT("native", "QQmlListProperty::object()")
    try{
        QQmlListProperty<QObject> *__qt_this = qtjambi_object_from_nativeId<QQmlListProperty<QObject> >(__this_nativeId);
        qtjambi_check_resource(__jni_env, __qt_this, typeid(QQmlListProperty<QObject>));
        return qtjambi_cast<jobject>(__jni_env, __qt_this->object);
    }catch(const JavaException& exn){
        exn.raiseInJava(__jni_env);
        return nullptr;
    }
}

// emitting (AbstractMetaClass::NormalFunctions|AbstractMetaClass::AbstractFunctions writeFinalFunction)
// emitting Field accessors (writeFieldAccessors)
// emitting (writeInterfaceCastFunction)
// emitting (writeSignalInitialization)
// emitting (writeJavaLangObjectOverrideFunctions)

void initialize_meta_info_QQmlListProperty(){
    setQmlReportDestruction(&QQmlPrivate::qdeclarativeelement_destructor);
    const std::type_info& typeId = registerTypeInfo<QQmlListProperty<QObject>>("QQmlListProperty", "io/qt/qml/QQmlListProperty");
    registerOperators<QQmlListProperty<QObject>>();
    registerConstructorInfos(typeId, &__qt_destruct_QQmlListProperty, QVector<ConstructorInfo>()
        << ConstructorInfo(&__qt_create_new_QQmlListProperty_0, nullptr)
        << ConstructorInfo(&__qt_create_new_QQmlListProperty_1, "Lio/qt/core/QObject;Ljava/util/List;") );
    registerDeleter(typeId, &deleter_QQmlListProperty);
    int metaTypeID = registerMetaType<QQmlListProperty<QObject>>("QQmlListProperty<QObject>",
                            &qtjambiGenericDestructor<QQmlListProperty<QObject>>,
                            [](void *where, const void *ptr) -> void *{
                                QQmlListProperty<QObject>* listProperty = new(where) QQmlListProperty<QObject>();
                                if (ptr){
                                    const QQmlListProperty<QObject>* _ptr = reinterpret_cast<const QQmlListProperty<QObject>*>(ptr);
                                    listProperty->object = _ptr->object;
                                    listProperty->data = _ptr->data;
                                    listProperty->append = _ptr->append;
                                    listProperty->at = _ptr->at;
                                    listProperty->clear = _ptr->clear;
                                    listProperty->count = _ptr->count;
                                    listProperty->dummy1 = _ptr->dummy1;
                                    listProperty->dummy2 = _ptr->dummy2;
                                }
                                return listProperty;
                            }
                        );
    QMetaType::registerNormalizedTypedef("QQmlListProperty", metaTypeID);
}

