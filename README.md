# invisteg
A program that can hide any file inside transparent pixels of a PNG image.
![Video hidden inside a PNG](example.png)

## Requirements
- [C standard library](https://en.wikipedia.org/wiki/C_standard_library) (such as [glibc](https://www.gnu.org/software/libc/))
- [libpng](http://www.libpng.org/pub/png/libpng.html)
- [C compiler](https://en.wikipedia.org/wiki/List_of_compilers#C_compilers) (such as [gcc](https://gcc.gnu.org/) or [clang](https://clang.llvm.org/))
- [Make](https://en.wikipedia.org/wiki/Make_(software)) (such as [GNU Make](https://www.gnu.org/software/make/))

## Building
Make sure you have all the dependencies installed and run:
```
make
```

## Install
Run as root to install globally or as other user to install locally:
```
make install
```

## Usage
```
Usage: invisteg [input_image] [input_data/output_data] [output_image]

Instructions:
	If 3 files are provided the program will write input_data to
	transparent pixels in input_image and save it to output_image.

	If 2 files are provided the program will read data from transparent
	pixels in the input_image and write it to output_data.
```
