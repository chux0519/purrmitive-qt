use purrmitive::core::*;
use purrmitive::graphics::*;
use purrmitive::*;

use std::ffi::CStr;
use std::os::raw::c_char;

use env_logger::Builder;
use log::{error, info, LevelFilter};
use once_cell::sync::OnceCell;

static mut RUNNER: OnceCell<Box<dyn PurrModelRunner<M = PurrHillClimbModel> + Send + Sync>> =
    OnceCell::new();

static mut MODEL: OnceCell<PurrHillClimbModel> = OnceCell::new();

#[repr(C)]
pub struct PurrmitiveParam {
    pub alpha: u8,
    pub mode: i32,
    pub resize: u32,
    pub size: u32,
    pub count: u32,
    pub input: *const c_char,
}

#[repr(C)]
pub struct PurrmitiveColor {
    pub r: u8,
    pub g: u8,
    pub b: u8,
    pub a: u8,
}

fn create_cb<T: PurrShape + std::fmt::Debug>() -> Box<dyn FnMut(PurrState<T>) + Send + Sync> {
    let mut step = 1;
    Box::new(move |x| {
        info!("step {}: {:?}", step, x);
        step += 1;
    })
}

#[no_mangle]
pub unsafe extern "C" fn purrmitive_set_verbose(verbose: i32) {
    let mut logger_builder = Builder::new();
    let level = match verbose {
        0 => LevelFilter::Error,
        1 => LevelFilter::Info,
        2 => LevelFilter::Debug,
        3 | _ => LevelFilter::Trace,
    };
    logger_builder.filter_level(level);
    logger_builder.init();
}

#[no_mangle]
pub unsafe extern "C" fn purrmitive_init(param: *const PurrmitiveParam) {
    let input = CStr::from_ptr((*param).input)
        .to_string_lossy()
        .into_owned();
    let ctx = PurrContext::new(input, (*param).resize, (*param).size, (*param).alpha, None);
    let model = PurrHillClimbModel::new(ctx, 1000, 16, 100);
    match MODEL.set(model) {
        Ok(()) => {}
        Err(e) => error!("Failed to init model: {:?}", e),
    };

    let runner = model_runner!(
        (*param).mode,
        (*param).count,
        num_cpus::get() as u32,
        create_cb
    );
    match RUNNER.set(runner) {
        Ok(()) => {}
        Err(_) => error!("Failed to init runner!"),
    };
}

#[no_mangle]
pub unsafe extern "C" fn purrmitive_step() {
    match RUNNER.get_mut() {
        Some(r) => match MODEL.get_mut() {
            Some(m) => {
                r.step(m);
            }
            None => {
                error!("Failed to step: Model not found!")
            }
        },
        None => {
            error!("Failed to step: Runner not found!")
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn purrmitive_stop() {
    match RUNNER.get_mut() {
        Some(r) => {
            r.stop();
        }
        None => {
            error!("Failed to stop: Runner not found!")
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn purrmitive_get_bg() -> PurrmitiveColor {
    match MODEL.get() {
        Some(m) => PurrmitiveColor {
            r: m.context.bg.0[0],
            g: m.context.bg.0[1],
            b: m.context.bg.0[2],
            a: m.context.bg.0[3],
        },
        None => {
            error!("No bg color: Model is not found!");
            PurrmitiveColor {
                r: 0,
                g: 0,
                b: 0,
                a: 0,
            }
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn purrmitive_get_last_shape() -> *const u8 {
    match RUNNER.get() {
        Some(r) => r.get_last_shape().as_ptr(),
        None => {
            error!("No frame found: Runner is not found");
            "".as_ptr()
        }
    }
}
