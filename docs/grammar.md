# The Grammar of The Kithare Programming Language

 Importing/including modules
```py
import name; /* Imports `name.kh` and refer to the contents with a `name` namespace */ 
import name as other; /* Does the same above but with a `other` namespace */
import a.b; /* Imports `a/b.kh` and refer to the contents with a `b` namespace */
include c; /* Includes `c.kh` and put its content without a namespace */
```

 Function declaration and definition
```py
/* Declares a function named `function` which returns nothing (void) */
def function() {}

/* Declares a function named `function` which returns an integer */
def function() -> int {}
```

## TODO: Fill this with actual understandable guide
