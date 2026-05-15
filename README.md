# Gest

![GitHub License](https://img.shields.io/github/license/kaiserthe13th/gest)
![GitHub last commit](https://img.shields.io/github/last-commit/kaiserthe13th/gest)

Gest aims to be a smalltalk-like pure OOP language, but with modern features, and one that has good performance. View [the design](DESIGN.md) for more details.

## :stopwatch: Current Development Status

- [x] Requirements analysis completed
- [x] Initial design completed
- [x] Coding standards decided
- [ ] Lexer completed (ongoing)
- [ ] Parse completed
- [ ] Codegen completed
- [ ] VM completed

## :wave: Hello World

```smalltalk
Module import: '@gest/common', for: #{#identity => #id}.

args := Env args asArray.
name := args index?: 1.
"If there is a name, use that name, if not, default to World."
name := name nil? ifFalse: id, else: 'World'.

Module
    export: [ Io printLn: 'Hello ' ++ name ++ '!'. ],
    as: #main.
```

## :rocket: Getting started

Requirements: [premake](https://premake.github.io/), [uv](https://docs.astral.sh/uv/)

1. Clone the repo.

   ```console
   $ git clone https://github.com/kaiserthe13th/gest.git
   Cloning into 'gest'...
   done.
   $ cd gest
   ```

2. Set up the environment using premake.

   ```console
   $ premake5 uv-setup # to set up the python environment
   $ premake5 download-unicode # downloads the unicode UCD files necessary
   $ premake5 gen-unicode-headers # generates unicode headers
   ```

3. You can play around and compile with premake.

   ```console
   $ premake5 vs2022/gmake2 etc.
   $ cd build
   
   $ msbuild /p:Configuration=StaticDebug ...
   or
   $ make config=staticdebug_bits64
   ```

   > [!TIP] Use the `--test` flag on premake to get tests compiled.

## :handshake: Contributor's Guide

### :art: Code Style

We use code formatting tools for code styling, on top of a small [style guide](CODE_STYLE.md) for things that tools can't cover.

- For C, use [clang-format](https://clang.llvm.org/docs/ClangFormat.html).
- For Python, use [ruff](https://docs.astral.sh/ruff/).

### :test_tube: Testing

1. Build testing build.

   ```console
   $ premake5 vs2022/gmake2 --test
   $ cd build
   $ ...
   ```

2. Run the test by running the generate gest-test executable.

### :scroll: License Linting

We use a tool called xg for checking licensing of our files. Please use the tool and ensure all files are licensed.

You can easily use it via premake with:

```console
$ premake5 xg
```

## :balance_scale: Licensing

The project uses the Apache 2.0 License with the LLVM Exception for most of its files. However, some files use the Unicode license for their data.

See [LICENSE](./LICENSE), and [NOTICE.xg](NOTICE.xg).
