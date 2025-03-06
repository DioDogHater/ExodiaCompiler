#ifndef PARSER_H
#define PARSER_H


// "Prototype" of the expression node
union NodeExpr;

// Binary expression (math and stuff)
struct NodeBinExpr{
	enum TokenType type; // _bin_expr
	enum TokenType op; // Either _add, _mult, _sub, _div or _mod
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

// Different types of statements
struct NodeStmtExit{
	enum TokenType type;
	union NodeExpr expr;
};
struct NodeStmtIntDcl{
	enum TokenType type;
	struct Token identifier;
	union NodeExpr expr;
};

// Statement node, basically a single line of code
union NodeStmt{
	enum TokenType type;
	struct NodeStmtExit exit;
	struct NodeStmtIntDcl int_dcl;
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


// Arena allocator
struct ArenaAlloc node_alloc=(struct ArenaAlloc){NULL,0,0};

bool parse_expr(union NodeExpr* expr_node);

// Get binary expression
void parse_bin_expr(union NodeExpr lhs, union NodeExpr* expr_node){
	union NodeExpr rhs;
	expr_node->bin_expr=(struct NodeBinExpr){_bin_expr,consume().type,NULL,NULL};
	if(!parse_expr(&rhs)) error("Expected expression");
	expr_node->bin_expr.lhs=(union NodeExpr*)arena_alloc(&node_alloc,sizeof(union NodeExpr));
	*expr_node->bin_expr.lhs=lhs;
	expr_node->bin_expr.rhs=(union NodeExpr*)arena_alloc(&node_alloc,sizeof(union NodeExpr));
	*expr_node->bin_expr.rhs=rhs;
}

// Parse a term (literal or identifier)
bool parse_term_expr(union NodeExpr* expr_node){
	if(get_token(_int_lit)){
		*expr_node=(union NodeExpr){.int_lit=consume()};
		return true;
	}else if(get_token(_identifier)){
		*expr_node=(union NodeExpr){.identifier=consume()};
		return true;
	}return false;
}

// Get expression if available
bool parse_expr(union NodeExpr* expr_node){
	// Get the expression
	if(parse_term_expr(expr_node)){
		// Any binary operator
		if(get_token(_add)){ // TODO -> add other operators
			parse_bin_expr(*expr_node,expr_node);
		}// Otherwise literal or identifier so just return as is
		return true;
	}return false;
}

// Get statement if available
bool parse_statement(union NodeStmt* statement){
	if(get_token(_obliterate)){
		struct NodeStmtExit exit_node; consume();
		if(get_token(_open_paren)) consume();
		else error("Missing '('!");
		if(!parse_expr(&exit_node.expr))
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
		if(!parse_expr(&dcl_node.expr))
			error("Invalid expression!");
		if(get_token(_semicolon)) consume();
		else error("Missing ';'!");
		dcl_node.type=_int_dcl;
		*statement=(union NodeStmt){.int_dcl=dcl_node};
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
		}
	}
}


#endif