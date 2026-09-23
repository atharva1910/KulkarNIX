use core::{error::Error, fmt::{self, Formatter}};

#[derive(Debug)]
pub enum KError {
    GeneralError,
    InvalidParameter
}

impl fmt::Display for KError {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        write!(f, "KError");
        Ok(())
    }
}

impl Error for KError {}
