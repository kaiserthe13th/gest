# Code Style

This file is present to cover the details of code style that is not covered by the [.clang-format](.clang-format) file.

## Identifier Naming

- `PascalCase` for types
- `camelCase` for values, variable names, functions, etc.

> [!TIP] Prefer descriptive names over short names, or abbreviations.

## Commenting Style

Comments describe what succeeds them which is determined by maximal coupling, unless there is an empty line after them. If there is an empty line after a comment and the comment comes directly after something, that something is being described by the comment. You should always prefer preceding comments over succeeding comments.

> [!WARNING] Trailing comments are frowned upon.

### Example

```c
// I describe the entire-if-else branch
if (cond1) {
    // I describe the if-branch (cond1), and the following line at once
    doA();
} else {
    // I describe the else branch

    // I describe the following line
    logB(&b);
}

// NOTE: I am a standalone comment, I am most likely an interjection. I am not tied to anything.

// I describe this variable
int a = 7;
```
