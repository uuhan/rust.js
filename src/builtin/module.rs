use std::path::Path;
use util::v8;
use util::v8::ValueT;

extern "C" fn require(arguments: v8::FunctionCallbackInfo) {
    let name = arguments.At(0).ToString();
    let cached = v8::Context::Global()
        .Get(v8::String::NewFromUtf8("_modules"))
        .ToObject();
    let exports = cached.Get(name);
    if exports.IsUndefined() {
        v8::Exception::ThrowReferenceError("Module not found");
        return;
    }
    arguments.GetReturnValue().Set(exports);
}

pub fn LoadBuiltinScript(name: &str) -> v8::Object {
    let base = Path::new("lib");
    let mut filename = String::new();
    filename.push_str(name);
    filename.push_str(".js");
    let script = v8::Script::CompileWithFile(base.join(filename).to_str().unwrap());
    if script.IsEmpty() {
        println!("Empty source found lib.");
    }
    let fval = script.Run();
    let func = v8::Function::Cast(&fval);
    let exports = v8::Object::New();
    func.Call(v8::Context::Global(), &[exports.as_val()]);
    return exports;
}

pub fn Setup() -> v8::Function {
    v8::FunctionTemplate::New(require).GetFunction()
}
