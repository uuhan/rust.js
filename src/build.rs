extern crate cc;

use std::env;

fn main() {
    let mut v8dir;
    let mut v8lib;
    if let Ok(t) = env::var("V8_TARGET") {
        v8dir = t.clone();
        v8lib = format!("{}/out.gn/x64.release.sample/obj", v8dir);
    } else {
        v8dir = ".".to_owned();
        v8lib = format!("{}/lib", v8dir);
    }
    cc::Build::new()
        .cpp(true)
        .flag("-std=c++11")
        .include(format!("{}/include", v8dir))
        .file("./deps/api.cc")
        .warnings(false)
        .compile("librustjs.a");

    println!("cargo:rustc-link-search={}", v8lib);
    println!("cargo:rustc-link-lib=static=v8_monolith");
}
