#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <utility>
#include <iomanip>
using namespace std;

//ヘ玡タ笲︽cycle
static int cycleNo = 0;

/* –operator cycle计
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
	string rs2;		// I-type讽 immediate
	char operate;
};

struct RS
{
	bool use;		//耞硂RS琌竒ㄏノ
	string rs;		//RS1,RS2,RS3, ...
	char operate;	//笲衡才腹
	string value1;
	string value2;
	int cyclenow;	//秈buffer材碭cycle铬
};

vector<pair<int, string>> rat;		//Registration Source Table
vector<pair<int, int>> rf;			//Register File
vector<RS> rsADDSUB, rsMULDIV;		//rs for three add or sub, two for mul and div
using RSiterator = RS*;
RSiterator BufferADDSUB, BufferMULDIV;	//when value are all exist, enter to buffer to execute

//玥笲衡
int Arithmetic(RS& buffer)
{
	int total = 0;
	if (buffer.operate - '+' == 0)
	{
		total = stoi(buffer.value1) + stoi(buffer.value2);
	}
	else if (buffer.operate - '-' == 0)
	{
		total = stoi(buffer.value1) - stoi(buffer.value2);
	}
	else if (buffer.operate - '*' == 0)
	{
		total = stoi(buffer.value1) * stoi(buffer.value2);
	}
	else if (buffer.operate - '/' == 0)
	{
		total = stoi(buffer.value1) / stoi(buffer.value2);
	}

	return total;
}

//Output Cycle Status
void printCycle()
{
	cout << "Cycle : " << cycleNo << endl << endl;

	cout << "__________RF_________" << endl;

	for (int i = 0; i < rf.size(); ++i)
	{
		cout << setw(5) << "F" << rf[i].first << " | " << setw(5) << rf[i].second << " |" << endl;
	}

	cout << "_________RAT__________" << endl;

	for (int i = 0; i < rat.size(); ++i)
	{
		cout << setw(5) << "F" << rat[i].first << " | " << setw(5) << rat[i].second << " |" << endl;
	}

	cout << "______________RS_______________" << endl;

	for (int i = 0; i < rsADDSUB.size(); ++i)
	{
		if (rsADDSUB[i].use)
			cout << setw(5) << rsADDSUB[i].rs << " | " << setw(1) << rsADDSUB[i].operate << " | " << setw(2) << rsADDSUB[i].value1 << " | " << setw(2) << rsADDSUB[i].value2 << " | " << endl;
		else
			cout << setw(5) << rsADDSUB[i].rs << " | " << setw(4) << " | " << setw(5) << " | " << setw(5) << " | " << endl;

		if (i == 2)
		{
			for (int j = 0; j < 30; ++j)
			{
				cout << "-";
			}
			cout << endl << "Buffered : " << endl << endl;
		}
	}

	for (int i = 0; i < rsMULDIV.size(); ++i)
	{
		if (rsMULDIV[i].use)
			cout << setw(5) << rsMULDIV[i].rs << " | " << setw(1) << rsMULDIV[i].operate << " | " << setw(2) << rsMULDIV[i].value1 << " | " << setw(2) << rsMULDIV[i].value2 << " | " << endl;
		else
			cout << setw(5) << rsMULDIV[i].rs << " | " << setw(4) << " | " << setw(5) << " | " << setw(5) << " | " << endl;

		if (i == 1)
		{
			for (int j = 0; j < 30; ++j)
			{
				cout << "-";
			}
			cout << endl << "Buffered : " << endl << endl;
		}
	}
}

//Issue Instruction to RS
void inputRS(Opcode& opcode, vector<RS>& rs)
{
	RS temp;
	temp.use = true;
	temp.operate = opcode.operate;
	int rd = opcode.rd[1] - '0';
	int rs1 = opcode.rs1[1] - '0';
	int rs2;

	//耞rs2琌琌计┪Τダ计
	istringstream convert(opcode.rs2);

	//琌计
	if (convert >> rs2)
		temp.value2 = rs2;
	else
	{
		//Τダ㎝计
		rs2 = opcode.rs2[1] - '0';

		//耞RATFΤ⊿Τ
		if (rat[rs2 - 1].second != "")
		{
			temp.value2 = rat[rs2 - 1].second;
		}
		else
		{
			temp.value2 = to_string(rf[rs2 - 1].second);
		}

	}

	//眖rf
	temp.value1 = to_string(rf[rs1 - 1].second);

	for (int i = 0; i < rs.size(); ++i)
	{
		if (!rs[i].use)
		{
			temp.rs = rs[i].rs;
			rs[i] = temp;

			rat[rd - 1].second = rs[i].rs;
			break;
		}
	}

	//rat change

	printCycle();

	system("pause");
}



//盢Inputだ澄instructionい
void SplitInstruction(Opcode& opcode, string& input)
{
	stringstream ss(input);

	getline(ss, opcode.name, ' ');
	getline(ss, opcode.rd, ',');
	getline(ss, opcode.rs1, ',');
	getline(ss, opcode.rs2);
}

bool RSADDSUBFull()
{
	int count = 0;

	for (int i = 0; i < rsADDSUB.size(); ++i)
	{
		if (rsADDSUB[i].use)
		{
			++count;
		}
	}

	if (count == 3)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool RSMULDIVFull()
{
	int count = 0;

	for (int i = 0; i < rsMULDIV.size(); ++i)
	{
		if (rsMULDIV[i].use)
		{
			++count;
		}
	}

	if (count == 2)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool RSMULDIVEmpty()
{
	for (int i = 0; i < rsMULDIV.size(); ++i)
	{
		if (rsMULDIV[i].use)
		{
			return false;
		}
	}

	return true;
}

bool RSADDSUBEmpty()
{
	for (int i = 0; i < rsADDSUB.size(); ++i)
	{
		if (rsADDSUB[i].use)
		{
			return false;
		}
	}

	return true;
}

bool RSEmpty()
{
	return (RSADDSUBEmpty() && RSMULDIVEmpty());
}

void Issue(vector<Opcode>& instruction, int& i)
{
	if (instruction[i].name == "ADD" || instruction[i].name == "ADDI")
	{
		if (!RSADDSUBFull())
		{
			instruction[i].operate = '+';
			inputRS(instruction[i], rsADDSUB);
			++i;
		}
	}
	else if (instruction[i].name == "SUB" || instruction[i].name == "SUBI")
	{
		if (!RSADDSUBFull())
		{
			instruction[i].operate = '-';
			inputRS(instruction[i], rsADDSUB);
			++i;
		}
	}
	else if (instruction[i].name == "MUL" || instruction[i].name == "MULI")
	{
		if (!RSMULDIVFull())
		{
			instruction[i].operate = '*';
			inputRS(instruction[i], rsMULDIV);
			++i;
		}
	}
	else if (instruction[i].name == "DIV" || instruction[i].name == "DIVI")
	{
		if (!RSMULDIVFull())
		{
			instruction[i].operate = '/';
			inputRS(instruction[i], rsMULDIV);
			++i;
		}
	}
}

bool BufferADDSUBEmpty()
{
	return !BufferADDSUB->use;
}

bool BufferMULDIVEmpty()
{
	return !BufferMULDIV->use;
}

void Execute()
{
	//ΤADDSUBMULDIV ㄢ ALU
	if (BufferADDSUBEmpty())
	{
		int canBuffer = 0;
		int tmp = 0;

		//ADDSUB
		for (int i = 0; i < rsADDSUB.size(); ++i)
		{
			canBuffer = 0;
			tmp = 0;

			//耞rs2琌琌计┪Τダ计
			istringstream convert1(rsADDSUB[i].value1);

			//琌计
			if (convert1 >> tmp)
				canBuffer++;

			//耞rs2琌琌计┪Τダ计
			istringstream convert2(rsADDSUB[i].value2);

			//琌计
			if (convert2 >> tmp)
				canBuffer++;

			//ㄢ计常Τ
			if (canBuffer == 2)
			{
				BufferADDSUB = &rsADDSUB[i];

				if (BufferADDSUB->operate - '+' == 0)
				{
					//璸衡铬cycle计
					BufferADDSUB->cyclenow = cycleNo + needCycle[0];
				}
				else
				{
					//璸衡铬cycle计
					BufferADDSUB->cyclenow = cycleNo + needCycle[1];
				}

			}
		}
	}

	if (BufferMULDIVEmpty())
	{
		int canBuffer = 0;
		int tmp = 0;

		//MULDIV
		for (int i = 0; i < rsMULDIV.size(); ++i)
		{
			canBuffer = 0;
			tmp = 0;

			//耞rs2琌琌计┪Τダ计
			istringstream convert1(rsMULDIV[i].value1);

			//琌计
			if (convert1 >> tmp)
				canBuffer++;

			//耞rs2琌琌计┪Τダ计
			istringstream convert2(rsMULDIV[i].value2);

			//琌计
			if (convert2 >> tmp)
				canBuffer++;

			//ㄢ计常Τ
			if (canBuffer == 2)
			{
				BufferMULDIV = &rsMULDIV[i];

				if (BufferMULDIV->operate - '*' == 0)
				{
					//璸衡铬cycle计
					BufferMULDIV->cyclenow = cycleNo + needCycle[2];
				}
				else
				{
					//璸衡铬cycle计
					BufferMULDIV->cyclenow = cycleNo + needCycle[3];
				}
			}
		}
	}
}

//Instruction瞒秨RS璶write resultRF 蛤 тRS籔RAT才腹
void leaveRS()
{
	//耞Bufferず琌cycle竒禲Ч
	if (BufferADDSUB->cyclenow == cycleNo)
	{
		//禲Ч璶琵RS睲
		BufferADDSUB->use = false;
		int result = 0;

		//тRATΤ礚癸莱Τ璶эRF
		for (int i = 0; i < rat.size(); ++i)
		{
			if (rat[i].second == BufferADDSUB->rs)
			{
				result = Arithmetic(*BufferADDSUB);
				rf[rat[i].first - 1].second = result;
				break;
			}
		}

		//RSずㄤ笲衡Τ礚惠璶
		for (int i = 0; i < rsADDSUB.size(); ++i)
		{
			if (rsADDSUB[i].value1 == BufferADDSUB->rs)
			{
				rsADDSUB[i].value1 = to_string(result);
			}

			if (rsADDSUB[i].value2 == BufferADDSUB->rs)
			{
				rsADDSUB[i].value2 = to_string(result);
			}
		}

		for (int i = 0; i < rsMULDIV.size(); ++i)
		{
			if (rsMULDIV[i].value1 == BufferADDSUB->rs)
			{
				rsMULDIV[i].value1 = to_string(result);
			}

			if (rsMULDIV[i].value2 == BufferADDSUB->rs)
			{
				rsMULDIV[i].value2 = to_string(result);
			}
		}
	}

	//耞Bufferず琌cycle竒禲Ч
	if (BufferMULDIV->cyclenow == cycleNo)
	{
		//禲Ч璶琵RS睲
		BufferMULDIV->use = false;
		int result = 0;

		//тRATΤ礚癸莱Τ璶эRF
		for (int i = 0; i < rat.size(); ++i)
		{
			if (rat[i].second == BufferMULDIV->rs)
			{
				result = Arithmetic(*BufferMULDIV);
				rf[rat[i].first - 1].second = result;
				break;
			}
		}

		//RSずㄤ笲衡Τ礚惠璶
		for (int i = 0; i < rsADDSUB.size(); ++i)
		{
			if (rsADDSUB[i].value1 == BufferMULDIV->rs)
			{
				rsADDSUB[i].value1 = to_string(result);
			}

			if (rsADDSUB[i].value2 == BufferMULDIV->rs)
			{
				rsADDSUB[i].value2 = to_string(result);
			}
		}

		for (int i = 0; i < rsMULDIV.size(); ++i)
		{
			if (rsMULDIV[i].value1 == BufferMULDIV->rs)
			{
				rsMULDIV[i].value1 = to_string(result);
			}

			if (rsMULDIV[i].value2 == BufferMULDIV->rs)
			{
				rsMULDIV[i].value2 = to_string(result);
			}
		}
	}
}

int main()
{
	vector<Opcode> instruction;			//Instruction Queue ノ
	Opcode opInput;
	string input;						//input handler

	//盢癸莱RF
	for (int i = 0; i < 5; ++i)
	{
		rf.push_back(make_pair<int, int>(i + 1, 2 * i));
	}

	//RF
	for (int i = 0; i < 5; ++i)
	{
		rat.push_back(make_pair<int, string>(i + 1, ""));
	}

	//create RS
	for (int i = 1; i < 6; ++i)
	{
		RS tmp;
		tmp.rs = "RS" + to_string(i);
		tmp.use = false;
		tmp.operate = '\0';
		tmp.value1 = "";
		tmp.value2 = "";

		if (i < 4)
		{
			rsADDSUB.push_back(tmp);
		}
		else
		{
			rsMULDIV.push_back(tmp);
		}
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
		++cycleNo;

		if (instruction.size() > i)
		{
			Issue(instruction, i);
		}

		Execute();

		leaveRS();

	} while (!RSEmpty());


	system("pause");
	return 0;
}