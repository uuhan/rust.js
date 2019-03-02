extern crate cc;

use std::env;

fn main() {
    let mut v8dir;
    let mut v8lib;
    if let Ok(t) = env::var("V8_TARGET") {
        v8dir = t.clone();
        v8lib = format!("{}/out.gn/x64.release.sample/obj", v8dir);
        cc::Build::new()
            .cpp(true)
            .flag("-std=c++11")
            .flag("-Wno-return-type-c-linkage")
            .include(format!("{}/include", v8dir))
            .file("./deps/api.cc")
            .warnings(false)
            .compile("librustjs.a");

        println!("cargo:rustc-link-search={}", v8lib);
        println!("cargo:rustc-link-lib=static=v8_monolith");
    } else {
        v8dir = "/usr/local".to_owned();
        v8lib = format!("{}/lib", v8dir);
        cc::Build::new()
            .cpp(true)
            .flag("-std=c++11")
            .flag("-Wno-return-type-c-linkage")
            .include(format!("{}/include", v8dir))
            .file("./deps/api.cc")
            .warnings(false)
            .compile("librustjs.a");

        println!("cargo:rustc-link-search={}", v8lib);
        println!("cargo:rustc-link-lib=static=v8");
        println!("cargo:rustc-link-lib=static=v8_libplatform");
    }
}
