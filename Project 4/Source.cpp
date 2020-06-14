#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <utility>
#include <iomanip>
using namespace std;

//�ثe���b�B�檺cycle
static int cycleNo = 0;

/* �C��operator ��cycle��
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
	string rs2;		// �bI-type��@ immediate
	char operate;
};

struct RS
{
	bool use;		//�P�_�o��RS�O�_�w�g�ϥ�
	string rs;		//RS1,RS2,RS3, ...
	char operate;	//�B��Ÿ�
	string value1;
	string value2;
	int cyclenow;	//�ibuffer�ĴX��cycle�i�H���X
};

vector<pair<int, string>> rat;		//Registration Source Table
vector<pair<int, int>> rf;			//Register File
vector<RS> rsADDSUB, rsMULDIV;		//rs for three add or sub, two for mul and div
using RSiterator = RS*;
RSiterator BufferADDSUB, BufferMULDIV;	//when value are all exist, enter to buffer to execute

//�|�h�B��
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

	//�P�_rs2�O�_�O�¼Ʀr�Φ��r���Ʀr
	istringstream convert(opcode.rs2);

	//�O�Ʀr
	if (convert >> rs2)
		temp.value2 = rs2;
	else
	{
		//���r���M�Ʀr
		rs2 = opcode.rs2[1] - '0';

		//�P�_RAT��F���S����
		if (rat[rs2 - 1].second != "")
		{
			temp.value2 = rat[rs2 - 1].second;
		}
		else
		{
			temp.value2 = to_string(rf[rs2 - 1].second);
		}

	}

	//�qrf����
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



//�NInput���Ϋ�s�Jinstruction��
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
	//��ADDSUB�BMULDIV ��� ALU
	if (BufferADDSUBEmpty())
	{
		int canBuffer = 0;
		int tmp = 0;

		//ADDSUB
		for (int i = 0; i < rsADDSUB.size(); ++i)
		{
			canBuffer = 0;
			tmp = 0;

			//�P�_rs2�O�_�O�¼Ʀr�Φ��r���Ʀr
			istringstream convert1(rsADDSUB[i].value1);

			//�O�Ʀr
			if (convert1 >> tmp)
				canBuffer++;

			//�P�_rs2�O�_�O�¼Ʀr�Φ��r���Ʀr
			istringstream convert2(rsADDSUB[i].value2);

			//�O�Ʀr
			if (convert2 >> tmp)
				canBuffer++;

			//��ӼƦr����
			if (canBuffer == 2)
			{
				BufferADDSUB = &rsADDSUB[i];

				if (BufferADDSUB->operate - '+' == 0)
				{
					//�p����X��cycle��
					BufferADDSUB->cyclenow = cycleNo + needCycle[0];
				}
				else
				{
					//�p����X��cycle��
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

			//�P�_rs2�O�_�O�¼Ʀr�Φ��r���Ʀr
			istringstream convert1(rsMULDIV[i].value1);

			//�O�Ʀr
			if (convert1 >> tmp)
				canBuffer++;

			//�P�_rs2�O�_�O�¼Ʀr�Φ��r���Ʀr
			istringstream convert2(rsMULDIV[i].value2);

			//�O�Ʀr
			if (convert2 >> tmp)
				canBuffer++;

			//��ӼƦr����
			if (canBuffer == 2)
			{
				BufferMULDIV = &rsMULDIV[i];

				if (BufferMULDIV->operate - '*' == 0)
				{
					//�p����X��cycle��
					BufferMULDIV->cyclenow = cycleNo + needCycle[2];
				}
				else
				{
					//�p����X��cycle��
					BufferMULDIV->cyclenow = cycleNo + needCycle[3];
				}
			}
		}
	}
}

//Instruction���}RS�ɡA�nwrite result�^RF �� ��RS�PRAT�۲Ū��N��
void leaveRS()
{
	//�P�_Buffer���O�_cycle�w�g�]��
	if (BufferADDSUB->cyclenow == cycleNo)
	{
		//�]���A�n��RS�M��
		BufferADDSUB->use = false;
		int result = 0;

		//����RAT���L�������ȡA���n��RF
		for (int i = 0; i < rat.size(); ++i)
		{
			if (rat[i].second == BufferADDSUB->rs)
			{
				result = Arithmetic(*BufferADDSUB);
				rf[rat[i].first - 1].second = result;
				break;
			}
		}

		//��RS����L�B�⦳�L�ݭn
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

	//�P�_Buffer���O�_cycle�w�g�]��
	if (BufferMULDIV->cyclenow == cycleNo)
	{
		//�]���A�n��RS�M��
		BufferMULDIV->use = false;
		int result = 0;

		//����RAT���L�������ȡA���n��RF
		for (int i = 0; i < rat.size(); ++i)
		{
			if (rat[i].second == BufferMULDIV->rs)
			{
				result = Arithmetic(*BufferMULDIV);
				rf[rat[i].first - 1].second = result;
				break;
			}
		}

		//��RS����L�B�⦳�L�ݭn
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
	vector<Opcode> instruction;			//Instruction Queue ��
	Opcode opInput;
	string input;						//input handler

	//�N������RF�Ȧs�J
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