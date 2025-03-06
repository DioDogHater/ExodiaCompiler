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

// Stores the stack's location
struct VarMap var_map=(struct VarMap){NULL,0};
size_t stack_size=0;

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
		gen_expr(output,*expr.bin_expr.rhs);
		pop_reg(output,"rax");
		pop_reg(output,"rbx");
		pushback_string(*output,"	add rax, rbx\n");
		push_reg(output,"rax");
	}else
		error("unknown expression type!");
}

// Generate assembly for statement
void generate_statement(struct String* output, union NodeStmt statement){
	switch(statement.type){
		case _obliterate: // Exit program
			gen_expr(output,statement.exit.expr);
			pushback_string(*output,"	mov rax, 60\n");
			pop_reg(output,"rdi");
			pushback_string(*output,"	syscall\n");
			break;
		case _int_dcl: // Declaration of int variable
			if(varmap_contains(var_map,statement.int_dcl.identifier.value) != -1) error("Identifier %s already declared!",statement.int_dcl.identifier.value);
			size_t var_start=stack_size;
			gen_expr(output,statement.int_dcl.expr);
			pushback(var_map,((struct Var){statement.int_dcl.identifier.value,var_start}));
			break;
	}
}

// Generate the assembly of our program using the parse tree
char* generate_assembly(struct NodeProg prog){
	// String storing the assembly code
	struct String output=(struct String){NULL,1};
	
	// Reset the stack's location
	stack_size=0;
	
	// Initialize the assembly program
	pushback_string(output,"global _start\n_start:\n");
	
	// Go through each parsed statement and generate the according assembly
	for(int i=0; i<prog.size; i++){
		generate_statement(&output,at(prog,i));
	}
	
	// Add the exit code at the end in case no obliterate was used in the program
	pushback_string(output,"	mov rax, 60\n	mov rdi, 0\n	syscall\n");
	
	// Free the variables
	free_vector(var_map);
	arena_free(node_alloc);
	
	// Return the char string stored in output
	return output.arr;
}

#endif