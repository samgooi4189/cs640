#include <iostream>
#include <fstream>
#include <string.h>
using namespace std;

class Tokenizer	{
	string source;
	char delim;
	bool is_double_quote;
	bool complete_double_quote;
	bool is_single_quote;
	string::iterator  start_ptr;
	string::iterator end_ptr;
public:
	Tokenizer (string& str, char& d);
	string tokenize();
	string getToken();
};

Tokenizer::Tokenizer(string& str, char& d){
	source = str;
	cout<<source<<endl;
	delim = d;
	cout<<delim<<endl;
	is_double_quote = false;
	complete_double_quote = false;
	is_single_quote = false;
	start_ptr = source.begin();
	end_ptr = source.end();
}

string Tokenizer::tokenize(){
	for(string::iterator it=start_ptr; *it != '\0'; ++it){
		if(start_ptr > end_ptr){
			cout<<"why break?"<<*start_ptr<<*end_ptr<<endl;
			break;
		}

		if(*it == '"'){
			is_double_quote? is_double_quote = false : is_double_quote = true;
			complete_double_quote = true;
		}
		if(*it == '\''){
			is_single_quote = true;
		}

		if(!is_double_quote && (*it == delim || *(it+1) =='\0')){
			cout<<"in tokenize"<<endl;
			string ret_token(start_ptr, it);
			if(start_ptr == it){ret_token =  "null";}
			start_ptr = it+1;
			return ret_token;
		}

	}

	return "";
}

string Tokenizer::getToken(){
	string ret = tokenize();
	if (ret == ""){ return "";}
	
	string token = ret;
	//check for double quote
	if(complete_double_quote){
		token = ret.assign(ret.begin()+1, ret.end()-1);
		complete_double_quote = false;
	}

	//check for single quote
	if(is_single_quote){
		token = "";
		string::iterator pos = ret.begin();
		while( (*pos) !='\0'){
			if( (*pos) == '\''){
				cout<<"we have detected single quote"<<endl;
				token.append("''");
				is_single_quote = false;
			}
			else{
				token.append(1,*pos);
			}
			pos++;
		}
	}

	//check for X
	if(strncmp(ret.c_str(), "X", 1) == 0 || strncmp(ret.c_str(), ",", 1) == 0){
		token = "null";
	}

	return token;
}

int main(){
	string source = "test1,X,test2,,\"test3,test3\",te'st4,\"tes,t5\", ";
	char delim = ',';
	Tokenizer tk(source, delim );
	string token = tk.getToken();
	cout<<"before while loop"<<endl;
	while(token != ""){
		cout<<token<<endl;
		token = tk.getToken();
	}

	return 0;
}
