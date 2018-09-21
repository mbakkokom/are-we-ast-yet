#pragma once

#include "lexical.hpp"

#include <unordered_map>
#include <regex>

using namespace std;

class ASTInterpreter : public ASTLex {
public:
	ASTInterpreter();
	double Run(string s, Entity **e = nullptr, bool verbose=false);
	double Resolve(Entity *e);
	double ResolveParenthesis(ParenthesisEntity *e);
	double ResolveCompound(CompoundEntity *e);
	double ResolveOperand(OperandEntity *e);
	double ResolveLiteral(LiteralEntity *e);
	bool SymbolExists(string k);
	void SetSymbol(string k, double v);
	double GetSymbol(string k, bool ignore_error=false);
	bool DirectiveExists(string k);
	void SetDirective(string k, string v);
	void CallDirective(string k, bool ignore_error=false);
	~ASTInterpreter();
protected:
	std::unordered_map<string, double> mSymbols;
	std::unordered_map<string, Entity*> mDirectives;
	regex *mCommentPattern = nullptr;
	regex *mSymbolSetPattern = nullptr;
	regex *mDirectivePattern = nullptr, *mDirectiveSetPattern = nullptr, *mDirectiveCallPattern = nullptr, *mDirectiveIncludePattern = nullptr;
};