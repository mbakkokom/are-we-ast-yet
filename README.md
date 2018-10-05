# Are we AST yet?

A unique AST-based(?) interpreter.

## Example

    # set directive `sqrt` to process square root
	@[$sqrt$_^0.5]
	
	# set symbols
	@x=8
	@y=6

	# set symbol `length` using directive `sqrt`
	@length=sqrt(x^2+y^2)

	# print out the content of `_1`
	length
