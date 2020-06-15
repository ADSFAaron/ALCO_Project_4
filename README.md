# ALCO_Project_4
Homework for Assembled Language and Computer Organization Project Three

## Project Goal
可以將輸入的一段 RISC-V 組合語言的 code 將 branch 的 instruction 做 Prediction

Input : 一段 RISC-V 組合語言的 Code (需寫在test.txt檔裡面)， entry 數

Output : entry 以及 Prediction 的 state 轉換，預測的狀態及實際狀態，每個 entry misprediction 的次數。

## Requirement
:page_facing_up: 需先行建立一個 RISC V Instruction code 的檔案，並且檔名為 `test.txt` ，並且將 test.txt 儲存於與專案同個資料夾。

:warning: RISC V Instruction code 最後結束的 Label 必須是 `End`。

## Project Method
透過讀檔，將RISC-V code讀入程式中。

實際做每一行RISC-V的實際運行內容。

在有 beq 的 function 中執行 Prediction 的程式。

Prediction 處理 state 的轉換，判斷預測結果是否跟真實結果相符，以及 Output。

在 RISC-V 的程式跳到 END 後即結束。

## How to Use?
Sample Input : 
```
	li R1,0 //等同addi R1,R0,0
	li R2,4
Loop:
	beq R1,R2,End
	addi R2,R2,-1
	beq R0,R0,Loop //R0就是我們常用的x0唷
End:
```

Sample Output :
```
//input: number of entries
8

entry: 2        beq R1,R2,End           //beq R1,R2,End 使用編號2的entry
(00, SN, SN, SN, SN) N N                misprediction: 0
//狀態            預測值 實際值            本預測器miss次數 (從頭統計至今)
entry: 4        beq R0,R0,Loop
(00, SN, SN, SN, SN) N T                misprediction: 1

entry: 2        beq R1,R2,End
(00, SN, SN, SN, SN) N N                misprediction: 0

entry: 4        beq R0,R0,Loop
(01, WN, SN, SN, SN) N T                misprediction: 2

entry: 2        beq R1,R2,End
(00, SN, SN, SN, SN) N N                misprediction: 0

entry: 4        beq R0,R0,Loop
(11, WN, WN, SN, SN) N T                misprediction: 3

entry: 2        beq R1,R2,End
(00, SN, SN, SN, SN) N N                misprediction: 0

entry: 4        beq R0,R0,Loop
(11, WN, WN, SN, WN) N T                misprediction: 4

entry: 2        beq R1,R2,End
(00, SN, SN, SN, SN) N T                misprediction: 1
```

## Code Explain

```c++
#include<iostream>
#include<iomanip>
#include<fstream>
#include<sstream>
#include<string>
#include<vector>
using namespace std;
```

`include<iostream>`  用來在Terminal輸入輸出

`include<iomanip>`   用在輸出格式上，以方便觀看

`include<fstream>`  讀取檔案

`include<sstream>`  分割string用

`include<string>`  用來使用string的 operator[]、find 等功能

`include<vector>`  儲存所有Opreator 和輸入的Instruction的空間


```c++
while (getline(infile, input))
{
	//label存在，將label名字和行數儲存
	if (label_detected(input))
	{
		tmp.name = input;
		tmp.pos = row;
		Label.push_back(tmp);
	}
	else
	{
		stringstream ss(input);

		//分割instruction name
		getline(ss, file.name, ' ');

		if (file.name == "li")
		{
			getline(ss, file.reg1, ',');
			getline(ss, file.imm, '\n');
		}
		else if (file.name == "addi" || file.name == "beq")
		{
			getline(ss, file.reg1, ',');
			getline(ss, file.reg2, ',');
			getline(ss, file.imm, '\n');
		}
		else
		{
			cout << "error infile \n\n";
			system("pause");
		}
		++row;

		//將Instruction儲存到vector中
		implement.push_back(file);
	}
}
```

讀檔，將檔案的 RISC-V CODE 傳到C++ CODE，並分別不同的 instruction 到對應的 struct。

```c++
while (rowPos > -1)
{
	//判斷是哪個instruction
	if (implement[rowPos].name == "li")
	{
		li(implement[rowPos].reg1, implement[rowPos].imm);
	}
	else if (implement[rowPos].name == "addi")
	{
		addi(implement[rowPos].reg1, implement[rowPos].reg2, implement[rowPos].imm);
	}
	else if (implement[rowPos].name == "beq")
	{
		beq(implement[rowPos].reg1, implement[rowPos].reg2, implement[rowPos].imm);
	}

	++rowPos;
}
```

用rowPos判斷，從第一行開始跑，判斷 instruction ，並跑到對應的 function

這邊以 beq 舉例

```c++
void beq(string rs1, string rs2, string label)
{
	//將 rs1 去掉第一個字，取數字
	int r1 = atoi(rs1.substr(1, rs1.length() - 1).c_str());
	int r2 = atoi(rs2.substr(1, rs2.length() - 1).c_str());

	//呼叫下方
	prediction(r1, r2, label);

	//判斷是否是結束label
	if (reg[r1] == reg[r2] && label == "End")
	{
		rowPos = -2;
		return;
	}

	//if(r1 == r2) 
	//	jump to Label
	for (int i = 0; i < Label.size(); ++i)
	{
		//判斷到對應的 label 並跳至指定行數
		if (reg[r1] == reg[r2] && label == Label[i].name)
		{
			//避免 function 跑完還有 +1 造成行數的錯誤
			rowPos = Label[i].pos - 1;
		}
	}
}
```

function 內即為 instruction 實際運行的 C++ Code。

```c++
void prediction(int rs1, int rs2, string label)
{
	string nowstate;	//判斷目前在哪個state
	int statenum;		//判斷第幾個state
	string result = "T";	//state實際結果
	int beqCount = 0;	//判斷是跑which beq

	//找instruction中所有beq
	for (int i = 0; i < implement.size(); ++i)
	{
		if (implement[i].name == "beq")
		{
			pred[beqCount].entry = i % entry;
			pred[beqCount].label = implement[i].imm;
			pred[beqCount].rs1 = implement[i].reg1;
			pred[beqCount].rs2 = implement[i].reg2;
			++beqCount;
		}
	}

	int predtime = 0;

	//找出所有beq
	for (int i = 0; i < beqCount; ++i)
	{
		if (pred[i].label == label)
		{
			predtime = pred[i].entry;

			//印出beq對應的entry
			cout << "entry: " << predtime << setw(7) << "beq " << pred[i].rs1 << "," << pred[i].rs2 << "," << label << endl;
			break;
		}
	}

	//判斷 2-bit history 為哪個
	if (State[predtime].his[0] == "N")
		State[predtime].pred1 = 0;
	else
		State[predtime].pred1 = 1;

	if (State[predtime].his[1] == "N")
		State[predtime].pred2 = 0;
	else
		State[predtime].pred2 = 1;

	//按照2-bit history設定目前state
	if (State[predtime].pred2 == 0 && State[predtime].pred1 == 0)
	{
		nowstate = "00";
		statenum = 0;
	}
	else if (State[predtime].pred2 == 0 && State[predtime].pred1 == 1)
	{
		nowstate = "01";
		statenum = 1;
	}
	else if (State[predtime].pred2 == 1 && State[predtime].pred1 == 0)
	{
		nowstate = "10";
		statenum = 2;
	}
	else
	{
		nowstate = "11";
		statenum = 3;
	}

	// 預測是N or T
	if (State[predtime].state[statenum] == "SN" || State[predtime].state[statenum] == "WN")
		State[predtime].pred = "N";
	else
		State[predtime].pred = "T";

	// 輸出結果
	cout << "(" << nowstate << ", " << State[predtime].state[0] << ", " << State[predtime].state[1] << ", " << State[predtime].state[2] << ", " << State[predtime].state[3] << ") "
		<< State[predtime].pred << " ";

	// 實際beq結果
	if (reg[rs1] == reg[rs2])
	{
		result = "T";

		// 改變state裡的狀態
		if (result == State[predtime].pred)
		{
			if (State[predtime].state[statenum] == "WT")
				State[predtime].state[statenum] = "ST";
		}
		else
		{
			if (State[predtime].state[statenum] == "SN")
				State[predtime].state[statenum] = "WN";
			else if (State[predtime].state[statenum] == "WN")
				State[predtime].state[statenum] = "WT";
			State[predtime].miss++;
		}
	}
	else
	{
		result = "N";

		//改變state狀態
		if (result == State[predtime].pred)
		{
			if (State[predtime].state[statenum] == "WN")
				State[predtime].state[statenum] = "SN";
		}
		else
		{
			if (State[predtime].state[statenum] == "ST")
				State[predtime].state[statenum] = "WT";
			else if (State[predtime].state[statenum] == "WT")
				State[predtime].state[statenum] = "WN";

			State[predtime].miss++;
		}
	}

	cout << result << setw(18) << "misprediction: " << State[predtime].miss << endl << endl;

	State[predtime].his[1] = State[predtime].his[0];
	State[predtime].his[0] = result;
}

```

Prediction 首先判斷是否有足夠的 entry 給每一個branch predictor，沒有則用同一個

預設原本的 2-bit history 為 00，後面的則依之後的實際結果作為 2-bit history

預測結果為 2-bit history 對應到的 state，與實際結果比較後，計算是否 misprediction

最後 State 依照預測成功 or 失敗的結果，改變那個 entry 的 Predictor state
