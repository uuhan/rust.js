use crate::util::v8;

pub trait Stream {}

pub trait Readable: Stream {
    fn pipe(&self, w: &Writable);
}

pub trait Writable: Stream {
    fn write(&self);
}

pub fn Init() -> v8::Object {
    v8::Object::New()
}
