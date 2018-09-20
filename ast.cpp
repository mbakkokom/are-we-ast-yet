// NO BRACKET MATCHING, BUT EVERYTHING WORKS JUST FINE
// ALSO MEMORY LEAKS

#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <exception>

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
		throw new ASTException("getting string value on base Entity");
	}

	virtual ~Entity() {}
};

class SingleValueEntity : public Entity {
public:
	string GetValue() {
		return mValue;
	}

	string GetString() override {
		throw new ASTException("getting string value on base SingleValueEntity");
	}

	virtual void SetValue(string value) {
		throw new ASTException("setting value on base SingleValueEntity");
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
		regex pattern("^[A-Za-z]+$");
		return regex_match(value, pattern);
	}

	EntityType GetType() override {
		return OPERAND_ENTITY;
	}

	void SetValue(string value) override {
		if (OperandEntity::IsValid(value))
			mValue = value;
		else
			throw new ASTValueError("invalid value for operand");
	}

	string GetString() override {
		return GetValue();
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
		return regex_match(value, pattern);
	}

	EntityType GetType() override {
		return LITERAL_ENTITY;
	}

	void SetValue(string value) override {
		if (LiteralEntity::IsValid(value))
			mValue = value;
		else
			throw new ASTValueError("invalid value for literal");
	}

	string GetString() override {
		return GetValue();
	}

	~LiteralEntity() {}
};

class CompoundEntity final : public Entity {
public:
	typedef enum {
		LEFT_ENTITY,
		RIGHT_ENTITY,
	} EntityPosition;

	typedef enum {
		OPERATOR_NONE = -1,
		ARITHMETIC_ADD,
		ARITHMETIC_SUB,
		ARITHMETIC_MUL,
		ARITHMETIC_DIV,
		ARITHMETIC_MOD,
		ARITHMETIC_POW
	} OperatorType;

	CompoundEntity() {}

	CompoundEntity(OperatorType type, Entity *left = nullptr, Entity *right = nullptr) {
		SetOperator(type);
		Set(LEFT_ENTITY, left);
		Set(RIGHT_ENTITY, right);
	}

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
		} else {
			return OPERATOR_NONE;
		}
	}

	static int PRECEDENCE(OperatorType type) {
		switch(type) {
		case ARITHMETIC_ADD:
			return 2;
			break;
		case ARITHMETIC_SUB:
			return 2;
			break;
		case ARITHMETIC_MUL:
			return 1;
			break;
		case ARITHMETIC_DIV:
			return 1;
			break;
		case ARITHMETIC_MOD:
			return 1;
			break;
		case ARITHMETIC_POW:
			return 0;
			break;
		default:
			return -1;
		}
	}

	EntityType GetType() override {
		return COMPOUND_ENTITY;
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
		default:
			return "UNKNOWN_OPERATOR";
		}
	}

	int GetOperatorPrecedence() {
		return CompoundEntity::PRECEDENCE(mOperatorType);
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
		Entity::EntityType lt, rt;

		if (le == nullptr)
			ls = "NULL";
		else if ((lt = le->GetType()) == Entity::OPERAND_ENTITY || lt == Entity::LITERAL_ENTITY)
			ls = ((SingleValueEntity*) le)->GetValue();
		else if (lt == Entity::COMPOUND_ENTITY)
			ls = ((CompoundEntity*) le)->GetString();

		if (re == nullptr)
			rs = "NULL";
		else if ((rt = re->GetType()) == Entity::OPERAND_ENTITY || rt == Entity::LITERAL_ENTITY)
			rs = ((SingleValueEntity*) re)->GetValue();
		else if (rt == Entity::COMPOUND_ENTITY)
			rs = ((CompoundEntity*) re)->GetString();

		return string("(") + ls + GetOperatorString() + rs + ")";
	}

	void Set(EntityPosition pos, Entity *value) {
		if (value != nullptr && value->GetType() == Entity::INVALID_ENTITY) {
			throw new ASTValueError("invalid entity");
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

	void SetOperator(OperatorType type) {
		mOperatorType = type;
	}

	~CompoundEntity() {
		if (mLeft != nullptr)
			switch(mLeft->GetType()) {
			case COMPOUND_ENTITY:
				delete (CompoundEntity*)mLeft;
				break;
			case OPERAND_ENTITY:
				delete (OperandEntity*)mLeft;
				break;
			case LITERAL_ENTITY:
				delete (LiteralEntity*)mLeft;
				break;
			case INVALID_ENTITY:
			default:
				delete mLeft;
			}

		if (mRight != nullptr)
			switch(mRight->GetType()) {
			case COMPOUND_ENTITY:
				delete (CompoundEntity*)mRight;
				break;
			case OPERAND_ENTITY:
				delete (OperandEntity*)mRight;
				break;
			case LITERAL_ENTITY:
				delete (LiteralEntity*)mRight;
				break;
			case INVALID_ENTITY:
			default:
				delete mRight;
			}
	}
private:
	Entity *mLeft = nullptr, *mRight = nullptr;
	OperatorType mOperatorType = OPERATOR_NONE;
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
			throw new ASTSyntaxError("invalid syntax");
		}
		return VAL;
	}

	Entity* Tokenize(string code) {
		Entity* HEAD = nullptr;
		CompoundEntity* TMP = new CompoundEntity();

		string tmp_str;

		for (string::iterator it = code.begin(); it != code.end(); ++it) {
			char c = *it;
			CompoundEntity::OperatorType t;

			if (c == ' ' || c == '\t' || c == '\n') {
				// ignore whitespace
				continue;
			} else if ((t = CompoundEntity::OPERATOR(c)) != CompoundEntity::OPERATOR_NONE) {
				auto OP = TMP->GetOperator();
				if (t == CompoundEntity::ARITHMETIC_SUB && tmp_str.empty()) {
					// negative sign for literal
					tmp_str += c;
					continue;
				} else if (OP == CompoundEntity::OPERATOR_NONE) {
					// set left and operator
					TMP->SetOperator(t);
					TMP->Set(CompoundEntity::LEFT_ENTITY, GetEntityFrom(tmp_str));
					tmp_str.clear();
				} else {
					// set right
					TMP->Set(CompoundEntity::RIGHT_ENTITY, GetEntityFrom(tmp_str));

					// process
					CompoundEntity *CUR = (CompoundEntity*) HEAD;

					if (HEAD == nullptr) {
						HEAD = TMP;
					} else if (CUR->GetOperatorPrecedence() <= TMP->GetOperatorPrecedence()) {
						TMP->Set(CompoundEntity::LEFT_ENTITY, CUR);
						HEAD = TMP;
					} else {
						if (CUR->GetOperatorPrecedence() <= TMP->GetOperatorPrecedence()) {
							for(;;) {
								Entity *n = CUR->Get(CompoundEntity::RIGHT_ENTITY);
								if (n->GetType() == Entity::COMPOUND_ENTITY) {
									if (((CompoundEntity*)n)->GetOperatorPrecedence() > TMP->GetOperatorPrecedence()) {
										CUR = (CompoundEntity*) n;
									} else {
										break;
									}
								} else {
									break;
								}
							}
						}
						TMP->Set(CompoundEntity::LEFT_ENTITY, CUR->Get(CompoundEntity::RIGHT_ENTITY));
						CUR->Set(CompoundEntity::RIGHT_ENTITY, TMP);
					}

					// then store operator back
					TMP = new CompoundEntity(t, nullptr, nullptr);
					tmp_str.clear();
				}
			} else {
				tmp_str += c;
			}
		}

		if (HEAD == nullptr) {
			if (TMP->GetOperator() == CompoundEntity::OPERATOR_NONE) {
				// single value
				delete TMP; // we don't need this anymore.

				if (OperandEntity::IsValid(tmp_str))
					HEAD = new OperandEntity(tmp_str);
				else if (LiteralEntity::IsValid(tmp_str))
					HEAD = new LiteralEntity(tmp_str);
				else
					throw new ASTSyntaxError("invalid syntax");
			} else {
				TMP->Set(CompoundEntity::RIGHT_ENTITY, GetEntityFrom(tmp_str));
				HEAD = TMP;
			}
		} else {
			// set right
			TMP->Set(CompoundEntity::RIGHT_ENTITY, GetEntityFrom(tmp_str));

			// process
			CompoundEntity *CUR = (CompoundEntity*) HEAD;

			if (CUR->GetOperatorPrecedence() <= TMP->GetOperatorPrecedence()) {
				TMP->Set(CompoundEntity::LEFT_ENTITY, CUR);
				HEAD = TMP;
			} else {
				if (CUR->GetOperatorPrecedence() <= TMP->GetOperatorPrecedence()) {
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
				}
				TMP->Set(CompoundEntity::LEFT_ENTITY, CUR->Get(CompoundEntity::RIGHT_ENTITY));
				CUR->Set(CompoundEntity::RIGHT_ENTITY, TMP);
			}
		}

		return HEAD;
	}

	~ASTLex() {}
};

int main() {
	ASTLex l;

	while (cin.good())
	try {
		string input;
		Entity *tmp;

		cout << "> ";
		getline(cin, input, '\n');

		tmp = l.Tokenize(input);
		cout << "RETURN(" << tmp->GetTypeString() << "): " << tmp->GetString() << endl;

		delete tmp;
	} catch(ASTException *ex) {
		cout << "ERROR: " << ex->what() << endl;
	}

	return 0;
}