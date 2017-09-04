#include "../comut_utility.h"
#include "oeaa.h"

bool OEAA::ValidateDomain(const std::set<std::string> &domain)
{
	set<string> valid_domain{"="};

	for (auto it: domain)
  	if (valid_domain.find(it) == valid_domain.end())
    	// cannot find input domain inside valid domain
      return false;

  return true;
}

bool OEAA::ValidateRange(const std::set<std::string> &range)
{
	set<string> valid_range{"+=", "-=", "*=", "/=", "%="};

	for (auto it: range)
  	if (valid_range.find(it) == valid_range.end())
    	// cannot find input range inside valid range
      return false;

  return true;
}

void OEAA::setDomain(std::set<std::string> &domain)
{
	if (domain.empty())
		domain_ = {"="};
	else
		domain_ = domain;
}

void OEAA::setRange(std::set<std::string> &range)
{
	if (range.empty())
		range_ = {"+=", "-=", "*=", "/=", "%="};
	else
		range_ = range;
}

bool OEAA::CanMutate(clang::Expr *e, ComutContext *context)
{
	if (BinaryOperator *bo = dyn_cast<BinaryOperator>(e))
	{
		string binary_operator{bo->getOpcodeStr()};
		SourceLocation start_loc = bo->getOperatorLoc();
		SourceManager &src_mgr = context->comp_inst->getSourceManager();
		SourceLocation end_loc = src_mgr.translateLineCol(
				src_mgr.getMainFileID(),
				GetLineNumber(src_mgr, start_loc),
				GetColumnNumber(src_mgr, start_loc) + binary_operator.length());

		// Return False if expr is not in mutation range, inside array decl size
		// and inside enum declaration.
		if (!Range1IsPartOfRange2(
				SourceRange(start_loc, end_loc), 
				SourceRange(*(context->userinput->getStartOfMutationRange()),
										*(context->userinput->getEndOfMutationRange()))) ||
				context->is_inside_array_decl_size ||
				context->is_inside_enumdecl ||
				domain_.find(binary_operator) == domain_.end())
			return false;

		// OEAA can only be applied under the following cases
		// 		- left and right side are both scalar
		// 		- only subtraction between same-type pointers is allowed
		// 		- only ptr+int and ptr-int are allowed
		if (CanMutate(bo, context->comp_inst))
			return true;
	}

	return false;
}

bool OEAA::CanMutate(clang::Stmt *s, ComutContext *context)
{
	return false;
}

void OEAA::Mutate(clang::Expr *e, ComutContext *context)
{
	BinaryOperator *bo;
	if (!(bo = dyn_cast<BinaryOperator>(e)))
		return;

	string token{bo->getOpcodeStr()};
	SourceLocation start_loc = bo->getOperatorLoc();
	SourceManager &src_mgr = context->comp_inst->getSourceManager();
	SourceLocation end_loc = src_mgr.translateLineCol(
			src_mgr.getMainFileID(),
			GetLineNumber(src_mgr, start_loc),
			GetColumnNumber(src_mgr, start_loc) + token.length());

	for (auto mutated_token: range_)
	{
		// exact same token -> duplicate mutant
		if (token.compare(mutated_token) == 0)
			continue;

		if (only_minus_ && 
				mutated_token.compare("-=") != 0)
			continue;

		if (only_plus_ && 
				mutated_token.compare("+=") != 0)
			continue;

		if (only_plus_minus_ &&
				mutated_token.compare("-=") != 0 &&
				mutated_token.compare("+=") != 0)
			continue;

		// modulo only takes integral operands
		if (mutated_token.compare("%=") == 0)
		{
			Expr *lhs = bo->getLHS()->IgnoreImpCasts();
			Expr *rhs = bo->getRHS()->IgnoreImpCasts();

			if (!ExprIsIntegral(context->comp_inst, lhs) ||
					!ExprIsIntegral(context->comp_inst, rhs))
					continue;
		}

		GenerateMutantFile(context, start_loc, end_loc, mutated_token);
		WriteMutantInfoToMutantDbFile(context, start_loc, end_loc, 
																	token, mutated_token);
	}
	only_plus_ = false;
	only_minus_ = false;
	only_plus_minus_ = false;
}

void OEAA::Mutate(clang::Stmt *s, ComutContext *context)
{}

bool OEAA::CanMutate(clang::BinaryOperator * const bo, 
										 CompilerInstance *comp_inst)
{
	Expr *lhs = bo->getLHS()->IgnoreImpCasts();
	Expr *rhs = bo->getRHS()->IgnoreImpCasts();


	if (ExprIsScalar(lhs) && ExprIsScalar(rhs))
		return true;

	if (ExprIsIntegral(comp_inst, lhs) && ExprIsPointer(rhs))
	{
		only_plus_ = true;
		return true;
	}

	if (ExprIsPointer(lhs))
		if (ExprIsPointer(rhs))
		{
			string rhs_type_str{getPointerType(rhs->getType())};
			string lhs_type_str{getPointerType(lhs->getType())};

			// OEAA does not mutate this type of expr
			// 		pointer = (void *) pointer
			// only subtraction between same-type ptr is allowed
			if (rhs_type_str.compare("void *") == 0 ||
					rhs_type_str.compare(lhs_type_str) != 0)
				return false;

			only_minus_ = true;
			return true;
		}
		else if (ExprIsScalar(rhs))
		{
			only_plus_minus_ = true;
			return true;
		}

	return false;
}