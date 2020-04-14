// GetCovidDataFromWorlodmeters.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <map>
#include <sstream>

#include "curl.h"

#include "GetData.h"


bool getCountries(std::string mainURL, std::map<std::string, std::string>& countries);

int main()
{
		
	std::string path = "C:/Users/Aram/Documents/COVID-19/DataWorldometers/";
	std::string mainURL = "https://www.worldometers.info/coronavirus/";
	std::map<std::string, std::string> countries;
	if (getCountries(mainURL, countries)) {
		std::cout << "Country list has been generated\n";
	}
	else {
		std::cout << "Country list generation failed\n";
	}

	std::map<std::string, std::string>::iterator it;
	

	for (it = countries.begin(); it != countries.end(); ++it)
	{
		GetData gd;
		std::string country = it->first;
		std::string url = mainURL + it->second;
		
		if (gd.getContent(url, country)) {
			if (gd.saveData(path)) {
				std::cout << url << " Ok!\n";
			}
			else {
				std::cout << url << " Failed to load data!\n";
			}
		}
		else {
			if (gd.getContent(url, country)) {
				if (gd.saveData(path)) {
					std::cout << url << " Ok!\n";
				}
				else {
					std::cout << url << " Failed to load data!\n";
				}
			}
			else {
				std::cout << url << " failed to connect!\n";
			}

		}
		
	}

	return EXIT_SUCCESS;
}

bool getCountries(std::string mainURL, std::map<std::string, std::string>& countries)
{
	CURL *curl;
	CURLcode res;
	std::string buf;

	curl = curl_easy_init();
	if (curl == nullptr) {
		return false;
	}

	curl_easy_setopt(curl, CURLOPT_URL, mainURL.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	if (buf.empty()) {
		return false;
	}

	size_t p = buf.find("total_row_world row_continent");
	if (p == std::string::npos) {
		return false;
	}
	buf = buf.substr(p + 30);	
	p = buf.find("</tbody>");
	if (p == std::string::npos) {
		return false;
	}
	buf = buf.substr(0, p);

	std::string country;
	std::string url;
	while (p != std::string::npos) {
		p = buf.find("country");
		if (p == std::string::npos) {
			break;
		}
		url = buf.substr(p);
		buf = buf.substr(p + 8);		
		p = url.find("\"");
		if (p == std::string::npos) {
			break;
		}
		url = url.substr(0, p);
		p = buf.find(">");
		if (p == std::string::npos) {
			break;
		}
		country = buf.substr(p + 1);
		p = country.find("<");
		if (p == std::string::npos) {
			break;
		}
		country = country.substr(0, p);
		if (countries.count(country) == 0) {
			countries.insert(std::pair<std::string, std::string>(country, url));
		}
		p = buf.find("</td>");
		if (p == std::string::npos) {
			break;
		}
		if (buf.size() <= p + 6) {
			break;
		}
		buf = buf.substr(p + 6);
	}


	return true;
}
