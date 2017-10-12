#include <iostream>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <string.h>
#include <algorithm>

/*
 * Dongmin An
 * 09/10/17
 * 21600397@handong.edu
 *
 * Sudoku solver using a SAT-solver
 *
 * The program utilizes minisat to solve sudoku puzzle which is given in an input file.
 * The input file will be transformed into a boolean satisfiability problem and 
 * solved by the minisat program, which will in turn be translated back into the
 * given format from the input file. 
 *
 *
 *
 * A. input from input.txt, generate DIMACS formula file formula.txt as output
 * B. Execute SAT solver to solve formula.txt. Recieve SAT solver output
 * C. Interpret SAT solver result, save solution to output.txt, or print no solution
 *
 * 
 *
 * NOTE - input.txt will be in a format that 0 will represent a blank space, else it 
 * will be filled from numbers 1~9
 * 
 *
 * Resources: 
 * https://www.dwheeler.com/essays/minisat-user-guide.html
 * http://www.cs.qub.ac.uk/~I.Spence/SuDoku/SuDoku.html
 * https://ubuntuforums.org/archive/index.php/t-285287.html
 * 
 */

#define MAX_SIZE 9

int n_cell[MAX_SIZE];

void load(std::string fname, int *given_cell);
void unload(std::string fname, int *given_cell);
void cli_run(std::string in, std::string form, std::string out);
int transform(int n);
int concat(int a, int b, int c);
int reverse(std::string str);

int main()
{	
	std::string inp_name = "input.txt";
	std::string form 	 = "formula.txt";
	std::string min_out  = "min_out.txt";
	std::string out_name = "output.txt";


	load(inp_name, n_cell);
	unload(form, n_cell);
	cli_run(form, min_out, out_name);

	return 0;
}

void load(std::string fname, int *given_cell)
{
	std::fstream read_f;
	std::size_t pos;
	std::string delim = " ";
	std::string token;
	int i = 1;
	int j = 1;
	int k = 0;
	int temp;

	read_f.open(fname);

	if (!read_f)		
	{
		std::cout << "Error in file IO. \n Terminating program." << std::endl;
	}

	if (read_f)
	{
		std::string content((std::istreambuf_iterator<char>(read_f)), std::istreambuf_iterator<char>());	//read content of string using istreambuf stl

		while((pos = content.find(delim)) != std::string::npos)		//tokenize using substr and find
		{
			token = content.substr(0, pos);
			token.erase(std::remove(token.begin(), token.end(), '\n'), token.end());

			std::stringstream myStream(token);						//convert string to number
			myStream >> temp;

			if(temp != 0)
			{
				given_cell[k] = concat(j,i,temp);					//store into array as (i,j,k)
				k++;
			}

			content.erase(0, pos + delim.length());					//delete up to delim after storing string
			i++;
			if(i > MAX_SIZE)										//reset row col values
			{
				j++;
				i = 1;
			}
		}

	}
}

void unload(std::string fname, int *given_cell)
{
	std::ofstream write_f(fname);
	std::string clean;
	int cnt = 0;
	int n_clauses = 0;
	int temp = 0;
	int k = 0;


	if (!write_f)
	{
		std::cout << "Error in file IO. \n Terminating program." << std::endl;	
	}

	if (write_f)
	{
		write_f << "p cnf 729 11897" << "\n";	
				
		write_f << "c given caluses\n";
		for(int i = 0; i < 28; i++)											
		{
			write_f << transform(given_cell[i]) << " 0\n";
		}

		write_f << "c row clauses\n";
		for(int i = 1; i <= MAX_SIZE; i++)								
		{ 	
			for(int k = 1; k <= MAX_SIZE; k++)
			{
				for(int j = 1; j <= MAX_SIZE; j++)
				{
					write_f << transform(concat(i, j, k)) << " ";
				}
				write_f << " 0\n";
			}
		}

		write_f << "c col clauses\n";
		for(int j = 1; j <= MAX_SIZE; j++)								
		{
			for(int k = 1; k <= MAX_SIZE; k++)
			{
				for(int i = 1; i <= MAX_SIZE; i++)
				{
					write_f << transform(concat(i, j, k)) << " ";
				}
				write_f << " 0\n";
			}
		}

		write_f << "c box clauses\n";
		for(int r = 0; r <= 6; r += 3)
		{
			for(int s = 0; s <= 6; s += 3)
			{
				for(int n = 1; n <= MAX_SIZE; n++)
				{
					for(int i = 1; i <= 3; i++)
					{
						for(int j = 1; j <= 3; j++)
						{
							write_f << transform(concat(r+i, s+j, n)) << " ";
						}
					}
					write_f << "0\n";
				}
			}
		}
		
		write_f << "c restrictions\n";

		for(int i = 1; i <= MAX_SIZE; i++)
		{
			for(int j = 1; j <= MAX_SIZE; j++)
			{
				for(int n = 1; n <= 8; n++)
				{
					for(int m = n + 1; m <= MAX_SIZE; m++)
					{
						write_f << "-" << transform(concat(i,j,n)) << " -" << transform(concat(i,j,m)) << " 0\n";
						//write_f << "-" << transform(concat(i,j,m)) << " -" << transform(concat(i,j,n)) << " 0\n";
					}
				}
			}
		}

		write_f.close();
	}
}

void cli_run(std::string in, std::string form, std::string out)
{
	std::string str= "./minisat ";
	str = str + " " + in + " " + form;
	const char *tempchar = str.c_str();

	system(tempchar);


	std::ifstream read_f;
	std::ofstream write_f(out);
	std::size_t pos;
	std::string delim = " ";
	std::string token;
	int ncell[MAX_SIZE][MAX_SIZE];
	int val;
	int temp;
	int cnt = 1;
	int t_cnt = 1;
	bool s_switch = true;

	read_f.open(form);

	if (read_f)
	{
		std::string content((std::istreambuf_iterator<char>(read_f)), std::istreambuf_iterator<char>());	

		while((pos = content.find(delim)) != std::string::npos && s_switch == true)		//tokenize using substr and find
		{
			token = content.substr(0, pos);
			if(token.rfind("SAT", 0) == 0)
			{
				token.erase(0, 4);
				t_cnt++;
			}
			else if(t_cnt == 1)
			{
				std::cout << "no solution"; 
				s_switch = false;
			}
			if(token.rfind("-",0) != 0)
			{
				temp = reverse(token);
				val = temp % 10;

				write_f << val << " ";
				if(cnt == 9)
				{
					write_f << "\n";
					cnt = 1;
				}
				else
				{
					cnt++;
				}
			}
			content.erase(0, pos + delim.length());					//delete up to delim after storing string
		}
	}
}

int transform(int n)			//converts (i,j,k) format to simply a number between 1~729
{
	int last = 0;
	int mid = 0;
	int first = 0;
	int sum = 0;

	last = n % 10;
	n = n / 10;

	mid = n % 10;
	n = n / 10;

	first = n;

	sum = (last) + (mid - 1)*9 + (first - 1) * 81;

	return sum;
}

int concat(int a, int b, int c)
{
	int first = a * 100;
	int mid = b * 10;
	int sum = 0;

	sum = first + mid + c;

	return sum;
}


int reverse(std::string str)	//reverse of transform, converts 1~729 to (i,j,k) format
{
	int num;
	int row, col, val;
	std::stringstream myStream(str);
	myStream >> num;

	
	row = ((num - 1)/81 + 1);
	col = (((num - 1) / 9) % 9 + 1);
	val = ((num - 1) % 9 + 1);

	return concat(row, col, val);
}