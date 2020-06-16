# ALCO_Project_4
Homework for Assembled Language and Computer Organization Project four

## Project Goal
實作Instruction Scheduling

Input : RISC V Instruction，結尾輸入exit。

Output : 每個cycle的狀態，RS RAT RF的改變。


## Project Method
輸入完 Instruction 後，從 Issue 開始進入 RS。

Execution 判斷進入 RS 的 Instruction 能否進 Buffer。

leaveRS 判斷執行完的 Instruction 是否需要 write result 回 RS 或 RF，更新RAT。

做完後清除該 Buffer 所指向的那個 RS。

Output 是每一個 Cycle 最後的結果。

## How to Use?
Sample Input : 
```
ADDI F1,F2,1
SUB F1,F3,F4
DIV F1,F2,F3
MUL F2,F3,F4
ADD F2,F4,F2
ADDI F4,F1,2
MUL F5,F5,F5
ADD F1,F4,F4
exit
```

Sample Output :

過於冗長，這邊不顯示 Sample Output。

## Code Explain

```c++
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <utility>
#include <iomanip>
using namespace std;
```

`include<iostream>`  用來在 Terminal 輸入輸出

`include<string>`   使用 string 的功能

`include<vector>`   儲存 Instruction, RS, RAT, ...等

`include<fstream>`  沒用，忘記刪了

`include<sstream>`  分割 string 用

`include<utility>`  用於 RAT、RF 的 pair<型態，型態>

`include<iomanip>`  用在輸出格式上，以方便閱讀


```c++
//目前正在運行的cycle
static int cycleNo = 0;
```

總 Cycle 數

```c++
/* 每個operator 的cycle數
*
*	+ : 2
*	- : 2
*	* : 10
*	/ : 40
*/
static int needCycle[4] = { 2,2,10,40 };
```

各個 Instruction 執行所需的 cycle 數，由左至右為，ADD SUB MUL DIV。

```c++
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
	char operate;		//運算符號
	string value1;
	string value2;
	int cyclenow;		//進buffer第幾個cycle可以跳出
	int cyclebuffer;	//值都有時，要等一個cycle才可以進buffer，這是存可以進buffer的cycle
};

vector<pair<int, string>> rat;		//Registration Source Table
vector<pair<int, int>> rf;		//Register File
vector<RS> rsADDSUB, rsMULDIV;		//rs for three add or sub, two for mul and div
using RSiterator = RS*;
RSiterator BufferADDSUB, BufferMULDIV;	//when value are all exist, enter to buffer to execute
```

宣告所需要用到的變數和結構

```c++
int main()
{
	vector<Opcode> instruction;				//Instruction Queue 用
	Opcode opInput;						//儲存進Instruction 暫時用
	string input;						//input handler

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
```

建立 RS 所需的內容和 RF 值

```c++
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
```

input Instruction，透過 SplitInstruction() 分割成需要的資訊。

```c++
//將Input分割後存入instruction中
void SplitInstruction(Opcode& opcode, string& input)
{
	stringstream ss(input);

	getline(ss, opcode.name, ' ');
	getline(ss, opcode.rd, ',');
	getline(ss, opcode.rs1, ',');
	getline(ss, opcode.rs2);
}

```

```c++
	int i = 0;

	do
	{
		++cycleNo;

		leaveRS();

		if (instruction.size() > i)
		{
			Issue(instruction, i);
		}

		Execute();

		printCycle();

	} while (!RSEmpty());
```

計算 cycle，呼叫 function 的主要迴圈。

```c++
bool BufferADDSUBEmpty()
{
	if (BufferADDSUB == nullptr)
	{
		return true;
	}
	else
	{
		return !BufferADDSUB->use;
	}
}

bool BufferMULDIVEmpty()
{
	if (BufferMULDIV == nullptr)
	{
		return true;
	}
	else
	{
		return !BufferMULDIV->use;
	}
}

```

判斷 Buffer 是否有指向 RS

```c++
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

			if (BufferADDSUBEmpty())
				cout << endl << "Buffered : empty" << endl << endl;
			else
				cout << endl << "(" << BufferADDSUB->rs << ") " << BufferADDSUB->value1 << " " << BufferADDSUB->operate << " " << BufferADDSUB->value2 << endl << endl;
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

			if (BufferMULDIVEmpty())
				cout << endl << "Buffered : " << endl << endl;
			else
				cout << endl << "(" << BufferMULDIV->rs << ") " << BufferMULDIV->value1 << " " << BufferMULDIV->operate << " " << BufferMULDIV->value2 << endl << endl;
		}
	}
}
```

印出 Cycle 目前的 RAT RF RS 狀態

```c++
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
```

讓程式讀取 Instruction 判斷加減乘除以及進入哪個 ALU

```c++
//Issue Instruction to RS
void inputRS(Opcode& opcode, vector<RS>& rs)
{
	RS temp;
	temp.use = true;
	temp.operate = opcode.operate;
	int rd = opcode.rd[1] - '0';
	int rs1 = opcode.rs1[1] - '0';
	int rs2;

	//判斷rs2是否是純數字或有字母數字
	istringstream convert(opcode.rs2);

	//是數字
	if (convert >> rs2)
		temp.value2 = to_string(rs2);
	else
	{
		//有字母和數字
		rs2 = opcode.rs2[1] - '0';

		//判斷RAT的F有沒有值 for value2
		if (rat[rs2 - 1].second != "")
		{
			temp.value2 = rat[rs2 - 1].second;
		}
		else
		{
			temp.value2 = to_string(rf[rs2 - 1].second);
		}

	}
	// 判斷RAT的F有沒有值 for value1
	if (rat[rs1 - 1].second != "")
	{
		temp.value1 = rat[rs1 - 1].second;
	}
	else
	{
		temp.value1 = to_string(rf[rs1 - 1].second);
	}

	//尋找最小可以放入rs的空間
	for (int i = 0; i < rs.size(); ++i)
	{
		if (!rs[i].use)
		{
			temp.rs = rs[i].rs;
			rs[i] = temp;
			rs[i].use = true;
			rs[i].cyclebuffer = cycleNo + 1;

			rat[rd - 1].second = rs[i].rs;
			break;
		}
	}
}
```

Issue 進 RS 的 Function

```c++
void Execute()
{
	//有ADDSUB、MULDIV 兩個 ALU
	if (BufferADDSUBEmpty())
	{
		int canBuffer = 0;
		int tmp = 0;

		//ADDSUB
		for (int i = 0; i < rsADDSUB.size(); ++i)
		{
			canBuffer = 0;
			tmp = 0;

			//判斷rs1是否是純數字或有字母數字
			istringstream convert1(rsADDSUB[i].value1);

			//是數字
			if (convert1 >> tmp)
				canBuffer++;

			//判斷rs2是否是純數字或有字母數字
			istringstream convert2(rsADDSUB[i].value2);

			//是數字
			if (convert2 >> tmp)
				canBuffer++;

			//兩個數字都有
			if (canBuffer == 2)
			{
				if (rsADDSUB[i].cyclebuffer <= cycleNo)
				{
					BufferADDSUB = &rsADDSUB[i];

					if (BufferADDSUB->operate - '+' == 0)
					{
						//計算跳出的cycle數
						BufferADDSUB->cyclenow = cycleNo + needCycle[0];
					}
					else
					{
						//計算跳出的cycle數
						BufferADDSUB->cyclenow = cycleNo + needCycle[1];
					}

					BufferADDSUB->cyclebuffer = {};
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

			//判斷rs2是否是純數字或有字母數字
			istringstream convert1(rsMULDIV[i].value1);

			//是數字
			if (convert1 >> tmp)
				canBuffer++;

			//判斷rs2是否是純數字或有字母數字
			istringstream convert2(rsMULDIV[i].value2);

			//是數字
			if (convert2 >> tmp)
				canBuffer++;

			//兩個數字都有
			if (canBuffer == 2)
			{
				if (rsMULDIV[i].cyclebuffer <= cycleNo)
				{
					BufferMULDIV = &rsMULDIV[i];

					if (BufferMULDIV->operate - '*' == 0)
					{
						//計算跳出的cycle數
						BufferMULDIV->cyclenow = cycleNo + needCycle[2];
					}
					else
					{
						//計算跳出的cycle數
						BufferMULDIV->cyclenow = cycleNo + needCycle[3];
					}

					BufferMULDIV->cyclebuffer = {};
				}


			}
		}
	}
}
```

判斷RS是否有Instruction能進入Buffer，是的話進入Buffer且開始計算幾個cycle後執行完畢。

```c++
//Instruction離開RS時，要write result回RF 跟 找RS與RAT相符的代號
void leaveRS()
{
	if (!BufferADDSUBEmpty())
	{
		//判斷Buffer內是否cycle已經跑完
		if (BufferADDSUB->cyclenow == cycleNo)
		{
			//跑完，要讓RS清空
			BufferADDSUB->use = false;
			int result = Arithmetic(*BufferADDSUB);;

			//先找RAT有無對應的值，有要改RF
			for (int i = 0; i < rat.size(); ++i)
			{
				if (rat[i].second == BufferADDSUB->rs)
				{
					rf[rat[i].first - 1].second = result;
					rat[i].second = "";
					break;
				}
			}

			//看RS內其他運算有無需要
			for (int i = 0; i < rsADDSUB.size(); ++i)
			{
				if (rsADDSUB[i].value1 == BufferADDSUB->rs)
				{
					rsADDSUB[i].value1 = to_string(result);
					rsADDSUB[i].cyclebuffer = cycleNo + 1;
				}

				if (rsADDSUB[i].value2 == BufferADDSUB->rs)
				{
					rsADDSUB[i].value2 = to_string(result);
					rsADDSUB[i].cyclebuffer = cycleNo + 1;
				}
			}

			for (int i = 0; i < rsMULDIV.size(); ++i)
			{
				if (rsMULDIV[i].value1 == BufferADDSUB->rs)
				{
					rsMULDIV[i].value1 = to_string(result);
					rsMULDIV[i].cyclebuffer = cycleNo + 1;
				}

				if (rsMULDIV[i].value2 == BufferADDSUB->rs)
				{
					rsMULDIV[i].value2 = to_string(result);
					rsMULDIV[i].cyclebuffer = cycleNo + 1;
				}
			}

			//唉呦 要清跑完的那個RS拉
			BufferADDSUB->cyclenow = 0;
			BufferADDSUB->operate = '\0';
			BufferADDSUB->value1 = {};
			BufferADDSUB->value2 = {};
			BufferADDSUB = nullptr;
		}
	}

	if (!BufferMULDIVEmpty())
	{
		//判斷Buffer內是否cycle已經跑完
		if (BufferMULDIV->cyclenow == cycleNo)
		{
			//跑完，要讓RS清空
			BufferMULDIV->use = false;
			int result = Arithmetic(*BufferMULDIV);

			//先找RAT有無對應的值，有要改RF
			for (int i = 0; i < rat.size(); ++i)
			{
				if (rat[i].second == BufferMULDIV->rs)
				{
					rf[rat[i].first - 1].second = result;
					rat[i].second = "";
					break;
				}
			}

			//看RS內其他運算有無需要
			for (int i = 0; i < rsADDSUB.size(); ++i)
			{
				if (rsADDSUB[i].value1 == BufferMULDIV->rs)
				{
					rsADDSUB[i].value1 = to_string(result);
					rsADDSUB[i].cyclebuffer = cycleNo + 1;
				}

				if (rsADDSUB[i].value2 == BufferMULDIV->rs)
				{
					rsADDSUB[i].value2 = to_string(result);
					rsADDSUB[i].cyclebuffer = cycleNo + 1;
				}
			}

			for (int i = 0; i < rsMULDIV.size(); ++i)
			{
				if (rsMULDIV[i].value1 == BufferMULDIV->rs)
				{
					rsMULDIV[i].value1 = to_string(result);
					rsMULDIV[i].cyclebuffer = cycleNo + 1;
				}

				if (rsMULDIV[i].value2 == BufferMULDIV->rs)
				{
					rsMULDIV[i].value2 = to_string(result);
					rsMULDIV[i].cyclebuffer = cycleNo + 1;
				}
			}

			//唉呦 要清跑完的那個RS拉
			BufferMULDIV->cyclenow = 0;
			BufferMULDIV->operate = '\0';
			BufferMULDIV->value1 = {};
			BufferMULDIV->value2 = {};
			BufferMULDIV = nullptr;
		}
	}
}
```

Buffer內的Instruction要write result回RF跟RS的Function

如果Rat有對應的RS，傳回RF，RAT清空。

尋找ALU內是否有需要的RS，有的話改成計算完的值。

最後清空RS跟Buffer的位置出來。

```c++
//四則運算
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
```

實際運算 Instruction 執行的結果

```c++
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
```

判斷 RS 狀態的 Function
