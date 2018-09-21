#include "exceptions.hpp"

// -- MARK: ASTException

ASTException::ASTException(string str) {
	mMessage = str;
}

ASTException::ASTException(const char *str) {
	mMessage = str;
}

const char* ASTException::what() const noexcept {
	return mMessage.c_str();
}

// -- MARK: ASTInvalidOperation

ASTInvalidOperation::ASTInvalidOperation(string str) : ASTException(str) {}
ASTInvalidOperation::ASTInvalidOperation(const char *str) : ASTException(str) {}

// -- MARK: ASTNotFound

ASTNotFound::ASTNotFound(string str) : ASTException(str) {}
ASTNotFound::ASTNotFound(const char *str) : ASTException(str) {}

// -- MARK: ASTTypeError

ASTTypeError::ASTTypeError(string str) : ASTException(str) {}
ASTTypeError::ASTTypeError(const char *str) : ASTException(str) {}

// -- MARK: ASTValueError

ASTValueError::ASTValueError(string str) : ASTException(str) {}
ASTValueError::ASTValueError(const char *str) : ASTException(str) {}

// -- MARK: ASTSyntaxError

ASTSyntaxError::ASTSyntaxError(string str) : ASTException(str) {}
ASTSyntaxError::ASTSyntaxError(const char *str) : ASTException(str) {}

