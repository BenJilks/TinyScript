#pragma once
#include <string>
#include <vector>
#include "Tokenizer.hpp"
#include "Symbol.hpp"
#include "Expression.hpp"
#include "CodeGen.hpp"
using namespace std;

class Function
{
public:
    Function(string name, int location, GlobalScope *global, Tokenizer *tk, Scope *attrs);
    CodeGen OutputCode();
	void AddSelf(SymbolType *type);
	void Compile();
    void CompileBlock();
	void CompileStatement();
    
    inline int Location() const { return location; }
    inline string Name() const { return name; }
	inline bool IsSysCall() const { return is_syscall; }
	inline SymbolType *ReturnType() const { return type; }
    
private:
	SymbolType *ParseConstType();
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
	SymbolType *type;

    CodeGen code;
	Tokenizer *tk;
	Expression expression;
	bool is_syscall;
	Scope scope;

};
