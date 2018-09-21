#pragma once

#include <exception>
#include <string>

using namespace std;

class ASTException : public exception {
public:
	ASTException(string str);
	ASTException(const char *str);

	const char* what() const noexcept override;
private:
	string mMessage;
};

class ASTInvalidOperation : public ASTException {
public:
	ASTInvalidOperation(string str);
	ASTInvalidOperation(const char *str);
};

class ASTNotFound : public ASTException {
public:
	ASTNotFound(string str);
	ASTNotFound(const char *str);
};

class ASTTypeError : public ASTException {
public:
	ASTTypeError(string str);
	ASTTypeError(const char *str);
};

class ASTValueError : public ASTException {
public:
	ASTValueError(string str);
	ASTValueError(const char *str);
};

class ASTSyntaxError : public ASTException {
public:
	ASTSyntaxError(string str);
	ASTSyntaxError(const char *str);
};