# Are we AST yet?

A unique AST-based(?) interpreter.

## Example

    # include other script
    # `@[!__path__]`
    @[!/absolute/path/to/script]
    @[!./relative/path/to/script]

    # set/declare directives (function)
    # `@[$__func_name__$__code__]`
    @[$pow$_^_]
    @[$sqrt$pow(_;0.5)]

    # set/declare symbols
    # note the use of '@' symbol to suppress output when setting symbols
    @x=8
    @y=6

    # set symbol with the return of function
    @length=sqrt(pow(x;2)+pow(y;2))

    # print out the content of `length`
    length
