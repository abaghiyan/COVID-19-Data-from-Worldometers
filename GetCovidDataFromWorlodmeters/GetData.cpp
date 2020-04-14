#include "GetData.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "curl.h"


GetData::GetData()
{
}


GetData::~GetData()
{
}

bool GetData::getContent(const std::string & url)
{
	m_url = url;
			
	CURL *curl;
	CURLcode res;
	std::string readBuffer;

	curl = curl_easy_init();
	if (curl == nullptr) {
		return false;
	}
	
	curl_easy_setopt(curl, CURLOPT_URL, m_url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	
	if (!parseHTML(readBuffer)) {
		return false;
	}	
	
	return true;
}

bool GetData::getContent(const std::string & url, const std::string & country)
{
	m_url = url;

	CURL *curl;
	CURLcode res;
	std::string readBuffer;

	curl = curl_easy_init();
	if (curl == nullptr) {
		return false;
	}

	curl_easy_setopt(curl, CURLOPT_URL, m_url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	if (!parseHTML(readBuffer, country)) {
		return false;
	}

	return true;
}

std::string GetData::country()
{
	return m_country;
}

std::vector<int> GetData::totalCases()
{
	return m_totalCases;
}

std::vector<int> GetData::activeCases()
{
	return m_activeCases;
}

std::vector<int> GetData::deaths()
{
	return m_deaths;
}

std::vector<int> GetData::casesWithOutcome()
{
	return m_casesWithOutcome;
}

bool GetData::saveData(const std::string & pathTosave, bool generalCase)
{
	if (pathTosave.empty()) {
		return false;
	}

	m_pathToSave = pathTosave;

	size_t p = pathTosave.find_last_of('\\');
	if (p == std::string::npos) {
		p = pathTosave.find_last_of('/');
		if (p = std::string::npos) {
			m_pathToSave = m_pathToSave + "/";
		}
		else if (p != (pathTosave.size() - 1)) {
			m_pathToSave = m_pathToSave + "/";
		}
	}
	else if (p != (pathTosave.size() - 1)) {
		m_pathToSave = m_pathToSave + "\\";
	}

	if (m_fileName.empty() || m_activeCases.empty() || m_casesWithOutcome.empty() || m_dates.empty() || m_totalCases.empty())
	{
		return false;

	}

	m_pathToSave = m_pathToSave + m_fileName;
	std::ofstream fileOut;
	fileOut.open(m_pathToSave);
	if (fileOut.is_open()) {
		if (generalCase) {
			fileOut << "Date" << "," << "Total Cases" << "," << "Active Cases" << "," << "Total Deaths" << ","
				<< "Cases with Outcome" << "\n";
			for (size_t i = 0; i < m_totalCases.size(); ++i) {
				fileOut << m_dates[i] << "," << m_totalCases[i] << "," << m_activeCases[i] << "," << m_deaths[i] << ","
					<< m_casesWithOutcome[i] << "\n";
			}

		}
		else {
			/** Prepare data for MATLAB **/
			for (size_t i = 0; i < m_totalCases.size(); ++i) {
				fileOut << m_totalCases[i] << "," << m_activeCases[i] << "," << m_deaths[i] << ","
					<< m_casesWithOutcome[i] << "\n";
			}
		}
		fileOut.close();
	}
	else {
		return false;
	}

	return true;
}

bool GetData::parseHTML(std::string buf)
{
	if (buf.empty()) {
		return false;
	}

	size_t p = buf.find("title");
	if (p == std::string::npos) {
		return false;
	}
	buf = buf.substr(p + 6);
	p = buf.find("Coronavirus");
	if (p == std::string::npos) {
		return false;
	}

	m_country = buf.substr(0, p - 1);

	p = buf.find("Total Cases");
	if (p == std::string::npos) {
		return false;
	}
	buf = buf.substr(p + 12);
	p = buf.find_first_of('[');
	if (p == std::string::npos) {
		return false;
	}
	std::string dates = buf.substr(p + 1);
	if (p == std::string::npos) {
		return false;
	}
	p = dates.find_first_of(']');
	if (p == std::string::npos) {
		return false;
	}
	dates = dates.substr(0, p);
	std::string s;
	std::istringstream d(dates);
	while (std::getline(d, s, ',')) {
		s = s.substr(1);
		p = s.find("\"");
		s = s.substr(0, p);
		m_dates.push_back(s);
	}
	m_fileName = m_country + "_" + m_dates.front() + "-" + m_dates.back() + ".csv";

	p = buf.find("data");
	if (p == std::string::npos) {
		return false;
	}
	buf = buf.substr(p);
	p = buf.find_first_of('['); 
	if (p == std::string::npos) {
		return false;
	}
	std::string totCases = buf.substr(p + 1);
	p = totCases.find_first_of(']');
	if (p == std::string::npos) {
		return false;
	}
	totCases = totCases.substr(0, p);
	std::istringstream tc(totCases);
	while (std::getline(tc, s, ',')) {
		m_totalCases.push_back(std::stoi(s));
	}	

	p = buf.find("Active Cases");
	if (p == std::string::npos) {
		return false;
	}
	buf = buf.substr(p + 13);
	p = buf.find("data");
	if (p == std::string::npos) {
		return false;
	}
	buf = buf.substr(p);
	p = buf.find_first_of('[');
	if (p == std::string::npos) {
		return false;
	}
	std::string actCases = buf.substr(p + 1);
	p = actCases.find_first_of(']');
	if (p == std::string::npos) {
		return false;
	}
	actCases = actCases.substr(0, p);
	std::istringstream ac(actCases);
	while (std::getline(ac, s, ',')) {
		m_activeCases.push_back(std::stoi(s));
	}

	p = buf.find("Total Deaths");
	if (p == std::string::npos) {
		return false;
	}
	buf = buf.substr(p + 13);
	p = buf.find("data");
	if (p == std::string::npos) {
		return false;
	}
	buf = buf.substr(p);
	p = buf.find_first_of('[');
	if (p == std::string::npos) {
		return false;
	}
	std::string deaths = buf.substr(p + 1);
	p = deaths.find_first_of(']');
	if (p == std::string::npos) {
		return false;
	}
	deaths = deaths.substr(0, p);
	std::istringstream dt(deaths);
	while (std::getline(dt, s, ',')) {
		m_deaths.push_back(std::stoi(s));
	}

	if (m_totalCases.size() != m_activeCases.size() || m_totalCases.size() != m_deaths.size()) {
		return false;
	}

	if(m_totalCases.empty()) {
		return false;
	}

	m_casesWithOutcome.resize(m_totalCases.size());

	for (size_t i = 0; i < m_totalCases.size(); ++i) {
		m_casesWithOutcome[i] = m_totalCases[i] - m_activeCases[i];
	}

	return true;
}

bool GetData::parseHTML(std::string buf, const std::string & country)
{
	if (buf.empty()) {
		return false;
	}

	size_t p = buf.find("title");
	if (p == std::string::npos) {
		return false;
	}
	buf = buf.substr(p + 6);

	m_country = country;

	p = buf.find("Total Cases");
	if (p == std::string::npos) {
		return false;
	}
	buf = buf.substr(p + 12);
	p = buf.find_first_of('[');
	if (p == std::string::npos) {
		return false;
	}
	std::string dates = buf.substr(p + 1);
	if (p == std::string::npos) {
		return false;
	}
	p = dates.find_first_of(']');
	if (p == std::string::npos) {
		return false;
	}
	dates = dates.substr(0, p);
	std::string s;
	std::istringstream d(dates);
	while (std::getline(d, s, ',')) {
		s = s.substr(1);
		p = s.find("\"");
		s = s.substr(0, p);
		m_dates.push_back(s);
	}
	m_fileName = m_country + "_" + m_dates.front() + "-" + m_dates.back() + ".csv";

	p = buf.find("data");
	if (p == std::string::npos) {
		return false;
	}
	buf = buf.substr(p);
	p = buf.find_first_of('[');
	if (p == std::string::npos) {
		return false;
	}
	std::string totCases = buf.substr(p + 1);
	p = totCases.find_first_of(']');
	if (p == std::string::npos) {
		return false;
	}
	totCases = totCases.substr(0, p);
	std::istringstream tc(totCases);
	while (std::getline(tc, s, ',')) {
		m_totalCases.push_back(std::stoi(s));
	}

	p = buf.find("Active Cases");
	if (p == std::string::npos) {
		return false;
	}
	buf = buf.substr(p + 13);
	p = buf.find("data");
	if (p == std::string::npos) {
		return false;
	}
	buf = buf.substr(p);
	p = buf.find_first_of('[');
	if (p == std::string::npos) {
		return false;
	}
	std::string actCases = buf.substr(p + 1);
	p = actCases.find_first_of(']');
	if (p == std::string::npos) {
		return false;
	}
	actCases = actCases.substr(0, p);
	std::istringstream ac(actCases);
	while (std::getline(ac, s, ',')) {
		m_activeCases.push_back(std::stoi(s));
	}

	p = buf.find("Total Deaths");
	if (p == std::string::npos) {
		return false;
	}
	buf = buf.substr(p + 13);
	p = buf.find("data");
	if (p == std::string::npos) {
		return false;
	}
	buf = buf.substr(p);
	p = buf.find_first_of('[');
	if (p == std::string::npos) {
		return false;
	}
	std::string deaths = buf.substr(p + 1);
	p = deaths.find_first_of(']');
	if (p == std::string::npos) {
		return false;
	}
	deaths = deaths.substr(0, p);
	std::istringstream dt(deaths);
	while (std::getline(dt, s, ',')) {
		m_deaths.push_back(std::stoi(s));
	}

	if (m_totalCases.size() != m_activeCases.size() || m_totalCases.size() != m_deaths.size()) {
		return false;
	}

	if (m_totalCases.empty()) {
		return false;
	}

	m_casesWithOutcome.resize(m_totalCases.size());

	for (size_t i = 0; i < m_totalCases.size(); ++i) {
		m_casesWithOutcome[i] = m_totalCases[i] - m_activeCases[i];
	}

	return true;
}

size_t WriteCallback(void * contents, size_t size, size_t nmemb, void * userp)
{	
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;	
}
