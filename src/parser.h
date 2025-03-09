#ifndef PARSER_H
#define PARSER_H


// "Prototype" of the expression node
union NodeExpr;

// Binary expression (math and stuff)
struct NodeBinExpr{
	enum TokenType type; // _bin_expr
	enum TokenType op; // Either _add, _mult, _sub, _div, _mod, or _open_paren (rhs is NULL)
	union NodeExpr* lhs;  // Now, these have to be allocated with the arena allocator
	union NodeExpr* rhs; // because it's impossible to just have them both reference each other like that
};

// Regular expression, either int literal, identifier (variable) or binary expression (math)
union NodeExpr{
	enum TokenType type; // See tokenization.h
	struct Token int_lit;//         -> _int_lit
	struct Token identifier;//      -> _identifier
	struct NodeBinExpr bin_expr;//  -> _bin_expr
};

// Statement node prototype
union NodeStmt;

// Different types of statements
struct NodeStmtExit{
	enum TokenType type;// _obliterate
	union NodeExpr expr;
};
struct NodeStmtIntDcl{
	enum TokenType type;// _int_dcl
	struct Token identifier;
	union NodeExpr expr;
};
struct NodeStmtVarAssign{
	enum TokenType type;// _equal_sign
	struct Token identifier;
	union NodeExpr expr;
};
struct NodeStmtScope{ // Scope, which is like a sub-vector of statements nodes
	enum TokenType type;// _open_bracket
	union NodeStmt* arr;
	int size;
};
struct NodeStmtIf{
	enum TokenType type;// _if
	union NodeExpr expr;
	struct NodeStmtScope scope;
};
struct NodeStmtElse{
	enum TokenType type;
	struct NodeStmtScope scope;
};

// Statement node, basically a single line of code
union NodeStmt{
	enum TokenType type;
	struct NodeStmtExit exit;
	struct NodeStmtIntDcl int_dcl;
	struct NodeStmtVarAssign var_assign;
	struct NodeStmtScope scope;
	struct NodeStmtIf if_stmt;
	struct NodeStmtElse else_stmt;
};

// Basically a vector of statement nodes
struct NodeProg{
	union NodeStmt* arr;
	int size;
};


// Global variables and macros for parsing
struct TokenVector tks;
int tks_pos=0;
#define set_parse_tokens(a) ({tks=(a);})
#define peek() (at(tks,tks_pos))
#define peekn(n) (at(tks,tks_pos+(n)))
#define consume() (at(tks,tks_pos++))
#define in_tks() (tks_pos<tks.size)
#define in_ntks(n) (tks_pos+(n)<tks.size)
#define get_token(p) (in_tks() && peek().type==(p))
#define get_ntoken(p,n) (in_ntks((n)) && peekn((n)).type==(p))


// Free the parse tree
void free_parse_tree(struct NodeProg prog_node){
	if(prog_node.arr == NULL) return;
	for(int i=0; i<prog_node.size; i++){
		if(at(prog_node,i).type == _open_bracket){
			free_vector(at(prog_node,i).scope);
		}
	}free_vector(prog_node);
}


// Arena allocator
struct ArenaAlloc node_alloc=(struct ArenaAlloc){NULL,0,0};

// parse_expr() prototype
bool parse_expr(union NodeExpr* expr_node, int min_prec);

// parse_condition() prototype
bool parse_condition(union NodeExpr* expr_node);

// parse_statement() prototype
bool parse_statement(union NodeStmt* statement);

// Parse a scope
struct NodeStmtScope parse_scope(){
	consume();
	struct NodeStmtScope scope_node={_open_bracket,NULL,0};
	while(in_tks() && !get_token(_close_bracket)){
		union NodeStmt sub_statement;
		if(parse_statement(&sub_statement)){
			pushback(scope_node,sub_statement);
		}else{
			printf("invalid token: %d, \"%s\"\n",peek().type,peek().value);
			error("Invalid statement!");
		}
	}if(!in_tks()) error("Missing '}' after start of scope!");
	consume();
	return scope_node;
}

// Parse a term (literal or identifier)
bool parse_term_expr(union NodeExpr* expr_node){
	if(get_token(_int_lit)){ // Positive integer literals
		*expr_node=(union NodeExpr){.int_lit=consume()};
	}else if(get_token(_sub)){ // Negation
		consume();
		union NodeExpr lhs;
		if(!parse_expr(&lhs,0)) error("Expected expression!");
		expr_node->bin_expr=(struct NodeBinExpr){_bin_expr,_negation,NULL,NULL};
		expr_node->bin_expr.lhs=(union NodeExpr*)arena_alloc(&node_alloc,sizeof(union NodeExpr));
		*expr_node->bin_expr.lhs=lhs;
	}else if(get_token(_identifier)){
		*expr_node=(union NodeExpr){.identifier=consume()};
	}else if(get_token(_open_paren)){
		consume();
		union NodeExpr lhs;
		if(!parse_expr(&lhs,0)) error("Expected expression!");
		if(!get_token(_close_paren)) error("Missing ')'!");
		consume();
		expr_node->bin_expr=(struct NodeBinExpr){_bin_expr,_open_paren,NULL,NULL};
		expr_node->bin_expr.lhs=(union NodeExpr*)arena_alloc(&node_alloc,sizeof(union NodeExpr));
		*expr_node->bin_expr.lhs=lhs;
	}else
		return false;
	return true;
}

// Get expression if available
bool parse_expr(union NodeExpr* expr_node, int min_prec){
	// Get the expression
	if(parse_term_expr(expr_node)){
		// While the expression is still getting parsed
		while(true){
			if(!in_tks()) break; // If we reach the end of tokens, we finished parsing the expression
			struct Token op_token=peek();
			int op_prec=get_binary_prec(op_token);
			if(op_prec == -1 || op_prec < min_prec) break; // If the precedence order of the current operator is lower, we wait for next parse
			consume(); // Go to the right hand side, where we will parse again the expression
			union NodeExpr lhs=*expr_node;
			union NodeExpr rhs;
			if(!parse_expr(&rhs,op_prec+1)) error("Expected expression!");
			// Setup the binary expression for next parse
			expr_node->bin_expr=(struct NodeBinExpr){_bin_expr,op_token.type,NULL,NULL};
			expr_node->bin_expr.lhs=(union NodeExpr*)arena_alloc(&node_alloc,sizeof(union NodeExpr));
			*expr_node->bin_expr.lhs=lhs;
			expr_node->bin_expr.rhs=(union NodeExpr*)arena_alloc(&node_alloc,sizeof(union NodeExpr));
			*expr_node->bin_expr.rhs=rhs;
		}
		// We finished parsing the expr, so return true
		return true;
	}return false; // If the first token isnt a term, there is no expression
}

bool parse_comparison(union NodeExpr* expr_node){
	union NodeExpr lhs;
	union NodeExpr rhs;
	// Get the left hand side expression
	if(parse_expr(&lhs,0)){
		if(!in_tks()) return false;
		struct Token op_token=peek();
		if(get_cond_jump(op_token.type) == NULL) { *expr_node=lhs; return true; }
		consume();
		if(!parse_expr(&rhs,0)) error("Expected 2nd expression in comparison!");
		expr_node->bin_expr=(struct NodeBinExpr){_bin_expr,op_token.type,NULL,NULL};
		expr_node->bin_expr.lhs=(union NodeExpr*)arena_alloc(&node_alloc,sizeof(union NodeExpr));
		*expr_node->bin_expr.lhs=lhs;
		expr_node->bin_expr.rhs=(union NodeExpr*)arena_alloc(&node_alloc,sizeof(union NodeExpr));
		*expr_node->bin_expr.rhs=rhs;
		return true;
	}
	return false;
}

// Get condition if available
bool parse_condition(union NodeExpr* expr_node){
	if(parse_comparison(expr_node)){
		if(get_token(_AND) || get_token(_OR)){
			struct Token op_token=consume();
			union NodeExpr lhs=*expr_node;
			union NodeExpr rhs;
			if(!parse_condition(&rhs)) error("Expected comparison!");
			expr_node->bin_expr=(struct NodeBinExpr){_bin_expr,op_token.type,NULL,NULL};
			expr_node->bin_expr.lhs=(union NodeExpr*)arena_alloc(&node_alloc,sizeof(union NodeExpr));
			*expr_node->bin_expr.lhs=lhs;
			expr_node->bin_expr.rhs=(union NodeExpr*)arena_alloc(&node_alloc,sizeof(union NodeExpr));
			*expr_node->bin_expr.rhs=rhs;
		}
		return true;
	}else
		return false;
}

// Get statement if available
bool parse_statement(union NodeStmt* statement){
	if(get_token(_obliterate)){
		struct NodeStmtExit exit_node; consume();
		if(get_token(_open_paren)) consume();
		else error("Missing '('!");
		if(!parse_expr(&exit_node.expr,0))
			error("Invalid expression!");
		if(get_token(_close_paren)) consume();
		else error("Missing ')'!");
		if(get_token(_semicolon)) consume();
		else error("Missing ';'!");
		exit_node.type=_obliterate;
		*statement=(union NodeStmt){.exit=exit_node};
		return true;
	}else if(get_token(_int_dcl)){
		struct NodeStmtIntDcl dcl_node; consume();
		if(get_token(_identifier)) dcl_node.identifier=consume();
		else error("Missing identifier!");
		if(get_token(_equal_sign)) consume();
		else error("Missing '='!");
		if(!parse_expr(&dcl_node.expr,0))
			error("Invalid expression!");
		if(get_token(_semicolon)) consume();
		else error("Missing ';'!");
		dcl_node.type=_int_dcl;
		*statement=(union NodeStmt){.int_dcl=dcl_node};
		return true;
	}else if(get_token(_identifier)){
		struct NodeStmtVarAssign assign_node;
		assign_node.identifier=consume();
		if(get_token(_equal_sign)) consume();
		else error("Identifier without valid statement!");
		if(!parse_expr(&assign_node.expr,0))
			error("Invalid expression!");
		if(get_token(_semicolon)) consume();
		else error("Missing ';'!");
		assign_node.type=_equal_sign;
		*statement=(union NodeStmt){.var_assign=assign_node};
		return true;
	}else if(get_token(_if)){
		struct NodeStmtIf if_stmt;
		if_stmt.type=_if; consume();
		if(get_token(_open_paren)) consume();
		else error("Missing '('!");
		if(!parse_condition(&if_stmt.expr))
			error("Invalid expression!");
		if(get_token(_close_paren)) consume();
		else error("Missing ')'!");
		if_stmt.scope=parse_scope();
		*statement=(union NodeStmt){.if_stmt=if_stmt};
		return true;
	}else if(get_token(_else)){
		struct NodeStmtElse else_stmt;
		else_stmt.type=_else; consume();
		else_stmt.scope=parse_scope();
		*statement=(union NodeStmt){.else_stmt=else_stmt};
		return true;
	}else if(get_token(_open_bracket)){
		*statement=(union NodeStmt){.scope=parse_scope()};
		return true;
	}
	return false;
}

// Parse through all statements
void parse(struct NodeProg* prog){
	arena_init(&node_alloc,1024*1024*4);
	tks_pos=0;
	while(in_tks()){
		union NodeStmt statement;
		if(parse_statement(&statement)){
			pushback(*prog,statement);
		}else{
			printf("invalid token: %d, \"%s\"\n",peek().type,peek().value);
			error("invalid statement!");
		}
	}
}


#endif