#include <iostream>
#include <fstream>
#include <sqlite3.h>
#include <string.h>
#include "load.h"
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
	//cout<<source<<endl;
	delim = d;
	//cout<<delim<<endl;
	is_double_quote = false;
	complete_double_quote = false;
	is_single_quote = false;
	start_ptr = source.begin();
	end_ptr = source.end();
}

string Tokenizer::tokenize(){
	for(string::iterator it=start_ptr; *it != '\0'; ++it){
		if(start_ptr > end_ptr){
			//cout<<"why break?"<<*start_ptr<<*end_ptr<<endl;
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
			//cout<<"in tokenize"<<endl;
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
				//cout<<"we have detected single quote"<<endl;
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

int main(int argc, const char* argv[]){
	int rc;
	sqlite3 *db;
	sqlite3_open("census.db", &db);
	char* error;
	string line;
	char x_null[] = "X";
	char delim = ',';

	//enable foreign keys constraints
	sqlite3_exec(db, "PRAGMA Foreign_Keys=ON", NULL, NULL, &error);

	//test run with sample table
	rc = sqlite3_exec(db, "DROP TABLE IF EXISTS Test; CREATE TABLE Test(num INTEGER, testStr VARCHAR(200))", NULL, NULL, &error);
	if(rc){
		cerr<<"Error creating table"<<endl;
	//	sqlite3_close(db);
	}
	else{
		cout<<"Table created!"<<endl;
		sqlite3_exec(db, "INSERT INTO Test VALUES(1, 'newloc')", NULL, NULL, &error);
	}
	
	//create table for SUMLEV
	rc = sqlite3_exec(db, createTable_Sumlev, NULL, NULL, &error);
	if(rc){
		cerr<<"**Error creating SUMLEV table!"<<endl<<error<<endl<<endl;
	}
	else{
		cout<< "Table SumLev created!"<<endl;
		ifstream in_file("Sumlev.txt");
		sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &error);
		int linecount =0;
		string stream;
		while (getline(in_file, line)){
			if(linecount == 0){
				linecount++;
				continue;
			}
			stream.append("INSERT INTO SUMLEV VALUES(");
			int charcount=0;
			string new_token = "";
			Tokenizer tk(line, delim);
			string token = tk.getToken();
			while(token != ""){
				if(charcount == 1 && token != "null"){
					new_token.append("'");
					new_token.append(token);
					new_token.append("'");
				}
				else
					stream.append(token);
					
				stream.append(new_token);
				token = tk.getToken();
				charcount++;
				if(token != ""){ stream.append(","); }
				new_token = "";
			}		
			/*char* target_line = (char*)line.c_str();
			char* token = strtok(target_line, ",");
			int charcount=0;
			string new_token = "";
			while(token != NULL){
				if(strcmp(token, x_null) != 0){
					if(charcount == 1){
						new_token.append("'");
						new_token.append(token);
						new_token.append("'");
					}
					else{
						new_token.append(token);
					}
					stream.append(new_token.c_str());
				}
				else{
					stream.append("null");
				}
				token = strtok(NULL, ",");
				charcount++;
				if(token != NULL){
					stream.append(",");
				}
				new_token = "";
			}*/
			stream.append(")");
			rc = sqlite3_exec(db, stream.c_str(), NULL, NULL, &error);
			if(rc){
				cerr<<error<<endl;
			}
			stream = "";
			linecount++;
		}
		sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, &error);

	}

	//create table for SEX
	rc = sqlite3_exec(db, createTable_Sex, NULL, NULL, &error);
	if(rc){
		cerr<<"**Error creating SEX table!"<<endl<<error<<endl<<endl;
	}
	else{
		cout<< "Table Sex created!"<<endl;
		ifstream in_file("Sex.txt");
		sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &error);
		int linecount =0;
		string stream;
		while (getline(in_file, line)){
			if(linecount == 0){
				linecount++;
				continue;
			}
			stream.append("INSERT INTO SEX VALUES(");
			char* target_line = (char*)line.c_str();
			char* token = strtok(target_line, ",");
			int charcount=0;
			string new_token = "";
			while(token != NULL){
				if(strcmp(token, x_null) != 0){
					if(charcount == 1){
						new_token.append("'");
						new_token.append(token);
						new_token.append("'");
					}
					else{
						new_token.append(token);
					}
					stream.append(new_token.c_str());
				}
				else{
					stream.append("null");
				}
				token = strtok(NULL, ",");
				charcount++;
				if(token != NULL){
					stream.append(",");
				}
				new_token = "";
			}
			stream.append(")");
			rc = sqlite3_exec(db, stream.c_str(), NULL, NULL, &error);
			if(rc){
				cerr<<error<<endl;
			}
			stream = "";
			linecount++;
		}
		sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, &error);

	}

	//create table for RACE
	rc = sqlite3_exec(db, createTable_Race, NULL, NULL, &error);
	if(rc){
		cerr<<"**Error creating RACE table!"<<endl<<error<<endl<<endl;
	}
	else{
		cout<< "Table Race created!"<<endl;
		ifstream in_file("Race.txt");
		sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &error);
		int linecount =0;
		string stream;
		while (getline(in_file, line)){
			if(linecount == 0){
				linecount++;
				continue;
			}
			stream.append("INSERT INTO RACE VALUES(");
			char* target_line = (char*)line.c_str();
			char* token = strtok(target_line, ",");
			int charcount=0;
			string new_token = "";
			while(token != NULL){
				if(strcmp(token, x_null) != 0){
					if(charcount == 1){
						new_token.append("'");
						new_token.append(token);
						new_token.append("'");
					}
					else{
						new_token.append(token);
					}
					stream.append(new_token.c_str());
				}
				else{
					stream.append("null");
				}
				token = strtok(NULL, ",");
				charcount++;
				if(token != NULL){
					stream.append(",");
				}
				new_token = "";
			}
			stream.append(")");
			rc = sqlite3_exec(db, stream.c_str(), NULL, NULL, &error);
			if(rc){
				cerr<<error<<endl;
			}
			stream = "";
			linecount++;
		}
		sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, &error);

	}

	//create table for REGION
	rc = sqlite3_exec(db, createTable_Region, NULL, NULL, &error);
	if(rc){
		cerr<<"**Error creating REGION table!"<<endl<<error<<endl<<endl;
	}
	else{
		cout<< "Table Region created!"<<endl;
		ifstream in_file("Region.txt");
		sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &error);
		int linecount =0;
		string stream;
		while (getline(in_file, line)){
			if(linecount == 0){
				linecount++;
				continue;
			}
			stream.append("INSERT INTO REGION VALUES(");
			int charcount=0;
			string new_token = "";
			Tokenizer tk(line, delim);
			string token = tk.getToken();
			while(token != ""){
				if(charcount == 1 && token != "null"){
					new_token.append("'");
					new_token.append(token);
					new_token.append("'");
				}
				else
					stream.append(token);
					
				stream.append(new_token);
				token = tk.getToken();
				charcount++;
				if(token != ""){ stream.append(","); }
				new_token = "";
			}
			/*char* target_line = (char*)line.c_str();
			char* token = strtok(target_line, ",");
			int charcount=0;
			string new_token = "";
			while(token != NULL){
				if(strcmp(token, x_null) != 0){
					if(charcount == 1){
						new_token.append("'");
						new_token.append(token);
						new_token.append("'");
					}
					else{
						new_token.append(token);
					}
					stream.append(new_token.c_str());
				}
				else{
					stream.append("null");
				}
				token = strtok(NULL, ",");
				charcount++;
				if(token != NULL){
					stream.append(",");
				}
				new_token = "";
			}*/
			stream.append(")");
			rc = sqlite3_exec(db, stream.c_str(), NULL, NULL, &error);
			if(rc){
				cerr<<error<<endl;
			}
			stream = "";
			linecount++;
		}
		sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, &error);

	}

	//create table for Division
	rc = sqlite3_exec(db, createTable_Division, NULL, NULL, &error);
	if(rc){
		cerr<<"**Error creating DIVISION table!"<<endl<<error<<endl<<endl;
	}
	else{
		cout<< "Table Division created!"<<endl;
		ifstream in_file("Division.txt");
		sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &error);
		int linecount =0;
		string stream;
		while (getline(in_file, line)){
			if(linecount == 0){
				linecount++;
				continue;
			}
			stream.append("INSERT INTO DIVISION VALUES(");
			char* target_line = (char*)line.c_str();
			char* token = strtok(target_line, ",");
			int charcount=0;
			string new_token = "";
			while(token != NULL){
				if(strcmp(token, x_null) != 0){
					if(charcount == 1){
						new_token.append("'");
						new_token.append(token);
						new_token.append("'");
					}
					else{
						new_token.append(token);
					}
					stream.append(new_token.c_str());
				}
				else{
					stream.append("null");
				}
				token = strtok(NULL, ",");
				charcount++;
				if(token != NULL){
					stream.append(",");
				}
				new_token = "";
			}
			stream.append(")");
			rc = sqlite3_exec(db, stream.c_str(), NULL, NULL, &error);
			if(rc){
				cerr<<error<<endl;
			}
			stream = "";
			linecount++;
		}
		sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, &error);

	}
	
	//create table for ORIGIN
	rc = sqlite3_exec(db, createTable_Origin, NULL, NULL, &error);
	if(rc){
		cerr<<"**Error creating ORIGIN table!"<<endl<<error<<endl<<endl;
	}
	else{
		cout<< "Table Origin created!"<<endl;
		ifstream in_file("Origin.txt");
		sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &error);
		int linecount =0;
		string stream;
		while (getline(in_file, line)){
			if(linecount == 0){
				linecount++;
				continue;
			}
			stream.append("INSERT INTO ORIGIN VALUES(");
			char* target_line = (char*)line.c_str();
			char* token = strtok(target_line, ",");
			int charcount=0;
			string new_token = "";
			while(token != NULL){
				if(strcmp(token, x_null) != 0){
					if(charcount == 1){
						new_token.append("'");
						new_token.append(token);
						new_token.append("'");
					}
					else{
						new_token.append(token);
					}
					stream.append(new_token.c_str());
				}
				else{
					stream.append("null");
				}
				token = strtok(NULL, ",");
				charcount++;
				if(token != NULL){
					stream.append(",");
				}
				new_token = "";
			}
			stream.append(")");
			rc = sqlite3_exec(db, stream.c_str(), NULL, NULL, &error);
			if(rc){
				cerr<<error<<endl;
			}
			stream = "";
			linecount++;
		}
		sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, &error);

	}

	//create table for Housing Unit State Level
	rc = sqlite3_exec(db, createTable_HuUnitStateLevel, NULL, NULL, &error);
	if(rc){
		cerr<<"**Error creating HU_UNIT_STATE_LEVEL table!"<<endl<<error<<endl<<endl;
	}
	else{
		cout<< "Table HuUnitStateLevel created!"<<endl;
		ifstream in_file("Housing_Units_State_Level.txt");
		sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &error);
		int linecount =0;
		string stream;
		while (getline(in_file, line)){
			if(linecount == 0){
				linecount++;
				continue;
			}
			stream.append("INSERT INTO HU_UNIT_STATE_LEVEL VALUES(");
			int charcount=0;
			string new_token = "";
			Tokenizer tk(line, delim);
			string token = tk.getToken();
			while(token != ""){
				if(charcount == 4 && token != "null"){
					new_token.append("'");
					new_token.append(token);
					new_token.append("'");
				}
				else
					stream.append(token);

				stream.append(new_token);
				token = tk.getToken();
				charcount++;
				if(token != ""){ stream.append(","); }
				new_token = "";
			}
			/*char* target_line = (char*)line.c_str();
			char* token = strtok(target_line, ",");
			int charcount=0;
			string new_token = "";
			while(token != NULL){
				if(strcmp(token, x_null) != 0){
					if(charcount == 4){
						new_token.append("'");
						new_token.append(token);
						new_token.append("'");
					}
					else{
						new_token.append(token);
					}
					stream.append(new_token.c_str());
				}
				else{
					stream.append("null");
				}
				token = strtok(NULL, ",");
				charcount++;
				if(token != NULL){
					stream.append(",");
				}
				new_token = "";
			}*/
			stream.append(")");
			rc = sqlite3_exec(db, stream.c_str(), NULL, NULL, &error);
			if(rc){
				cerr<<error<<endl;
			}
			stream = "";
			linecount++;
		}
		sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, &error);

	}


	//create table for Population Estimate Metro Micro
	rc = sqlite3_exec(db, createTable_PopEstMetroMicro, NULL, NULL, &error);
	if(rc){
		cerr<<"**Error creating POP_EST_METRI_MICRO table!"<<endl<<error<<endl<<endl;
	}
	else{
		cout<< "Table PopEstMetroMicro created!"<<endl;
		ifstream in_file("Pop_Estimate_Metro_Micro.txt");
		sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &error);
		int linecount =0;
		string stream;
		while (getline(in_file, line)){
			if(linecount == 0){
				linecount++;
				continue;
			}
			stream.append("INSERT INTO POP_EST_METRO_MICRO VALUES(");
			int charcount=0;
			string new_token = "";
			Tokenizer tk(line, delim);
			string token = tk.getToken();
			while(token != ""){
				if((charcount == 3 || charcount == 4) && token != "null"){
					new_token.append("'");
					new_token.append(token);
					new_token.append("'");
				}
				else
					stream.append(token);

				stream.append(new_token);
				token = tk.getToken();
				charcount++;
				if(token != ""){ stream.append(","); }
				new_token = "";
			}
			/*char* target_line = (char*)line.c_str();
			char* token = strtok(target_line, ",");
			int charcount=0;
			string new_token = "";
			while(token != NULL){
				if(strcmp(token, x_null) != 0 || strlen(token) < 1){
					if(charcount == 3 || charcount == 4){
						new_token.append("'");
						new_token.append(token);
						new_token.append("'");
					}
					else{
						new_token.append(token);
					}
					stream.append(new_token.c_str());
				}
				else{
					stream.append("null");
				}
				token = strtok(NULL, ",");
				charcount++;
				if(token != NULL){
					stream.append(",");
				}
				new_token = "";
			}*/
			stream.append(")");
			rc = sqlite3_exec(db, stream.c_str(), NULL, NULL, &error);
			if(rc){
				cerr<<error<<endl;
			}
			stream = "";
			linecount++;
		}
		sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, &error);
	}


	//create table for Population Estimate PR Municipio Sex Age
	rc = sqlite3_exec(db, createTable_PopEstPrMunSexAge, NULL, NULL, &error);
	if(rc){
		cerr<<"**Error creating POP_EST_MUN_SEX_AGE table!"<<endl<<error<<endl<<endl;
	}
	else{
		cout<< "Table PopEstPrMunSexAge created!"<<endl;
		ifstream in_file("Pop_Estimate_PR_Mun_Sex_Age.txt");
		sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &error);
		int linecount =0;
		string stream;
		while (getline(in_file, line)){
			if(linecount == 0){
				linecount++;
				continue;
			}
			stream.append("INSERT INTO POP_EST_PR_MUN_SEX_AGE VALUES(");
			int charcount=0;
			string new_token = "";
			Tokenizer tk(line, delim);
			string token = tk.getToken();
			while(token != ""){
				if(charcount == 2 && token != "null"){
					new_token.append("'");
					new_token.append(token);
					new_token.append("'");
				}
				else
					stream.append(token);
					
				stream.append(new_token);
				token = tk.getToken();
				charcount++;
				if(token != ""){ stream.append(","); }
				new_token = "";
			}
			/*char* target_line = (char*)line.c_str();
			char* token = strtok(target_line, ",");
			int charcount=0;
			string new_token = "";
			while(token != NULL){
				if(strcmp(token, x_null) != 0){
					if(charcount == 2){
						new_token.append("'");
						new_token.append(token);
						new_token.append("'");
					}
					else{
						new_token.append(token);
					}
					stream.append(new_token.c_str());
				}
				else{
					stream.append("null");
				}
				token = strtok(NULL, ",");
				charcount++;
				if(token != NULL){
					stream.append(",");
				}
				new_token = "";
			}*/
			stream.append(")");
			rc = sqlite3_exec(db, stream.c_str(), NULL, NULL, &error);
			if(rc){
				cerr<<error<<endl;
			}
			stream = "";
			linecount++;
		}
		sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, &error);
	}


	//create table for Population Estimate PR Municipio
	rc = sqlite3_exec(db, createTable_PopEstPrMunicipios, NULL, NULL, &error);
	if(rc){
		cerr<<"**Error creating POP_EST_PR_MUNICIPIOS table!"<<endl<<error<<endl<<endl;
	}
	else{
		cout<< "Table PopEstPrMunicipios created!"<<endl;
		ifstream in_file("Pop_Estimate_PR_Municipios.txt");
		sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &error);
		int linecount =0;
		string stream;
		while (getline(in_file, line)){
			if(linecount == 0){
				linecount++;
				continue;
			}
			stream.append("INSERT INTO POP_EST_PR_MUNICIPIOS VALUES(");
			int charcount=0;
			string new_token = "";
			Tokenizer tk(line, delim);
			string token = tk.getToken();
			while(token != ""){
				if(charcount == 2 && token != "null"){
					new_token.append("'");
					new_token.append(token);
					new_token.append("'");
				}
				else
					stream.append(token);

				stream.append(new_token);
				token = tk.getToken();
				charcount++;
				if(token != ""){ stream.append(","); }
				new_token = "";
			}
			/*char* target_line = (char*)line.c_str();
			char* token = strtok(target_line, ",");
			int charcount=0;
			string new_token = "";
			while(token != NULL){
				if(strcmp(token, x_null) != 0){
					if(charcount == 2){
						new_token.append("'");
						new_token.append(token);
						new_token.append("'");
					}
					else{
						new_token.append(token);
					}
					stream.append(new_token.c_str());
				}
				else{
					stream.append("null");
				}
				token = strtok(NULL, ",");
				charcount++;
				if(token != NULL){
					stream.append(",");
				}
				new_token = "";
			}*/
			stream.append(")");
			rc = sqlite3_exec(db, stream.c_str(), NULL, NULL, &error);
			if(rc){
				cerr<<error<<endl;
			}
			stream = "";
			linecount++;
		}
		sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, &error);
	}


	//create table for Population Estimate PR Sex Age
	rc = sqlite3_exec(db, createTable_PopEstPrSexAge, NULL, NULL, &error);
	if(rc){
		cerr<<"**Error creating POP_EST_PR_SEX_AGE table!"<<endl<<error<<endl<<endl;
	}
	else{
		cout<< "Table PopEstPrSexAge created!"<<endl;
		ifstream in_file("Pop_Estimate_PR_Sex_Age.txt");
		sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &error);
		int linecount =0;
		string stream;
		while (getline(in_file, line)){
			if(linecount == 0){
				linecount++;
				continue;
			}
			stream.append("INSERT INTO POP_EST_PR_SEX_AGE VALUES(");
			int charcount=0;
			string new_token = "";
			Tokenizer tk(line, delim);
			string token = tk.getToken();
			while(token != ""){
				if(charcount == 2 && token != "null"){
					new_token.append("'");
					new_token.append(token);
					new_token.append("'");
				}
				else
					stream.append(token);

				stream.append(new_token);
				token = tk.getToken();
				charcount++;
				if(token != ""){ stream.append(","); }
				new_token = "";
			}
			/*char* target_line = (char*)line.c_str();
			char* token = strtok(target_line, ",");
			int charcount=0;
			string new_token = "";
			while(token != NULL){
				if(strcmp(token, x_null) != 0){
					if(charcount == 2){
						new_token.append("'");
						new_token.append(token);
						new_token.append("'");
					}
					else{
						new_token.append(token);
					}
					stream.append(new_token.c_str());
				}
				else{
					stream.append("null");
				}
				token = strtok(NULL, ",");
				charcount++;
				if(token != NULL){
					stream.append(",");
				}
				new_token = "";
			}*/
			stream.append(")");
			rc = sqlite3_exec(db, stream.c_str(), NULL, NULL, &error);
			if(rc){
				cerr<<error<<endl;
			}
			stream = "";
			linecount++;
		}
		sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, &error);
	}


	//create table for Population Estimate State Age Sex Race Origin
	rc = sqlite3_exec(db, createTable_PopEstStateAgeSexRaceOrigin, NULL, NULL, &error);
	if(rc){
		cerr<<"**Error creating POP_EST_STATE_AGE_SEX_RACE_ORIGIN table!"<<endl<<error<<endl<<endl;
	}
	else{
		cout<< "Table PopEstStateAgeSexRaceOrigin created!"<<endl;
		ifstream in_file("Pop_Estimate_State_Age_Sex_Race_Origin.txt");
		sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &error);
		int linecount =0;
		string stream;
		while (getline(in_file, line)){
			if(linecount == 0){
				linecount++;
				continue;
			}
			stream.append("INSERT INTO POP_EST_STATE_AGE_SEX_RACE_ORIGIN VALUES(");
			int charcount=0;
			string new_token = "";
			Tokenizer tk(line, delim);
			string token = tk.getToken();
			while(token != ""){
				stream.append(token);
				token = tk.getToken();
				charcount++;
				if(token != ""){ stream.append(","); }
				new_token = "";
			}
			/*char* target_line = (char*)line.c_str();
			char* token = strtok(target_line, ",");
			int charcount=0;
			string new_token = "";
			while(token != NULL){
				if(strcmp(token, x_null) != 0){
					new_token.append(token);
					stream.append(new_token.c_str());
				}
				else{
					stream.append("null");
				}
				token = strtok(NULL, ",");
				charcount++;
				if(token != NULL){
					stream.append(",");
				}
				new_token = "";
			}*/
			stream.append(")");
			rc = sqlite3_exec(db, stream.c_str(), NULL, NULL, &error);
			if(rc){
				cerr<<error<<endl;
			}
			stream = "";
			linecount++;
		}
		sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, &error);
	}


	//create table for Population Estimate Cities Towns
	rc = sqlite3_exec(db, createTable_PopEstCitiesTowns, NULL, NULL, &error);
	if(rc){
		cerr<<"**Error creating POP_EST_CITIES_TOWNS table!"<<endl<<error<<endl<<endl;
	}
	else{
		cout<< "Table PopEstCitiesTowns created!"<<endl;
		ifstream in_file("Pop_Estimate_Cities_Towns.txt");
		sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &error);
		int linecount =0;
		string stream;
		while (getline(in_file, line)){
			if(linecount == 0){
				linecount++;
				continue;
			}
			stream.append("INSERT INTO POP_EST_CITIES_TOWNS VALUES(");
			int charcount=0;
			string new_token = "";
			Tokenizer tk(line, delim);
			string token = tk.getToken();
			while(token != ""){
				if((charcount == 6 || charcount == 7) && token != "null"){
					new_token.append("'");
					new_token.append(token);
					new_token.append("'");
				}
				else
					stream.append(token);

				stream.append(new_token);
				token = tk.getToken();
				charcount++;
				if(token != ""){ stream.append(","); }
				new_token = "";
			}
			/*char* target_line = (char*)line.c_str();
			char* token = strtok(target_line, ",");
			int charcount=0;
			string new_token = "";
			while(token != NULL){
				if(strcmp(token, x_null) != 0){
					if(charcount == 6 || charcount == 7){
						new_token.append("'");
						new_token.append(token);
						new_token.append("'");
					}
					else{
						new_token.append(token);
					}
					stream.append(new_token.c_str());
				}
				else{
					stream.append("null");
				}
				token = strtok(NULL, ",");
				charcount++;
				if(token != NULL){
					stream.append(",");
				}
				new_token = "";
			}*/
			stream.append(")");
			rc = sqlite3_exec(db, stream.c_str(), NULL, NULL, &error);
			if(rc){
				cerr<<error<<endl;
			}
			stream = "";
			linecount++;
		}
		sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, &error);
	}


	//create table for Population Estimate State County
	rc = sqlite3_exec(db, createTable_PopEstStateCounty, NULL, NULL, &error);
	if(rc){
		cerr<<"**Error creating POP_EST_STATE_COUNTY table!"<<endl<<error<<endl<<endl;
	}
	else{
		cout<< "Table PopEstStateCounty created!"<<endl;
		ifstream in_file("Pop_Estimate_State_County.txt");
		sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &error);
		int linecount =0;
		string stream;
		while (getline(in_file, line)){
			if(linecount == 0){
				linecount++;
				continue;
			}
			stream.append("INSERT INTO POP_EST_STATE_COUNTY VALUES(");
			int charcount=0;
			string new_token = "";
			Tokenizer tk(line, delim);
			string token = tk.getToken();
			while(token != ""){
				if((charcount == 5 || charcount == 6) && token != "null"){
					new_token.append("'");
					new_token.append(token);
					new_token.append("'");
				}
				else
					stream.append(token);

				stream.append(new_token);
				token = tk.getToken();
				charcount++;
				if(token != ""){ stream.append(","); }
				new_token = "";
			}
			/*char* target_line = (char*)line.c_str();
			char* token = strtok(target_line, ",");
			int charcount=0;
			string new_token = "";
			while(token != NULL){
				if(strcmp(token, x_null) != 0){
					if(charcount == 5 || charcount == 6){
						new_token.append("'");
						new_token.append(token);
						new_token.append("'");
					}
					else{
						new_token.append(token);
					}
					stream.append(new_token.c_str());
				}
				else{
					stream.append("null");
				}
				token = strtok(NULL, ",");
				charcount++;
				if(token != NULL){
					stream.append(",");
				}
				new_token = "";
			}*/
			stream.append(")");
			rc = sqlite3_exec(db, stream.c_str(), NULL, NULL, &error);
			if(rc){
				cerr<<error<<endl;
			}
			stream = "";
			linecount++;
		}
		sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, &error);
	}


	//create table for POP_EST_NATION_STATE_PR
	rc = sqlite3_exec(db, createTable_PopEstNationStatePr, NULL, NULL, &error );
	if(rc){
		cerr<<"**Error creating POP_EST_STATE_PR table!"<<endl<<error<<endl<<endl;
	}
	else{
		sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &error);
		cout<<"Table PopEstNationStatePr created!"<<endl;
		ifstream in_file("Pop_Estimate_Nation_State_PR.txt");
		int linecount = 0;
		string stream;
		while(getline(in_file, line)){
			if(linecount==0){
				linecount++;
				continue;
			}
			stream.append("INSERT INTO POP_EST_NATION_STATE_PR VALUES(");
			int charcount=0;
			string new_token = "";
			Tokenizer tk(line, delim);
			string token = tk.getToken();
			while(token != ""){
				if(charcount == 4 && token != "null"){
					new_token.append("'");
					new_token.append(token);
					new_token.append("'");
				}
				else
					new_token.append(token);
				stream.append(new_token);
				token = tk.getToken();
				charcount++;
				if(token != ""){ stream.append(","); }
				new_token = "";
			}
			/*char* target_line = (char*)line.c_str();
			char* token = strtok(target_line,",");
			int charcount=0;
			string new_token = "";
			while(token != NULL){
				//check for x and insert null for that
				if(strcmp(token, x_null) != 0){
					//field #5 is string type
					if(charcount == 4){
						new_token.append("'");
						new_token.append(token);
						new_token.append("'");
					}
					else{
						new_token.append(token);
					}
					stream.append(new_token.c_str());
				}
				else{
					stream.append("null");
				}
				token = strtok(NULL, ",");
				charcount++;
				if(token != NULL){
					stream.append(",");
				}
				new_token = "";
			}*/
			//stream.append(line.c_str());
			stream.append( ")");
			rc = sqlite3_exec(db, stream.c_str(), NULL, NULL, &error);
			if(rc){
				cerr<<error<<endl;
			}
			stream = "";
			linecount++;
		}
		sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, &error);
		//sqlite3_exec(db, "");
	}


	sqlite3_exec(db, "COMMIT", NULL, NULL, &error);	

	return 0;
}
