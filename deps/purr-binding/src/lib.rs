use purrmitive::core::*;
use purrmitive::graphics::*;
use purrmitive::*;

use std::ffi::CStr;
use std::os::raw::c_char;

use env_logger::Builder;
use log::{error, info, LevelFilter};
use once_cell::sync::OnceCell;

static COMBO_RUNNER: OnceCell<PurrMultiThreadRunner<Combo>> = OnceCell::new();
static TRIANGLE_RUNNER: OnceCell<PurrMultiThreadRunner<Triangle>> = OnceCell::new();
static RECTANGLE_RUNNER: OnceCell<PurrMultiThreadRunner<Rectangle>> = OnceCell::new();
static ELLIPSE_RUNNER: OnceCell<PurrMultiThreadRunner<Ellipse>> = OnceCell::new();
static CIRCLE_RUNNER: OnceCell<PurrMultiThreadRunner<Circle>> = OnceCell::new();
static ROTATEDRECTANGLE_RUNNER: OnceCell<PurrMultiThreadRunner<RotatedRectangle>> = OnceCell::new();
static QUADRATIC_RUNNER: OnceCell<PurrMultiThreadRunner<Quadratic>> = OnceCell::new();
static ROTATEDELLIPSE_RUNNER: OnceCell<PurrMultiThreadRunner<RotatedEllipse>> = OnceCell::new();
static POLYGON_RUNNER: OnceCell<PurrMultiThreadRunner<Polygon>> = OnceCell::new();

#[repr(C)]
pub struct PurrmitiveParam {
    pub alpha: u8,
    pub mode: i32,
    pub verbose: i32,
    pub resize: u32,
    pub size: u32,
    pub count: i32,
    pub input: *const c_char,
    pub output: *const c_char,
}

#[no_mangle]
pub unsafe extern "C" fn purrmitive_init(param: *const PurrmitiveParam) {
    let mut logger_builder = Builder::new();
    let level = match (*param).verbose {
        0 => LevelFilter::Error,
        1 => LevelFilter::Info,
        2 => LevelFilter::Debug,
        3 | _ => LevelFilter::Trace,
    };
    logger_builder.filter_level(level);
    logger_builder.init();

    let input = CStr::from_ptr((*param).input)
        .to_string_lossy()
        .into_owned();
    let ctx = PurrContext::new(input, (*param).resize, (*param).size, (*param).alpha, None);
    let model = PurrHillClimbModel::new(ctx, 1000, 16, 100);
    // TODO: reinit runner
}
