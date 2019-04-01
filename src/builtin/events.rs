use crate::builtin::module;
use crate::util::v8;

pub fn Init() -> v8::Object {
    module::LoadBuiltinScript("events")
}
