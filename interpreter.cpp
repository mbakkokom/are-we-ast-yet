#include "interpreter.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <cmath>

using namespace std;

ASTInterpreter::ASTInterpreter(bool verbose) : mVerbose(verbose) {
	mCommentPattern = new regex("^\\s*#(.*)$");
	mDirectivePattern = new regex("^\\s*@(.*)$");
	mDirectiveSetPattern = new regex("^\\[\\s*\\$([A-Za-z_]{1}[A-Za-z0-9_]*)\\$\\s*((.*)\\s*){0,1}\\]\\s*$");
	mDirectiveCallPattern = new regex("^\\[\\s*([A-Za-z_]{1}[A-Za-z0-9_]*)\\s*\\]\\s*$");
	mDirectiveIncludePattern = new regex("^\\[!(.+)\\]\\s*$");
	mSymbolSetPattern = new regex("^([A-Za-z_]{1}[A-Za-z0-9_]*)\\s*=\\s*(.*)\\s*$");
}

void ASTInterpreter::Run(string s, Entity **e) {
	smatch matches;
	Entity *tok = nullptr;

	if (regex_match(s, matches, *mDirectivePattern)) {
		string dir = matches.str(1);
		if (regex_match(dir, matches, *mDirectiveSetPattern)) {
			string k = matches.str(1), v = matches.str(3);
			SetDirective(k, v);
		} else if (regex_match(dir, matches, *mDirectiveCallPattern)) {
			string k = matches.str(1);
			CallDirective(k);
		} else if (regex_match(dir, matches, *mSymbolSetPattern)) {
			string k = matches.str(1), v = matches.str(2);
			// supress the output
			tok = Parse(v);
			try {
				Resolve(tok);
				SetSymbol(k, PopFromStack());
				delete tok;
				tok = nullptr;
			} catch(const ASTException &ex) {
				delete tok;
				throw ex;
			}
		} else if (regex_match(dir, matches, *mDirectiveIncludePattern)) {
			string str = matches.str(1);
			if (mVerbose)
				cout << "AST include_file " << str << endl;

			ifstream fp(str);
			if (!fp.is_open())
				throw ASTException("cannot open file \"" + str + "\"");

			str.clear();

			while(fp.good())
			try {
				string now;

				getline(fp, now, '\n');

				if (!fp.good() && now.empty())
					break;

				if (now.find('\\') == now.length() - 1) {
					str += now.substr(0, now.length() - 1);
					continue;
				} else {
					str += now;
				}

				if (mVerbose)
					cout << "| running " << str << endl;

				Run(str, nullptr);
				str.clear();
			} catch (const ASTException &ex) {
				fp.close();
				throw ex;
			}
		} else {
			throw ASTSyntaxError("invalid directive syntax");
		}
	} else if (!regex_match(s, *mCommentPattern)) {
		tok = Parse(s);
		Resolve(tok);
	}

	if (e != nullptr)
		*e = tok;
	else
		delete tok;
}

void ASTInterpreter::Resolve(Entity *e) {
	if (e == nullptr) {
		throw ASTValueError("cannot resolve null entity");
		return;
	}

	if (mVerbose)
		cout << "UNR " << GetPostfix(e) << endl;

	switch(e->GetType()) {
	case Entity::PARENTHESIS_ENTITY:
		ResolveParenthesis((ParenthesisEntity*)e);
		break;
	case Entity::COMPOUND_ENTITY:
		ResolveCompound((CompoundEntity*)e);
		break;
	case Entity::OPERAND_ENTITY:
		ResolveOperand((OperandEntity*)e);
		break;
	case Entity::LITERAL_ENTITY:
		ResolveLiteral((LiteralEntity*)e);
		break;
	case Entity::FUNCTION_ENTITY:
		ResolveFunction((FunctionEntity*)e);
		break;
	case Entity::INVALID_ENTITY:
	default:
		throw ASTTypeError(string("cannot resolve entity ") + e->GetTypeString());
	}
}

void ASTInterpreter::ResolveParenthesis(ParenthesisEntity *e) {
	Resolve(e->Get());

	if (e->IsNegative()) {
		PushToStack(-PopFromStack());
	}
}

void ASTInterpreter::ResolveCompound(CompoundEntity *e) {
	Entity *l = e->Get(CompoundEntity::LEFT_ENTITY),
		   *r = e->Get(CompoundEntity::RIGHT_ENTITY);
	//Entity::EntityType lt = l->GetType(), rt = r->GetType();
	double ld, rd;

	if (mVerbose)
		cout << "AST op " << e->GetOperatorString() << endl;

	// The order is reversed to preserve argument ordinality
	switch(e->GetOperator()) {
	case TieredEntity::ARITHMETIC_ADD:
		Resolve(l);
		ld = PopFromStack();
		Resolve(r);
		rd = PopFromStack();
		PushToStack(ld + rd);
		break;
	case TieredEntity::ARITHMETIC_SUB:
		Resolve(l);
		ld = PopFromStack();
		Resolve(r);
		rd = PopFromStack();
		PushToStack(ld - rd);
		break;
	case TieredEntity::ARITHMETIC_MUL:
		Resolve(l);
		ld = PopFromStack();
		Resolve(r);
		rd = PopFromStack();
		PushToStack(ld * rd);
		break;
	case TieredEntity::ARITHMETIC_DIV:
		Resolve(l);
		ld = PopFromStack();
		Resolve(r);
		rd = PopFromStack();
		PushToStack(ld / rd);
		break;
	case TieredEntity::ARITHMETIC_MOD:
		Resolve(l);
		ld = PopFromStack();
		Resolve(r);
		rd = PopFromStack();
		PushToStack(fmod(ld, rd));
		break;
	case TieredEntity::ARITHMETIC_POW:
		// not sure why
		Resolve(l);
		ld = PopFromStack();
		Resolve(r);
		rd = PopFromStack();
		PushToStack(pow(ld, rd));
		break;
	case TieredEntity::OPERATOR_SET:
		if (l->GetType() == Entity::OPERAND_ENTITY) {
			Resolve(r);
			rd = PopFromStack();
			SetSymbol(l->GetString(), rd);
			PushToStack(rd);
			break;
		} else {
			throw ASTInvalidOperation("invalid operand for assignment operation");
			return;
		}
		break;
	/*
	case TieredEntity::DIRECTIVE_CALL:
		if (r->GetType() == Entity::FUNCTION_ENTITY) {
			Resolve(l);
			Resolve(r);
		} else {
			throw ASTInvalidOperation("invalid operand for directive call " + e->GetTypeString());
			return;
		}
		break;
	case TieredEntity::DIRECTIVE_ARGS:
		Resolve(r);
		Resolve(l);
		break;
	*/
	default:
		throw ASTInvalidOperation(string("invalid operation ") + e->GetOperatorString());
		return;
	}

	return;
}

void ASTInterpreter::ResolveOperand(OperandEntity *e) {
	double r = GetSymbol(e->GetAbsValue(), e->IsNegative());

	if (mVerbose)
		cout << "RES " << r << endl;

	PushToStack(r);
}

void ASTInterpreter::ResolveLiteral(LiteralEntity *e) {
	const string v(e->GetAbsValue());
	istringstream i(v);
	double r;

	if (!(i >> r))
		throw ASTValueError("invalid literal \"" + v + "\"");
	else if (e->IsNegative())
		r = -r;

	if (mVerbose)
		cout << "RES " << r << endl;

	PushToStack(r);
}

void ASTInterpreter::ResolveFunction(FunctionEntity* e) {
	vector<Entity*> raw_args = e->GetArguments();
	vector<double> args;
	for (vector<Entity*>::iterator it = raw_args.begin(); it != raw_args.end(); ++it) {
		Resolve(*it);
		args.push_back(PopFromStack());
	}

	if (mVerbose)
		cout << "AST function_stack_push" << endl;

	for (vector<double>::reverse_iterator it = args.rbegin(); it != args.rend(); ++it) {
		PushToStack(*it);
	}

	CallDirective(e->GetAbsValue(), e->IsNegative());
}

bool ASTInterpreter::SymbolExists(string k) {
	bool r = false;

	try {
		mSymbols.at(k);
		r = true;
	} catch (const out_of_range &ex) {
		//. pass?
		r = false;
	}

	return r;
}

void ASTInterpreter::SetSymbol(string k, double v) {
	if (mVerbose)
		cout << "AST set_symbol " << k << "=" << v << endl;

	if (!OperandEntity::IsValid(k))
		throw ASTValueError("invalid symbol name");
	else if (k == "_")
		PushToStack(v);
	else if (k == "__")
		throw new ASTInvalidOperation("assignment to a reserved symbol");
	else
		mSymbols[k] = v;
}

double ASTInterpreter::GetSymbol(string k, bool negative, bool ignore_error) {
	double ret = 0;

	if (mVerbose)
		cout << "AST get_symbol " << k << endl;

	if (k == "_")
		ret = PopFromStack();
	else if (k == "__")
		ret = mStack.size();
	else if (SymbolExists(k))
		ret = mSymbols[k];
	else if (!ignore_error)
		throw ASTNotFound("cannot find symbol " + k);

	if (negative)
		ret = -ret;

	return ret;
}

bool ASTInterpreter::DirectiveExists(string k) {
	if (k == "_" || k == "__" ||
		k == "__cmp_eq__" || k == "__cmp_neq__" ||
		k == "__cmp_lt__" || k == "__cmp_lte__" ||
		k == "__cmp_gt__" || k == "__cmp_gte__")
		return true;

	try {
		mDirectives.at(k);
		return true;
	} catch (const out_of_range &ex) {
		//. pass?
	}

	return false;
}

void ASTInterpreter::SetDirective(string k, string v) {
	if (mVerbose)
		cout << "AST set_directive " << k << "=" << v << endl;

	if (!regex_match(k, regex("^[A-Za-z_]{1}[A-Za-z0-9_]*$"))) {
		throw ASTNotFound("invalid directive name " + k);
	} else if (k == "__cmp_eq__" || k == "__cmp_neq__" ||
				k == "__cmp_lt__" || k == "__cmp_lte__" ||
				k == "__cmp_gt__" || k == "__cmp_gte__") {
		throw ASTInvalidOperation("assignment to a reserved directive");
	}

	Entity *p = nullptr;
	if (!v.empty())
		p = Parse(v);

	if (DirectiveExists(k) && mDirectives[k] != nullptr)
		delete mDirectives[k];

	mDirectives[k] = p;
}

void ASTInterpreter::CallDirective(string k, bool negative, bool ignore_error) {
	if (mVerbose)
		cout << "AST call_directive " << k << endl;

	if (k == "__cmp_eq__") {
		if (!SymbolExists("_1") || !SymbolExists("_2") || !SymbolExists("_3") || !SymbolExists("_4"))
			throw ASTInvalidOperation("directive required symbols do not exists");
		else
			SetSymbol("_1", (GetSymbol("_1") == GetSymbol("_2") ? GetSymbol("_3") : GetSymbol("_4")));
	} else if (k == "__cmp_neq__") {
		if (!SymbolExists("_1") || !SymbolExists("_2") || !SymbolExists("_3") || !SymbolExists("_4"))
			throw ASTInvalidOperation("directive required symbols do not exists");
		else
			SetSymbol("_1", (GetSymbol("_1") != GetSymbol("_2") ? GetSymbol("_3") : GetSymbol("_4")));
	} else if (k == "__cmp_lt__") {
		if (!SymbolExists("_1") || !SymbolExists("_2") || !SymbolExists("_3") || !SymbolExists("_4"))
			throw ASTInvalidOperation("directive required symbols do not exists");
		else
			SetSymbol("_1", (GetSymbol("_1") < GetSymbol("_2") ? GetSymbol("_3") : GetSymbol("_4")));
	} else if (k == "__cmp_lte__") {
		if (!SymbolExists("_1") || !SymbolExists("_2") || !SymbolExists("_3") || !SymbolExists("_4"))
			throw ASTInvalidOperation("directive required symbols do not exists");
		else
			SetSymbol("_1", (GetSymbol("_1") <= GetSymbol("_2") ? GetSymbol("_3") : GetSymbol("_4")));
	} else if (k == "__cmp_gt__") {
		if (!SymbolExists("_1") || !SymbolExists("_2") || !SymbolExists("_3") || !SymbolExists("_4"))
			throw ASTInvalidOperation("directive required symbols do not exists");
		else
			SetSymbol("_1", (GetSymbol("_1") > GetSymbol("_2") ? GetSymbol("_3") : GetSymbol("_4")));
	} else if (k == "__cmp_gte__") {
		if (!SymbolExists("_1") || !SymbolExists("_2") || !SymbolExists("_3") || !SymbolExists("_4"))
			throw ASTInvalidOperation("directive required symbols do not exists");
		else
			SetSymbol("_1", (GetSymbol("_1") >= GetSymbol("_2") ? GetSymbol("_3") : GetSymbol("_4")));
	} else if (DirectiveExists(k)) {
		if (mDirectives[k] == nullptr) {
			throw ASTInvalidOperation("cannot call null directive " + k);
		} else {
			Resolve(mDirectives[k]);
			if (mVerbose) {
				double r = PopFromStack();
				cout << "RES " << r << endl;
				PushToStack(r);
			}
		}
	} else if (!ignore_error) {
		throw ASTNotFound("cannot find directive " + k);
	}

	if (negative) {
		PushToStack(-PopFromStack());
	}
}

void ASTInterpreter::SetVerbose(bool verbose) {
	mVerbose = verbose;
}

bool ASTInterpreter::GetVerbose() {
	return mVerbose;
}

bool ASTInterpreter::IsStackEmpty() {
	return mStack.empty();
}

double ASTInterpreter::PopFromStack() {
	if (IsStackEmpty()) {
		throw ASTInvalidOperation("stack is empty");
	}

	double d = mStack.top();
	if (mVerbose)
		cout << "AST stack_pop " << d << endl;

	mStack.pop();
	return d;
}

void ASTInterpreter::PushToStack(double v) {
	if (mVerbose)
		cout << "AST stack_push " << v << endl;
	mStack.push(v);
}

ASTInterpreter::~ASTInterpreter() {
	delete mDirectivePattern;
	delete mDirectiveSetPattern;
	delete mDirectiveCallPattern;
	delete mDirectiveIncludePattern;
	delete mSymbolSetPattern;

	// free all directives
	for (unordered_map<string, Entity*>::iterator it = mDirectives.begin(); it != mDirectives.end(); ++it) {
		if (it->second != nullptr)
			delete it->second;
	}

	if (mVerbose) {
		cout << "AST destroyed with " << mStack.size() << " item(s) on the stack" << endl;
	}
}
