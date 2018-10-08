#pragma once

#include "lexical.hpp"

#include <stack>
#include <unordered_map>
#include <regex>

using namespace std;

class ASTInterpreter : public ASTLex {
public:
	ASTInterpreter(bool verbose=false);
	void Run(string s, Entity **e = nullptr);
	void Resolve(Entity *e);
	void ResolveParenthesis(ParenthesisEntity *e);
	void ResolveCompound(CompoundEntity *e);
	void ResolveOperand(OperandEntity *e);
	void ResolveLiteral(LiteralEntity *e);
	void ResolveFunction(FunctionEntity* e);
	bool SymbolExists(string k);
	void SetSymbol(string k, double v);
	double GetSymbol(string k, bool negative=false, bool ignore_error=false);
	bool DirectiveExists(string k);
	void SetDirective(string k, string v);
	void CallDirective(string k, bool negative=false, bool ignore_error=false);
	void SetVerbose(bool verbose);
	bool GetVerbose();
	bool IsStackEmpty();
	double PopFromStack();
	void PushToStack(double v);
	~ASTInterpreter();
protected:
	bool mVerbose = false;
	stack<double> mStack;
	unordered_map<string, double> mSymbols;
	unordered_map<string, Entity*> mDirectives;
	regex *mCommentPattern = nullptr;
	regex *mSymbolSetPattern = nullptr;
	regex *mDirectivePattern = nullptr, *mDirectiveSetPattern = nullptr, *mDirectiveCallPattern = nullptr, *mDirectiveIncludePattern = nullptr;
};
