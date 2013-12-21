//
// File: wl.h
// 
//  Description: This program will assist the user to check for occurrence(s) for a word in a text file.
//  Student Name: Liang Zheng Gooi	 
//  UW Campus ID: 9066169518	
//  email: lgooi@wisc.edu
//  time: Jan 31, 2012 10.39pm

/*
 * @brief This program helps to detect occurrence of a word
 * @author Liang Zheng Gooi
 *
 * This program implements how a text file beng loaded and creating a virtual dictionary for the user.
 * The user will able to use the program to check the occurrence(s) of a word.
 *
 * */

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
char compare_cmd(char* str);

/*
 * Check whether the search word is valid
 *
 * @param str The search key that being entered by user.
 * 	delims The identifier for punctuation, except " ' ".
 *
 * @return false if the word is not valid.
 *
 * */
bool validWord(char* str, const char* delims);



