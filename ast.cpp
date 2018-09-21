#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <regex>
#include <exception>
#include <cmath>

using namespace std;

// -- MARK: Exceptions

class ASTException : public exception {
public:
	ASTException(string str) {
		mMessage = str;
	}

	ASTException(const char *str) {
		mMessage = str;
	}

	const char* what() const noexcept {
		return mMessage.c_str();
	}
private:
	string mMessage;
};

class ASTInvalidOperation : public ASTException {
public:
	ASTInvalidOperation(string str) : ASTException(str) {}
	ASTInvalidOperation(const char *str) : ASTException(str) {}
};

class ASTNotFound : public ASTException {
public:
	ASTNotFound(string str) : ASTException(str) {}
	ASTNotFound(const char *str) : ASTException(str) {}
};

class ASTTypeError : public ASTException {
public:
	ASTTypeError(string str) : ASTException(str) {}
	ASTTypeError(const char *str) : ASTException(str) {}
};

class ASTValueError : public ASTException {
public:
	ASTValueError(string str) : ASTException(str) {}
	ASTValueError(const char *str) : ASTException(str) {}
};

class ASTSyntaxError : public ASTException {
public:
	ASTSyntaxError(string str) : ASTException(str) {}
	ASTSyntaxError(const char *str) : ASTException(str) {}
};

// -- MARK: AST Entities

class Entity {
public:
	typedef enum {
		INVALID_ENTITY = -1,
		PARENTHESIS_ENTITY,
		COMPOUND_ENTITY,
		OPERAND_ENTITY,
		LITERAL_ENTITY,
	} EntityType;

	virtual EntityType GetType() {
		return INVALID_ENTITY;
	}

	virtual string GetTypeString() {
		switch(this->GetType()) {
		case COMPOUND_ENTITY:
			return "COMPOUND_ENTITY";
			break;
		case OPERAND_ENTITY:
			return "OPERAND_ENTITY";
			break;
		case LITERAL_ENTITY:
			return "LITERAL_ENTITY";
			break;
		case INVALID_ENTITY:
		default:
			return "INVALID_ENTITY";
		}
	}

	virtual string GetString() {
		throw ASTException("getting string value on base Entity");
	}

	virtual ~Entity() {}
};

class NegatableEntity {
public:
	bool IsNegative() {
		return mIsNegative;
	}

	void SetNegative(bool negative) {
		mIsNegative = negative;
	}
private:
	bool mIsNegative = false;
};

class SingleValueEntity : public Entity, public NegatableEntity {
public:
	virtual string GetString() {
		return IsNegative() ? "(-" + mValue + ")" : mValue;
	}

	virtual string GetValue() {
		return IsNegative() ? "-" + mValue : mValue;
	}

	virtual string GetAbsValue() {
		return mValue;
	}

	virtual void SetValue(string value) {
		throw ASTException("setting value on base SingleValueEntity");
	}

	virtual ~SingleValueEntity() {}
protected:
	string mValue;
};

class OperandEntity final : public SingleValueEntity {
public:
	OperandEntity(string value) {
		SetValue(value);
	}

	static bool IsValid(string value) {
		regex pattern("^-{0,1}[A-Za-z]+$");
		return value != "inf" && value != "nan" && regex_match(value, pattern);
	}

	EntityType GetType() override {
		return OPERAND_ENTITY;
	}

	void SetValue(string value) override {
		if (OperandEntity::IsValid(value)) {
			if (value[0] == '-') {
				mValue = value.substr(1);
				SetNegative(true);
			} else mValue = value;
		} else {
			throw ASTValueError("invalid value for operand");
		}
	}

	~OperandEntity() {}
};

class LiteralEntity final : public SingleValueEntity {
public:
	LiteralEntity(string value) {
		SetValue(value);
	}

	static bool IsValid(string value) {
		regex pattern("^-{0,1}[0-9]+(\\.[0-9]+){0,1}$");
		return value == "inf" || value == "nan" || regex_match(value, pattern);
	}

	EntityType GetType() override {
		return LITERAL_ENTITY;
	}

	void SetValue(string value) override {
		if (LiteralEntity::IsValid(value)) {
			if (value[0] == '-') {
				mValue = value.substr(1);
				SetNegative(true);
			} else mValue = value;
		} else {
			throw ASTValueError("invalid value for literal");
		}
	}

	~LiteralEntity() {}
};

class TieredEntity : public Entity {
public:
	typedef enum {
		OPERATOR_NONE = -1,
		ARITHMETIC_ADD,
		ARITHMETIC_SUB,
		ARITHMETIC_MUL,
		ARITHMETIC_DIV,
		ARITHMETIC_MOD,
		ARITHMETIC_POW,
		PARENTHESIS,
		OPERATOR_SET
	} OperatorType;

	typedef enum {
		ASSOC_INVALID = -1,
		ASSOC_LEFT,
		ASSOC_RIGHT
	} Associativity;

	static OperatorType OPERATOR(char op) {
		if (op == '+') {
			return ARITHMETIC_ADD;
		} else if (op == '-') {
			return ARITHMETIC_SUB;
		} else if (op == '*') {
			return ARITHMETIC_MUL;
		} else if (op == '/') {
			return ARITHMETIC_DIV;
		} else if (op == '%') {
			return ARITHMETIC_MOD;
		} else if (op == '^') {
			return ARITHMETIC_POW;
		} else if (op == '=') {
			return OPERATOR_SET;
		} else {
			return OPERATOR_NONE;
		}
	}

	static int PRECEDENCE(OperatorType type) {
		switch(type) {
		case ARITHMETIC_ADD:
			return 3;
			break;
		case ARITHMETIC_SUB:
			return 3;
			break;
		case ARITHMETIC_MUL:
			return 2;
			break;
		case ARITHMETIC_DIV:
			return 2;
			break;
		case ARITHMETIC_MOD:
			return 2;
			break;
		case ARITHMETIC_POW:
			return 1;
			break;
		case PARENTHESIS:
			return 0;
		case OPERATOR_SET:
			return -1;
		default:
			return -1;
		}
	}

	static Associativity ASSOCIATIVE(OperatorType type) {
		switch(type) {
		case ARITHMETIC_ADD:
			return ASSOC_LEFT;
			break;
		case ARITHMETIC_SUB:
			return ASSOC_LEFT;
			break;
		case ARITHMETIC_MUL:
			return ASSOC_LEFT;
			break;
		case ARITHMETIC_DIV:
			return ASSOC_LEFT;
			break;
		case ARITHMETIC_MOD:
			return ASSOC_LEFT;
			break;
		case ARITHMETIC_POW:
			return ASSOC_LEFT;
			break;
		case PARENTHESIS:
			return ASSOC_LEFT;
		case OPERATOR_SET:
			return ASSOC_RIGHT;
		default:
			return ASSOC_INVALID;
		}
	}

	OperatorType GetOperator() {
		return mOperatorType;
	}

	string GetOperatorString() {
		switch(mOperatorType) {
		case ARITHMETIC_ADD:
			return "+";
			break;
		case ARITHMETIC_SUB:
			return "-";
			break;
		case ARITHMETIC_MUL:
			return "*";
			break;
		case ARITHMETIC_DIV:
			return "/";
			break;
		case ARITHMETIC_MOD:
			return "%";
			break;
		case ARITHMETIC_POW:
			return "^";
			break;
		case OPERATOR_SET:
			return "=";
			break;
		default:
			return "UNKNOWN_OPERATOR";
		}
	}

	virtual int GetOperatorPrecedence() {
		return PRECEDENCE(mOperatorType);
	}

	virtual int GetOperatorAssociativity() {
		return ASSOCIATIVE(mOperatorType);
	}

	virtual void SetOperator(OperatorType type) {
		mOperatorType = type;
	}
private:
	OperatorType mOperatorType = OPERATOR_NONE;
};

class ParenthesisEntity final : public TieredEntity, public NegatableEntity {
public:
	ParenthesisEntity() {}

	ParenthesisEntity(Entity *e, bool negative=false) {
		Set(e);
		SetNegative(negative);
	}

	EntityType GetType() override {
		return PARENTHESIS_ENTITY;
	}

	Entity* Get() {
		return mValue;
	}

	virtual int GetOperatorPrecedence() override {
		return PRECEDENCE(PARENTHESIS);
	}

	string GetString() override {
		return (IsNegative() ? "-(" : "(") + mValue->GetString() + ")";
	}

	void Set(Entity *e) {
		mValue = e;
	}

	virtual void SetOperator(OperatorType type) override {
		throw ASTException("setting operator on ParenthesisEntity");
	}
private:
	Entity *mValue;
};

class CompoundEntity final : public TieredEntity {
public:
	typedef enum {
		LEFT_ENTITY,
		RIGHT_ENTITY,
	} EntityPosition;

	CompoundEntity() {}

	CompoundEntity(OperatorType type, Entity *left = nullptr, Entity *right = nullptr) {
		SetOperator(type);
		Set(LEFT_ENTITY, left);
		Set(RIGHT_ENTITY, right);
	}

	EntityType GetType() override {
		return COMPOUND_ENTITY;
	}

	Entity* Get(EntityPosition pos) {
		switch(pos) {
		case LEFT_ENTITY:
			return mLeft;
			break;
		case RIGHT_ENTITY:
			return mRight;
			break;
		default:
			// not suppose to be here.
			return nullptr;
		}
	}

	string GetString() override {
		string ls, rs;
		Entity *le = Get(LEFT_ENTITY), *re = Get(RIGHT_ENTITY);

		if (le == nullptr)
			ls = "NULL";
		else
			ls = le->GetString();

		if (re == nullptr)
			rs = "NULL";
		else
			rs = re->GetString();

		return ls + GetOperatorString() + rs;
	}

	void Set(EntityPosition pos, Entity *value) {
		if (value != nullptr && value->GetType() == Entity::INVALID_ENTITY) {
			throw ASTValueError("invalid entity");
		}

		switch(pos) {
		case LEFT_ENTITY:
			mLeft = value;
			break;
		case RIGHT_ENTITY:
			mRight = value;
			break;
		default:
			// not suppose to be here.
			break;
		}
	}

	~CompoundEntity() {
		if (mLeft != nullptr)
			delete mLeft;

		if (mRight != nullptr)
			delete mRight;
	}
private:
	Entity *mLeft = nullptr, *mRight = nullptr;
};

// -- MARK: AST Lexical Analysis

class ASTLex {
public:
	ASTLex() {}

	Entity* GetEntityFrom(string code) {
		Entity* VAL = nullptr;
		if (OperandEntity::IsValid(code)) {
			VAL = new OperandEntity(code);
		} else if (LiteralEntity::IsValid(code)) {
			VAL = new LiteralEntity(code);
		} else {
			throw ASTSyntaxError("invalid syntax0");
		}
		return VAL;
	}

	Entity* Parse(string code) {
		Entity* HEAD = nullptr;
		CompoundEntity* TMP = new CompoundEntity();

		Entity *ENT = nullptr;

		string tmp_str;

		for (string::iterator it = code.begin(); it != code.end(); ++it) {
			char c = *it;
			CompoundEntity::OperatorType t;

			if (c == ' ' || c == '\t' || c == '\n') {
				// ignore whitespace
				continue;
			} else if (c == ')') {
				CLEANUP(HEAD);
				CLEANUP(TMP);
				CLEANUP(ENT);
				throw ASTSyntaxError("invalid syntax1");
			} else if (c == '(') {
				int level = 1;
				size_t begin = ++it -  code.begin();
				bool negative = false;

				// negative sign
				if (!tmp_str.empty()) {
					if (tmp_str.length() != 1 || tmp_str[0] != '-') {
						CLEANUP(HEAD);
						CLEANUP(TMP);
						CLEANUP(ENT);
						throw ASTSyntaxError("invalid syntax2");
					} else negative = true;
				}

				while (it != code.end()) {
					c = *it;

					if (c == '(')
						level++;
					else if (c == ')')
						level--;

					//cout << "level " << level << ": " << c << endl;

					if (level == 0)
						break;

					++it;
 				}

 				//cout << "break" << endl;

				if (level != 0 || ENT != nullptr) {
					CLEANUP(HEAD);
					CLEANUP(TMP);
					CLEANUP(ENT);
					throw ASTSyntaxError("invalid syntax3");
				} else {
					try {
						//cout << "INNER " << code.substr(begin, it - code.begin() - begin) << endl;
						ENT = new ParenthesisEntity(Parse(code.substr(begin, it - code.begin() - begin)), negative);
					} catch (ASTException *ex) {
						//cout << "INNER ERROR" << endl;
						CLEANUP(HEAD);
						CLEANUP(TMP);
						CLEANUP(ENT);
						throw ex;
					}
				}

				tmp_str.clear();
			} else if ((t = TieredEntity::OPERATOR(c)) != TieredEntity::OPERATOR_NONE) {
				auto OP = TMP->GetOperator();
				if (t == TieredEntity::ARITHMETIC_SUB && tmp_str.empty() && ENT == nullptr) {
					// negative sign for literal
					tmp_str += c;
					continue;
				}
				
				try {
					if (ENT == nullptr)
						ENT = GetEntityFrom(tmp_str);
				} catch (ASTException *ex) {
					//cout << "HELP!" << endl;
					CLEANUP(HEAD);
					CLEANUP(TMP);
					CLEANUP(ENT);
					throw ex;
				}

				if (OP == TieredEntity::OPERATOR_NONE) {
					// set left and operator
					TMP->SetOperator(t);
					//cout << "OPERATOR: " << TMP->GetOperatorString() << endl;
					TMP->Set(CompoundEntity::LEFT_ENTITY, ENT);
					tmp_str.clear();
				} else {
					// set right
					TMP->Set(CompoundEntity::RIGHT_ENTITY, ENT);

					if (HEAD == nullptr) {
						HEAD = TMP;
					} else {
						switch(TMP->GetOperatorAssociativity()) {
						case TieredEntity::ASSOC_LEFT:
							LeftAssociate(&HEAD, TMP);
							break;
						case TieredEntity::ASSOC_RIGHT:
							RightAssociate(&HEAD, TMP);
							break;
						default:
							// should not go here
							break;
						}
					}

					// then store operator back
					TMP = new CompoundEntity(t, nullptr, nullptr);
					tmp_str.clear();
				}

				ENT = nullptr;
			} else {
				tmp_str += c;
			}
		}

		if (HEAD == nullptr) {
			if (TMP->GetOperator() == CompoundEntity::OPERATOR_NONE) {
				// single value
				CLEANUP(TMP); // we don't need this anymore.

				if (ENT != nullptr)
					HEAD = ENT;
				else if (OperandEntity::IsValid(tmp_str))
					HEAD = new OperandEntity(tmp_str);
				else if (LiteralEntity::IsValid(tmp_str)) {
					HEAD = new LiteralEntity(tmp_str);
				}
				else
					throw ASTSyntaxError("invalid syntax4");
			} else {
				try {
					if (ENT == nullptr)
						ENT = GetEntityFrom(tmp_str);
				} catch (ASTException *ex) {
					//cout << "HELP" << endl;
					CLEANUP(HEAD);
					CLEANUP(TMP);
					CLEANUP(ENT);
					throw ex;
				}

				TMP->Set(CompoundEntity::RIGHT_ENTITY, ENT);
				HEAD = TMP;
			}
		} else {
			try {
				if (ENT == nullptr)
					ENT = GetEntityFrom(tmp_str);
			} catch (ASTException *ex) {
				//cout << "HELP" << endl;
				CLEANUP(HEAD);
				CLEANUP(TMP);
				CLEANUP(ENT);
				throw ex;
			}

			// set right
			TMP->Set(CompoundEntity::RIGHT_ENTITY, ENT);

			if (HEAD == nullptr) {
				HEAD = TMP;
			} else {
				switch(TMP->GetOperatorAssociativity()) {
				case TieredEntity::ASSOC_LEFT:
					LeftAssociate(&HEAD, TMP);
					break;
				case TieredEntity::ASSOC_RIGHT:
					RightAssociate(&HEAD, TMP);
					break;
				default:
					// should not go here
					break;
				}
			}
		}

		return HEAD;
	}

	void LeftAssociate(Entity **HEAD, CompoundEntity *TMP) {
		// process
		CompoundEntity *CUR = (CompoundEntity*) *HEAD;

		if (CUR->GetOperatorPrecedence() <= TMP->GetOperatorPrecedence()) {
			TMP->Set(CompoundEntity::LEFT_ENTITY, CUR);
			*HEAD = TMP;
		} else {
			for(;;) {
				Entity *n = CUR->Get(CompoundEntity::RIGHT_ENTITY);
				if (n->GetType() == Entity::COMPOUND_ENTITY) {
					if (((CompoundEntity*)n)->GetOperatorPrecedence() > TMP->GetOperatorPrecedence())
						CUR = (CompoundEntity*) n;
					else
						break;
				} else {
					break;
				}
			}
			TMP->Set(CompoundEntity::LEFT_ENTITY, CUR->Get(CompoundEntity::RIGHT_ENTITY));
			CUR->Set(CompoundEntity::RIGHT_ENTITY, TMP);
		}
	}

	void RightAssociate(Entity **HEAD, CompoundEntity *TMP) {
		// process
		CompoundEntity *CUR = (CompoundEntity*) *HEAD;

		if (CUR->GetOperatorPrecedence() < TMP->GetOperatorPrecedence()) {
			TMP->Set(CompoundEntity::LEFT_ENTITY, CUR);
			*HEAD = TMP;
		} else {
			for(;;) {
				Entity *n = CUR->Get(CompoundEntity::RIGHT_ENTITY);
				if (n->GetType() == Entity::COMPOUND_ENTITY) {
					if (((CompoundEntity*)n)->GetOperatorPrecedence() >= TMP->GetOperatorPrecedence())
						CUR = (CompoundEntity*) n;
					else
						break;
				} else {
					break;
				}
			}
			TMP->Set(CompoundEntity::LEFT_ENTITY, CUR->Get(CompoundEntity::RIGHT_ENTITY));
			CUR->Set(CompoundEntity::RIGHT_ENTITY, TMP);
		}
	}

	~ASTLex() {}
protected:
	void CLEANUP(Entity *e) {
		if (e != nullptr)
			delete e;
	}
};

class ASTInterpreter : public ASTLex {
public:
	ASTInterpreter() {
		mCommentPattern = new regex("^#(.*)$");
		mDirectivePattern = new regex("^\\s*@(.*)$");
		mDirectiveSetPattern = new regex("^\\[\\s*\\$([A-Za-z_]+)\\s*=\\s*(.*)\\s*\\]\\s*$");
		mDirectiveCallPattern = new regex("^\\[\\s*([A-Za-z_]+)\\s*\\]\\s*$");
		mSymbolSetPattern = new regex("^([A-Za-z]+)\\s*=\\s*(.*)\\s*$");
	}

	double Run(string s, Entity **e = nullptr) {
		smatch matches;
		Entity *tok = nullptr;
		double r = 0;
		if (regex_match(s, matches, *mDirectivePattern)) {
			string dir = matches.str(1);
			if (regex_match(dir, matches, *mDirectiveSetPattern)) {
				SetDirective(matches.str(1), matches.str(2));
			} else if (regex_match(dir, matches, *mDirectiveCallPattern)) {
				CallDirective(matches.str(1));
			} else if (regex_match(dir, matches, *mSymbolSetPattern)) {
				tok = Parse(matches.str(2));
				r = Resolve(tok);
				SetSymbol(matches.str(1), r);
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

	double Resolve(Entity *e) {
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

	double ResolveParenthesis(ParenthesisEntity *e) {
		double r = Resolve(e->Get());

		if (e->IsNegative())
			r *= -1;

		return r;
	}

	double ResolveCompound(CompoundEntity *e) {
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

	double ResolveOperand(OperandEntity *e) {
		double r = GetSymbol(e->GetAbsValue());
		return e->IsNegative() ? -r : r;
	}

	double ResolveLiteral(LiteralEntity *e) {
		const string v(e->GetAbsValue());
		istringstream i(v);
		double r;

		if (!(i >> r))
			throw ASTValueError("invalid literal \"" + v + "\"");

		return e->IsNegative() ? -r : r;
	}

	bool SymbolExists(string k) {
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

	void SetSymbol(string k, double v) {
		if (!OperandEntity::IsValid(k))
			throw ASTValueError("invalid symbol name");
		else
			mSymbols[k] = v;
	}

	double GetSymbol(string k, bool ignore_error=false) {
		if (SymbolExists(k)) {
			return mSymbols[k];
		} else if (!ignore_error) {
			throw ASTNotFound("cannot find symbol " + k);
		} else {
			return 0;
		}
	}

	bool DirectiveExists(string k) {
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

	void SetDirective(string k, string v) {
		if (v.empty())
			mDirectives[k] = nullptr;
		else
			mDirectives[k] = Parse(v);
	}

	void CallDirective(string k, bool ignore_error=false) {
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

	~ASTInterpreter() {
		delete mDirectivePattern;
		delete mDirectiveSetPattern;
		delete mDirectiveCallPattern;;
		delete mSymbolSetPattern;

		// free all directives
		for (std::unordered_map<string, Entity*>::iterator it = mDirectives.begin(); it != mDirectives.end(); ++it) {
			if (it->second != nullptr)
				delete it->second;
		}
	}
protected:
	std::unordered_map<string, double> mSymbols;
	std::unordered_map<string, Entity*> mDirectives;
	regex *mCommentPattern = nullptr;
	regex *mSymbolSetPattern = nullptr;
	regex *mDirectivePattern = nullptr, *mDirectiveSetPattern = nullptr, *mDirectiveCallPattern = nullptr;
};

string GetPostfix(Entity *e) {
	if (e == nullptr) {
		return "[NULL]";
	} else if (e->GetType() != Entity::COMPOUND_ENTITY) {
		return e->GetString();
	} else {
		string ls, rs;
		CompoundEntity *c = (CompoundEntity*) e;
		Entity *le = c->Get(CompoundEntity::LEFT_ENTITY), *re = c->Get(CompoundEntity::RIGHT_ENTITY);

		if (le == nullptr)
			ls = "[NULL]";
		else ls = GetPostfix(le);

		if (re == nullptr)
			rs = "[NULL]";
		else rs = GetPostfix(re);

		return ls + rs + c->GetOperatorString();
	}
}

void TestSuite(ASTInterpreter *m, string fn) {
	ifstream fp(fn);
	bool unexpected = true;

	int ok = 0, miss = 0, error = 0;

	if (!fp.is_open()) {
		cout << "ERR Cannot open file " << fn << endl;
		return;
	}
	
	string input;
	Entity *tmp;

	while (fp.good())
	try {
		double r, o;
		bool nan;
		string now;
		string output;
		size_t p;

		tmp = nullptr;
		unexpected = true;

		getline(fp, now, '\n');
		//getline(fp, output, '\n');

		if (!fp.good() && now.empty()) {
			//cout << endl;
			break;
		}

		if (now.find('\\') == now.length() - 1) {
			input += now.substr(0, now.length() - 1);
			continue;
		} else {
			input += now;
		}

		p = input.find(',');
		if (p == string::npos || p == now.length()-1) {
			throw ASTException("wrong test suite syntax");
		} else {
			output = input.substr(p+1, string::npos);
			input = input.substr(0, p);
		}

		if (output == "ERROR") {
			unexpected = false;
		}

		r = m->Run(input, &tmp);

		if (!output.empty() && output != "ERROR" && output != "IGNORE" && (((nan = isnan(o = stod(output))) && isnan(r)) || (!nan && r == o))) {
			cout << "OK  " << input << " => " << GetPostfix(tmp) << " = " << r << endl;
			ok++;
		} else if (output != "IGNORE") {
			cout << "MIS " << input << " => " << GetPostfix(tmp) << " = " << r << endl;
			miss++;
		}

		delete tmp;
		input.clear();
	} catch(const ASTException &ex) {
		//cout << "ERR " << ex.what() << endl;
		if (unexpected) {
			cout << "ERR " << input << " => " << ex.what() << endl; 
			if (tmp != nullptr)
				delete tmp;
			error++;
		} else {
			cout << "OK  " << input << " => " << ex.what() << endl; 
			if (tmp != nullptr)
				delete tmp;
			ok++;
		}
		input.clear();
	}

	fp.close();

	cout << "COUNT---" << endl;
	cout << "OK: " << ok << endl;
	cout << "MIS: " << miss << endl;
	cout << "ERR: " << error << endl;
}

void REPL(ASTInterpreter *m) {
	string input;

	while (cin.good())
	try {
		Entity *tmp;
		double r;
		string now;

		cout << (input.empty() ? "> " : "  ");
		getline(cin, now, '\n');

		if (!cin.good() && now.empty()) {
			cout << endl;
			break;
		}

		if (now.find('\\') == now.length() - 1) {
			input += now.substr(0, now.length() - 1);
			continue;
		} else {
			input += now;
		}

		r = m->Run(input, &tmp);

		cout << "RETURN(" << (tmp == nullptr ? "INVALID_ENTITY" : tmp->GetTypeString()) << "): " << GetPostfix(tmp) << endl;

		cout << "EVAL: ";
		if (r == 0 && tmp == nullptr)
			cout << "[NULL]";
		else
			cout << r;
		cout << endl;

		delete tmp;
		input.clear();
	} catch(const ASTException &ex) {
		cout << "ERROR: " << ex.what() << endl;
		input.clear();
	}
}

int main(int argc, char **argv) {
	ASTInterpreter m;

	cout << "test " << (stod("nan") == stod("NAN") ? "NAN" : "NOPE") << endl;

	if (argc == 2) {
		TestSuite(&m, argv[1]);
	} else {
		REPL(&m);
	}

	return 0;
}