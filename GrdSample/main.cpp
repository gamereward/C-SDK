#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <cmath>
#include <time.h>
#include "GRD/GrdManager.h"

using namespace std;
const std::string scoreType = "GAME1_SCORE_TYPE";
#ifdef LINUX
void localtime_s(struct tm * _Tm, const time_t * _Time){
	localtime_r(_Time,_Tm);
}
void clearScreen(){
	system("clear");
}
void pauseScreen(){
	cout << "Enter to continue...";
	cin.ignore().get(); 
}
#else
void clearScreen(){
	system("cls");
}
void pauseScreen(){
	system("pause");
}
#endif
std::string username;
std::string userSelect = "";
void printError(const Grd::GrdResultBase &error){
	std::string input;
	cout << ("ERROR:" + to_string(error.error) + ",MESSAGE:" + error.message + "\r\n");
	pauseScreen();
}
std::string formatField(std::string st,int len){
	const std::string sfixed = "                                                                                                                       ";
	if (st.length() < len){
		st = sfixed.substr(sfixed.length() - len+st.length()) + st;
	}
	return st;
}
/*LEADER BOARD*/

void game_leaderboard(string title,string scoreType){
	cout << title << "\r\n";
	cout << "*********************************************************\r\n";
	Grd::GrdResult<std::vector<Grd::GrdLeaderBoard>>leaderBoard = Grd::GrdManager::getInstance()->getLeaderBoard(username, scoreType, 0, 20);
	if (!leaderBoard.error){
		cout << "+-RANK---+----NAME------------------------------------+---SCORE--+\r\n";
		for (std::vector<Grd::GrdLeaderBoard>::iterator it = leaderBoard.data.begin(); it != leaderBoard.data.end(); it++){
			Grd::GrdLeaderBoard l = *it;
			cout << "|" + formatField(to_string(l.rank), 8) << "|" + formatField(l.username, 44) << "|" + formatField(to_string(l.score), 10) + "|\r\n";
			cout << "+--------+--------------------------------------------+----------+\r\n";
		}
		pauseScreen();
	}
	else{
		printError(leaderBoard);
	}
}
/*ACCOUNT API TEST*/
void transfer(){
	clearScreen();
	std::string to;
	std::string svalue;
	BigDecimal value;
	cout << "TRANSFER MONEY\r\n";
	cout << "-----------------------------------\r\n";
	cout << "TO ADDRESS:";
	cin >> to;
	if (to.size() == 0){
		cout << "INVALID ADDRESS!";
		pauseScreen();
		return;
	}
	do{
		cout << "AMOUNT:";
		cin >> svalue;
		try{
			value = svalue;
		}
		catch (exception e){
		}
		if (value <= 0){
			cout << "The money need to be greater than 0.\r\n";
		}
		else{
			break;
		}
	} while (true);
	cout << "***************WARNING*************\r\n";
	cout << "IT IS REAL MONEY!\r\n";
	cout << "This action will transfer money from this account to " + to + "\r\n";
	cout << "***********************************\r\n";
	cout << "Please confirm this action (YES to confirm,other to cancel):";
	cin >> svalue;
	if (svalue == "YES"){
		Grd::GrdResultBase result=Grd::GrdManager::getInstance()->transfer(username, to, value);
		if (result.error){
			printError(result);
			return;
		}
		else{
			cout << "TRANSFER SUCCESSFULLY!";
			pauseScreen();
			return;
		}
	}
}
void chargeMoney(){
	cout << "CHARGE MONEY\r\n";
	std::string svalue;
	BigDecimal value;
	cout << "-----------------------------------\r\n";
	do{
		cout << "AMOUNT TO CHARGE:";
		cin >> svalue;
		try{
			value = svalue;
		}
		catch (exception e){
		}
		if (value <= 0){
			cout << "The money value need to be greater than 0.\r\n";
		}
		else{
			break;
		}
	} while (true);
	Grd::GrdResultBase result = Grd::GrdManager::getInstance()->chargeMoney(username, value);
	if (result.error){
		printError(result);
	}
	else{
		cout << "CHARGE SUCCESSFULLY!";
		pauseScreen();
	}
}

void payMoney(){
	cout << "PAY MONEY\r\n";
	std::string svalue;
	BigDecimal value;
	cout << "-----------------------------------\r\n";
	do{
		cout << "AMOUNT TO PAY TO USER:";
		cin >> svalue;
		try{
			value = svalue;
		}
		catch (exception e){
		}
		if (value <= 0){
			cout << "The money value need to be greater than 0.\r\n";
		}
		else{
			break;
		}
	} while (true);
	//Pay money value need to be < 0
	value = -value.toDouble();
	Grd::GrdResultBase result = Grd::GrdManager::getInstance()->chargeMoney(username, value);
	if (result.error){
		printError(result);
	}
	else{
		cout << "PAY SUCCESSFULLY!";
		pauseScreen();
	}
}
void listTransactions(){
	int pageSize = 10;
	int pageIndex = 0;
	Grd::GrdResult<int> resultCount = Grd::GrdManager::getInstance()->getTransactionCount(username);
	if (resultCount.error){
		printError(resultCount);
	}
	else{
		do{
			clearScreen();
			cout << "TRANSACTIONS\r\n";
			cout << "*********************************************************\r\n";
			Grd::GrdResult<std::vector<Grd::Transaction>>trans = Grd::GrdManager::getInstance()->getTransactions(username, pageIndex*pageSize, pageSize);
			if (!trans.error){
				for (std::vector<Grd::Transaction>::iterator it = trans.data.begin(); it != trans.data.end(); it++){
					Grd::Transaction tran = *it;
					time_t start = tran.transdate;
					struct tm tm;

					localtime_s(&tm, &start);
					char date[20];
					strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &tm);
					cout << "------------------------------------------------------------\r\n";
					cout << "tx:" << tran.tx << "\r\n";
					cout << "time:" << date << "\r\n";
					cout << "from:" << tran.from << "\r\n";
					cout << "to:" << tran.to << "\r\n";
					cout << "amount:" << tran.amount.toString() << "\r\n";
					cout << "type:" << (tran.transtype == Grd::TransactionType::INTERNAL_TRANS ? "Internal" : "External") << "\r\n";
					cout << "status:" << (tran.status == Grd::TransactionStatus::SUCCESS_TRANS ? "Success" : (tran.status == Grd::TransactionStatus::PENDING_TRANS ? "Pending" : "Error")) << "\r\n";
					cout << "------------------------------------------------------------\r\n";
				}

				cout << "*********************************************************\r\n";
				int pageCount = (int)ceil((double)resultCount.data / pageSize);
				cout << "Page:" << (pageIndex + 1) << "/" << pageCount << "| Next:1-Prev:2-Exit:10\r\n";
				cout << "YOUR CHOISE:";
				cin >> userSelect;
				if (strcmp(userSelect.c_str(), "2") == 0){
					if (pageIndex > 0){
						pageIndex--;
					}
				}
				else if (strcmp(userSelect.c_str(), "1") == 0){
					if (pageIndex < pageCount - 1){
						pageIndex++;
					}
				}
				else if (strcmp(userSelect.c_str(), "10") == 0){
					return;
				}
			}
			else{
				printError(trans);
			}

		} while (true);
	}
}
void accountInfo(){
	while (true)
	{
		clearScreen();
		Grd::GrdResult<Grd::AccountInfo> account = Grd::GrdManager::getInstance()->accountbalance(username);
		cout << "ACCOUNT INFORMATION\r\n";
		cout << "-----------------------------------\r\n";
		if (account.error){
			printError(account);
		}
		else{
			cout << "Username:" + username + "\r\n";
			cout << "Wallet address:" + account.data.address + "\r\n";
			cout << "Balance:" + account.data.balance.toString() + "\r\n";
		}
		cout << "-----------------------------------\r\n";
		cout << "TO DO\r\n";
		cout << "1.Transfer money to other address (use for user).\r\n";
		cout << "2.Charge from this account (use for game action).\r\n";
		cout << "3.Pay money to this account(use for game action).\r\n";
		cout << "4.List transactions\r\n";
		cout << "5.Refresh.\r\n";
		cout << "10.Go back...\r\n";
		cout << "-----------------------------------\r\n";
		cout << "YOUR CHOISE:";
		cin >> userSelect;
		if (std::strcmp(userSelect.c_str(), "1") == 0)
		{
			transfer();
		}
		else if (std::strcmp(userSelect.c_str(), "2") == 0){
			chargeMoney();
		}
		else if (std::strcmp(userSelect.c_str(), "3") == 0){
			payMoney();
		}
		else if (std::strcmp(userSelect.c_str(), "4") == 0){
			listTransactions();
		}
		else if (std::strcmp(userSelect.c_str(), "10") == 0){
			return;
		}
	}
}
/*END ACCOUNT TEST*/
/*SCRIPT SERVER API*/

void random09_history(){
	clearScreen();
	cout << "RANDOM 1-9 HISTORY\r\n";
	cout << "*********************************************************\r\n";
	Grd::GrdResult<std::vector<Grd::SessionData>>result = Grd::GrdManager::getInstance()->getUserSessionData(username, "GAME-9","rand", 0, 20);
	if (!result.error){
		cout << "+-TIME---------------+---SELECT--+----RESULT----+-----MONEY-----+\r\n";
		for (std::vector<Grd::SessionData>::iterator it = result.data.begin(); it != result.data.end(); it++){
			Grd::SessionData dt = *it;
			std::map<std::string,std::string>::iterator itValue = dt.values.find("rand");//Contains the rand key
			if (itValue != dt.values.end()){
				std::string value = itValue->second;
				char times[20];
				string yourNumber,randNumber,money;
				time_t t = dt.sessionstart;
				tm tm;
				localtime_s(&tm, &t);					
				strftime(times, sizeof(times), "%Y-%m-%d %H:%M:%S", &tm);
				int ipos = value.find(",");
				if (ipos > 0){
					yourNumber = value.substr(0, ipos);
					value = value.substr(ipos + 1);
				}
				ipos = value.find(",");
				if (ipos > 0){
					randNumber = value.substr(0, ipos);
					value = value.substr(ipos + 1);
				}
				money = value;
				cout << "|" + formatField(string(times), 20) << "|" + formatField(yourNumber, 11) << "|" + formatField(randNumber, 14) << "|" + formatField(money, 15) + "|\r\n";
				cout << "+--------+--------------------------------------------+----------+\r\n";
			}
		}
		pauseScreen();
	}
	else{
		printError(result);
	}
}
void random09Game(){
	std::string svalue;
	int number;
	BigDecimal value;
	cout << "RANDOM 1-9 GAME\r\n";
	while (true)
	{
		do{
			clearScreen();		
			cout << "-----------------------------------\r\n";
			cout << "1-9:YOUR LUCKY NUMBER.\r\n";
			cout << "10. LEADER BOARD.\r\n";
			cout << "11. HISTORY.\r\n";
			cout << "100. EXIT.\r\n";
			cout << "-----------------------------------\r\n";
			cout << "SELECT:";
			cin >> svalue;
			number = Grd::stoi(svalue);
			if ( number>=1&& number<=9)
			{
				break;
			}
			if (number==10){
				clearScreen();
				game_leaderboard("RANDOM 1-9 GAME LEADER BOARD","random9_score");
			}
			if (number==11){
				random09_history();
			}
			if (number==100){
				return;
			}
		} while (true);
		do{
			cout << "BET:";
			cin >> svalue;
			try{
				value = svalue;
				if (value > 0){
					break;
				}
			}
			catch (exception e){
			}
			cout << "Bet must be greater than 0!";
		} while (true);
		vector<std::string>paras;
		paras.push_back(to_string(number));
		paras.push_back(value.toString());
		Grd::GrdResult< JSONValue*>result = Grd::GrdManager::getInstance()->callServerScript(username, "testscript", "random9", paras);
		if (result.error){
			printError(result);
		}
		else{
			//Server response an array
			double results[4];
			size_t index = 0;
			JSONValue*json = result.data;
			results[0] = json->Child(index++)->AsNumber();
			if (results[0] == 0){
				results[1] = json->Child(index++)->AsNumber();
				results[2] = json->Child(index++)->AsNumber();
				results[3] = json->Child(index++)->AsNumber();
				cout << "SELECT:" << results[2] << ",RESULT:" << results[1] << "\r\n";
				cout << (results[3] > 0 ? "WIN:" : "LOSE:") << results[3] << "\r\n";
			}
			else{
				cout << json->Child(index++)->AsString();//Message in game
			}
			pauseScreen();
		}
	}
}

void lowhighgame_history(){
	clearScreen();
	cout << "LOW-HIGH GAME HISTORY\r\n";
	cout << "*********************************************************\r\n";
	Grd::GrdResult<std::vector<Grd::SessionData>>result = Grd::GrdManager::getInstance()->getUserSessionData(username, "LOWHIGHGAME", "result", 0, 20);
	if (!result.error){
		cout << "+-TIME---------------+-----CARD-----+---SELECT--+----RESULT----+-----MONEY-----+\r\n";
		for (std::vector<Grd::SessionData>::iterator it = result.data.begin(); it != result.data.end(); it++){
			Grd::SessionData dt = *it;
			std::map<std::string, std::string>::iterator itValue = dt.values.find("result");//Contains the rand key
			if (itValue != dt.values.end()){
				std::string value = itValue->second;
				char times[20];
				bool islow;
				double yourNumber, randNumber, money;
				time_t t = dt.sessionstart;
				tm tm;
				localtime_s(&tm, &t);
				strftime(times, sizeof(times), "%Y-%m-%d %H:%M:%S", &tm);
				//Read the array result
				JSONValue*jValue= JSON::Parse(value.c_str());
				islow = (int)jValue->Child(0)->AsNumber() == 1;
				yourNumber = (int)jValue->Child(1)->AsNumber();
				randNumber = (int)jValue->Child(2)->AsNumber();
				money = jValue->Child(3)->AsNumber();
				//
				cout << "|" + formatField(string(times), 20) << "|" + formatField(to_string(yourNumber), 14) << "|" + formatField((islow?"LOW":"HIGH"), 11) << "|" + formatField(to_string(randNumber), 14) << "|" + formatField(to_string(money), 15) + "|\r\n";
				cout << "+--------------------+--------------+-----------+--------------+---------------+\r\n";
			}
		}
		pauseScreen();
	}
	else{
		printError(result);
	}
}
void highlowgame(){
	std::string svalue;
	double number;
	BigDecimal value;
	bool islow = false;
	const int LOW = 3;
	const int HIGH = 13;
	while (true)
	{
		clearScreen();
		cout << "HIGH-LOW GAME\r\n";
		cout << "-----------------------------------\r\n";
		time_t seconds = time(nullptr);
		srand((unsigned int)seconds);
		number = rand() % (HIGH - LOW) + LOW;
		cout << "1. LOW: 2 To " << number << "(Bet Rate:" + to_string((14 - number) / (number - 2)) + "/1)\r\n";
		cout << "2. HIGH: " << number << " To 14(Bet Rate:" + to_string((number - 2) / (14 - number)) + "/1)\r\n";
		cout << "3. RANDOM NEXT\r\n";
		cout << "4. LEADER BOARD\r\n";
		cout << "5. HISTORY\r\n";
		cout << "10. EXIT...\r\n";
		cout << "SELECT:";
		cin >> svalue;
		if ((std::strcmp(svalue.c_str(), "1") == 0) || (std::strcmp(svalue.c_str(), "2") == 0))
		{
			islow = std::strcmp(svalue.c_str(), "1") == 0;
		}
		else if (strcmp("4", svalue.c_str()) == 0){
			clearScreen();
			game_leaderboard("LOW HIGH GAME LEADER BOARD", "lowhighgame_score");
			continue;
		}
		else if (strcmp("5", svalue.c_str()) == 0){
			clearScreen();
			lowhighgame_history();
			continue;
		}
		else if (strcmp("10", svalue.c_str()) == 0){
			return;
		}
		else{
			continue;
		}
		do{
			cout << "BET:";
			cin >> svalue;
			try{
				value = svalue;
				if (value > 0){
					break;
				}
			}
			catch (exception e){
			}
			cout << "Bet must be greater than 0!";
		} while (true);
		vector<std::string>paras;
		paras.push_back((islow ? "1" : "0"));
		paras.push_back(to_string(number));
		paras.push_back(value.toString());
		Grd::GrdResult<JSONValue*>result = Grd::GrdManager::getInstance()->callServerScript(username, "testscript", "lowhighgame", paras);
		if (result.error){
			printError(result);
		}
		else{
			//Server response an array
			double results[4];
			JSONValue*json = result.data;
			results[0] = json->Child(0)->AsNumber();
			if (results[0] == 0){
				results[1] = json->Child(1)->Child("symbol")->AsNumber();
				results[2] = json->Child(1)->Child("suit")->AsNumber();
				results[3] = json->Child(2)->AsNumber();
				cout << "NUMBER:" << number << ",SELECT:" << (islow ? "LOW" : "HIGH") << ",RESULT:" << results[1] << "\r\n";
				cout << (results[3] > 0 ? "WIN:" : "LOSE:") << results[3] << "\r\n";
			}
			else{
				cout << json->Child(2)->AsString();//Message in game
			}
			delete json;
			pauseScreen();
		}
	}
}
void scriptServerMenu(){
	do{
		clearScreen();
		cout << "SCRIPT SERVER\r\n";
		cout << "-----------------------------------\r\n";
		cout << "1. RANDOM 1-9 GAME.\r\n";
		cout << "2. HIGH LOW GAME.\r\n";
		cout << "10. Exit.\r\n";
		cout << "-----------------------------------\r\n";
		cout << "SELECT:";
		cin >> userSelect;
		if (std::strcmp(userSelect.c_str(), "1") == 0)
		{
			random09Game();
		}
		else if (std::strcmp(userSelect.c_str(), "2") == 0)
		{
			highlowgame();
		}
		else if (std::strcmp(userSelect.c_str(), "10") == 0)
		{
			break;
		}
	} while (true);
}
void setScore(){
	std::string value;
	BigDecimal score;
	cout << "-----------------------------------\r\n";
	cout << "SET USER SCORE:";
	cin >> value;
	score = value;
	Grd::GrdResultBase result= Grd::GrdManager::getInstance()->saveUserScore(username, scoreType, score.toDouble());
	if (result.error){
		printError(result);
	}
	else{
		cout << "SAVE SCORE SUCCESSFULLY!";
		pauseScreen();
	}
}
void increaseScore(){
	std::string value;
	BigDecimal score;
	cout << "-----------------------------------\r\n";
	cout << "INCREASE USER SCORE:";
	cin >> value;
	score = value;
	Grd::GrdResultBase result = Grd::GrdManager::getInstance()->increaseUserScore(username, scoreType, score.toDouble());
	if (result.error){
		printError(result);
	}
	else{
		cout << "INCREASE SCORE SUCCESSFULLY!";
		pauseScreen();
	}
}

void ScoreApiTest(){
	while (true)
	{
		Grd::GrdResult<Grd::GrdLeaderBoard>userScore=	Grd::GrdManager::getInstance()->getUserScoreRank(username, scoreType);
		clearScreen();
		cout << "SCORE API TEST\r\n";
		cout << "Score type test:" + scoreType + "\r\n";
		cout << "User score:" << userScore.data.score << "\r\n";
		cout << "User rank:" << userScore.data.rank << "\r\n";
		cout << "-----------------------------------\r\n";
		cout << "1. Set score\r\n";
		cout << "2. Increase score\r\n";
		cout << "3. Leader board\r\n";
		cout << "10.Exit.\r\n";
		cout << "-----------------------------------\r\n";
		cout << "YOUR CHOISE:";
		cin >> userSelect;
		if (std::strcmp(userSelect.c_str(), "1") == 0)
		{
			setScore();
		}
		else if (std::strcmp(userSelect.c_str(), "2") == 0)
		{
			increaseScore();
		}
		else if (std::strcmp(userSelect.c_str(), "3") == 0)
		{
			game_leaderboard("GAME LEADER BOARD SCORETYPE:"+scoreType, scoreType);
		}
		else if (std::strcmp(userSelect.c_str(), "10") == 0)
		{
			return;
		}
	}
}
void testMenu(){
	while (true)
	{
		clearScreen();
		cout << "SELECT MENU\r\n";
		cout << "-----------------------------------\r\n";
		cout << "1.Account API.\r\n";
		cout << "2.Scores API.\r\n";
		cout << "3.Script Server-OAPI.\r\n";
		cout << "10.Exit.\r\n";
		cout << "-----------------------------------\r\n";
		cout << "YOUR CHOISE:";
		cin >> userSelect;
		if (std::strcmp(userSelect.c_str(), "1") == 0)
		{
			accountInfo();
		}
		else if (std::strcmp(userSelect.c_str(), "2") == 0)
		{
			ScoreApiTest();
		}
		else if (std::strcmp(userSelect.c_str(), "3") == 0)
		{
			scriptServerMenu();
		}
		else if (std::strcmp(userSelect.c_str(), "10") == 0)
		{
			return;
		}
	}
}
int main(int argc, char **argv)
{
	const std::string appId = "6e672e888487bd8346b946a715c74890077dc332";
	const std::string secret = "acc3e0404646c57502b480dc052c4fe15878a7ab84fb43402106c575658472faf7e9050c92a851b0016442ab604b0488aab3e67537fcfda3650ad6cfd43f7974";
	Grd::GrdManager::init(appId, secret, Grd::GrdNet::TEST_NET);
	clearScreen();
	cout << "USER NAME:";
	char buffer[100];
	cin >> buffer;
	username = buffer;
	if (username.size() == 0){
		return 0;
	}
	Grd::GrdManager::getInstance()->accountbalance(username);//create user if is new user
	testMenu();
}