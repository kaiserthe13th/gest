# Gest

Gest aims to be a smalltalk-like pure OOP language that has good performance.

## :rocket: Getting started

Requirements: [premake](https://premake.github.io/), [uv](https://docs.astral.sh/uv/)

1. Clone the repo.
    ```console
    $ git clone https://github.com/kaiserthe13th/gest.git
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

We use a tool called xg.py for checking licensing of our files. Please use the tool and ensure all files are licensed.

```console
$ uv run --project script script/xg.py -vall ./NOTICE.xg
```
