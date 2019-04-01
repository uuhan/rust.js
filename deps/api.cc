
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <libplatform/libplatform.h>
#include <v8.h>

#include "api.h"

using namespace v8;

class ArrayBufferAllocator : public ArrayBuffer::Allocator
{
public:
    static ArrayBufferAllocator the_singleton;
    virtual void *Allocate(size_t length);
    virtual void *AllocateUninitialized(size_t length);
    virtual void Free(void *data, size_t);
};

ArrayBufferAllocator ArrayBufferAllocator::the_singleton;

void *
ArrayBufferAllocator::Allocate(size_t length)
{
    void *data = AllocateUninitialized(length);
    return data == NULL ? data : memset(data, 0, length);
}

void *
ArrayBufferAllocator::AllocateUninitialized(size_t length)
{
    return malloc(length);
}

void
ArrayBufferAllocator::Free(void *data, size_t)
{
    free(data);
}

extern "C" {

static Platform *default_platform;
static ArrayBufferAllocator array_buffer_allocator;

// The global isolate
Isolate *isolate;
// The global context
Local<Context> context;

/**
 * v8 methods
 * @class v8
 */

/**
 * free the platform variable
 * @method v8_free_platform()
 * @return {bool} always return true
 */
bool
v8_free_platform()
{
    // delete default_platform;
    // default_platform = nullptr;
    V8::Dispose();
    V8::ShutdownPlatform();
    return true;
}

/**
 * initialize the platform
 * @method v8_initialize_platform
 * @return {bool} always return true
 */
std::unique_ptr<Platform>
v8_initialize_platform()
{
    std::unique_ptr<Platform> platform = platform::NewDefaultPlatform();
    V8::InitializePlatform(platform.get());
    V8::Initialize();
    return platform;
}

/**
 * initilize the v8 instance
 * @method v8_initialize
 * @return {bool} if there is some error during the process
 */
bool
v8_initialize()
{
    return V8::Initialize();
}

/**
 * dispose the v8
 * @method v8_dispose
 * @return {bool} if there is some error during the process
 */
bool
v8_dispose()
{
    return V8::Dispose();
}

/**
 * set the global array buffer
 * @method v8_set_array_buffer_allocator
 * @return {bool} always be true
 */
bool
v8_set_array_buffer_allocator()
{
    return true;
}

/**
 * check if the global isolate is locked
 * @method v8_locker_is_locked
 * @return {bool} the result, true or false
 */
bool
v8_locker_is_locked()
{
    return Locker::IsLocked(isolate);
}

/**
 * check if the global isolate is active
 * @method v8_locker_is_active
 * @return {bool} the result, true or false
 */
bool
v8_locker_is_active()
{
    return Locker::IsActive();
}

/**
 * create new locker with a locked callback
 * @method v8_locker
 * @param {rust_callback} callback - The rust closure function
 * @return {void} doesn't return anything
 */
void
v8_locker(rust_callback callback)
{
    Locker locker(isolate);
    callback();
}

/**
 * create new handle scope with a scoped callback
 * @method v8_handle_scope
 * @param {rust_callback} callback - The rust closure function
 * @return {void} doesn't return anything
 */
void
v8_handle_scope(rust_callback callback)
{
    HandleScope handle_scope(isolate);
    callback();
}

/**
 * create new isolate and assign to global isolate variable
 * @method v8_isolate_new
 * @return {void} doesn't return anything
 */
void
v8_isolate_new()
{
    Isolate::CreateParams create_params;
    create_params.array_buffer_allocator =
        v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    isolate = Isolate::New(create_params);
}

/**
 * dispose the global isolate
 * @method v8_isolate_dispose
 * @return {void} doesn't return anything
 */
void
v8_isolate_dispose()
{
    isolate->Dispose();
    isolate = nullptr;
}

/**
 * call isolate->Enter() at rust level
 * @method v8_isolate_enter
 * @return {void} doesn't return anything
 */
void
v8_isolate_enter()
{
    isolate->Enter();
}

/**
 * call isolate->Exit() at rust level
 * @method v8_isolate_exit
 * @return {void} doesn't return anything
 */
void
v8_isolate_exit()
{
    isolate->Exit();
}

/**
 * create new context and assign to global context variable
 * @method v8_context_new
 * @return {void} doesn't return anything
 */
void
v8_context_new()
{
    context = Context::New(isolate);
}

/**
 * call context->Enter() at rust level
 * @method v8_context_enter
 * @return {void} doesn't return anything
 */
void
v8_context_enter()
{
    context->Enter();
}

/**
 * call context->Exit() at rust level
 * @method v8_context_exit
 * @return {void} doesn't return anything
 */
void
v8_context_exit()
{
    context->Exit();
}

/**
 * get global variable namely context->Global()
 * @method v8_context_global
 * @return {Value} The global value
 */
Local<Value>
v8_context_global()
{
    return context->Global();
}

/**
 * create new context scope with a scoped callback
 * @method v8_context_scope
 * @param {rust_callback} callback - The rust closure function
 * @return {void} doesn't return anything
 */
void
v8_context_scope(rust_callback callback)
{
    Context::Scope context_scope(context);
    callback();
}

/**
 * The script class
 * @class Script
 */

/**
 * compile a script
 * @method v8_script_compile
 * @param {char *} data - The source of script
 * @return {Script} the compiled script object
 */
Local<Script>
v8_script_compile(char *data)
{
    Local<String> source = String::NewFromUtf8(isolate, data);
    Local<Script> script;
    if (!Script::Compile(context, source).ToLocal(&script)) {
        // TODO
    }
    return script;
}

/**
 * Compiles the specified script using the specified file
 * name object (typically a string) as the script's origin.
 * @method v8_script_compile_with_filename
 * @param {char *} data - The source of script
 * @param {char *} path - the file path
 * @return {Script} the compiled script object
 */
Local<Script>
v8_script_compile_with_filename(char *data, char *path)
{
    Local<String> source = String::NewFromUtf8(isolate, data);
    Local<String> filename = String::NewFromUtf8(isolate, path);
    Local<Script> script;
    if (!Script::Compile(context, source).ToLocal(&script)) {
        // TODO
    }
    return script;
}

/**
 * run the script
 * @method v8_script_run
 * @param {Script} this
 * @return {Value} the result
 */
Local<Value>
v8_script_run(Script **script)
{
    TryCatch try_catch(isolate);
    Local<Value> val;
    if ((*script)->Run(context).ToLocal(&val)) {
        // TODO
    }
    if (try_catch.HasCaught()) {
        String::Utf8Value msg(isolate, try_catch.Exception());
        printf("%s\n", *msg);
    }
    return val;
}

/**
 * check script is empty
 * @method v8_script_is_empty
 * @param {Script} this
 * @return {bool}
 */
bool
v8_script_is_empty(Local<Script> *script)
{
    return (*script).IsEmpty();
}

/**
 * The value class
 * @class Value
 */

/**
 * check the value if it's a string
 * @method v8_value_is_string
 * @param {Value} this
 * @return {bool} the result
 */
bool
v8_value_is_string(Value **val)
{
    return (*val)->IsString();
}

/**
 * check the value if it's a function
 * @method v8_value_is_function
 * @param {Value} this
 * @return {bool} the result
 */
bool
v8_value_is_function(Value **val)
{
    return (*val)->IsFunction();
}

/**
 * check the value if it's empty
 * @method v8_value_is_empty
 * @param {Value} this
 * @return {bool} the result
 */
bool
v8_value_is_undefined(Value **val)
{
    return (*val)->IsUndefined();
}

/**
 * call val->ToNumber()
 * @method v8_value_to_number
 * @param {Value} this
 * @return {Number} the result
 */
Local<Number>
v8_value_to_number(Value **val)
{
    return (*val)->ToNumber(context).ToLocalChecked();
}

/**
 * call val->ToInteger()
 * @method v8_value_to_integer
 * @param {Value} this
 * @return {Number} the result
 */
Local<Integer>
v8_value_to_integer(Value **val)
{
    return (*val)->ToInteger(context).ToLocalChecked();
}

/**
 * call val->ToBoolean()
 * @method v8_value_to_boolean
 * @param {Value} this
 * @return {Boolean} the result
 */
Local<Boolean>
v8_value_to_boolean(Value **val)
{
    return (*val)->ToBoolean(context).ToLocalChecked();
}

/**
 * call val->ToString()
 * @method v8_value_to_string
 * @param {Value} this
 * @return {String} the result
 */
Local<String>
v8_value_to_string(Value **val)
{
    return (*val)->ToString(context).ToLocalChecked();
}

/**
 * call val->ToObject()
 * @method v8_value_to_object
 * @param {Value} this
 * @return {Object} the result
 */
Local<Object>
v8_value_to_object(Value **val)
{
    return (*val)->ToObject(context).ToLocalChecked();
}

/**
 * call val->Int32Value()
 * @method v8_value_as_int32
 * @param {Value} this
 * @return {int32_t} the result
 */
int32_t
v8_value_as_int32(Value **val)
{
    return (*val)->Int32Value(context).ToChecked();
}

/**
 * call val->IntegerValue()
 * @method v8_value_as_int64
 * @param {Value} this
 * @return {int64_t} the result
 */
int64_t
v8_value_as_int64(Value **val)
{
    return (*val)->IntegerValue(context).ToChecked();
}

/**
 * call val->Uint32Value()
 * @method v8_value_as_uint32
 * @param {Value} this
 * @return {uint32_t} the result
 */
uint32_t
v8_value_as_uint32(Value **val)
{
    return (*val)->Uint32Value(context).ToChecked();
}

/**
 * The String class
 * @class String
 */

/**
 * create new v8::String from utf8 string
 * @method v8_string_new_from_utf8
 * @param {char*} data
 * @return {String} the v8::String value
 */
Local<String>
v8_string_new_from_utf8(char *data)
{
    return String::NewFromUtf8(isolate, data);
}

/**
 * empty the current string
 * @method v8_string_empty
 * @param {String} this
 * @return {String}
 */
Local<String>
v8_string_empty(String *str)
{
    return str->Empty(isolate);
}

/**
 * convert string to c/c++ string
 * @method v8_string_as_string
 * @param {String} this
 * @return {char*}
 */
char *
v8_string_as_string(Local<String> *str)
{
    // Local<String> s = reinterpret_cast<Local<String>&>(str);
    String::Utf8Value val(isolate, *str);
    return *val;
}

/**
 * The Number class
 * @class Number
 */

Local<Number>
v8_number_new_from_u16(uint16_t n)
{
    return Number::New(isolate, n);
}

Local<Number>
v8_number_new_from_u32(uint32_t n)
{
    return Number::New(isolate, n);
}

Local<Number>
v8_number_new_from_u64(uint64_t n)
{
    return Number::New(isolate, n);
}

Local<Number>
v8_number_new_from_i16(int16_t n)
{
    return Number::New(isolate, n);
}

Local<Number>
v8_number_new_from_i32(int32_t n)
{
    return Number::New(isolate, n);
}

Local<Number>
v8_number_new_from_i64(int64_t n)
{
    return Number::New(isolate, n);
}

/**
 * The Integer class
 * @class Integer
 */

Local<Boolean>
v8_boolean_new(bool val)
{
    return Boolean::New(isolate, val);
}

/**
 * The Boolean class
 * @class Boolean
 */
bool
v8_boolean_value(Boolean **val)
{
    return (*val)->Value();
}

/**
 * The Object class
 * @class Object
 */

/**
 * create new object in current isolate
 * @method v8_object_new
 * @return {Object} the returned object
 */
Local<Object>
v8_object_new()
{
    return Object::New(isolate);
}

/**
 * get the current isolate
 * @method v8_object_get_isolate
 * @return {Object} the returned isolate
 */
Isolate *
v8_object_get_isolate(Object **object)
{
    return (*object)->GetIsolate();
}

/**
 * get the value by key from an object
 * @method v8_object_get
 * @param {Object} this
 * @param {Value} key
 * @return {Value} The value
 */
Local<Value>
v8_object_get(Object **object, Local<Value> *key)
{
    return (*object)->Get(*key);
}

/**
 * set the value of key from an object
 * @method v8_object_set
 * @param {Object} this
 * @param {Value} key
 * @param {Value} val
 * @return {bool}
 */
bool
v8_object_set(Object **object, Local<Value> *key, Local<Value> *val)
{
    return (*object)->Set(*key, *val);
}

/**
 * The Array class
 * @class Array
 */
Local<Array>
v8_array_new()
{
    return Array::New(isolate);
}

Local<Value>
v8_array_get(Array **arr, Local<Value> *key)
{
    return (*arr)->Get(*key);
}

bool
v8_array_set(Array **arr, Local<Value> *key, Local<Value> *val)
{
    return (*arr)->Set(*key, *val);
}

bool
v8_array_push(Array **arr, Local<Value> *val)
{
    Local<String> key = String::NewFromUtf8(isolate, "push");
    Local<Value> func = (*arr)->Get(key);
    if (func->IsFunction()) {
        Local<Function> push = func.As<Function>();
        Local<Value> argv[1] = {*val};
        push->Call(context, (*arr)->ToObject(context).ToLocalChecked(), 1,
                   argv);
        return true;
    } else {
        return false;
    }
}

/**
 * The Function class
 * @class Function
 */

Local<Function>
v8_function_cast(Local<Value> *fval)
{
    return Local<Function>::Cast(*fval);
}

/**
 * call the function with global and args
 * @method v8_function_call
 * @param {Function} this
 * @param {Value} global - The global variable
 * @param {Value[]} argv - The arguments
 * @param {Int} argc - the count of argv
 * @return {Value} the result
 */
Local<Value>
v8_function_call(Function **func, Local<Value> global, Local<Value> **argv,
                 uint32_t argc)
{
    Local<Value> *args = new Local<Value>[argc];
    for (int32_t i = 0; i < argc; i++) {
        args[i] = *argv[i];
    }
    return (*func)->Call(context, global, argc, args).ToLocalChecked();
}

/**
 * The FunctionCallbackInfo class
 * @class FunctionCallbackInfo
 */

/**
 * get length of arguments
 * @method v8_function_callback_info_length
 * @param {FunctionCallbackInfo} callbackInfo
 * @return {int} the result
 */
int
v8_function_callback_info_length(FunctionCallbackInfo<Value> **callbackInfo)
{
    return (*callbackInfo)->Length();
}

/**
 * get value from function arguments by index
 * @method v8_function_callback_info_at
 * @param {FunctionCallbackInfo} callbackInfo
 * @param {int} index
 * @return {Value} the result
 */
Local<Value>
v8_function_callback_info_at(FunctionCallbackInfo<Value> **callbackInfo,
                             int index)
{
    return (**callbackInfo)[index];
}

/**
 * get this
 * @method v8_function_callback_info_this
 * @param {FunctionCallbackInfo} callbackInfo
 * @return {Value} the result
 */
Local<Object>
v8_function_callback_info_this(FunctionCallbackInfo<Value> **callbackInfo)
{
    return (*callbackInfo)->This();
}

/**
 * get holder
 * @method v8_function_callback_info_holder
 * @param {FunctionCallbackInfo} callbackInfo
 * @return {Value} the result
 */
Local<Object>
v8_function_callback_info_holder(FunctionCallbackInfo<Value> **callbackInfo)
{
    return (*callbackInfo)->Holder();
}

/**
 * get return value
 * @method v8_function_callback_info_holder
 * @param {FunctionCallbackInfo} callbackInfo
 * @return {ReturnValue} the result
 */
ReturnValue<Value>
v8_function_callback_info_get_return_value(
    FunctionCallbackInfo<Value> **callbackInfo)
{
    return (*callbackInfo)->GetReturnValue();
}

/**
 * get if it is in construct call
 * @method v8_function_callback_info_is_constructcall
 * @param {FunctionCallbackInfo} callbackInfo
 * @return {Boolean} the result
 */
bool
v8_function_callback_info_is_constructcall(
    FunctionCallbackInfo<Value> **callbackInfo)
{
    return (*callbackInfo)->IsConstructCall();
}

/**
 * The ReturnValue class
 * @class ReturnValue
 */

/**
 * set with object
 * @method v8_return_value_set
 * @param {ReturnValue} retval
 * @param {Value} handle
 */
void
v8_return_value_set(ReturnValue<Value> *retval, Local<Value> *val)
{
    return retval->Set(*val);
}

/**
 * set with object
 * @method v8_return_value_set_bool
 * @param {ReturnValue} retval
 * @param {bool} val
 */
void
v8_return_value_set_bool(ReturnValue<Value> *retval, bool val)
{
    return retval->Set(val);
}

/**
 * set with object
 * @method v8_return_value_set_int32
 * @param {ReturnValue} retval
 * @param {bool} val
 */
void
v8_return_value_set_int32(ReturnValue<Value> *retval, int32_t val)
{
    return retval->Set(val);
}

/**
 * set with object
 * @method v8_return_value_set_uint32
 * @param {ReturnValue} retval
 * @param {bool} val
 */
void
v8_return_value_set_uint32(ReturnValue<Value> *retval, uint32_t val)
{
    return retval->Set(val);
}

/**
 * set null
 * @method v8_return_value_set_null
 * @param {ReturnValue} retval
 */
void
v8_return_value_set_null(ReturnValue<Value> *retval)
{
    return retval->SetNull();
}

/**
 * set undefined
 * @method v8_return_value_set_null
 * @param {ReturnValue} retval
 */
void
v8_return_value_set_undefined(ReturnValue<Value> *retval)
{
    return retval->SetUndefined();
}

/**
 * set empty string
 * @method v8_return_value_set_null
 * @param {ReturnValue} retval
 */
void
v8_return_value_set_empty_string(ReturnValue<Value> *retval)
{
    return retval->SetEmptyString();
}

/**
 * The FunctionTemplate class
 * @class FunctionTemplate
 */

/**
 * create new FunctionTemplate instance
 * @method v8_function_tmpl_new
 * @constructor
 * @return {FunctionTemplate} new instance
 */
Local<FunctionTemplate>
v8_function_tmpl_new()
{
    return FunctionTemplate::New(isolate);
}

/**
 * create new FunctionTemplate instance with callback
 * @method v8_function_tmpl_new
 * @constructor
 * @param {FunctionCallback} callback
 * @return {FunctionTemplate} new instance
 */
Local<FunctionTemplate>
v8_function_tmpl_new_with_callback(FunctionCallback *callback)
{
    return FunctionTemplate::New(isolate, *callback);
}

Local<FunctionTemplate>
v8_function_tmpl_new_with_pointer_callback(FunctionCallback *callback)
{
    return FunctionTemplate::New(isolate, *callback);
}

/**
 * get Function object from FunctionTemplate
 * @method v8_function_tmpl_get_function
 * @param {FunctionTemplate} this
 * @return {Function} the result
 */
Local<Function>
v8_function_tmpl_get_function(FunctionTemplate **ft)
{
    return (*ft)->GetFunction(context).ToLocalChecked();
}

/**
 * set class name of FunctionTemplate object
 * @method v8_function_tmpl_set_class_name
 * @param {FunctionTemplate} this
 * @param {char*} the class name
 * @return {void}
 */
void
v8_function_tmpl_set_class_name(FunctionTemplate **ft, char *name)
{
    (*ft)->SetClassName(String::NewFromUtf8(isolate, name));
}

/**
 * set class name of FunctionTemplate fieldCount
 * @method v8_function_tmpl_set_internal_fieldcount
 * @param {FunctionTemplate} this
 * @param {uint32_t} fieldCount
 * @return {void}
 */
void
v8_function_tmpl_set_internal_fieldcount(FunctionTemplate **ft,
                                         uint32_t fieldCount)
{
    (*ft)->InstanceTemplate()->SetInternalFieldCount(fieldCount);
}

/**
 * set property method
 * @method v8_function_tmpl_set_property_method
 * @param {FunctionTemplate} this
 * @param {char*} name
 * @param {FunctionCallback*} method
 * @return {void}
 */
void
v8_function_tmpl_set_property_method(FunctionTemplate **ft, char *name,
                                     FunctionCallback *method)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::FunctionTemplate> t =
        v8::FunctionTemplate::New(isolate, *method);
    v8::Local<v8::String> fn_name = v8::String::NewFromUtf8(isolate, name);
    t->SetClassName(fn_name);
    (*ft)->PrototypeTemplate()->Set(fn_name, t);
}

/**
 * get a new instance(object) from FunctionTemplate
 * @method v8_function_tmpl_new_instance
 * @param {FunctionTemplate} this
 * @return {Object} the returned object
 */
Local<Object>
v8_function_tmpl_new_instance(FunctionTemplate **ft)
{
    return (*ft)
        ->GetFunction(context)
        .ToLocalChecked()
        ->NewInstance(context)
        .ToLocalChecked();
}

/**
 * @class Exception
 */

/**
 * create an exception
 * @method v8_exception_throw_error
 */
Local<Value>
v8_exception_throw_error(char *msg)
{
    auto ret = Exception::Error(String::NewFromUtf8(isolate, msg));
    return isolate->ThrowException(ret);
}

/**
 * create an exception for range
 * @method v8_exception_throw_range_error
 */
Local<Value>
v8_exception_throw_range_error(char *msg)
{
    auto ret = Exception::RangeError(String::NewFromUtf8(isolate, msg));
    return isolate->ThrowException(ret);
}

/**
 * create an exception for reference
 * @method v8_exception_throw_reference_error
 */
Local<Value>
v8_exception_throw_reference_error(char *msg)
{
    auto ret = Exception::ReferenceError(String::NewFromUtf8(isolate, msg));
    return isolate->ThrowException(ret);
}

/**
 * throws an exception for syntax error
 * @method v8_exception_throw_syntax_error
 */
Local<Value>
v8_exception_throw_syntax_error(char *msg)
{
    auto ret = Exception::SyntaxError(String::NewFromUtf8(isolate, msg));
    return isolate->ThrowException(ret);
}

/**
 * create an exception for type errors
 * @method v8_exception_throw_type_error
 */
Local<Value>
v8_exception_throw_type_error(char *msg)
{
    auto ret = Exception::TypeError(String::NewFromUtf8(isolate, msg));
    return isolate->ThrowException(ret);
}

/**
 * utilities
 */

bool
utils_is_big_endian()
{
    const union {
        uint8_t u8[2];
        uint16_t u16;
    } u = {{1, 0}};
    return u.u16 == 1 ? false : true;
}
}
