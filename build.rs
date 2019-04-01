use pkg_config::Library;

fn main() {
    let Library {
        include_paths,
        link_paths,
        libs,
        ..
    } = pkg_config::Config::new().probe("v8").unwrap();

    let ref cflags: Vec<String> = include_paths
        .clone()
        .into_iter()
        .map(|pathbuf| format!("-I{}", pathbuf.to_str().unwrap()))
        .collect();
    let ref link_flags: Vec<String> = link_paths
        .clone()
        .into_iter()
        .map(|pathbuf| format!("-L{}", pathbuf.to_str().unwrap()))
        .collect();
    let ref link_libs: Vec<String> = libs
        .clone()
        .into_iter()
        .map(|lib| format!("-l{}", lib))
        .collect();

    cc::Build::new()
        .cpp(true)
        .flag("-w")
        .flag("-std=c++11")
        .flag(cflags.join(" ").as_str())
        .flag(link_flags.join(" ").as_str())
        .flag(link_libs.join(" ").as_str())
        .file("./deps/api.cc")
        .warnings(false)
        .compile("rustjs");

    println!("cargo:rerun-if-changed=deps/api.cc");
    println!("cargo:rerun-if-changed=deps/api.h");
}
