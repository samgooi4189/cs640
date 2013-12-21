//
// File: wl.h
// 
//  Description: This program will assist the user to check for occurrence(s) for a word in a text file.
//  Student Name: Liang Zheng Gooi	 
//  UW Campus ID: 9066169518	
//  email: lgooi@wisc.edu
//  time: Jan 31, 2012 10.39pm
#include <iostream>
#include <cstring>
#include <string>
#include <map>
#include <cctype>
#include <vector>
#include <fstream>
using namespace std;

/*
* Get the representation on which command is being executed.
*  N = new 
*  E = end 
*  L = load 
*  F = locate 
*  P = problem 
*
* @param str The command that being entered by the user.
*
* @return A character representation on the command.
*/
char compare_cmd(char* str){
	//make the token lower case
	for(int i=0; str[i]!='\0'; i++){
		str[i] = tolower(str[i]);
	}
	
	if(strcmp(str, "new")==0){
		return 'N';	
	}
	else if (strcmp(str, "end")==0){
		return 'E';
	}
	else if (strcmp(str, "locate")==0){
		return 'F';
	}
	else if (strcmp(str, "load")==0){
		return 'L';
	}
	
	return 'P';
}

/*
 * Check whether the search word is valid
 *
 * @param str The search key that being entered by user.
 * 	delims The identifier for punctuation, except " ' ".
 *
 * @return false if the word is not valid.
 *
 * */
bool validWord(char* str, const char* delims){
	for(unsigned int i=0; i<strlen(str);i++){
		for(unsigned int j=0; j<strlen(delims);j++){
			if(str[i]==delims[j])
				return false;
		}
	}
	
	return true;
}

/*
 * @brief This program helps to detect occurrence of a word
 * @author Liang Zheng Gooi
 *
 * This program implements how a text file beng loaded and creating a virtual dictionary for the user.
 * The user will able to use the program to check the occurrence(s) of a word.
 *
 * */
int main()
{
	const char* delims = " ][!#$%&()*+,./:;<=>?\"@\\^_`{|}~-\n\t";
	char* newLine;
	/*Register for indicating which operation
	 *  N = new 
	 *  E = end
	 *  L = load
	 *  F = locate
	 *  P = problem
	 * */
	char reg = 'E';
	bool cmd_checked = false;
	map<string, vector<int>> word_dictionary;

	string inputStr;
	//while loop start
	do{
		//taking input from user
		cmd_checked = false;
		cout << ">";
		getline(cin, inputStr);

		//Split the string into tokens
		char* buffer = new char[inputStr.length()+1];
		strcpy (buffer, inputStr.c_str());
		inputStr.clear();
		char* tokens = strtok(buffer, " ");
		while(tokens != NULL){
			//check if wether this is the command word
			if(!cmd_checked){
				reg = compare_cmd(tokens);
			}
			
			//continue to tokenize
			tokens = strtok(NULL, " ");		

			if(reg == 'N'){
				if(tokens == NULL){
					word_dictionary.clear();
					break;			
				}
				else{
					reg = 'P';		
				}
			}
			else if(reg=='E'){
				if(tokens != NULL){
					reg = 'P';					
				}
				else{
					break;
				}
			}
			else if(reg == 'L'){
				cmd_checked = true;
				//open file
				if(tokens!=NULL){
					char* filename = tokens;
					
					//check for invalid command
					tokens = strtok(NULL, " ");
					if(tokens!=NULL){
						reg = 'P';
						continue;
					}

					int file_length = 0;
					fstream file_stream;
					file_stream.open(filename, fstream::in);
					
					if(file_stream.is_open()){
						//get length of file
						file_stream.seekg(0, ios::end);
						file_length = file_stream.tellg();
						file_stream.seekg(0, ios::beg);

						//allocate memory
						newLine = new char[file_length];

						//read data as a block
						file_stream.read(newLine, file_length);
						file_stream.close();

						//clear of the string in strtok
						while(tokens != NULL){tokens = strtok(NULL," ");};

						//generate dictionary from the string
						char* word = strtok(newLine, delims);
						for(int j=0; word!=NULL && j<file_length; j++){
							//make it lowercase
							for(int i=0; word[i]!='\0'; i++){
								word[i] = tolower(word[i]);
							}	
							map<string, vector<int>>::iterator dic_it = word_dictionary.find(word);
							//dictionary does not have the word
							if(dic_it == word_dictionary.end()){
								dic_it = word_dictionary.begin();
								vector<int> value (1, j+1);
								word_dictionary.insert(pair<string,vector<int>>(word, value));
							}
							else{
								//dictionary already has the word
								vector<int>::iterator vec_itr = (dic_it->second).end();
								dic_it->second.insert(vec_itr, j+1);
							}
							word = strtok(NULL, delims);							
						}
						delete[] newLine;
	
					}
					else{
						//if the file is invalid
						reg = 'P';
					}
			
				}
		

			}
			else if(reg == 'F'){
				char* target_word = NULL;
				int target_index = -1;
				
				//get key from user
				if(tokens != NULL){
					target_word = tokens;
					//to lower case
					for(int x=0; target_word[x]!='\0';x++){
						target_word[x] = tolower(target_word[x]);
					}
					tokens = strtok(NULL, " ");
				}
				else{
					reg = 'P';
				}

				//get index from user
				if(tokens != NULL){
					target_index = atoi(tokens);
					target_index -= 1;
					tokens = strtok(NULL, " ");
				}
				else{
					reg = 'P';
				}
				
				//check whether the command line is more than 3 words or the key is not valid
				if(tokens!=NULL || !validWord(target_word, delims) || reg=='P'){
					reg = 'P';
				}
				//check for empty dictionary or negative index
				else if(word_dictionary.empty() || target_index < 0){
					cout<<"No matching entry"<<endl;
				}
				else if(target_word != NULL && target_index>=0){
					vector<int> ret_vector = word_dictionary[target_word];
					//check for empty value from dictionary and prevent search out of bound
					if(ret_vector.empty() || (int)ret_vector.size() < target_index + 1){
						cout << "No matching entry" << endl;
					}
					else{
						cout << ret_vector[target_index] << endl;
					}
				}
			}
		
			//Display error message and stop string spliting
			if(reg == 'P'){
				cout << "ERROR: Invalid command"<< endl;
				break;
			}

		}
	
		//make sure that strtok is being cleared
		while(tokens!=NULL){tokens = strtok(NULL," ");}
		
		//free memory
		delete[] buffer;
		delete[] tokens;

	}while(reg != 'E');	//end of while loop

   return 0;
}
