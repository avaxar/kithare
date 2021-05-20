# Contributing to Kithare

- New Contributors are most welcome to come and help us in making Kithare better.
- First of all, all contributors are expected to follow the [code of conduct](code_of_conduct).
- To contribute to the repo, feel free to open a PR on Github. The collaborators will review your changes, make suggestions and merge it in when they are happy with it.
- If you want to contribute a major change, it's a good idea to discuss with us
first, either on our discord server, or on GitHub issues section.
- Be sure to reformat the code with the given `.clang-format` before submitting a commit/PR.
- It is expected that contributors follow code formatting rules while contributing
to Kithare. If any *unclean* code is opened up in a PR, that PR will be waited for a cleaning commit.

### Naming convention
- `snake_case` for variable, namespace, file, and folder names.
- `SCREAMING_SNAKE_CASE` for constants, enum members, and macros (An exception for `kprint` and `kprintln` as they are intended to be like functions).
- `camelCase` (Or some call `lowerCamelCase`) for function, method, and label names.
- `PascalCase` for classes, structs, and enums.

### Coding style
```cpp
/* All kinds of comments should use multi-line comment */

/* Opening curly braces should be in the same line */
if (condition) {

}

/* Pointers and references should be placed to the side of the type */
int* var = nullptr;
int& reference_var = *var;

/* Always explicitly specify `const` for values that aren't meant to be changed in the function.
 * For strings and vectors, use a `const` reference. */
void function(const std::string& str, const int value) {

}

/* NO */
using namespace std;
using namespace kh;
```
