use builtin::module;
use util::v8;

pub fn Init() -> v8::Object {
    module::LoadBuiltinScript("events")
}
