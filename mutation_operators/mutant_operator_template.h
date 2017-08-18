#ifndef COMUT_MUTANT_OPERATOR_TEMPLATE_H_
#define COMUT_MUTANT_OPERATOR_TEMPLATE_H_	

#include <string>
#include <set>

#include "clang/AST/Stmt.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ASTContext.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Rewrite/Frontend/Rewriters.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Frontend/Utils.h"

#include "../comut_context.h"

class MutantOperatorTemplate
{
protected:
	std::string name_;
	int num_of_generated_mutants_;
	std::set<string> domain_;
	std::set<string> range_;

public:
	MutantOperatorTemplate(const std::string name) 
		: num_of_generated_mutants_(0), name_(name)
	{}

	virtual bool ValidateDomain(const std::set<std::string> &domain) = 0;
	virtual bool ValidateRange(const std::set<std::string> &range) = 0;

	virtual void setDomain(std::set<std::string> &domain);
  virtual void setRange(std::set<std::string> &range);

	// Return True if the mutant operator can mutate this expression
	virtual bool CanMutate(clang::Expr *e, ComutContext *context) = 0;

	// Return True if the mutant operator can mutate this statement
	virtual bool CanMutate(clang::Stmt *s, ComutContext *context) = 0;

	virtual void Mutate(clang::Expr *e, ComutContext *context) = 0;
	virtual void Mutate(clang::Stmt *s, ComutContext *context) = 0;
	
protected:
	void GenerateMutantFile(ComutContext *context, 
													const clang::SourceLocation &start_loc,
													const clang::SourceLocation &end_loc, 
													const std::string &mutated_token);

	// Return the name of the next mutated code file
  std::string GetNextMutantFilename(const ComutContext *context);

  void WriteMutantInfoToMutantDbFile(
		ComutContext *context, const clang::SourceLocation &start_loc, 
		const clang::SourceLocation &end_loc, const std::string &token, 
		const std::string &mutated_token);
};

#endif	// COMUT_MUTANT_OPERATOR_TEMPLATE_H_