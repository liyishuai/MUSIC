#ifndef COMUT_OEAA_H_
#define COMUT_OEAA_H_

#include "mutant_operator_template.h"

class OEAA : public MutantOperatorTemplate
{
public:
	OEAA(const std::string name = "OEAA")
		: MutantOperatorTemplate(name), only_plus_minus_(false),
			only_minus_(false), only_plus_(false)
	{}

	virtual bool ValidateDomain(const std::set<std::string> &domain);
	virtual bool ValidateRange(const std::set<std::string> &range);

	virtual void setDomain(std::set<std::string> &domain);
  virtual void setRange(std::set<std::string> &range);

	// Return True if the mutant operator can mutate this expression
	virtual bool CanMutate(clang::Expr *e, ComutContext *context);

	// Return True if the mutant operator can mutate this statement
	virtual bool CanMutate(clang::Stmt *s, ComutContext *context);

	virtual void Mutate(clang::Expr *e, ComutContext *context);
	virtual void Mutate(clang::Stmt *s, ComutContext *context);

private:
	bool only_plus_minus_;
	bool only_minus_;
	bool only_plus_;

	bool CanMutate(clang::BinaryOperator * const bo, 
								 CompilerInstance *comp_inst);
};

#endif	// COMUT_OEAA_H_