#ifndef GETDATA_H
#define GETDATA_H

#include <stdio.h>
#include <string>
#include <vector>

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

class GetData
{
public:
	GetData();
	~GetData();
	bool getContent(const std::string& url);
	bool getContent(const std::string& url, const std::string& country);
	std::string country();
	std::vector<int> totalCases();
	std::vector<int> activeCases();
	std::vector<int> deaths();
	std::vector<int> casesWithOutcome();
	bool saveData(const std::string& pathTosave, bool generalCase = true);

private:
	
	bool parseHTML(std::string buf);
	bool parseHTML(std::string buf, const std::string& country);

	std::string m_url;
	std::string m_pathToSave;
	std::string m_fileName;
	std::string m_country;	
	std::vector<std::string> m_dates;
	std::vector<int> m_totalCases;
	std::vector<int> m_activeCases;
	std::vector<int> m_deaths;
	std::vector<int> m_casesWithOutcome;
};

typedef std::vector<GetData> GetDataSet;

#endif // !GETDATA_H



