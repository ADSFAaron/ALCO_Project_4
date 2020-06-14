#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <utility>
using namespace std;

//目前正在運行的cycle
static int cycleNo = 0;

/* 每個operator 的cycle數
*
*	+ : 2
*	- : 2
*	* : 10
*	/ : 40
*/
static int needCycle[4] = { 2,2,8,40 };

struct Opcode
{
	string name;
	string rd;
	string rs1;
	string rs2;		// 在I-type當作 immediate
	char operate;
};

struct RS
{
	bool use;		//判斷這個RS是否已經使用
	string rs;		//RS1,RS2,RS3, ...
	char operate;	//運算符號
	int value1;
	int value2;
	int cyclenow;	//進buffer第幾個cycle可以跳出
};

void printCycle(vector<pair<int, string>>& rat, vector<RS>& rsADDSUB, vector<RS>& rsMULDIV, vector<pair<int, int>>& rf)
{
	cout << "____RF____" << endl;

	for (int i = 0; i < rf.size(); ++i)
	{
		cout << "F" << rf[i].first << " | " << rf[i].second << " |" << endl;
	}

	cout << "____RAT_____" << endl;

	for (int i = 0; i < rat.size(); ++i)
	{
		cout << "F" << rat[i].first << " | " << rat[i].second << " |" << endl;
	}

	cout << "____RS_____" << endl;

	for (int i = 0; i < rsADDSUB.size(); ++i)
	{
		cout << rsADDSUB[i].rs << " | " << rsADDSUB[i].operate << " | " << rsADDSUB[i].value1 << " | " << rsADDSUB[i].value2 << " | " << endl;

		if (i == 2)
		{
			for (int j = 0; j < 30; ++j)
			{
				cout << "-";
			}
			cout << "Buffered : " << endl;

		}
		else if (i == 4)
		{
			for (int j = 0; j < 30; ++j)
			{
				cout << "-";
			}
			cout << "Buffered : " << endl;
		}
	}
}

//Issue Instruction to RS
void inputRS(Opcode& opcode, vector<RS>& rs, vector<pair<int, int>>& rf, vector<pair<int, string>>& rat)
{
	RS temp;
	temp.use = true;
	temp.operate = opcode.operate;
	int rd = opcode.rd[1] - '0';
	int rs1 = opcode.rs1[1] - '0';

}

//將Input分割後存入instruction中
void SplitInstruction(Opcode& opcode, string& input)
{
	stringstream ss(input);

	getline(ss, opcode.name, ' ');
	getline(ss, opcode.rd, ',');
	getline(ss, opcode.rs1, ',');
	getline(ss, opcode.rs2);
}

int main()
{
	vector<Opcode> instruction;			//Instruction Queue 用
	Opcode opInput;
	string input;						//input handler
	vector<pair<int, string>> rat;		//Registration Source Table
	vector<pair<int, int>> rf;			//Register File
	vector<RS> rsADDSUB, rsMULDIV;		//rs for three add or sub, two for mul and div

	//將對應的RF值存入
	for (int i = 0; i < 5; ++i)
	{
		rf.push_back(make_pair<int, int>(i + 1, 2 * i));
	}

	//RF
	for (int i = 0; i < 5; ++i)
	{
		rat.push_back(make_pair<int, string>(i + 1, ""));
	}

	while (true)
	{
		getline(cin, input);

		if (input != "exit")
		{
			SplitInstruction(opInput, input);

			instruction.push_back(opInput);
		}
		else
		{
			break;
		}
	}
	int i = 0;
	do
	{
		cycleNo++;
		if (rsADDSUB.size() < 3) {
			if (instruction[i].name == "ADD" || instruction[i].name == "ADDI")
			{
				instruction[i].operate = '+';
				inputRS(instruction[i], rsADDSUB, rf, rat);
			}
			else if (instruction[i].name == "SUB" || instruction[i].name == "SUBI")
			{
				instruction[i].operate = '-';
				//SUB();
			}
			i++;
		}
		else if (rsMULDIV.size() < 2) {
			if (instruction[i].name == "MUL" || instruction[i].name == "MULI")
			{
				instruction[i].operate = '*';
				//MUL();
			}
			else if (instruction[i].name == "DIV" || instruction[i].name == "DIVI")
			{
				instruction[i].operate = '/';
				//DIV();
			}
			i++;
		}
	} while (!rsADDSUB.empty() && !rsMULDIV.empty());



	system("pause");
	return 0;
}