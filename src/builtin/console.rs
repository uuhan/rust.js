use std::io::{self, Write};
use crate::util::v8;
use crate::util::v8::ValueT;

fn stringify(obj: &v8::Value) -> v8::String {
    let global = v8::Context::Global();
    let JSON = global.Get(v8::String::NewFromUtf8("JSON")).ToObject();
    let stringify: v8::Function =
        v8::Function::Cast(&JSON.Get(v8::String::NewFromUtf8("stringify")));
    let replacer = v8::Number::NewFromInt32(0);
    let space = v8::Number::NewFromInt32(2);

    stringify
        .Call(JSON, &[obj, &replacer.as_val(), &space.as_val()])
        .ToString()
}

extern "C" fn info(arguments: v8::FunctionCallbackInfo) {
    let firstArg = arguments.At(0);
    let msg: v8::String;

    if firstArg.IsString() || firstArg.IsFunction() {
        msg = firstArg.ToString();
    } else {
        msg = stringify(&firstArg);
    }

    let stdout = io::stdout();
    let mut handle = stdout.lock();

    match handle.write_fmt(format_args!("{}\n", msg.as_string())) {
        Ok(v) => v,
        Err(err) => println!("{:?}", err),
    }
}

extern "C" fn warn(arguments: v8::FunctionCallbackInfo) {
    let firstArg = arguments.At(0);
    let msg;

    if firstArg.IsString() || firstArg.IsFunction() {
        msg = firstArg.ToString();
    } else {
        msg = stringify(&firstArg);
    }

    let stderr = io::stderr();
    let mut handle = stderr.lock();
    match handle.write_fmt(format_args!("{}\n", msg.as_string())) {
        Ok(v) => v,
        Err(err) => println!("{:?}", err),
    }
}

pub fn Init() -> v8::Object {
    let Console = v8::Object::New();
    Console.Set(
        v8::String::NewFromUtf8("log"),
        v8::FunctionTemplate::New(info).GetFunction(),
    );
    Console.Set(
        v8::String::NewFromUtf8("info"),
        v8::FunctionTemplate::New(info).GetFunction(),
    );
    Console.Set(
        v8::String::NewFromUtf8("warn"),
        v8::FunctionTemplate::New(warn).GetFunction(),
    );
    Console.Set(
        v8::String::NewFromUtf8("error"),
        v8::FunctionTemplate::New(warn).GetFunction(),
    );
    Console
}
