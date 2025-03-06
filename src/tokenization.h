#ifndef TOKENIZATION_H
#define TOKENIZATION_H

// Possible types of tokens
enum TokenType{
	_obliterate,
	_int_lit,
	_semicolon,
	_open_paren,
	_close_paren,
	_equal_sign,
	_int_dcl,
	_identifier,
	_add,
	_mult,
	_sub,
	_div,
	_mod,
	_bin_expr // <- Only used in parsing and generation
};

// A single token
struct Token{
	enum TokenType type;
	char* value;
};

// Vector of tokens
struct TokenVector{
	struct Token* arr;
	int size;
};

// Free a token vector and the values inside the tokens if they are not null
void free_token_vector(struct TokenVector vector){
	if(vector.arr!=NULL){
		for(int i=0;i<vector.size;i++) { if(vector.arr[i].value!=NULL) free(vector.arr[i].value); }
		free(vector.arr);
	}
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
			while(isalnum(str[i]) && i<str_len){
				token_end=++i;
			}
			// Allocate and setup the token string buffer
			token_str=(char*)malloc(token_end-token_start+1);
			token_str[token_end-token_start]='\0';
			memcpy(token_str,str+token_start,token_end-token_start);
			
			// Compare all possible tokens
			if(!strcmp(token_str,"obliterate"))
				token=(struct Token){_obliterate,NULL};
			
			else if(!strcmp(token_str,"int"))
				token=(struct Token){_int_dcl,NULL};
			
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
			while(isdigit(str[i]) && i<str_len){
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
			if(isspace(str[i])) { i++; continue; }
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
					token=(struct Token){_equal_sign,NULL};
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
					token=(struct Token){_div,NULL};
					break;
				default: // We have an undefined token, so throw an error
					error("stray symbol %c!",str[i]);
			}
			pushback(tokens,token);
		}
		// In case we didnt find anything, just skip to the next character
		i++;
	}
	// Return the token vector
	return tokens;
}


#endif