#include "interpreter.hpp"

#include <sstream>
#include <unordered_map>
#include <cmath>

using namespace std;

ASTInterpreter::ASTInterpreter() {
	mCommentPattern = new regex("^#(.*)$");
	mDirectivePattern = new regex("^\\s*@(.*)$");
	mDirectiveSetPattern = new regex("^\\[\\s*\\$([A-Za-z_]{1}[A-Za-z0-9_]*)\\$\\s*((.*)\\s*){0,1}\\]\\s*$");
	mDirectiveCallPattern = new regex("^\\[\\s*([A-Za-z_]{1}[A-Za-z0-9_]*)\\s*\\]\\s*$");
	mSymbolSetPattern = new regex("^([A-Za-z_]{1}[A-Za-z0-9_]*)\\s*=\\s*(.*)\\s*$");
}

double ASTInterpreter::Run(string s, Entity **e) {
	smatch matches;
	Entity *tok = nullptr;
	double r = 0;
	if (regex_match(s, matches, *mDirectivePattern)) {
		string dir = matches.str(1);
		if (regex_match(dir, matches, *mDirectiveSetPattern)) {
			SetDirective(matches.str(1), matches.str(3));
		} else if (regex_match(dir, matches, *mDirectiveCallPattern)) {
			CallDirective(matches.str(1));
		} else if (regex_match(dir, matches, *mSymbolSetPattern)) {
			// supress the output
			tok = Parse(matches.str(2));
			try {
				SetSymbol(matches.str(1), Resolve(tok));
				delete tok;
				tok = nullptr;
			} catch(const ASTException &ex) {
				delete tok;
				throw ex;
			}
		} else {
			throw ASTSyntaxError("invalid directive syntax");
		}
	} else if (!regex_match(s, *mCommentPattern)) {
		tok = Parse(s);
		r = Resolve(tok);
	}

	if (e != nullptr)
		*e = tok;
	else
		delete tok;

	return r;
}

double ASTInterpreter::Resolve(Entity *e) {
	double r;

	if (e == nullptr)
		throw ASTValueError("cannot resolve null entity");

	switch(e->GetType()) {
	case Entity::PARENTHESIS_ENTITY:
		r = ResolveParenthesis((ParenthesisEntity*)e);
		break;
	case Entity::COMPOUND_ENTITY:
		r = ResolveCompound((CompoundEntity*)e);
		break;
	case Entity::OPERAND_ENTITY:
		r = ResolveOperand((OperandEntity*)e);
		break;
	case Entity::LITERAL_ENTITY:
		r = ResolveLiteral((LiteralEntity*)e);
		break;
	case Entity::INVALID_ENTITY:
	default:
		throw ASTTypeError("cannot resolve entity " + e->GetTypeString());
	}

	return r;
}

double ASTInterpreter::ResolveParenthesis(ParenthesisEntity *e) {
	double r = Resolve(e->Get());

	if (e->IsNegative())
		r *= -1;

	return r;
}

double ASTInterpreter::ResolveCompound(CompoundEntity *e) {
	Entity *l = e->Get(CompoundEntity::LEFT_ENTITY),
		   *r = e->Get(CompoundEntity::RIGHT_ENTITY);
	double ret = 0;

	switch(e->GetOperator()) {
	case TieredEntity::ARITHMETIC_ADD:
		ret = Resolve(l) + Resolve(r);
		break;
	case TieredEntity::ARITHMETIC_SUB:
		ret = Resolve(l) - Resolve(r);
		break;
	case TieredEntity::ARITHMETIC_MUL:
		ret = Resolve(l) * Resolve(r);
		break;
	case TieredEntity::ARITHMETIC_DIV:
		ret = Resolve(l) / Resolve(r);
		break;
	case TieredEntity::ARITHMETIC_MOD:
		ret = fmod(Resolve(l), Resolve(r));
		break;
	case TieredEntity::ARITHMETIC_POW:
		ret = pow(Resolve(l), Resolve(r));
		break;
	case TieredEntity::OPERATOR_SET:
		if (l->GetType() == Entity::OPERAND_ENTITY) {
			ret = Resolve(r);
			SetSymbol(l->GetString(), ret);
			break;
		} else {
			throw ASTInvalidOperation("invalid operand for assignment operation");
			return -1;
		}
	default:
		throw ASTInvalidOperation("invalid operation " + e->GetOperatorString());
		return -1;
	}

	return ret;
}

double ASTInterpreter::ResolveOperand(OperandEntity *e) {
	double r = GetSymbol(e->GetAbsValue());
	return e->IsNegative() ? -r : r;
}

double ASTInterpreter::ResolveLiteral(LiteralEntity *e) {
	const string v(e->GetAbsValue());
	istringstream i(v);
	double r;

	if (!(i >> r))
		throw ASTValueError("invalid literal \"" + v + "\"");

	return e->IsNegative() ? -r : r;
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
	if (!OperandEntity::IsValid(k))
		throw ASTValueError("invalid symbol name");
	else
		mSymbols[k] = v;
}

double ASTInterpreter::GetSymbol(string k, bool ignore_error) {
	if (SymbolExists(k)) {
		return mSymbols[k];
	} else if (!ignore_error) {
		throw ASTNotFound("cannot find symbol " + k);
	} else {
		return 0;
	}
}

bool ASTInterpreter::DirectiveExists(string k) {
	bool r = false;

	try {
		mDirectives.at(k);
		r = true;
	} catch (const out_of_range &ex) {
		//. pass?
		r = false;
	}
	
	return r;
}

void ASTInterpreter::SetDirective(string k, string v) {
	if (!regex_match(k, regex("^[A-Za-z_]{1}[A-Za-z0-9_]*$"))) {
		throw ASTNotFound("invalid directive name " + k);
	}

	Entity *p = nullptr;
	if (!v.empty())
		p = Parse(v);

	if (DirectiveExists(k) && mDirectives[k] != nullptr)
		delete mDirectives[k];

	mDirectives[k] = p;
}

void ASTInterpreter::CallDirective(string k, bool ignore_error) {
	//cout << "AST call_directive " << k << endl;
	if (DirectiveExists(k)) {
		if (mDirectives[k] == nullptr) {
			throw ASTInvalidOperation("cannot call null directive " + k);
		} else {
			Resolve(mDirectives[k]);
		}
	} else if (!ignore_error) {
		throw ASTNotFound("cannot find directive " + k);
	}
}

ASTInterpreter::~ASTInterpreter() {
	delete mDirectivePattern;
	delete mDirectiveSetPattern;
	delete mDirectiveCallPattern;;
	delete mSymbolSetPattern;

	// free all directives
	for (unordered_map<string, Entity*>::iterator it = mDirectives.begin(); it != mDirectives.end(); ++it) {
		if (it->second != nullptr)
			delete it->second;
	}
}