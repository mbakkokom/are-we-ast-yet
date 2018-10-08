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

Entity* ASTLex::Parse(string code, char separator) {
	// We kind of skipped the tokenization part of the parser here, TODO
	Entity* HEAD = nullptr;
	CompoundEntity* TMP = new CompoundEntity(TieredEntity::OPERATOR_INVALID);

	Entity *ENT = nullptr;

	string tmp_str;

	for (string::iterator it = code.begin(); it != code.end(); ++it) {
		char c = *it;
		CompoundEntity::OperatorType t;

		if (c == separator) {
			try {
				string s(code.substr(++it - code.begin()));
				Entity *n = Parse(s, separator);
				if (ENT != nullptr) {
					CLEANUP(TMP);
					return new CompoundEntity(TieredEntity::DIRECTIVE_ARGS, ENT, n);
				} if (!tmp_str.empty()) {
					CLEANUP(TMP);
					return new CompoundEntity(TieredEntity::DIRECTIVE_ARGS, Parse(tmp_str), n);
				} else {
					CLEANUP(TMP);
					return n;
				}
				break;
			} catch (const ASTException &ex) {
				CLEANUP(HEAD);
				CLEANUP(TMP);
				CLEANUP(ENT);
				throw ex;
			}
		} if (c == ' ' || c == '\t' || c == '\n') {
			// ignore whitespace
			continue;
		} else if (c == ')') {
			CLEANUP(HEAD);
			CLEANUP(TMP);
			CLEANUP(ENT);
			throw ASTSyntaxError("invalid syntax1");
		} else if (c == '(') {
			if (ENT != nullptr) {
				CLEANUP(HEAD);
				CLEANUP(TMP);
				CLEANUP(ENT);
				throw ASTSyntaxError("invalid syntax(");
			}

			int level = 1;
			size_t begin = ++it -  code.begin();
			bool negative = false, caller = false;

			// negative sign
			if (!tmp_str.empty()) {
				if (tmp_str[0] == '-') {
					negative = true;
					caller = !((tmp_str = tmp_str.substr(1)).empty());
				} else caller = true;
			}

			for (;it != code.end(); ++it) {
				c = *it;

				if (c == '(')
					level++;
				else if (c == ')')
					level--;

				//cout << "level " << level << ": " << c << endl;

				if (level == 0) {
					//cout << "BREAK" << endl;
					break;
				}
 			}

 			//cout << "break" << endl;

			if (level != 0 || ENT != nullptr) {
				CLEANUP(HEAD);
				CLEANUP(TMP);
				CLEANUP(ENT);
				throw ASTSyntaxError("invalid syntax3");
			} else {
				string sc(code.substr(begin, it - code.begin() - begin));
				//cout << "sc " << sc << endl;

				try {
					if (caller) {
						Entity *arg;
						bool parenthesis = false;
						FunctionEntity *cl = new FunctionEntity(tmp_str);
						cl->SetNegative(negative);
						ENT = cl;

						level = 0;

						string tchild;
						for (string::iterator st=sc.begin(); st != sc.end(); ++st) {
							char c = *st;
							if (c == '(') {
								parenthesis = true;
								level++;
							} else if (c == ')') {
								parenthesis = false;
								level--;
							}

							if (level == 0 && !parenthesis && c == ';') {
								//cout << "tchild0[" << level << "] " << tchild << endl;
								arg = Parse(tchild);
								cl->AddArgument(arg);
								tchild.clear();
								//cout << "tchild CLEARED" << endl;
							} else tchild += c;
						}

						//cout << "tchild1[" << level << "] " << tchild << endl;

						arg = Parse(tchild);
						cl->AddArgument(arg);
						tchild.clear();

						/*
						Entity *s = Parse(sc, ';');
						ENT = s;

						if (s == nullptr)
							ENT = cl;
						else
							ENT = new CompoundEntity(TieredEntity::DIRECTIVE_CALL, s, cl);
						*/
					} else {
						ENT = new ParenthesisEntity(Parse(sc), negative);
					}
				} catch (ASTException *ex) {
					//cout << "INNER ERROR" << endl;
					CLEANUP(HEAD);
					CLEANUP(TMP);
					CLEANUP(ENT);

					throw ex;
				}
			}

			tmp_str.clear();
		} else if ((t = TieredEntity::OPERATOR(c)) != TieredEntity::OPERATOR_INVALID) {
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

			if (OP == TieredEntity::OPERATOR_INVALID) {
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
		if (TMP->GetOperator() == CompoundEntity::OPERATOR_INVALID) {
			// single value
			CLEANUP(TMP); // we don't need this anymore.

			if (ENT != nullptr)
				HEAD = ENT;
			else if (OperandEntity::IsValid(tmp_str))
				HEAD = new OperandEntity(tmp_str);
			else if (LiteralEntity::IsValid(tmp_str))
				HEAD = new LiteralEntity(tmp_str);
			else if (!tmp_str.empty())
				throw ASTSyntaxError("invalid syntax4: " + tmp_str);
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
			if (n == nullptr) {
				break;
			} else if (n->GetType() == Entity::COMPOUND_ENTITY) {
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
			if (n == nullptr) {
				break;
			} else if (n->GetType() == Entity::COMPOUND_ENTITY) {
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

// -- MARK: Get postfix representation

string ASTLex::GetPostfix(Entity *e) {
	if (e == nullptr) {
		return "[NULL]";
	} else {
		Entity::EntityType type = e->GetType();
		if (type == Entity::FUNCTION_ENTITY) {
			FunctionEntity *f = (FunctionEntity*) e;
			string r("(func:" + e->GetString());

			if (f->HasArguments()) {
				vector<Entity*> args = f->GetArguments();
				r += ":";
				for (vector<Entity*>::iterator it = args.begin(); it != args.end(); ++it) {
					r += GetPostfix(*it);
					if (it != args.end() - 1) {
						r += ";";
					}
				}
			}

			r += ")";

			return r;
		} else if (type != Entity::COMPOUND_ENTITY) {
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
}

ASTLex::~ASTLex() {}

void ASTLex::CLEANUP(Entity *e) {
	if (e != nullptr)
		delete e;
}
