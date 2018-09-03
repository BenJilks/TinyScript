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
    Function(string name, int func_id, GlobalScope *global, Tokenizer *tk, Scope *attrs);
    CodeGen OutputCode();
	void AddSelf(SymbolType *type);
	void Compile();
    void CompileBlock();
	void CompileStatement();
    
    inline string Name() const { return name; }
	inline int Length() const { return scope.Length(); }
	inline int FuncID() const { return func_id; }
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
	int func_id;
	SymbolType *type;

    CodeGen code;
	Tokenizer *tk;
	Expression expression;
	bool is_syscall;
	Scope scope;

};
