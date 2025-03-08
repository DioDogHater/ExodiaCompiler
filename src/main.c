#include "exolib.h"

long int f_size;
char* f_contents;

void load_input_file(char* filepath){
	FILE* fptr=fopen(filepath,"r");
	if(fptr == NULL){ perror("Input file does not exist or cannot be opened!\n"); exit(-1); }
	fseek(fptr,0L,SEEK_END); // Get size
	f_size=ftell(fptr);
	rewind(fptr); // Go back to start of file
	f_contents=(char*)malloc(f_size+1);
	if(f_contents == NULL) { perror("Something went wrong allocating memory!\nExiting compilation...\n"); exit(-1); }
	if(fread(f_contents,1,f_size,fptr) != f_size){
		fprintf(stderr,"Something went wrong while reading data...\nRead should be of expected size of %d bytes!\n",(int)f_size);
		fclose(fptr);
		exit(-1);
	}f_contents[f_size]='\0';
	fclose(fptr);
	fptr=NULL;
}

void write_output_file(char* filepath, char* src_str){
	FILE* fptr=fopen(filepath,"w");
	if(fptr == NULL){ perror("Unable to open output file!\n"); exit(-1); }
	if(fputs(src_str,fptr) == -1){ perror("Unable to write into output file!\n"); exit(-1); }
	fclose(fptr);
}

int main(int argc, char* argv[]){
	if(argc != 2){
		printf("Incorrect command! Syntax is as follows:\nexodia <input.exo>\n");
		return -1;
	}
	
	load_input_file(argv[1]);
	
	// Load source code, tokenize it and store the tokens in a TokenVector
	struct TokenVector tokens=tokenize(f_contents);
	free(f_contents);
	printf("Finished tokenization!\n");
	
	
	printf("\nDEBUG:\n");
	for(int i=0; i<tokens.size; i++){
		printf("token #%d\nid: %d\nvalue: %s\n\n",i,at(tokens,i).type,at(tokens,i).value);
	}
	
	
	// Parse through our tokens and create a parse tree
	set_parse_tokens(tokens);
	struct NodeProg parse_tree=(struct NodeProg){NULL,0};
	parse(&parse_tree);
	printf("Finished parsing!\n");
	
	// Generate our assembly code with the parse tree
	char* asm_str=generate_assembly(parse_tree);
	printf("Finished generation!\n");
	//printf("ASM:\n%s",asm_str);
	write_output_file("out.asm",asm_str);
	free(asm_str);
	
	// Free our tokens
	free_token_vector(tokens);
	
	// Assemble the assembly code
	system("nasm -felf64 out.asm");
	printf("\nASSEMBLED PROGRAM!\n");
	
	// Link the object file
	system("ld out.o -o out");
	printf("LINKED PROGRAM!\n");
	
	printf("\nCOMPILATION SUCCESS!\n\n");
	
	return 0;
}