#include "GrdManager.h"
#include "md5.h"
#include <chrono>
#include <curl/curl.h>
namespace Grd{
	static inline void* createSpriteWithBase64(const std::string& data)
	{
		if (data.length() == 0)
			return nullptr;
		int len = 0;
		unsigned char *buffer;

		return nullptr;
	}
	static GrdManager*instance = nullptr;
	void GrdManager::init(std::string apiId, std::string secret){
		if (instance != nullptr){
			delete instance;
		}
		instance = new GrdManager(apiId, secret);

	}
	Grd::GrdManager*Grd::GrdManager::getInstance(){
		return instance;
	}
	Grd::GrdManager::GrdManager(std::string appId, std::string secret)
	{
		this->apiId = appId;
		this->apiSecret = secret;
	}

	std::string Grd::GrdManager::getRequestKey()
	{
		std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		long sec = ms.count() / 1000;
		int t = (int)sec;
		t = t / 15;
		int len = apiSecret.length() / 20;
		int k = t % 20;
		std::string str = apiSecret.substr(k * len, len);
		str = md5(str+to_string(t));
		return str;
	}
	size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string* data) {
		data->append((char*)ptr, size * nmemb);
		return size * nmemb;
	}
	void Grd::GrdManager::requestHttp(const std::string& username, const std::string &action, std::map<std::string, std::string>*params, bool isGet, GrdReponse&response)
	{
		std::string data = "api_id=" + apiId + "&api_key=" + this->getRequestKey();
		if (username.size() > 0){
			data += "&username=" + username;
		}
		if (params != NULL)
		{
			std::map<std::string, std::string>::iterator it;
			for (it = params->begin(); it != params->end(); it++)
			{
				data = data + "&" + it->first + "=" + it->second;
			}
		}
		CURL *curl;
		CURLcode res;
		std::string pResponse;
		/* In windows, this will init the winsock stuff */
		curl_global_init(CURL_GLOBAL_ALL);

		/* get a curl handle */
		curl = curl_easy_init();
		if (curl) {
			/* First set the URL that is about to receive our POST. This URL can
			just as well be a https:// URL if that is what should receive the
			data. */
			std::string surl = this->apiUrl + action;
			const char* urlbuffer = surl.c_str();
			curl_easy_setopt(curl, CURLOPT_URL, urlbuffer);
			curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
			/* Now specify the POST data */
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());        
			curl_easy_setopt(curl, CURLOPT_POST, 1L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
			curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 1.1.4322; .NET CLR 2.0.5");


			int response_code;
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &pResponse);
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);



			/* Perform the request, res will get the return code */
			res = curl_easy_perform(curl);
			/* Check for errors */
			if (res != CURLE_OK){
				pResponse = curl_easy_strerror(res);
				pResponse = "{\"error\":500,\"message\":\"" + pResponse + "\"}";
			}

			/* always cleanup */
			curl_easy_cleanup(curl);
		}
		curl_global_cleanup();
		if (pResponse.size() == 0){
			pResponse = "{\"error\":500,\"message\":\"" + pResponse + "\"}";
		}
		int pos = pResponse.find("{");
		if (pos> 0){
			//remove unicode first characters
			pResponse = pResponse.substr(pos);
		}
		response.json = JSON::Parse(pResponse.c_str());
		if (response.json){
			try{
				response.error = (int)response.json->Child("error")->AsNumber();
			}
			catch (int e){
				response.error = 100;
				response.message = "Invalid JSON:" + pResponse;
			}
			if (response.error){
				response.message = response.json->Child("message")->AsString();
			}
		}
	}
	//public action
	GrdResult<JSONValue*> Grd::GrdManager::callServerScript(const std::string& username, const std::string &scriptname, const std::string &funcname, std::vector<std::string>params){
		std::map<std::string, std::string>data;
		std::string vars = "[";
		std::vector<std::string>::iterator it;
		for (it = params.begin(); it != params.end(); it++){
			vars += "" + *it + ",";
		}
		if (vars.length() > 0){
			vars = vars.substr(0, vars.length() - 1);
		}
		vars += "]";
		data.insert(std::pair<std::string, std::string>("script", scriptname));
		data.insert(std::pair<std::string, std::string>("fn", funcname));
		data.insert(std::pair<std::string, std::string>("vars", vars));
		Grd::GrdManager::GrdReponse  response;
		this->requestHttp(username, CALL_SERVERSCRIPT_ACTION, &data, false, response);
		JSONValue* value = nullptr;
		if (!response.error){
			value = response.json->Child("result");
			response.json->RemoveChild("result");//Remove to do not delete it
		}
		GrdResult<JSONValue*> result(response.error, response.message, value);
		return result;
	}
	GrdResultBase Grd::GrdManager::chargeMoney(const std::string& username, const BigDecimal& amount){
		std::map<std::string, std::string>data;
		BigDecimal sval = amount;
		data.insert(std::pair<std::string, std::string>("value", sval.toString()));
		Grd::GrdManager::GrdReponse  response;
		this->requestHttp(username, CHARGEMONEY_ACTION, &data, false, response);
		return GrdResultBase(response.error, response.message);
	}

	GrdResult<AccountInfo> Grd::GrdManager::accountbalance(const std::string&username){
		std::map<std::string, std::string>data;
		Grd::GrdManager::GrdReponse  response;
		this->requestHttp(username, ACCOUNT_BALANCE_ACTION, &data, false, response);
		AccountInfo value;
		if (!response.error){
			value.address = response.json->Child("address")->AsString();
			value.balance = response.json->Child("balance")->AsString();
		}
		return GrdResult<AccountInfo>(response.error,response.message,value);
	}
	GrdResult<std::string> Grd::GrdManager::getQRCodeAddress(const std::string& address){
		std::map<std::string, std::string>data;
		data.insert(std::pair<std::string, std::string>("text", "gamereward:" + address));
		data.insert(std::pair<std::string, std::string>("type", "1"));
		Grd::GrdManager::GrdReponse response;
		this->requestHttp("", GET_QRCODE_ACTION, &data, false, response);
		if (response.error){
			return GrdResult<std::string>(response.error,response.message,"");
		}
		return GrdResult<std::string>(response.error, response.message,response.json->Child("qrcode")->AsString());
	}
	GrdResultBase Grd::GrdManager::transfer(const std::string&username, const std::string&toAddress, const BigDecimal& amount)
	{
		std::map<std::string, std::string>data;
		data.insert(std::pair<std::string, std::string>("to", toAddress));
		BigDecimal v = amount;
		data.insert(std::pair<std::string, std::string>("value", v.toString()));
		Grd::GrdManager::GrdReponse response;
		this->requestHttp(username, TRANSFER_ACTION, &data, false, response);
		return GrdResultBase(response.error, response.message);
	}
	GrdResultBase GrdManager::saveUserScore(const std::string& username, const std::string& scoretype, const double&core){
		std::map<std::string, std::string>data;
		data.insert(std::pair<std::string, std::string>("scoretype", scoretype));
		data.insert(std::pair<std::string, std::string>("score", to_string(core)));
		Grd::GrdManager::GrdReponse response;
		this->requestHttp(username, SAVE_USERSCORE_ACTION, &data, false, response);
		return GrdResultBase(response.error, response.message);
	}
	GrdResultBase GrdManager::increaseUserScore(const std::string& username, const std::string& scoretype, const double&core){
		std::map<std::string, std::string>data;
		data.insert(std::pair<std::string, std::string>("scoretype", scoretype));
		data.insert(std::pair<std::string, std::string>("score", to_string(core)));
		Grd::GrdManager::GrdReponse response;
		this->requestHttp(username, INCREASE_USERSCORE_ACTION, &data, false, response);
		return GrdResultBase(response.error, response.message);
	}
	GrdResult<GrdLeaderBoard> GrdManager::getUserScoreRank(const std::string& username, const std::string& scoretype){
		std::map<std::string, std::string>data;
		data.insert(std::pair<std::string, std::string>("scoretype", scoretype));
		Grd::GrdManager::GrdReponse response;
		this->requestHttp(username, GET_USERSCORE_ACTION, &data, false, response);
		if (!response.error){
			GrdLeaderBoard value(response.json);
			return GrdResult<GrdLeaderBoard>(response.error, response.message, value);
		}
		return GrdResult<GrdLeaderBoard>(response.error, response.message, NULL);
	}
	GrdResult<std::vector<Grd::GrdLeaderBoard>> Grd::GrdManager::getLeaderBoard(const std::string&username, const std::string& scoretype, const int & start, const int& count){
		std::map<std::string, std::string>data;
		data.insert(std::pair<std::string, std::string>("scoretype", scoretype));
		data.insert(std::pair<std::string, std::string>("start", to_string(start)));
		data.insert(std::pair<std::string, std::string>("count", to_string(count)));
		Grd::GrdManager::GrdReponse response;
		this->requestHttp(username, GET_LEADERBOARD_ACTION, &data, false, response);
		std::vector<Grd::GrdLeaderBoard>leaderBoard;
		if (!response.error){
			JSONValue*item = response.json->Child("leaderboard");
			if (item->IsArray()){
				int childCount = item->CountChildren();
				for (int i = 0; i < childCount; i++)
				{
					GrdLeaderBoard litem = item->Child(i);
					leaderBoard.push_back(litem);
				}
			}
		}
		return GrdResult<std::vector<Grd::GrdLeaderBoard>>(response.error,response.message, leaderBoard);
	}
	GrdResult<std::vector<Grd::SessionData>> Grd::GrdManager::getUserSessionData(const std::string&username, const std::string& store, const std::string& key, const int &start, const int &count){
		std::map<std::string, std::string>data;
		data.insert(std::pair<std::string, std::string>("store", store));
		data.insert(std::pair<std::string, std::string>("keys", key));
		data.insert(std::pair<std::string, std::string>("start", to_string(start)));
		data.insert(std::pair<std::string, std::string>("count", to_string(count)));
		Grd::GrdManager::GrdReponse response;
		this->requestHttp(username, GET_SESSIONDATA_ACTION, &data, false, response);
		std::vector<Grd::SessionData>sessions;
		if (!response.error){
			JSONValue*item = response.json->Child("data");
			if (item->IsArray()){
				int childCount = item->CountChildren();
				for (int i = 0; i < childCount; i++)
				{
					SessionData litem = item->Child(i);
					sessions.push_back(litem);
				}
			}
		}
		return GrdResult<std::vector<Grd::SessionData>>(response.error,response.message,sessions);
	}
	GrdResult<std::vector<Grd::SessionData>>  Grd::GrdManager::getUserSessionData(const std::string& username, const std::string& store, std::vector<std::string> keys, const int& start, const int& count){
		std::map<std::string, std::string>data;
		std::string st = "";
		for (std::vector<std::string>::iterator key = keys.begin(); key != keys.end(); key++)
		{
			st += "," + *key;
		}
		if (keys.size() > 0){
			st = st.substr(1);
		}
		return this->getUserSessionData(username, store, st, start, count);
	}
	GrdResult<int>  Grd::GrdManager::getTransactionCount(const std::string& username){
		std::map<std::string, std::string>data; Grd::GrdManager::GrdReponse response;
		this->requestHttp(username, COUNT_TRANSACTION_ACTION, &data, false, response);
		if (response.error){
			return GrdResult<int>(response.error,response.message, 0);
		}
		return GrdResult<int>(response.error, response.message, ((int)response.json->Child("count")->AsNumber()));
	}
	GrdResult<std::vector<Transaction>>  Grd::GrdManager::getTransactions(const std::string& username, const int &start, const int&count){
		std::map<std::string, std::string>data; 
		data.insert(std::pair<std::string, std::string>("start", to_string(start)));
		data.insert(std::pair<std::string, std::string>("count", to_string(count)));
		std::vector<Transaction>trans;
		Grd::GrdManager::GrdReponse response;
		this->requestHttp(username, GET_TRANSACTION_ACTION, &data, false, response);
		if (!response.error){
			JSONValue*list = response.json->Child("transactions");
			int count = list->CountChildren();
			for (int i = 0; i < count; i++){
				Transaction t = list->Child(i);
				trans.push_back(t);
			}
		}
		return GrdResult<std::vector<Transaction>>(response.error,response.message,trans);
	}
}