#include "lexical.hpp"

ASTLex::ASTLex() {}

Entity* ASTLex::GetEntityFrom(string code) {
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

Entity* ASTLex::Parse(string code) {
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

void ASTLex::LeftAssociate(Entity **HEAD, CompoundEntity *TMP) {
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

void ASTLex::RightAssociate(Entity **HEAD, CompoundEntity *TMP) {
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

ASTLex::~ASTLex() {}

void ASTLex::CLEANUP(Entity *e) {
	if (e != nullptr)
		delete e;
}