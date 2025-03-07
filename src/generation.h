#ifndef GENERATION_H
#define GENERATION_H


// Types used to check vars
struct Var{
	char* name;
	size_t stack_loc;
};
struct VarMap{
	struct Var* arr;
	int size;
};
int varmap_contains(struct VarMap m, char* b){
	if(m.size==0) return -1;
    for(int i=0; i<m.size; i++){ if(!strcmp(at(m,i).name,b)) return i; } return -1;
}

// Vector of size_t
struct SizeTVector{
	size_t* arr;
	int size;
};

// Stores the stack's location
struct VarMap var_map=(struct VarMap){NULL,0};
struct SizeTVector scope_vector=(struct SizeTVector){NULL,0};
size_t stack_size=0;
int lbl_count=0;

// Push a register's value onto the stack
void push_reg(struct String* output, char* reg){
	pushback_string(*output,"	push %s\n",reg);
	stack_size++;
}

// Pop the newest value from the stack and store it in reg
void pop_reg(struct String* output, char* reg){
	pushback_string(*output,"	pop %s\n",reg);
	stack_size--;
}

// Start a scope
void begin_scope(){
	pushback(scope_vector,var_map.size);
}

// End a scope
void end_scope(struct String* output){
	int pop_count=var_map.size-at_back(scope_vector);
	if(pop_count != 0){
		pushback_string(*output,"	add rsp, %d\n",pop_count*8);
		for(int i=0; i<pop_count; i++){
			popback(var_map);
		}
	}
	popback(scope_vector);
}

// Create a label
void create_label(struct String* output, int lbl_num){
	pushback_string(*output,"	lbl%d:\n",lbl_num);
}

// Generate assembly for an expression
void gen_expr(struct String* output, union NodeExpr expr){
	if(expr.type == _int_lit){
		pushback_string(*output,"	mov rax, %s\n",expr.int_lit.value);
		push_reg(output,"rax");
	}else if(expr.type == _identifier){
		int var_index=varmap_contains(var_map,expr.identifier.value);
		if(var_index == -1) error("Identifier %s is undeclared!",expr.identifier.value);
		char ptr_str[30]; memset(ptr_str,0,30);
		sprintf(
			ptr_str,"QWORD [rsp+%d]",
			(int)((stack_size-at(var_map,var_index).stack_loc-1)*8)
		);
		push_reg(output,ptr_str);
	}else if(expr.type == _bin_expr){
		gen_expr(output,*expr.bin_expr.lhs);
		if(expr.bin_expr.op != _open_paren && expr.bin_expr.op != _negation)
			gen_expr(output,*expr.bin_expr.rhs);
		switch(expr.bin_expr.op){
			case _add:
				pop_reg(output,"rax");
				pop_reg(output,"rbx");
				pushback_string(*output,"	add rax, rbx\n");
				push_reg(output,"rax");
				break;
			case _sub:
				pop_reg(output,"rbx");
				pop_reg(output,"rax");
				pushback_string(*output,"	sub rax, rbx\n");
				push_reg(output,"rax");
				break;
			case _mult:
				pop_reg(output,"rax");
				pop_reg(output,"rbx");
				pushback_string(*output,"	imul rbx\n");
				push_reg(output,"rax");
				break;
			case _div:
				pop_reg(output,"rbx");
				pop_reg(output,"rax");
				pushback_string(*output,"	idiv rbx\n");
				push_reg(output,"rax");
				break;
			case _mod:
				pop_reg(output,"rbx");
				pop_reg(output,"rax");
				pushback_string(*output,"	idiv rbx\n");
				push_reg(output,"rdx");
				break;
			case _open_paren:
				break;
			case _negation:
				pop_reg(output,"rax");
				pushback_string(*output,"	neg rax\n");
				push_reg(output,"rax");
				break;
			default:
				error("unimplemented binary expression!");
		}
	}else
		error("unknown expression type!");
}

// generate_statement() prototype
void generate_statement(struct String* output, union NodeStmt statement);

// Generate assembly for statement
void generate_statement(struct String* output, union NodeStmt statement){
	switch(statement.type){
		case _obliterate:{ // Exit program
			gen_expr(output,statement.exit.expr);
			pop_reg(output,"rdi");
			pushback_string(*output,"	call _exit\n");
			break;
		}case _int_dcl:{ // Declaration of int variable
			if(varmap_contains(var_map,statement.int_dcl.identifier.value) != -1) error("Identifier %s already declared!",statement.int_dcl.identifier.value);
			size_t var_start=stack_size;
			gen_expr(output,statement.int_dcl.expr);
			pushback(var_map,((struct Var){statement.int_dcl.identifier.value,var_start}));
			break;
		}case _equal_sign:{
			int var_index=varmap_contains(var_map,statement.var_assign.identifier.value);
			if(var_index == -1) error("Identifier %s is not declared!",statement.var_assign.identifier.value);
			gen_expr(output,statement.var_assign.expr);
			pop_reg(output,"rax");
			pushback_string(
				*output,"	mov [rsp+%d], rax\n",
				(int)((stack_size-at(var_map,var_index).stack_loc-1)*8)
			);
			break;
		}case _open_bracket:{
			begin_scope();
			for(int i=0; i<statement.scope.size; i++)
				generate_statement(output,at(statement.scope,i));
			end_scope(output);
			break;
		}case _if:{
			int lbl_num=lbl_count++;
			gen_expr(output,statement.if_stmt.expr);
			pop_reg(output,"rax");
			pushback_string(*output,"	test rax, rax\n");
			pushback_string(*output,"	jz lbl%d\n",lbl_num);
			begin_scope();
			for(int i=0; i<statement.if_stmt.scope.size; i++)
				generate_statement(output,at(statement.if_stmt.scope,i));
			end_scope(output);
			create_label(output,lbl_num);
			break;
		}default:
			error("unimplemented statement type!");
	}
}

// Generate the assembly of our program using the parse tree
char* generate_assembly(struct NodeProg prog){
	// String storing the assembly code
	struct String output=(struct String){NULL,1};
	
	// Reset the stack's location
	stack_size=0;
	
	// Reset the label counter
	lbl_count=0;
	
	// Initialize the assembly program
	pushback_string(output,"section .text\n"); // Start the asembly program's text section (where the code is)
	pushback_string(output,"_exit:\n	mov rax, 60\n	syscall\n"); // Exit label for shorter assembly programs
	pushback_string(output,"global _start\n_start:\n"); // Declare the _start label, which is the entry label
	
	// Go through each parsed statement and generate the according assembly
	for(int i=0; i<prog.size; i++){
		generate_statement(&output,at(prog,i));
	}
	
	// Add the exit code at the end in case no obliterate was used in the program
	pushback_string(output,"	mov rdi, 0\n	call _exit\n");
	
	// Free the variables
	free_vector(var_map);
	free_vector(scope_vector);
	free_parse_tree(prog);
	arena_free(node_alloc);
	
	// Return the char string stored in output
	return output.arr;
}

#endif