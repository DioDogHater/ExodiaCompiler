#ifndef GENERATION_H
#define GENERATION_H


// Types of variables
const uint8_t VAR_INT=0x01;
const uint8_t VAR_CHAR=0x02;
#define get_byte_str(v) (byte_size_names[byte_sizes[at(var_map,(v)).type]])
uint8_t get_var_type(char* type){
	if(!strcmp(type,"int"))
		return VAR_INT;
	else if(!strcmp(type,"char"))
		return VAR_CHAR;
	else
		return 0x00;
}

// Types used to check vars
struct Var{
	char* name;
	uint8_t type;
	size_t stack_loc;
};
struct VarMap{
	struct Var* arr;
	size_t size;
};
size_t varmap_contains(struct VarMap m, char* b){
	if(m.size==0) return -1;
    for(size_t i=0; i<m.size; i++){ if(!strcmp(at(m,i).name,b)) return i; } return -1;
}

// Vector of string literals (their index will be decided on compile)
struct StringLiteralVector{
	char** arr;
	size_t size;
};

// Vector of size_t
struct SizeTVector{
	size_t* arr;
	size_t size;
};

// Global variables necessary for code generation
struct VarMap var_map=(struct VarMap){NULL,0};
struct SizeTVector scope_vector=(struct SizeTVector){NULL,0};
struct StringLiteralVector str_literals=(struct StringLiteralVector){NULL,0};
size_t stack_size=0;
size_t lbl_count=0;

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
	stack_size-=pop_count;
}

// Create a label
void create_label(struct String* output, int lbl_num){
	pushback_string(*output,"	lbl%d:\n",lbl_num);
}

// Generate assembly for an expression
void gen_expr(struct String* output, union NodeExpr expr){
	switch(expr.type){
	case _int_lit:
		pushback_string(*output,"	mov rax, %s\n",expr.int_lit.value);
		push_reg(output,"rax");
		break;
	case _str_lit:
		pushback_string(*output,"	mov rax, strlen%lu\n",str_literals.size);
		push_reg(output,"rax");
		pushback_string(*output,"	mov rax, str%lu\n",str_literals.size);
		push_reg(output,"rax");
		pushback(str_literals,expr.str_lit.value);
		break;
	case _char_lit:
		pushback_string(*output,"	mov rax, %d\n",(int)expr.char_lit.value[0]);
		push_reg(output,"rax");
		break;
	case _identifier:
		int var_index=varmap_contains(var_map,expr.identifier.value);
		if(var_index == -1) error("Identifier %s is undeclared!",expr.identifier.value);
		char ptr_str[30]; memset(ptr_str,0,30);
		sprintf(
			ptr_str,"QWORD [rsp+%lu]",
			(stack_size-at(var_map,var_index).stack_loc-1)*8
		);
		push_reg(output,ptr_str);
		break;
	case _bin_expr:
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
				pushback_string(*output,"	xor rdx, rdx\n");
				pushback_string(*output,"	idiv rbx\n");
				push_reg(output,"rax");
				break;
			case _mod:
				pop_reg(output,"rbx");
				pop_reg(output,"rax");
				pushback_string(*output,"	xor rdx, rdx\n");
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
				error("unimplemented binary expression %d!",expr.bin_expr.op);
		}break;
	default:
		error("unknown expression type %d!",expr.type);
	}
}

// gen_condition() prototype
void gen_condition(struct String* output, union NodeExpr expr, size_t lbl_num);

void gen_comparison(struct String* output, union NodeExpr expr, size_t lbl_num, bool check_false){
	if(expr.type != _bin_expr){
		gen_expr(output,expr);
		pop_reg(output,"rax");
		pushback_string(*output,"	test rax, rax\n");
		pushback_string(*output,"	%s lbl%lu\n",check_false?"jz":"jnz",lbl_num);
	}else{
		char* cmp_instr;
		if(check_false) cmp_instr=get_cond_jump_opp(expr.bin_expr.op);
		else cmp_instr=get_cond_jump(expr.bin_expr.op);
		if(cmp_instr == NULL && expr.bin_expr.op != _OR && expr.bin_expr.op != _AND){
			gen_expr(output,expr);
			pop_reg(output,"rax");
			pushback_string(*output,"	test rax, rax\n");
			pushback_string(*output,"	%s lbl%lu\n",check_false?"jz":"jnz",lbl_num);
		}else{
			if(expr.bin_expr.op == _OR || expr.bin_expr.op == _AND)
				gen_condition(output,expr,lbl_num);
			else{
				gen_expr(output,*expr.bin_expr.lhs);
				gen_expr(output,*expr.bin_expr.rhs);
				pop_reg(output,"rax");
				pop_reg(output,"rbx");
				pushback_string(*output,"	cmp rbx, rax\n");
				pushback_string(*output,"	%s lbl%lu\n",cmp_instr,lbl_num);
			}
		}
	}
}

void gen_condition(struct String* output, union NodeExpr expr, size_t lbl_num){
	if(expr.type != _bin_expr) gen_comparison(output,expr,lbl_num,true);
	else{
		if(expr.bin_expr.op == _AND){ // AND = check both
			gen_comparison(output,*expr.bin_expr.lhs,lbl_num,true);
			gen_comparison(output,*expr.bin_expr.rhs,lbl_num,true);
		}else if(expr.bin_expr.op == _OR){
			gen_comparison(output,*expr.bin_expr.lhs,++lbl_count,false);
			gen_comparison(output,*expr.bin_expr.rhs,lbl_num,true);
			create_label(output,lbl_count);
		}else
			gen_comparison(output,expr,lbl_num,true);
	}
}

// generate_statement() prototype
void generate_statement(struct String* output, union NodeStmt statement, union NodeStmt* last, union NodeStmt* next);

#define gen_statement(o,sv,index) ({\
	generate_statement((o), at((sv),(index)),\
		((index)>0) ? &(at((sv),(index)-1)) : NULL,\
		((index)+1<(sv).size) ? &(at((sv),(index)+1)) : NULL\
	);\
})

// Generate assembly for statement
void generate_statement(struct String* output, union NodeStmt statement, union NodeStmt* last, union NodeStmt* next){
	switch(statement.type){
		case _obliterate:{ // Exit program
			gen_expr(output,statement.exit.expr);
			pop_reg(output,"rdi");
			pushback_string(*output,"	call _exit\n");
			break;
		}case _println:{
			gen_expr(output,statement.print.expr);
			pop_reg(output,"rsi");
			pop_reg(output,"rdx");
			pushback_string(*output,"	call _println\n");
			break;
		}case _printnum:{
			gen_expr(output,statement.print.expr);
			pop_reg(output,"rax");
			pushback_string(*output,"	call _printnum\n");
			break;
		}case _getnum:{
			size_t var_index=varmap_contains(var_map,statement.getnum.identifier.value);
			pushback_string(*output,"	mov rsi, num_str\n	mov rdx, 18\n	call _getstring\n	mov byte [num_str-1+rax], 0\n	mov rdi, num_str\n	call _atoi\n");
			if(var_index == -1){ // if the variable doesnt exist, create a new one
				size_t var_start=stack_size;
				push_reg(output,"rax");
				pushback(var_map,((struct Var){statement.getnum.identifier.value,VAR_INT,var_start}));
			}else{ // set the value of the variable passed to the result
				if(at(var_map,var_index).type != VAR_INT) error("getnum() expects an int variable to be passed!");
				pushback_string(
					*output,"	mov [rsp+%lu], rax\n",
					(stack_size-at(var_map,var_index).stack_loc-1)*8
				);
			}
			break;
		}case _var_dcl:{ // Declaration of int variable
			if(varmap_contains(var_map,statement.var_dcl.identifier.value) != -1)
				error("Identifier %s already declared!",statement.var_dcl.identifier.value);
			size_t var_start=stack_size;
			uint8_t var_type=get_var_type(statement.var_dcl.var_type.value);
			if(var_type == 0x00) error("Variable type %s is not defined!",statement.var_dcl.var_type.value);
			gen_expr(output,statement.var_dcl.expr);
			pushback(var_map,((struct Var){statement.var_dcl.identifier.value,var_type,var_start}));
			break;
		}case _equal_sign:{
			size_t var_index=varmap_contains(var_map,statement.var_assign.identifier.value);
			if(var_index == -1) error("Identifier %s is not declared!",statement.var_assign.identifier.value);
			gen_expr(output,statement.var_assign.expr);
			pop_reg(output,"rax");
			pushback_string(
				*output,"	mov [rsp+%lu], rax\n",
				(stack_size-at(var_map,var_index).stack_loc-1)*8
			);
			break;
		}case _open_bracket:{
			begin_scope();
			for(int i=0; i<statement.scope.size; i++)
				gen_statement(output,statement.scope,i);
			end_scope(output);
			break;
		}case _if:{
			size_t lbl_num=++lbl_count;
			gen_condition(output,statement.if_stmt.expr,lbl_num);
			begin_scope();
			for(int i=0; i<statement.if_stmt.scope.size; i++)
				gen_statement(output,statement.if_stmt.scope,i);
			end_scope(output);
			if(next != NULL && next->type == _else){
				pushback_string(*output,"	jmp lbl%lu\n",lbl_count);
			}create_label(output,lbl_num);
			break;
		}case _else:{
			if(last == NULL || last->type != _if) error("Missing \"if\" before \"else\" statement!");
			size_t lbl_num=++lbl_count;
			begin_scope();
			for(int i=0; i<statement.else_stmt.scope.size; i++)
				gen_statement(output,statement.else_stmt.scope,i);
			end_scope(output);
			create_label(output,lbl_num);
			break;
		}case _while:{
			size_t loop_lbl_num=++lbl_count;
			size_t end_lbl_num=++lbl_count;
			create_label(output,loop_lbl_num);
			gen_condition(output,statement.while_stmt.expr,end_lbl_num);
			begin_scope();
			for(int i=0; i<statement.while_stmt.scope.size; i++)
				gen_statement(output,statement.while_stmt.scope,i);
			end_scope(output);
			pushback_string(*output,"	jmp lbl%lu\n",loop_lbl_num);
			create_label(output,end_lbl_num);
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
	pushback_string(output,"_println:\n	mov rax, 1\n	mov rdi, 1\n	syscall\n	ret\n"); // Println label to print strings
	pushback_string(output,
	"_printnum:\n	lea r9, [num_str+18]\n	mov r10, r9\n	mov rbx, 10\n	mov byte [r9], 10\n	sub r9, 1\n	.divbyten:\n"
	"	xor rdx, rdx\n	div rbx\n	add rdx, 0x30\n	test rax,rax\n	jz .lastremainder\n	mov byte [r9], dl\n	sub r9, 1\n	jmp .divbyten\n	.lastremainder:\n"
	"	test dl, dl\n	jz .checkbuffer\n	mov byte [r9], dl\n	sub r9, 1\n	.checkbuffer:\n	cmp r9, r10\n	jne .printbuffer\n	mov byte [r9], '0'\n"
	"	sub r9, 1\n	.printbuffer:\n	add r9, 1\n	sub r10, r9\n	add r10, 1\n	mov rax, 1\n	mov rdi, 1\n	mov rsi, r9\n	mov rdx, r10\n	syscall\n	ret\n"
	);
	pushback_string(output,"_getstring:\n	mov rax, 0\n	mov rdi, 0\n	syscall\n	ret\n");
	pushback_string(output,
	"_atoi:\n	mov rax, 0\n	.convert:\n	movzx rsi, byte [rdi]\n	test rsi, rsi\n	je .done\n	cmp rsi, 48\n	jl .error\n	cmp rsi, 57\n"
	"	jg .error\n	sub rsi, 48\n	imul rax, 10\n	add rax, rsi\n	inc rdi\n	jmp .convert\n	.error:\n	mov rsi, atoi_e\n	mov rdx, atoi_elen\n"
	"	call _println\n	xor rax, rax\n	.done:\n	ret\n"
	);
	pushback_string(output,"global _start\n_start:\n"); // Declare the _start label, which is the entry label
	
	// Go through each parsed statement and generate the according assembly
	for(int i=0; i<prog.size; i++)
		gen_statement(&output,prog,i);
	
	// Add the exit code at the end in case no obliterate was used in the program
	pushback_string(output,"	mov rdi, 0\n	call _exit\n");
	
	// Generate the string literals in the .data section
	pushback_string(output,"section .data\n");
	pushback_string(output,"	atoi_e db \"Input is not an integer! (defaulting to zero)\",10\n	atoi_elen equ $-atoi_e\n");
	for(size_t i=0; i<str_literals.size; i++){
		if(_STRING_APPEND<strlen(at(str_literals,i))+20) _STRING_APPEND=strlen(at(str_literals,i))+20;
		pushback_string(output,"	str%lu db %s\n	strlen%lu equ $-str%lu\n",i,at(str_literals,i),i,i);
	}
	
	// Reserve 19 bytes for printnum's char buffer
	pushback_string(output,"section .bss\n	num_str resb 19\n");
	
	// Free the variables
	free_vector(var_map);
	free_vector(str_literals);
	free_vector(scope_vector);
	free_parse_tree(prog);
	arena_free(node_alloc);
	
	// Return the char string stored in output
	return output.arr;
}

#endif