//
// File: wl.h
// 
//  Description: Add stuff here ... 
//  Student Name: Liang Zheng Gooi	 
//  UW Campus ID: 9066169518	
//  email: lgooi@wisc.edu
#include <iostream>
#include <cstring>
#include <string>
#include <map>
#include <cctype>
#include <vector>
#include <fstream>
#include <iterator>
#include <sstream>
using namespace std;

char compare_cmd(char* str){
	//make the token lower case
	for(int i=0; str[i]!='\0'; i++){
		str[i] = tolower(str[i]);
	}
	//TEST: see what hppen after tolower
	cout<< str <<endl;
	
	if(strcmp(str, "new")==0){
		cout << "N\n";
		return 'N';	
	}
	else if (strcmp(str, "end")==0){
		cout << "E\n";
		return 'E';
	}
	else if (strcmp(str, "locate")==0){
		cout << "F\n";
		return 'F';
	}
	else if (strcmp(str, "load")==0){
		cout << "L\n";
		return 'L';
	}
	
	cout << "P\n";
	return 'P';
	
}

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
	//start loop here=========
	do{
		cmd_checked = false;
		cout << "> ";
		getline(cin, inputStr);
		// TEST: see whatin inputStr
		cout << inputStr << endl;
		//=======================================================TEST case======================================
		//vector<int> temp_vector (1, 12);
		//word_dictionary.insert(pair<string, vector<int>>("one", temp_vector));
		//+++++++++++++++++++++++++++++++++++++++++++++++++++TEST case =============================================


		/*Split the string into tokens
		 */
		char* buffer = new char[inputStr.length()+1];
		strcpy (buffer, inputStr.c_str());
		inputStr.clear();
		char* tokens = strtok(buffer, " ");
		while(tokens != NULL){
			//TEST: see what is in the token
			cout<< tokens << endl;
		
			if(!cmd_checked){
				reg = compare_cmd(tokens);
			}
		
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
					cout<<"open file"<<endl;
					char* filename = tokens;
					//check for invalid command
					tokens = strtok(NULL, " ");
					if(tokens!=NULL){
						reg = 'P';
						continue;
					}
					int file_length=0;
					fstream file_stream;
					file_stream.open(filename, fstream::in);
					if(file_stream.is_open()){
						//get length of file
						file_stream.seekg(0, ios::end);
						file_length = file_stream.tellg();
						file_stream.seekg(0, ios::beg);
						cout<< "File size is "<< file_length << endl;

						//allocate memory
						newLine = new char[file_length];

						//read data as a block
						file_stream.read(newLine, file_length);
						file_stream.close();
						
						cout<<newLine<<endl;

						while(tokens!=NULL){tokens=strtok(NULL," ");};

						//generate dictionary from the string
						//char* word = strtok(newLine, " _?!~,.:;\n");
						char* word = strtok(newLine, delims);
						
						for(int j=0; word!=NULL && j<file_length; j++){
							//make it lowercase
							for(int i=0; word[i]!='\0'; i++){
								word[i] = tolower(word[i]);
							}	
							map<string, vector<int>>::iterator dic_it = word_dictionary.find(word);
							if(dic_it == word_dictionary.end()){
								dic_it = word_dictionary.begin();
								vector<int> value (1, j+1);
								word_dictionary.insert(pair<string,vector<int>>(word, value));
							}
							else{
								vector<int>::iterator vec_itr = (dic_it->second).end();
								dic_it->second.insert(vec_itr, j+1);
							}

							//word = strtok(NULL, " _?!~,.:;\n");
							word = strtok(NULL, delims);							
							
							if(j>10000){
								cout<<"exceed bound!!!!"<<endl;
								break;
							}
						}
						//============================see what is in dictionary ===========
						map<string, vector<int>>::iterator dictionary_itr = word_dictionary.begin();
						while(dictionary_itr!=word_dictionary.end()){
							//if(word_dictionary.size()>419){
							//	cout << "word_dictionary has "<<  word_dictionary.size()<< "instead of";
							//	cout << file_length << endl;
							//	break;
							//}
							cout << dictionary_itr->first << " is ";
							vector<int> printInt_vec = dictionary_itr -> second;
							for(unsigned int k=0; k < printInt_vec.size();k++){
								cout<<printInt_vec[k]<< " ";
							}
							cout<<endl;
							dictionary_itr++;
						}
						cout<< "File length is "<< file_length << endl;
						delete[] newLine;
	
					}
					else{
						cout << "FILE is invalid" << endl;
						reg = 'P';
					}
			
				}
		

			}
			else if(reg == 'F'){
				char* target_word=NULL;
				int target_index = -1;
				if(tokens != NULL){
					target_word = tokens;
					for(int x=0; target_word[x]!='\0';x++){
						target_word[x] = tolower(target_word[x]);
					}
					tokens = strtok(NULL, " ");
				}
				else{
					reg = 'P';
				}

				if(tokens != NULL){
					target_index = atoi(tokens);
					target_index -= 1;
					tokens = strtok(NULL, " ");
				}
				else{
					reg = 'P';
				}

				if(tokens!=NULL){
					reg = 'P';
				}
				else if(word_dictionary.empty() || target_index < 0){
					cout<<"No matching entry"<<endl;
				}
				else if(target_word != NULL && target_index>=0){
					vector<int> ret_vector = word_dictionary[target_word];
					if(ret_vector.empty() || (int)ret_vector.size() < target_index + 1){
						cout << "No matching entry" << endl;
					}
					else{
						cout << ret_vector[target_index] << endl;
					}
				}
			}
		
			//Error msg
			if(reg == 'P'){
				cout << "ERROR: Invalid command"<< endl;
				break;
			}

		}
	
		//make sure that strtok is being cleared
		while(tokens!=NULL){tokens = strtok(NULL," ");}
	
		delete[] buffer;
		delete[] tokens;

		cout << "endfile"<<"\t"<<reg;
		cout << endl;
	}while(reg != 'E');
   return 0;
}
