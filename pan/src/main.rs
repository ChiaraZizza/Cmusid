extern crate chromaprint;

use std::fs::File;
use std::io::Read;

fn main() {
  read()
}

fn read() {
  let file = File::open("./input/cool.flac");
  match file {
    Ok(mut f) => {
      let mut vec = Vec::new();
      f.read_to_end(&mut vec);
      println!("{:?}",vec);
    },
    Err(e) => println!("{}",e)
  }
}
