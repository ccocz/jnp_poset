/*
 * JNP - ZADANIE 1 (kasa biletowa)
 * Autorzy: Cezary Bednarz, Mikołaj Mazurczyk
 *
 */

#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <regex>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <unordered_map>
#include <climits>
#include <unordered_set>

using ULL = unsigned long long;
const ULL INF = 1e18 + 10;

// makra do odpowiadajace typom zapytania z wejscia
using BusRoute = std::pair<std::string, std::vector<std::pair<int, std::string>>>;
using Ticket = std::tuple<std::string, ULL, int>;
using TicketRequest = std::pair<std::vector<std::string>, std::vector<std::string>>;

int timeToMin(std::string time) {
    std::string hours;
    std::string mins;
    hours += time[0];
    const std::string semi = ":";
    const std::string zero = "0";
    if(time[2] == semi[0]) {
        hours += time[1];
        mins += time[3];
        mins += time[4];
    }
    else {
        mins += time[2];
        mins += time[3];
    }

    return std::stoi(hours) * 60 + std::stoi(mins);
}

// funkcje parsujace linijki na zmienne (lub false gdy linijki sa niepoprawne)
std::pair<bool, BusRoute> parseBusRouteCommand(const std::string& command,
                                               std::unordered_set<std::string>& lineNumSet,
                                               std::unordered_map<std::string, std::unordered_set<std::string>>& busLines) {

    std::pair<bool, BusRoute> busRoute;

    std::string lineNum = "";
    int x = 0;
    const std::string zero = "0";

    while(command[x] == zero[0]) {
        x++;
    }

    while(std::isdigit(command[x])) {
        lineNum += command[x];
        x++;
    }

    if(lineNum == "") {
        lineNum = "0";
    }

    if(lineNumSet.find(lineNum) != lineNumSet.end()) {
        return {false, {}};
    }
    else {
        busRoute.second.first = lineNum;
    }

    const std::regex reg("(5:5[5-9]|([6-9]|1[0-9]|20):\\d\\d|21:1[0-9]|21:2[0-1])|([a-zA-Z]|_|\\^)+");
    auto itBegin = std::sregex_iterator(command.begin(), command.end(), reg);
    auto itEnd = std::sregex_iterator();
    const std::regex stopsReg("([a-zA-Z]|_|\\^)+");
    const std::regex timeReg("\\d+:\\d\\d");
    std::unordered_set<std::string> stopsSet;
    std::vector<std::string> stops;
    std::vector<int> stopTimes;
    int mins;
    int lastMins = -1;

    for (std::sregex_iterator i = itBegin; i != itEnd; ++i) {
        std::smatch match = *i;
        std::string matchStr = match.str();
        if(std::regex_match(matchStr, stopsReg)) {
            if(stopsSet.find(matchStr) != stopsSet.end()) {
                return {false, {}};
            }
            else {
                stopsSet.insert(matchStr);
                stops.push_back(matchStr);
            }
        }
        else if(std::regex_match(matchStr, timeReg)) {
            mins = timeToMin(matchStr);
            if(lastMins >= mins) {
                return {false, {}};
            }
            stopTimes.push_back(mins);
            lastMins = mins;
        }
    }

    lineNumSet.insert(lineNum);

    std::pair<int, std::string> p;
    for(size_t i = 0; i < stopTimes.size(); i++) {
        p.first = stopTimes[i];
        p.second = stops[i];
        busRoute.second.second.push_back(p);
    }

    for(const auto& pp : busRoute.second.second) {
        busLines[busRoute.second.first].insert(pp.second);
    }

    busRoute.first = true;
    return busRoute;
}

std::pair<bool, Ticket> parseNewTicketCommand(const std::string& command,
                                              std::unordered_set<std::string>& ticketNameSet) {

    std::pair<bool, Ticket> ticket;

    std::string ticketName;

    int x = 0;

    while(!std::isdigit(command[x+1])) {
        ticketName += command[x];
        x += 1;
    }

    if(ticketNameSet.find(ticketName) != ticketNameSet.end()) {
        return {false, {}};
    }

    const std::regex reg("\\d+\\.\\d\\d|[1-9][0-9]*");
    auto itBegin = std::sregex_iterator(command.begin(), command.end(), reg);
    auto itEnd = std::sregex_iterator();
    const std::regex priceReg("\\d+\\.\\d\\d");
    const std::regex durReg("[1-9][0-9]*");
    ULL ticketPrice;
    int ticketDur;

    for (std::sregex_iterator i = itBegin; i != itEnd; ++i) {
        std::smatch match = *i;
        std::string matchStr = match.str();
        if(std::regex_match(matchStr, priceReg)) {
            matchStr.erase(matchStr.length()-3, 1);
            int x = 0;
            const std::string zero = "0";

            while (matchStr[x] == zero[0]) {
                x++;
            }

            matchStr.erase(0, x);

            if(matchStr == "") {
                matchStr = "0";
            }

            if(18 < matchStr.length()) {
                return {false, {}};
            }
            else {
                ticketPrice = std::stoull(matchStr);
            }
        }
        else if(std::regex_match(matchStr, durReg)) {
            if(std::stoi(matchStr) > 60*24) {
                return {false, {}};
            }
            else {
                ticketDur = std::stoi(matchStr);
            }
        }
    }

    ticketNameSet.insert(ticketName);
    return {true, {ticketName, ticketPrice, ticketDur}};
}

std::pair<bool, TicketRequest> parseTicketRequestCommand(const std::string& command,
                                                         const std::unordered_set<std::string>& lineNumSet,
                                                         std::unordered_map<std::string, std::unordered_set<std::string>>& busLines) {

    const std::regex reg("\\d+|([a-zA-Z]|_|\\^)+");
    auto itBegin = std::sregex_iterator(command.begin(), command.end(), reg);
    auto itEnd = std::sregex_iterator();
    const std::regex lineNumReg("\\d+");
    const std::regex stopNameReg("([a-zA-Z]|_|\\^)+");
    std::vector<std::string> stops;
    std::vector<std::string> lineNumbers;
    const std::string zero = "0";

    for (std::sregex_iterator i = itBegin; i != itEnd; ++i) {
        std::smatch match = *i;
        std::string matchStr = match.str();

        if(std::regex_match(matchStr, lineNumReg)) {
            int x = 0;
            std::string zero = "0";
            std::string dot = ".";

            while (matchStr[x] == zero[0]) {
                x++;
            }

            matchStr.erase(0, x);

            if(matchStr == "") {
                matchStr = "0";
            }

            if(lineNumSet.find(matchStr) == lineNumSet.end()) {
                return {false, {}};
            }
            else {
                lineNumbers.push_back(matchStr);
            }
        }
        else if(std::regex_match(matchStr, stopNameReg)) {
            stops.push_back(matchStr);
        }
    }

    for(size_t i = 0; i < lineNumbers.size(); i++) {
        if(busLines[lineNumbers[i]].find(stops[i]) == busLines[lineNumbers[i]].end()) {
            return {false, {}};
        }
    }

    if(busLines[lineNumbers[lineNumbers.size()-1]].find(stops[stops.size()-1]) == busLines[lineNumbers[lineNumbers.size()-1]].end()) {
        return {false, {}};
    }

    for(size_t i = 1; i < stops.size(); i++) {
        if(stops[i] == stops[i - 1]) {
            return {false, {}};
        }
    }


    return {true, {stops, lineNumbers}};
}

// funkcje wykonujace odpowiednie linijki z wejscia
void executeBusRoute(const BusRoute& busRoute,
                     std::map<std::string, std::map<std::string, int>>& busStops) {

    auto name = busRoute.first;
    auto timetable = busRoute.second;

    for(const auto& p : timetable) {
        auto time = p.first;
        auto busStop = p.second;

        busStops[busStop][name] = time;
    }
}

void executeNewTicket(const Ticket& ticket, std::vector<Ticket>& tickets) {
    tickets.push_back(ticket);
}

std::string executeTicketRequest(const TicketRequest& ticketRequest,
                                 std::map<std::string, std::map<std::string, int>>& busStops,
                                 std::vector<Ticket>& tickets,
                                 int& numberOfTickets) {

    auto stops = ticketRequest.first;
    auto routes = ticketRequest.second;

    for(size_t i = 1; i < routes.size(); i++) {
        auto currStop = stops[i];
        auto prevRoute = routes[i - 1];
        auto nextRoute = routes[i];

        int prevTime = busStops[currStop][prevRoute];
        int nextTime = busStops[currStop][nextRoute];

        if(prevTime < nextTime) {
            return ":-( " + currStop;
        }
        else if(prevTime > nextTime) {
            return "";
        }
    }

    if(routes.size() == 1) {
        if(busStops[stops.back()][routes.back()] < busStops[stops[0]][routes[0]]) {
            return "";
        }
    }


    int time = busStops[stops.back()][routes.back()] - busStops[stops[0]][routes[0]] + 1;

    std::vector<Ticket> bestTickets;
    ULL lowestPrice = INF;

    for(size_t i = 0; i < tickets.size(); i++) {
        for(size_t j = i; j <= tickets.size(); j++) {
            for(size_t k = j; k <= tickets.size(); k++) {

                std::vector<Ticket> curr;

                curr.push_back(tickets[i]);
                if(j != tickets.size())
                    curr.push_back(tickets[j]);
                if(k != tickets.size())
                    curr.push_back(tickets[k]);

                int currTime = 0;
                for(const auto& t : curr)
                    currTime += std::get<2>(t);

                if(currTime >= time) {
                    ULL currPrice = 0;
                    for(const auto& t : curr)
                        currPrice += std::get<1>(t);

                    if(currPrice < lowestPrice) {
                        lowestPrice = currPrice;
                        bestTickets = curr;
                    }
                }
            }
        }
    }

    if(bestTickets.empty()) {
        return ":-|";
    }

    std::string ret = "!";
    for(auto t : bestTickets) {
        ret.append(" " + std::get<0>(t) + ";");
    }
    ret.pop_back();

    numberOfTickets += (int)bestTickets.size();

    return ret;
}

int main() {

    // regexy do wstepnego sprawdzenia linijki
    const std::regex regBusRouteCommand("\\d+( (5:5[5-9]|([6-9]|1[0-9]|20):[0-5]\\d|21:1[0-9]|21:2[0-1]) ([a-zA-Z]|_|\\^)+)+");
    const std::regex regTicketCommand("([a-zA-Z]| )+ \\d+\\.\\d\\d [1-9][0-9]*");
    const std::regex regTicketRequestCommand("\\?( ([a-zA-Z]|_|\\^)+ \\d+)+ ([a-zA-Z]|\\^|_)+");

    // struktury do parsowania wejscia
    std::unordered_set<std::string> ticketNameSet;
    std::unordered_set<std::string> lineNumSet;
    std::unordered_map<std::string, std::unordered_set<std::string>> busLines;

    // struktury do odpowiedzi na zapytania
    std::map<std::string, std::map<std::string, int>> busStops;
    std::vector<Ticket> tickets;


    int lineId = 0;
    std::string line;
    int numberOfTickets = 0;

    // wczytywanie linijka po linijce
    while(getline(std::cin, line)) {

        lineId++;

        if(line.empty()) {
            continue;
        }

        bool badLine = false;

        // sprawdzanie 3 mozliwych komend
        if(isdigit(line[0]) || line[0] == '_' || line[0] == '^') {
            if(!regex_match(line, regBusRouteCommand)) {
                badLine = true;
            }
            else {
                auto p = parseBusRouteCommand(line, lineNumSet, busLines);
                if (p.first)
                    executeBusRoute(p.second, busStops);
                else
                    badLine = true;
            }
        }
        else if(isalpha(line[0]) || isspace(line[0])) {
            if(!regex_match(line, regTicketCommand)) {
                badLine = true;
            }
            else {
                auto p = parseNewTicketCommand(line, ticketNameSet);
                if(p.first)
                    executeNewTicket(p.second, tickets);
                else
                    badLine = true;
            }
        }
        else if(line[0] == '?') {
            if(!regex_match(line, regTicketRequestCommand)) {
                badLine = true;
            }
            else {
                auto p = parseTicketRequestCommand(line, lineNumSet, busLines);
                if(p.first) {
                    std::string ticketRequestOutput = executeTicketRequest(p.second, busStops, tickets, numberOfTickets);
                    if(ticketRequestOutput == "")
                        badLine = true;
                    else
                        std::cout << ticketRequestOutput << "\n";
                }
                else
                    badLine = true;
            }
        }
        else {
            badLine = true;
        }

        if(badLine) {
            std::cerr << "Error in line " << lineId << ": " << line << "\n";
        }
    }

    std::cout << numberOfTickets << "\n";

    return 0;
}