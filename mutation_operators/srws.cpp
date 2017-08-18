#include "../comut_utility.h"
#include "srws.h"

bool SRWS::ValidateDomain(const std::set<std::string> &domain)
{
	return domain.empty();
}

bool SRWS::ValidateRange(const std::set<std::string> &range)
{
	return range.empty();
}

// Return True if the mutant operator can mutate this expression
bool SRWS::CanMutate(clang::Expr *e, ComutContext *context)
{
	if (StringLiteral *sl = dyn_cast<StringLiteral>(e))
	{
		SourceLocation start_loc = sl->getLocStart();
    SourceLocation end_loc = GetEndLocOfStringLiteral(
    		context->comp_inst->getSourceManager(), start_loc);

    // Mutation is applicable if this expression is in mutation range,
    // not inside an enum declaration and not inside field decl range.
    // FieldDecl is a member of a struct or union.
    return Range1IsPartOfRange2(
				SourceRange(start_loc, end_loc), 
				SourceRange(*(context->userinput->getStartOfMutationRange()),
										*(context->userinput->getEndOfMutationRange()))) &&
    			 !context->is_inside_enumdecl &&
    			 !LocationIsInRange(start_loc, *(context->fielddecl_range));
	}

	return false;
}

// Return True if the mutant operator can mutate this statement
bool SRWS::CanMutate(clang::Stmt *s, ComutContext *context)
{
	return false;
}

void SRWS::Mutate(clang::Expr *e, ComutContext *context)
{
	SourceManager &src_mgr = context->comp_inst->getSourceManager();
	SourceLocation start_loc = e->getLocStart();
  SourceLocation end_loc = GetEndLocOfStringLiteral(src_mgr, start_loc);

  Rewriter rewriter;
	rewriter.setSourceMgr(src_mgr, context->comp_inst->getLangOpts());
	string token{rewriter.ConvertToString(e)};

	int first_non_whitespace = GetFirstNonWhitespaceIndex(token);
  int last_non_whitespace = GetLastNonWhitespaceIndex(token);

  // Generate mutant by removing whitespaces
  // only when there is some whitespace in front
  if (first_non_whitespace != 1)
  {
    string mutated_token = "\"" + token.substr(first_non_whitespace);
    
    GenerateMutantFile(context, start_loc, end_loc, mutated_token);
		WriteMutantInfoToMutantDbFile(context, start_loc, end_loc, 
																	token, mutated_token);
  }

  // Generate mutant by removing trailing whitespaces
  // only when there is whitespace in the back
  if (last_non_whitespace < token.length()-2)
  {
    string mutated_token = token.substr(0, last_non_whitespace+1) + "\"";
    GenerateMutantFile(context, start_loc, end_loc, mutated_token);
		WriteMutantInfoToMutantDbFile(context, start_loc, end_loc, 
																	token, mutated_token);

    // Generate mutant by removing whitespaces in the front and back
    if (first_non_whitespace != 1)
    {
      string mutated_token = "\"";
      int str_length = last_non_whitespace - first_non_whitespace + 1;

      mutated_token += token.substr(first_non_whitespace, str_length);
      mutated_token += "\"";

      GenerateMutantFile(context, start_loc, end_loc, mutated_token);
			WriteMutantInfoToMutantDbFile(context, start_loc, end_loc, 
																	token, mutated_token);
    }
  }
}

void SRWS::Mutate(clang::Stmt *s, ComutContext *context)
{}