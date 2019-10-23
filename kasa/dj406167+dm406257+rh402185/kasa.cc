#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <tuple>
#include <map>
#include <set>
using namespace std;

set<uint32_t> lineNumbers;

map<string, pair<uint32_t, uint32_t> > tickets;

map<pair<uint32_t, string>, uint32_t> schedule;

regex lineRegex("^0*(\\d{1,9})((?: (?:1|2)?\\d:[0-5]\\d [a-zA-Z^_]+){2,})$");
regex stopRegex("([1-2]?\\d:[0-5]\\d) ([a-zA-Z^_]+)");
regex ticketRegex("^([a-zA-Z ]+) (0|[1-9]\\d{0,6})\\.(\\d\\d) ([1-9]\\d*)$");
regex requestRegex("^[?]((?:(?: [a-zA-Z^_]+ (?:0|[1-9]\\d*))+) ([a-zA-Z^_]+))$");
regex requestSegmentRegex("([a-zA-Z^_]+) (0|[1-9]\\d*)");
regex timeRegex("([1-2]?\\d):([0-5]\\d)");

uint32_t timeToInt(string time)
{
    smatch timeMatch;
    regex_match(time, timeMatch, timeRegex);
    return stoi(timeMatch[2]) + 60 * stoi(timeMatch[1]);
}

uint32_t minTime = timeToInt("5:55");
uint32_t maxTime = timeToInt("21:21");

//Czyta linijkę pasującą do szablonu dodawania kursu.
pair<uint32_t, vector<pair<uint32_t, string> > > readLine(smatch lineDesc)
{
    vector<pair<uint32_t, string> > line;

    //Jeśli numer linii wychodzi poza zakres uint32_t, to wektor opisujący przebieg linii zwracamy pusty.
    try
    {
        stoi(lineDesc[0]);
    }
    catch(out_of_range)
    {
        return make_pair(0, line);
    }

    uint32_t number = stoi(lineDesc[0]);
    string desc = lineDesc[2];
    smatch stops;
    //Moment w którym byłem na poprzednim przystanku.
    uint32_t lastTime = minTime - 1;

    while(regex_search(desc, stops, stopRegex))
    {
        uint32_t stopTime = timeToInt(stops[1]);

        //Jeśli czasy nie są rosnące lub wychodzą poza zakres, to wektor opisujący przebieg linii zwracamy pusty.
        if(lastTime >= stopTime || stopTime > maxTime || stopTime < minTime)
        {
            line.clear();
            break;
        }

        line.push_back(make_pair(timeToInt(stops[1]), stops[2]));
        desc = stops.suffix();
    }

    return make_pair(number, line);
}

//Czyta linijkę pasującą do szablonu dodawania biletu.
tuple<string, uint32_t, uint32_t> readTicket(smatch ticketDesc)
{
    uint32_t priceBig = 0;
    uint32_t time = 0;

    //Jeśli cena lub czas ważności biletu wychodzi poza zakres uint32_t, to zwracamy bilet o cenie i czasie ważności 0.
    try
    {
        priceBig = stoi(ticketDesc[2]);
        time = stoi(ticketDesc[4]);
    }
    catch(out_of_range)
    {
        return make_tuple("", 0, 0);
    }

    uint32_t priceSmall = stoi(ticketDesc[3]);

    if(priceBig > (numeric_limits<uint32_t>::max() - priceSmall) / 100)
        return make_tuple("", 0, 0);

    return make_tuple(ticketDesc[1], 100 * priceBig + priceSmall, stoi(ticketDesc[4]));
}

//Czyta linijkę pasującą do szablonu zapytania.s
pair<vector<uint32_t>, vector<string>> readRequest(smatch requestDesc)
{
    vector<uint32_t> lines;
    vector<string> stops;

    string desc = requestDesc[1];
    smatch segments;

    while(regex_search(desc, segments, requestSegmentRegex))
    {
        uint32_t lineNumber = 0;

        //Jeśli numer linii wychodzi poza zakres uint32_t, to wektor z numerami kursów zwracamy pusty.
        try
        {
            lineNumber = stoi(segments[2]);
        }
        catch(out_of_range)
        {
            lines.clear();
            return (make_pair(lines, stops));
        }

        lines.push_back(lineNumber);
        stops.push_back(segments[1]);
        desc = segments.suffix();
    }

    stops.push_back(requestDesc[2]);
    return make_pair(lines, stops);
}

bool lineExists(uint32_t number)
{
    return lineNumbers.find(number) != lineNumbers.end();
}

void addLine(uint32_t number, vector<pair<uint32_t, string> > line)
{
    for(auto it:line)
        schedule[make_pair(number, it.second)] = it.first;

    lineNumbers.insert(number);
}

bool addTicket(string name, uint32_t price, uint32_t time)
{
    if(tickets.find(name) != tickets.end())
        return false;

    tickets[name] =  make_pair(price, time);
    return true;
}

//Sprawdza czy odpowiednie odcinki trasy istnieją oraz czy da się przejechać nią jednego dnia.
bool correctRoute(vector<uint32_t> lines, vector<string> stops)
{
    uint32_t segments = lines.size();

    for(uint32_t i = 0; i < segments; ++i)
    {
        auto key1 = make_pair(lines[i], stops[i]);
        auto key2 = make_pair(lines[i], stops[i+1]);

        if(schedule.find(key1) == schedule.end() || schedule.find(key2) == schedule.end())
            return false;

        if(stops[i] == stops[i+1])
            return false;
    }

    for(uint32_t i = 1; i < segments; ++i)
    {
        auto key1 = make_pair(lines[i-1], stops[i]);
        auto key2 = make_pair(lines[i], stops[i]);

        if(schedule[key1] > schedule[key2])
            return false;
    }

    return true;
}

//Sprawdza czy dany plan podróży wymusza czekanie na przystanku.
string haveToWait(vector<uint32_t> lines, vector<string> stops)
{
    uint32_t segments = lines.size();

    for(uint32_t i = 1; i < segments; ++i)
    {
        auto key1 = make_pair(lines[i-1], stops[i]);
        auto key2 = make_pair(lines[i], stops[i]);

        if(schedule[key1] != schedule[key2])
            return stops[i];
    }

    return "";
}

//Dla danego czasu przejazdu wybiera najtańszy zbiór biletów który zapewnia conajmniej tak długi przejazd.
vector<string> chooseTickets(uint32_t time)
{
    vector<string> bestAns;
    uint64_t bestCost = numeric_limits<uint32_t>::max();

    for(auto t1:tickets)
    {
        for(auto t2:tickets)
        {
            for(auto t3:tickets)
            {
                vector<string> currAns;
                uint64_t currCost = 0;
                uint32_t currTime = 0;

                if(t1.first != "")
                {
                    currAns.push_back(t1.first);
                    currCost += t1.second.first;
                    currTime += t1.second.second;
                }

                if(t2.first != "")
                {
                    currAns.push_back(t2.first);
                    currCost += t2.second.first;
                    currTime += t2.second.second;
                }

                if(t3.first != "")
                {
                    currAns.push_back(t3.first);
                    currCost += t3.second.first;
                    currTime += t3.second.second;
                }

                if(currTime >= time && currCost < bestCost)
                {
                    bestAns = currAns;
                    bestCost = currCost;
                }
            }
        }
    }

    return bestAns;
}

//Znajduje optymalny zbiór biletów dla danego przejazdu.
vector<string> findTickets(vector<uint32_t> lines, vector<string> stops)
{
    auto firstKey = make_pair(lines[0], stops[0]);
    auto lastKey = make_pair(lines.back(), stops.back());
    uint32_t firstMinute = schedule[firstKey];
    uint32_t lastMinute = schedule[lastKey];
    uint32_t totalTime = lastMinute - firstMinute + 1;
    return chooseTickets(totalTime);
}

void init()
{
    //Dodaje pusty bilet by nie rozpatrywać oddzielnie przypadków wzięcia jednego, dwóch lub trzech biletów.
    tickets[""] = make_pair(0, 0);
}

int main()
{
    init();

    uint32_t totalSold = 0;
    uint32_t currLine = 0;

    while(!cin.eof())
    {
        currLine++;
        string s;
        getline(cin, s);

        if(s.empty())
            continue;

        smatch match;
        bool correct = false;

        if(regex_match(s, match, lineRegex))
        {
            pair<uint32_t, vector<pair<uint32_t, string> > > line = readLine(match);

            if(!line.second.empty() && !lineExists(line.first))
            {
                addLine(line.first, line.second);
                correct = true;
            }
        }
        else if(regex_match(s, match, ticketRegex))
        {
            tuple<string, uint32_t, uint32_t> ticket = readTicket(match);
            addTicket(get<0>(ticket), get<1>(ticket), get<2>(ticket));
            correct = true;
        }
        else if(regex_match(s, match, requestRegex))
        {
            pair<vector<uint32_t>, vector<string>> request = readRequest(match);

            if(!request.first.empty() && correctRoute(request.first, request.second))
            {
                correct = true;

                if(haveToWait(request.first, request.second) != "")
                    cout << ":-( " << haveToWait(request.first, request.second) << endl;
                else
                {
                    auto found = findTickets(request.first, request.second);

                    if(found.empty())
                        cout << ":-|" << endl;
                    else
                    {
                        totalSold += found.size();

                        cout << "! ";

                        for(uint32_t i = 0; i+1 < found.size(); ++i)
                            cout << found[i] << "; ";

                        cout << found.back() << endl;
                    }
                }
            }
        }

        if(!correct)
            cerr << "Error in line " << currLine << ": " << s << endl;
    }

    cout << totalSold << endl;
    return 0;
}
