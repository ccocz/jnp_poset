/*
    JNP1 - Zad. 1
    Program reprezentujący działanie kasy tramwajowej
    Autorzy: Szymon Borowy, Krzysztof Rolf
    2019/2020
*/

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <climits>
#include <regex>
using namespace std;

/* Sprawdza, czy wiersz wejściowy odpowiadający dodaniu kursu autobusu jest poprawny. */
bool matchStringCourse(string line)
{
	regex pattern("^[0-9]+(?:(?:(?: [6-9]| 1[0-9]| 20):[0-5][0-9]| 5:5[5-9]| 21:(?:[01][0-9]|2[01])) [a-zA-Z_^]+)+$");
	smatch result;
	if (regex_search(line, result, pattern))
	{
		return true;
	}
	return false;
}

/* Sprawdza, czy wiersz wejściowy odpowiadający dodaniu biletu jest poprawny. */
/* nazwa biletu - zawiera spacje i litery angielskiego alfabetu
 * cena biletu - a.b gdzie a*100+b < INT_MAX
 * czas waznosci - t<INT_MAX
 * INT_MAX = 2147483647
 * 2147483647
*/

bool matchStringTicket(string line)
{

	regex pattern("^[a-zA-Z ]+ (?:([1-9][0-9]*|0)\\.([0-9]{2})) ([1-9][0-9]*)$");
	smatch result;
	if (regex_search(line, result, pattern))
	{
		regex compareInt("^([1-9][0-9]{0,8}|1[0-9]{0,9}|20[0-9]{0,8}|21[0-3][0-9]{0,7}|214[0-6][0-9]{0,6}|2147[0-3][0-9]{0,5}|21474[0-7][0-9]{0,4}|214748[0-2][0-9]{0,3}|2147483[0-5][0-9]{0,2}|21474836[0-3][0-9]{0,1}|214748364[0-7])$");
		smatch compareTime;
		smatch comparePrice;
		string time = result[3];
		string price = result[1];
		string price2 = result[2];
		if (price[0] != '0')
		{
			price += price2;
		}
		else
		{
			if (price2[0] != '0')
			{
				price = price2;
			}
			else
			{
				if (price2[1] == '0')
				{
					return false;
				}
				else
				{
					price = price2.substr(1,1);
				}
			}
		}

		if (regex_search(time, compareTime, compareInt) && regex_search(price, comparePrice, compareInt))
		{
			return true;
		}
	}

	return false;
}

/* Sprawdza, czy wiersz wejściowy odpowiadający zapytaniu o trasę jest poprawny. */
bool matchStringQuery(string line)
{
	regex pattern("^[?] (?:[a-zA-Z_^]+ [0-9]+ )+[a-zA-Z_^]+$");
	smatch result;
	if (regex_search(line, result, pattern))
	{
		return true;
	}
	return false;
}

/* Porównuje dwa stringi, które reprezentują godziny w formacie hh:mm.
 * Zwraca 0, gdy są równe.
 * Zwraca 1, gdy pierwsza z godzin jest późniejsza.
 * Zwraca 2, gdy pierwsza z godzin jest wcześniejsza.
 */
int compareTime(string time1, string time2)
{
	string time1c = "", time2c = "";

	for (unsigned i = 0; i < time1.size(); i++)
	{
		if (time1[i] != ':')
			time1c += time1[i];
	}

	for (unsigned i = 0; i < time2.size(); i++)
	{
		if (time2[i] != ':')
			time2c += time2[i];
	}

	int x = stoi(time1c, nullptr);
	int y = stoi(time2c, nullptr);

	if (x > y)
		return 1;
	else if (x == y)
		return 0;
	else
		return 2;
}

/* Przyjmuje dwa stringi reprezentujące godziny, przy czym time2 > time1.
 * Zwraca różnicę pomiędzy godzinami podaną w minutach.
 */
unsigned timeDifference(string time1, string time2)
{
	string hours1 = "", hours2 = "", minutes1 = "", minutes2 = "";
	bool swapper = true;

	for (unsigned i = 0; i < time1.size(); i++)
	{
		if (time1[i] == ':')
		{
			swapper = !swapper;
			continue;
		}

		if (swapper)
			hours1 += time1[i];
		else if (time1[i] != '0' || i == time1.size() - 1)
			minutes1 += time1[i];
	}

	swapper = true;
	for (unsigned i = 0; i < time2.size(); i++)
	{
		if (time2[i] == ':')
		{
			swapper = !swapper;
			continue;
		}

		if (swapper)
			hours2 += time2[i];
		else if (time2[i] != '0' || i == time2.size() - 1)
			minutes2 += time2[i];
	}

	int hours = stoi(hours1, nullptr) - stoi(hours2, nullptr);
	int minutes = stoi(minutes1, nullptr) - stoi(minutes2, nullptr);

	return -hours * 60 - minutes;
}

/* Jeżeli to możliwe dodaje bilet do naszego słownika biletów, wtedy zwraca true.
 * W przeciwnym wypadku zwraca false.
 */
bool addTicket(map<string, pair<unsigned, unsigned>> &t, string line)
{
	string ticketName = "", price = "", validityTime = "";

	regex pattern("^([a-zA-Z ]+) (?:([1-9][0-9]*|0)\\.([0-9]{2})) ([1-9][0-9]*)$");
	smatch result;
	regex_search(line, result, pattern);

	ticketName = result[1];
	price = result[2];
	string price2 = result[3];
	validityTime = result[4];

	if (price[0] != '0')
	{
		price += price2;
	}
	else
	{
		if (price2[0] != '0')
		{
			price = price2;
		}
		else
		{
			if (price2[1] == '0')
			{
				return false;
			}
			else
			{
				price = price2.substr(1,1);
			}
		}
	}
	if (t.count(ticketName) > 0)
		return false;

	t[ticketName] = make_pair(stoi(price, nullptr), stoi(validityTime, nullptr));
	return true;
}

/* Jeżeli to możliwe dodaje kurs autobusu do naszego słownika busów, wtedy zwraca true.
 * W przeciwnym wypadku zwraca false.
 */
bool addBus(map<string, map<string, string>> &b, string line)
{
	string courseNumber = "", stop = "", time = "";

	//znajdz numer linii
	regex pattern("^[0]*([0-9]+)");
	smatch result;
	regex_search(line, result, pattern);

	courseNumber = result[1];
	if (courseNumber == "")
		courseNumber = "0";

	if (b.count(courseNumber) > 0)
		return false;

	//ZNAJDZ liste przystankow

	regex words_regex("((?:[0-9]+:[0-9][0-9]) (?:[a-zA-Z_^]+))");
	auto words_begin =
		sregex_iterator(line.begin(), line.end(), words_regex);
	auto words_end = sregex_iterator();

	int oldTime = 0;
	int newTime = 0;

	for (sregex_iterator i = words_begin; i != words_end; ++i)
	{
		smatch match = *i;
		string match_str = match.str();
		regex compareTime("([0-9]+:[0-9][0-9])");
		regex compareStop("([a-zA-Z_^]+)");
		smatch timeReg;
		smatch stopReg;
		regex_search(match_str, timeReg, compareTime);
		regex_search(match_str, stopReg, compareStop);
		stop = stopReg[1];
		time = timeReg[1];
		regex calculateTIme("([0-9]+):([0-9][0-9])");
		smatch timeCalculation;

		regex_search(time, timeCalculation, calculateTIme);
		string hours = timeCalculation[1];
		string minutes = timeCalculation[2];
		newTime = stoi(hours) * 60 + stoi(minutes);
		if (b[courseNumber].count(stop) > 0 || newTime <= oldTime)
		{
			b.erase(courseNumber);
			return false;
		}

		oldTime = newTime;
		b[courseNumber][stop] = time;
	}
	return true;
}

/* Znajduje optymalny pod względem ceny zestaw biletów starczający na podany czas.
 * Zwraca zestaw w postaci ((bilet1, bilet2), (bilet3, liczbaBiletów)), gdzile bileti dla i = 1..3
 * może być biletem pustym lub nazwą biletu.
 */
pair<pair<string, string>, pair<string, int>> findTickets(map<string, pair<unsigned, unsigned>> &t, unsigned time)
{
	pair<pair<string, string>, pair<string, int>> matchingTickets = make_pair(make_pair("", ""), make_pair("", 0));
	map<string, pair<unsigned, unsigned>>::iterator it1;
	map<string, pair<unsigned, unsigned>>::iterator it2;
	map<string, pair<unsigned, unsigned>>::iterator it3;
	unsigned min = INT_MAX;

	for (it1 = t.begin(); it1 != t.end(); it1++)
	{
		for (it2 = it1; it2 != t.end(); it2++)
		{
			for (it3 = it2; it3 != t.end(); it3++)
			{
				if (it1->second.second + it2->second.second + it3->second.second >= time)
				{
					unsigned x = it1->second.first + it2->second.first + it3->second.first;

					if (x < min)
					{
						int counter = 0;
						min = x;
						if (it1->first != "EMPTY1" && it1->first != "EMPTY2")
							counter++;
						if (it2->first != "EMPTY1" && it2->first != "EMPTY2")
							counter++;
						if (it3->first != "EMPTY1" && it3->first != "EMPTY2")
							counter++;

						matchingTickets.first.first = it1->first;
						matchingTickets.first.second = it2->first;
						matchingTickets.second.first = it3->first;
						matchingTickets.second.second = counter;
					}
				}
			}
		}
	}

	//Przypadek, gdy nie ma zestawu biletów dla danego czasu.
	if (matchingTickets.first.first == "" && matchingTickets.first.second == "" && matchingTickets.second.first == "")
		matchingTickets.first.first = "3EMPTY";
	return matchingTickets;
}

/* Sprawdza, czy trasa podana w zapytaniu jest możliwa do zrealizowania
 * i czy są bilety pozwalające na zrealizowanie tej trasy.
 */
bool checkRoute(map<string, map<string, string>> &b, map<string, pair<unsigned, unsigned>> &t,
				string line, unsigned long long &tickets)
{
	string stop = "", course1 = "", course2 = "", timeStart = "", timeEnd = "", timeActual = "";
	unsigned i = 2;
	bool swapper = true;
	while (i < line.size())
	{
		stop += line[i];
		i++;
		if (line[i] == 32)
			break;
	}

	i++;

	while (i < line.size())
	{
		course1 += line[i];
		i++;
		if (line[i] == 32)
			break;
	}

	i++;

	if (b.count(course1) == 0 || b[course1].count(stop) == 0)
		return false;

	timeStart = b[course1][stop];
	timeActual = timeStart;
	stop = "";

	while (i < line.size())
	{
		if (line[i] == 32)
			swapper = !swapper;
		else if (swapper == false)
		{
			course2 += line[i];
		}
		else if (swapper == true)
		{
			stop += line[i];
		}

		if (swapper == true && course2.size() != 0)
		{
			if (b.count(course2) == 0 || b[course1].count(stop) == 0 || b[course2].count(stop) == 0)
				return false;
			if (compareTime(timeStart, b[course1][stop]) < 2)
				return false;
			int comparison = compareTime(b[course1][stop], b[course2][stop]);

			if (comparison == 1)
				return false;
			if (comparison == 2)
			{
				cout << ":-( " << stop << endl;
				return true;
			}

			timeActual = b[course2][stop];
			course1 = course2;
			course2 = "";
			stop = "";
		}

		if (i == line.size() - 1)
		{
			if (b[course1].count(stop) == 0)
				return false;

			timeEnd = b[course1][stop];
			if (compareTime(timeActual, timeEnd) != 2)
				return false;
		}

		i++;
	}

	unsigned time = timeDifference(timeStart, timeEnd);
	pair<pair<string, string>, pair<string, int>> result = findTickets(t, time + 1);

	if (result.first.first == "3EMPTY")
		cout << ":-|" << endl;
	else
	{
		cout << "! ";

		if (result.second.second == 1)
		{
			if (result.first.first != "EMPTY1" && result.first.first != "EMPTY2")
				cout << result.first.first << endl;
			else if (result.first.second != "EMPTY1" && result.first.second != "EMPTY2")
				cout << result.first.second << endl;
			else if (result.second.first != "EMPTY1" && result.second.first != "EMPTY2")
				cout << result.second.first << endl;
		}
		else if (result.second.second == 2)
		{
			if (result.first.first == "EMPTY1" || result.first.first == "EMPTY2")
				cout << result.first.second << "; " << result.second.first << endl;
			else if (result.first.second == "EMPTY1" || result.first.second == "EMPTY2")
				cout << result.first.first << "; " << result.second.first << endl;
			else if (result.second.first == "EMPTY1" || result.second.first == "EMPTY2")
				cout << result.first.first << "; " << result.first.second << endl;
		}
		else
			cout << result.first.first << "; " << result.first.second << "; " << result.second.first << endl;
	}
	tickets += result.second.second;

	return true;
}

int main()
{
	map<string, map<string, string>> busMap;
	map<string, pair<unsigned, unsigned>> ticketsAvailable;
	//Dodajemy do słownika biletów dwa bilety o 0 koszcie i 0 czasie działania.
	//Ułatwi to nam wyszukanie odopowiedniego zestawu biletu.
	//Nazwy biletów "EMPTY1" i "EMPTY2" są spoza możliwej dziedziny nazw,
	//zatem łatwo potem rozpoznamy, że jest to pusty bilet.
	ticketsAvailable["EMPTY1"] = make_pair(0, 0);
	ticketsAvailable["EMPTY2"] = make_pair(0, 0);

	string line;
	unsigned lineNumber = 0;
	unsigned long long ticketsCounter = 0;

	while (getline(cin, line))
	{
		lineNumber++;

		if (matchStringQuery(line))
		{
			if (!checkRoute(busMap, ticketsAvailable, line, ticketsCounter))
				cerr << "Error in line " << lineNumber << ": " << line << "\n";
		}
		else if (matchStringCourse(line))
		{
			if (!addBus(busMap, line))
				cerr << "Error in line " << lineNumber << ": " << line << "\n";
		}
		else if (matchStringTicket(line))
		{
			if (!addTicket(ticketsAvailable, line))
				cerr << "Error in line " << lineNumber << ": " << line << "\n";
		}
		else if (line == "")
			continue;
		else
		{
			cerr << "Error in line " << lineNumber << ": " << line << "\n";
		}
	}

	cout << ticketsCounter<<endl;
}
