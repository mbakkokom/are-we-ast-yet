#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "entities/entities.hpp"
#include "exceptions.hpp"
#include "interpreter.hpp"

using namespace std;

// -- MARK: Test suite
void TestSuite(ASTInterpreter *m, istream *fp, bool verbose=false) {
	bool unexpected = true;
	int ok = 0, miss = 0, error = 0;
	string input;
	Entity *tmp;
	int line = 0;

	while (fp->good())
	try {
		double r = NAN, o;
		bool nan;
		string now;
		string output;
		size_t p;

		tmp = nullptr;
		unexpected = true;

		if (fp == &cin)
			cout << (input.empty() ? "> " : "  ");
		else
			line++;

		getline(*fp, now, '\n');
		//getline(fp, output, '\n');

		if (!fp->good() && now.empty()) {
			if (fp == &cin)
				cout << endl;
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

		m->Run(input, &tmp);

		if (!m->IsStackEmpty())
			r = m->PopFromStack();

		if (!output.empty() && output != "ERROR" && output != "IGNORE" && (((nan = isnan(o = stod(output))) && isnan(r)) || (!nan && r == o))) {
			if (verbose)
				cout << "OK  [" << input << "] => ops[" << m->GetPostfix(tmp) << "] (return " << r << ") on line " << line << endl;
			ok++;
		} else if (output != "IGNORE") {
			cout << "MIS [" << input << "] => ops[" << m->GetPostfix(tmp) << "] (return " << r << ") on line " << line << endl;
			miss++;
		} else {
			ok++;
		}

		delete tmp;
		input.clear();
	} catch(const ASTException &ex) {
		//cout << "ERR " << ex.what() << endl;
		if (unexpected) {
			cout << "ERR [" << input << "] => err[" << ex.what() << "] on line " << line << endl; 
			if (tmp != nullptr)
				delete tmp;
			error++;
		} else {
			if (verbose)
				cout << "OK  [" << input << "] => err[" << ex.what() << "] on line " << line << endl; 
			if (tmp != nullptr)
				delete tmp;
			ok++;
		}
		input.clear();
	}

	cout << "COUNT---" << endl;
	cout << "OK: " << ok << endl;
	cout << "MIS: " << miss << endl;
	cout << "ERR: " << error << endl;
}


// -- MARK: REPL

void REPL(ASTInterpreter *m, istream *fp, bool verbose=false) {
	string input;
	int line = 0;

	while (fp->good())
	try {
		Entity *tmp;
		double r;
		string now;

		if (fp == &cin)
			cout << (input.empty() ? "> " : "  ");
		else
			line++;

		getline(*fp, now, '\n');

		if (!fp->good() && now.empty()) {
			if (fp == &cin)
				cout << endl;
			break;
		}

		if (now.find('\\') == now.length() - 1) {
			input += now.substr(0, now.length() - 1);
			continue;
		} else {
			input += now;
		}

		m->Run(input, &tmp);
		input.clear();

		if (verbose) {
			cout << "=> [" << (tmp == nullptr ? "INVALID_ENTITY" : tmp->GetTypeString()) << " " << m->GetPostfix(tmp) << "] " << endl;
		}

		if (!m->IsStackEmpty()) {
			do {
				r = m->PopFromStack();

				if (verbose) {
					cout << "   ";

					if (r == 0 && tmp == nullptr)
						cout << "[NULL]";
					else
						cout << r << endl;
				} else {
					cout << r << endl;
				}
			} while(!m->IsStackEmpty());
		} else if (tmp == nullptr && verbose) {
			cout << "[NULL]" << endl;
		}

		delete tmp;
	} catch(const ASTException &ex) {
		cout << "Error on line " << line << ": " << ex.what() << endl;
		input.clear();

		if (fp != &cin) {
			break;
		}
	}
}

// -- MARK: main

int main(int argc, char **argv) {
	ASTInterpreter m;

	bool verbose = false;
	bool test = false;

	string filename;

	for (int i=1; i<argc; i++) {
		string opt(argv[i]);
		if (!opt.empty()) {
			if(opt[0] == '-') {
				if (opt == "-verbose") {
					verbose = true;
				} else if (opt == "-test") {
					test = true;
				} else if (opt.length() == 1 && i == argc-1) {
					break;
				} else {
					cout << "Invalid option: " << opt << endl;
					return 0;
				}
			} else if (i == argc-1) {
				filename = opt;
			} else {
				cout << "Invalid option: " << opt << endl;
				return 0;
			}
		}
	}

	m.SetVerbose(verbose);

	ifstream src;
	istream *in;

	if (!filename.empty()) {
		src.open(filename);

		if (!src.is_open()) {
			cout << "Cannot open file " << filename << endl;
			return 0;
		}

		in = &src;
	} else {
		in = &cin;
	}

	if (test) {
		TestSuite(&m, in, verbose);
	} else {
		REPL(&m, in, verbose);
	}

	src.close();

	return 0;
}