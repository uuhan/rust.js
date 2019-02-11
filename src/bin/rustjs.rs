#[macro_use]
extern crate log;
extern crate env_logger;
extern crate rustjs;

use env_logger::Env;

fn main() {
    let env = Env::default()
        .filter_or("LOG_LEVEL", "info")
        .write_style("LOG_STYLE");
    env_logger::Builder::from_env(env)
        .default_format_module_path(true)
        .init();

    rustjs::new_instance();
}
