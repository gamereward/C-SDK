#ifndef __GAME_REWARD__
#define __GAME_REWARD__
#include <vector>
#include <map>
#include <string>
#include <functional>
#include "BigDecimal.h"
#include "JSON.h"
namespace Grd{
	typedef class GrdManager;
	typedef class GrdLeaderBoard;
	typedef class SessionData;
	typedef class Transaction;
	enum TransactionStatus
	{
		PENDING_TRANS = 0,
		SUCCESS_TRANS = 1,
		ERROR_TRANS = 2
	};
	enum TransactionType
	{
		BASE_TRANS = 1,
		INTERNAL_TRANS = 2,
		EXTERNAL_TRANS = 3
	};
	inline static long stol(std::string value)
	{
		BigDecimal d = value;
		return d.toLongLong();
	}
	inline static long stod(std::string value)
	{
		BigDecimal d = value;
		return d.toDouble();
	}
	inline static int stoi(std::string value){
		BigDecimal d = value;
		return d.toInt();
	}
	class AccountInfo{
	public:
		std::string address;
		BigDecimal balance;
	};
	class GrdResultBase 
	{
	public:
		int error = 0;
		std::string message;
		GrdResultBase(){

		};
		GrdResultBase(int error, std::string message) 
		{

		};
	};
	template<typename T>
	class GrdResult : public GrdResultBase
	{
	public:
		T data;
		GrdResult(){

		}
		GrdResult(int error, std::string message, T data): GrdResultBase(error,message){
			this->data = data;
		}
	private:

	};
	
	class Transaction{
	public:
		Transaction(){

		}
		Transaction(JSONValue* json){
			if (json){
				tx = json->Child("tx")->AsString();
				from = json->Child("from")->AsString();
				to = json->Child("to")->AsString();
				amount = json->Child("amount")->AsNumber();
				status = static_cast<TransactionStatus>((int)json->Child("status")->AsNumber());
				transtype = static_cast<TransactionType>((int)json->Child("transtype")->AsNumber());
				long t = Grd::stol(json->Child("transdate")->AsString());
				transdate = t;
			}
		}
		std::string tx;
		std::string from;
		std::string to;
		BigDecimal amount;
		TransactionStatus status;
		TransactionType transtype;
		long transdate;
	};
	class SessionData{
	public:
		SessionData(){

		}
		SessionData(JSONValue * json){
			if (!json)return;
			JSONValue*value = json->Child("sessionid");
			if (value){
				sessionid = (long)(value->AsNumber());
			}
			value = json->Child("sessionstart");
			if (value){
				sessionstart = (long)(value->AsNumber());
			}
			value = json->Child("values");
			if (value){
				std::vector<std::string> keys = value->ObjectKeys();
				for (std::vector<std::string>::iterator key = keys.begin(); key != keys.end(); key++){
					std::string skey = *key;
					std::string svalue = value->Child(skey)->AsString();
					values.insert(std::pair<std::string, std::string>(skey, svalue));
				}
			}
		};
		long sessionid;
		long sessionstart;
		std::map<std::string, std::string>values;
	};
	class GrdLeaderBoard
	{
	public:
		GrdLeaderBoard(){

		};
		GrdLeaderBoard(JSONValue*json){
			if (!json)return;
			JSONValue*value = json->Child("username");
			if (value){
				username = value->AsString();
			}
			value = json->Child("score");
			if (value){
				score = Grd::stod(value->AsString());
			}
			value = json->Child("rank");
			if (value){
				rank = (int)value->AsNumber();
			}
		}
		~GrdLeaderBoard(){

		}
		std::string username;
		double score;
		int rank;
	private:

	};
	class GrdManager
	{
	public:
		static GrdManager* getInstance();
		static void init(std::string appId, std::string apiSecret);
		GrdResult<AccountInfo> accountbalance(const std::string&username);
		GrdResult<JSONValue*> callServerScript(const std::string&username, const std::string &scriptname, const std::string &funcname, const std::vector<std::string>params);
		GrdResultBase transfer(const std::string&username, const std::string& toAddress, const BigDecimal& amount);
		GrdResultBase chargeMoney(const std::string& username, const BigDecimal& amount);
		GrdResult<std::string>getQRCodeAddress(const std::string&address);
		GrdResult<std::vector<GrdLeaderBoard>> getLeaderBoard(const std::string&username, const std::string& scoretype, const int& start, const int& count);
		GrdResult<std::vector<SessionData>> getUserSessionData(const std::string&username, const std::string& store, const std::string& key, const int& start, const int& count);
		GrdResult<std::vector<SessionData>> getUserSessionData(const std::string&username, const std::string& store, std::vector<std::string> keys, const int & start, const int& count);
		GrdResult<GrdLeaderBoard> getUserScoreRank(const std::string& username, const std::string& scoretype);
		GrdResultBase saveUserScore(const std::string& username, const std::string& scoretype, const double&core);
		GrdResultBase increaseUserScore(const std::string& username, const std::string& scoretype, const double&core);
		GrdResult<int> getTransactionCount(const std::string& username);
		GrdResult<std::vector<Transaction>> getTransactions(const std::string& username, const int& start, const int& count);
	private:
		class GrdReponse
		{
		public:
			GrdReponse(){

			}
			~GrdReponse(){
				if (json){
					delete json;
				}
			};
			JSONValue*json;
			int error;
			std::string message;
		};

		GrdManager(std::string apiId, std::string secret);
		const std::string ACCOUNT_BALANCE_ACTION = "accountbalance";
		const std::string CALL_SERVERSCRIPT_ACTION = "callserverscript";
		const std::string TRANSFER_ACTION = "transfer";
		const std::string CHARGEMONEY_ACTION = "chargemoney";
		const std::string GET_QRCODE_ACTION = "qrcode";
		const std::string GET_LEADERBOARD_ACTION = "getleaderboard";
		const std::string GET_SESSIONDATA_ACTION = "getusersessiondata";
		const std::string GET_USERSCORE_ACTION = "getuserscore";
		const std::string SAVE_USERSCORE_ACTION = "saveuserscore";
		const std::string INCREASE_USERSCORE_ACTION = "increaseuserscore";
		const std::string COUNT_TRANSACTION_ACTION = "counttransactions";
		const std::string GET_TRANSACTION_ACTION = "transactions";
		const std::string apiUrl = "https://www.gamereward.io/appapi/";
		std::string apiId = "";
		std::string apiSecret = "";
		std::string getRequestKey();
		void requestHttp(const std::string& username, const std::string &action, std::map<std::string, std::string>*params, bool isGet, GrdReponse&response);
	};


}
#endif