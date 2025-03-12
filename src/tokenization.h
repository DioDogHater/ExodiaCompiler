#ifndef TOKENIZATION_H
#define TOKENIZATION_H

// Possible types of tokens
enum TokenType{
	// Prebuilt functions
	_obliterate,
	_println,
	_printnum,
	_getnum,
	
	// Declarations, literals and keywords
	_var_dcl,
	_int_lit,
	_str_lit,
	_char_lit,
	_identifier,
	_if,
	_else,
	_else_if,
	_while,
	
	// Symbols
	_semicolon,
	_open_paren, _close_paren,
	_open_bracket, _close_bracket,
	_equal_sign,
	_add, _sub,
	_mult, _div,
	_mod,
	_equal_to, _not_equal,
	_greater, _greater_equ,
	_lower, _lower_equ,
	_AND, _OR,
	
	// Types for parsing and generation
	_bin_expr,
	_negation // otherwise, substraction will be confused with negation
};

// A single token
struct Token{
	enum TokenType type;
	char* value;
};

// Vector of tokens
struct TokenVector{
	struct Token* arr;
	size_t size;
};


// Free a token vector and the values inside the tokens if they are not null
void free_token_vector(struct TokenVector vector){
	if(vector.arr!=NULL){
		for(int i=0; i<vector.size; i++) { if(at(vector,i).value != NULL) free(at(vector,i).value); }
		free(vector.arr);
	}
}

// Returns the precedence value of arithmetic operator
int get_binary_prec(struct Token tk){
	switch(tk.type){
		case _add:
			return 0;
		case _sub:
			return 0;
		case _mult:
			return 1;
		case _div:
			return 1;
		case _mod:
			return 1;
		default:
			return -1;
	}
}

// Returns the conditional jump instruction that is the opposite of a condition
char* get_cond_jump_opp(enum TokenType type){
	switch(type){
		case _equal_to:
			return "jne";
		case _not_equal:
			return "je";
		case _greater:
			return "jle";
		case _greater_equ:
			return "jl";
		case _lower:
			return "jge";
		case _lower_equ:
			return "jg";
		default:
			return NULL;
	}
}

char* get_cond_jump(enum TokenType type){
	switch(type){
		case _equal_to:
			return "je";
		case _not_equal:
			return "jne";
		case _greater:
			return "jg";
		case _greater_equ:
			return "jge";
		case _lower:
			return "jl";
		case _lower_equ:
			return "jle";
	}
	return NULL;
}

// Turns str into tokens
struct TokenVector tokenize(char* str){
	// Length of str
	size_t str_len=strlen(str);
	
	// Vector of tokens to be returned
	struct TokenVector tokens=(struct TokenVector){NULL,0};
	
	// Buffer info
	int token_start=0, token_end=0, i=0;
	char* token_str;
	
	// Token prototype to feed to the vector
	struct Token token;
	
	while(i<str_len){
		// Check for alphanumeric tokens
		if(isalpha(str[i])){
			// Start here
			token_start=i;
			// End only when we either find a non-alphanumeric character or we reach the end of str
			do{
				token_end=++i;
			}while(i<str_len && isalnum(str[i]));
			// Allocate and setup the token string buffer
			token_str=(char*)malloc(token_end-token_start+1);
			token_str[token_end-token_start]='\0';
			memcpy(token_str,str+token_start,token_end-token_start);
			
			// Compare all possible tokens
			if(!strcmp(token_str,"obliterate"))
				token=(struct Token){_obliterate,NULL};
			
			else if(!strcmp(token_str,"println"))
				token=(struct Token){_println,NULL};
			
			else if(!strcmp(token_str,"printnum"))
				token=(struct Token){_printnum,NULL};
			
			else if(!strcmp(token_str,"getnum"))
				token=(struct Token){_getnum,NULL};
			
			else if(!strcmp(token_str,"int") || !strcmp(token_str,"char")){
				token=(struct Token){_var_dcl,token_str};
				pushback(tokens,token);
				token_str=NULL;
				continue;
			}
			
			else if(!strcmp(token_str,"if")){
				// check if "if" is after an "else" to see if it should be "else if" instead
				if(at_back(tokens).type == _else){
					at_back(tokens).type=_else_if;
					free(token_str);
					continue;
				}
				token=(struct Token){_if,NULL};
			}
			
			else if(!strcmp(token_str,"else"))
				token=(struct Token){_else,NULL};
			
			else if(!strcmp(token_str,"while"))
				token=(struct Token){_while,NULL};
			
			else if(!strcmp(token_str,"true")){
				token_str[0]='1'; token_str[1]='\0';
				token=(struct Token){_int_lit,token_str};
				pushback(tokens,token);
				token_str=NULL;
				continue;
			}
			
			else if(!strcmp(token_str,"false")){
				token_str[0]='0'; token_str[1]='\0';
				token=(struct Token){_int_lit,token_str};
				pushback(tokens,token);
				token_str=NULL;
				continue;
			}
			
			// In case of other token, we deduce it's an indentifier and we dont free the token_str buffer
			else{
				token=(struct Token){_identifier,token_str};
				pushback(tokens,token);
				token_str=NULL; // Prepare the buffer for the next token
				continue;
			}
			
			// Push the token
			pushback(tokens,token);
			
			// Free buffer and continue to next token
			free(token_str);
			continue;
		}
		// Check for digit composed tokens i.e. integer literals
		else if(isdigit(str[i])){
			// Start here
			token_start=i;
			// End only when we either find a non-alphanumeric character or we reach the end of str
			while(i<str_len && isdigit(str[i])){
				token_end=++i;
			}
			// Allocate the buffer and set it up
			token_str=(char*)malloc(token_end-token_start+1);
			token_str[token_end-token_start]='\0';
			memcpy(token_str,str+token_start,token_end-token_start);
			
			// Setup the token as a integer literal
			token=(struct Token){_int_lit,token_str};
			pushback(tokens,token); // Push it at the back of the vector
			
			// Prepare buffer for next token
			token_str=NULL;
			continue;
		}
		// Ponctuation tokens or special character tokens
		else{
			// If it's whitespace, just ignore it
			if(isspace(str[i]) || !isprint(str[i])) { i++; continue; }
			// Default, if stays, then dont consider token
			token=(struct Token){_obliterate,NULL};
			// Check which token can be used for the character
			switch(str[i]){
				case ';': // Ends the statement
					token=(struct Token){_semicolon,NULL};
					break;
				case '(':
					token=(struct Token){_open_paren,NULL};
					break;
				case ')':
					token=(struct Token){_close_paren,NULL};
					break;
				case '=':
					if(i+1<str_len && str[i+1] == '='){
						token=(struct Token){_equal_to,NULL};
						i++;
					}else
						token=(struct Token){_equal_sign,NULL};
					break;
				case '"':{
					token_start=++i;
					token_end=token_start;
					token_str=(char*)malloc(2048); // Arbitrary value of 2 KB
					token_str[(token_end++)-token_start]='\"';
					while(i<str_len && str[i] != '"'){
						if(str[i] == '\\' && i+1<str_len && str[i+1] == 'n'){
							token_str[(token_end++)-token_start]='"';
							token_str[(token_end++)-token_start]=',';
							token_str[(token_end++)-token_start]='1';
							token_str[(token_end++)-token_start]='0';
							i+=2;
							if(i<str_len && str[i] != '"'){
								token_str[(token_end++)-token_start]=',';
								token_str[(token_end++)-token_start]='"';
							}continue;
						}else if(i+1<str_len && str[i+1] == '"'){
							token_str[(token_end++)-token_start]=str[i++];
							token_str[(token_end++)-token_start]='"';
							break;
						}token_str[(token_end++)-token_start]=str[i++];
					}
					token_str[token_end-token_start]='\0';
					token_str=(char*)realloc(token_str,token_end-token_start+1);
					token=(struct Token){_str_lit,token_str};
					token_str=NULL;
					break;
				}case '\'':
					if(i+1<str_len && i+2<str_len && str[i+2] == '\''){
						token_str=(char*)malloc(2);
						token_str[0]=str[++i]; token_str[1]='\0';
						token=(struct Token){_char_lit,token_str};
						token_str=NULL;
						i++;
					}else
						error("Invalid char literal!");
					break;
				case '+':
					token=(struct Token){_add,NULL};
					break;
				case '*':
					token=(struct Token){_mult,NULL};
					break;
				case '-':
					token=(struct Token){_sub,NULL};
					break;
				case '/':
					if(i+1<str_len && str[i+1] == '/'){
						i+=2; // Skip the "//"
						while(i<str_len && str[i] != '\n') i++; // Skip until end of line
					}else if(i+1<str_len && str[i+1] == '*'){
						i+=2; // Skip the "/*"
						while(i<str_len && i+1<str_len && str[i] != '*' || str[i+1] != '/') i++; // Skip until we reach the * of "*/"
						i++; // Skip the last slash
					}else
						token=(struct Token){_div,NULL};
					break;
				case '%':
					token=(struct Token){_mod,NULL};
					break;
				case '{':
					token=(struct Token){_open_bracket,NULL};
					break;
				case '}':
					token=(struct Token){_close_bracket,NULL};
					break;
				case '>':
					if(i+1<str_len && str[i+1] == '='){
						token=(struct Token){_greater_equ,NULL};
						i++;
					}else
						token=(struct Token){_greater,NULL};
					break;
				case '<':
					if(i+1<str_len && str[i+1] == '='){
						token=(struct Token){_lower_equ,NULL};
						i++;
					}else
						token=(struct Token){_lower,NULL};
					break;
				case '!':
					if(i+1<str_len && str[i+1] == '='){
						token=(struct Token){_not_equal,NULL};
						i++;
					}else
						error("stray symbol '!'!");
					break;
				case '&':
					if(i+1<str_len && str[i+1] == '&'){
						token=(struct Token){_AND,NULL};
						i++;
					}else
						error("stray symbol '&'!");
					break;
				case '|':
					if(i+1<str_len && str[i+1] == '|'){
						token=(struct Token){_OR,NULL};
						i++;
					}else
						error("stray symbol '|'!");
					break;
				default: // We have an undefined token, so throw an error
					error("stray symbol '%c'!",str[i]);
			}
			if(token.type != _obliterate)
				pushback(tokens,token);
		}
		// In case we didnt find anything, just skip to the next character
		i++;
	}
	// Return the token vector
	return tokens;
}


#endif