#pragma once
#include <string>
#include <vector>
#include "Tokenizer.hpp"
#include "Symbol.hpp"
#include "Expression.hpp"
using namespace std;

class Function
{
public:
	Function(string name, int id) : name(name), location(id), is_syscall(true), type(NULL) {}
    Function(string name, int location, SymbolTable table, Tokenizer *tk);
    vector<char> OutputCode();
	void Compile();
    void CompileBlock();
	void CompileStatement();
    
    inline int Location() const { return location; }
    inline string Name() const { return name; }
	inline bool IsSysCall() const { return is_syscall; }
	inline Class *StaticType() const { return type; }
    
private:
	void AssignConstType(string var);
	void CompileStaticReturn();
	void CompileParams();
	void CompileAssign();
	void CompileReturn();
	void CompileIf();
	void CompileIter(vector<ExpressionPath> path);
	void CompileFor();
	void CompileWhile();

	string name;
	int location;
	Class *type;

    vector<char> code;
	Tokenizer *tk;
	SymbolTable table;
	Expression expression;
	bool is_syscall;

};
