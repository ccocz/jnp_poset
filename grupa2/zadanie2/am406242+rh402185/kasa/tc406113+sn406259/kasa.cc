#include <iostream>
#include <regex>
#include <tuple>
#include <map>
#include <utility>
#include <list>
#include <set>

enum Command {
    ROUTE_ADDITION, TICKET_ADDITION, TICKET_QUERY, LINE_DISREGARDING, NONE
};

int const MAX_LENGTH_TRIP = 15 * 60 + 27; // One over max actual length

using ticketOfficeState = std::tuple<
        std::map<std::pair<long long, std::string>, int>,
        std::map<int, std::pair<std::string, int>>,
        std::set<long long>,
        std::set<std::string>, int>;

ticketOfficeState initializeState() {
    return {std::map<std::pair<long long, std::string>, int>(), std::map<int, std::pair<std::string, int>>(),
            std::set<long long>(), std::set<std::string>(), 0};
}

/* Input parsing functions */

void ticketOffice();

std::vector<std::regex> initializeRegexes();

Command getCommand(std::string const &line, std::vector<std::regex> const &regexes);

void parseLineIntoTokens(std::vector<std::string> &tokens, const std::string &line);

void handleCommand(Command command, const std::vector<std::string> &tokens, ticketOfficeState &state,
                   const std::string &line);

int getTimeFromString(const std::string &str);

void printErrorMessage(int lineNumber, const std::string &line);

void trimLeadingZeros(std::string &str);

void handleRouteAddition(const std::vector<std::string> &tokens, ticketOfficeState &state);

bool doesEachTwoStopNamesDiffer(const std::list<std::string> &stopNames);

bool isSequenceOfTimesIncreasing(const std::list<int> &times);

void handleTicketAddition(ticketOfficeState &state, const std::string &line);

void parseLineIntoParts(std::vector<std::string> &parts, const std::string &line);

void handleTicketQuery(const std::vector<std::string> &tokens, ticketOfficeState &state);

bool areFollowingStopsDifferent(std::list<std::string> &stopNames);

/* Engine functions */

bool queryTicket(ticketOfficeState &state, std::list<std::string> stopNames, std::list<long long> serviceNos);

bool addTicket(ticketOfficeState &state, const std::string &ticketName, long long price, long long validTime);

bool addService(ticketOfficeState &state, long long serviceNo, std::list<int> times, std::list<std::string> stopNames);

void printTotalSold(ticketOfficeState &state);

int main() {
    ticketOffice();

    return 0;
}

void ticketOffice() {
    size_t lineNumber = 1;
    std::string line;
    std::vector<std::string> tokens;

    std::vector<std::regex> regexes = initializeRegexes();

    ticketOfficeState state = initializeState();

    while (getline(std::cin, line)) {

        try {
            // match loaded command
            Command command = getCommand(line, regexes);
            // tokenizing input string
            parseLineIntoTokens(tokens, line);
            //
            handleCommand(command, tokens, state, line);
        } catch (std::invalid_argument &a) {

            printErrorMessage(lineNumber, line);
        }

        tokens.clear();
        ++lineNumber;
    }
    printTotalSold(state);
}

void parseLineIntoTokens(std::vector<std::string> &tokens, const std::string &line) {
    size_t last = 0;
    size_t next = 0;
    std::string delimiter = " ";
    while ((next = line.find(delimiter, last)) != std::string::npos) {
        tokens.push_back(line.substr(last, next - last));
        last = next + 1;
    }
    tokens.push_back(line.substr(last));
}

std::vector<std::regex> initializeRegexes() {
    std::vector<std::regex> regexes;

    // route addition
    regexes.emplace_back(R"(^[0-9]+( ([0-9]|1[0-9]|2[0-3]):[0-5][0-9] [a-zA-Z_\^]+){2,}$)");
    // ticket addition
    regexes.emplace_back(R"(^[a-zA-Z ]+ [0-9]+\.[0-9]{2} [1-9][0-9]*$)");
    // question about tickets
    regexes.emplace_back(R"(^\? [a-zA-Z_^]+( [0-9]+ [a-zA-Z_^]+)+$)");
    // ignore line
    regexes.emplace_back(R"(^$)");

    return regexes;
}

Command getCommand(std::string const &line, std::vector<std::regex> const &regexes) {

    if (std::regex_match(line, regexes[0]))
        return ROUTE_ADDITION;
    else if (std::regex_match(line, regexes[1]))
        return TICKET_ADDITION;
    else if (std::regex_match(line, regexes[2]))
        return TICKET_QUERY;
    else if (std::regex_match(line, regexes[3]))
        return LINE_DISREGARDING;
    else
        return NONE;
}

void printErrorMessage(int lineNumber, std::string const &line) {
    std::cerr << "Error in line " << lineNumber << ": " << line << "\n";
}

void handleCommand(Command command, const std::vector<std::string> &tokens, ticketOfficeState &state,
                   const std::string &line) {
    switch (command) {
        case ROUTE_ADDITION:
            handleRouteAddition(tokens, state);
            break;
        case TICKET_ADDITION:
            handleTicketAddition(state, line);
            break;
        case TICKET_QUERY:
            handleTicketQuery(tokens, state);
            break;
        case LINE_DISREGARDING:
            return;
        default:
            throw std::invalid_argument("");
    }
}

void handleRouteAddition(const std::vector<std::string> &tokens, ticketOfficeState &state) {
    long long serviceNo = 0;
    std::list<int> times;
    std::list<std::string> stopNames;

    try {
        std::string candidate = tokens[0];
        trimLeadingZeros(candidate);
        serviceNo = std::stoll(candidate);
    } catch (std::invalid_argument const &ia) {
        throw std::invalid_argument("");
    } catch (std::out_of_range const &oor) {
        throw std::invalid_argument("");
    }

    for (size_t i = 1, len = tokens.size(); i < len; i++) {
        if (i % 2 == 0) {
            stopNames.push_back(tokens[i]);
        } else {
            times.push_back(getTimeFromString(tokens[i]));
        }
    }

    if (!isSequenceOfTimesIncreasing(times)) {
        throw std::invalid_argument("");
    }

    if (!doesEachTwoStopNamesDiffer(stopNames)) {
        throw std::invalid_argument("");
    }

    if (!addService(state, serviceNo, times, stopNames)) {
        throw std::invalid_argument("");
    }
}

int getTimeFromString(const std::string &str) {
    int hours = 0;
    int minutes = 0;

    if (str.length() == 4) {
        hours = std::stoi(str.substr(0, 1));
        minutes = std::stoi(str.substr(2, 2));
    } else {
        hours = std::stoi(str.substr(0, 2));
        minutes = std::stoi(str.substr(3, 2));
    }

    int time = hours * 60 + minutes;

    return time;
}

void trimLeadingZeros(std::string &str) {
    str.erase(0, std::min(str.find_first_not_of('0'), str.size() - 1));
}

bool doesEachTwoStopNamesDiffer(const std::list<std::string> &stopNames) {
    std::set<std::string> setOfStopNames;

    for (const auto &name : stopNames) {
        setOfStopNames.insert(name);
    }

    return setOfStopNames.size() == stopNames.size();
}

bool isSequenceOfTimesIncreasing(const std::list<int> &times) {
    int prevNumOfMins = 5 * 60 + 54;
    int currNumOfMins = 0;

    for (const auto &time : times) {
        currNumOfMins = time;
        if (prevNumOfMins >= currNumOfMins) {
            return false;
        }
        prevNumOfMins = currNumOfMins;
    }

    currNumOfMins = 21 * 60 + 22;
    return prevNumOfMins < currNumOfMins;

}

void handleTicketAddition(ticketOfficeState &state, const std::string &line) {
    long long price = 0;
    std::string ticketName;
    long long validTime = 0;

    std::vector<std::string> parts;
    parseLineIntoParts(parts, line);

    ticketName = parts[0];

    try {
        std::string candidate = parts[1];
        trimLeadingZeros(candidate);
        // removing dot from string representing price
        candidate.erase(candidate.length() - 3, 1);
        price = std::stoll(candidate);

        candidate = parts[2];
        validTime = std::stoll(candidate);
    } catch (std::invalid_argument const &ia) {
        throw std::invalid_argument("");
    } catch (std::out_of_range const &oor) {
        throw std::invalid_argument("");
    }

    // tickets cannot be free of a charge
    if (price == 0) {
        throw std::invalid_argument("");
    }

    if (!addTicket(state, ticketName, price, validTime)) {
        throw std::invalid_argument("");
    }
}

void parseLineIntoParts(std::vector<std::string> &parts, const std::string &line) {
    std::string ticketName;
    std::string ticketPrice;
    std::string ticketDuration;

    int i = 0;
    for (char j : line) {
        if (i == 0) {
            if (isdigit(j)) {
                // additional space
                ticketName.erase(ticketName.length() - 1, 1);
                ticketPrice += j;
                // ticket price now
                i = 1;
            } else {
                ticketName += j;
            }
        } else if (i == 1) {
            if (j == ' ') {
                i = 2;
            } else {
                ticketPrice += j;
            }
        } else {
            ticketDuration += j;
        }
    }

    parts.push_back(ticketName);
    parts.push_back(ticketPrice);
    parts.push_back(ticketDuration);
}

void handleTicketQuery(const std::vector<std::string> &tokens, ticketOfficeState &state) {
    std::list<std::string> stopNames;
    std::list<long long> serviceNos;

    for (int i = 1, len = tokens.size(); i < len; i++) {
        if (i % 2 == 0) {
            try {
                std::string candidate = tokens[i];
                trimLeadingZeros(candidate);
                long long serviceNo = std::stoll(candidate);
                serviceNos.push_back(serviceNo);
            } catch (std::invalid_argument const &ia) {
                throw std::invalid_argument("");
            } catch (std::out_of_range const &oor) {
                throw std::invalid_argument("");
            }
        } else {
            stopNames.push_back(tokens[i]);
        }
    }

    if (!areFollowingStopsDifferent(stopNames)) {
        throw std::invalid_argument("");
    }

    if (!queryTicket(state, stopNames, serviceNos)) {
        throw std::invalid_argument("");
    }
}

bool areFollowingStopsDifferent(std::list<std::string> &stopNames) {
    std::string prevStop = stopNames.front();
    auto it = stopNames.begin();
    auto end = stopNames.end();
    std::advance(it, 1);
    while (it != end) {
        std::string currStop = *it;
        if (prevStop == currStop) {
            return false;
        }
        prevStop = currStop;
        std::advance(it, 1);
    }
    return true;
}

bool queryTicket(ticketOfficeState &state, std::list<std::string> stopNames, std::list<long long> serviceNos) {
    auto &[arrivalTimes, tickets, previousServices, previousTickets, totalSold] = state;
    (void) previousServices; //Structured binding does not use std::ignore, so we have to ignore explicitly
    (void) previousTickets; //Structured binding does not use std::ignore, so we have to ignore explicitly
    auto firstStop = stopNames.begin();
    auto secondStop = std::next(firstStop);
    auto currentTram = serviceNos.begin();
    int totalTime = 0;
    try {
        int currentTime = arrivalTimes.at({*currentTram, *firstStop});
        while (secondStop != stopNames.end()) {
            int firstTime = arrivalTimes.at({*currentTram, *firstStop});
            int secondTime = arrivalTimes.at({*currentTram, *secondStop});
            if (secondTime < firstTime) { //Stops not in order, does not check being late for the tram
                return false;
            }
            if (firstTime < currentTime) { //We are late for the bus
                return false;
            }
            if (firstTime > currentTime) {
                std::cout << ":-( " << *firstStop << std::endl; //We would have to wait
                return true;
            }
            totalTime += secondTime - firstTime;
            std::advance(firstStop, 1);
            std::advance(secondStop, 1);
            std::advance(currentTram, 1);
            currentTime = secondTime;
        }
    } catch (std::out_of_range &) {// Entry not found in map: given tram does not stop at the given stop
        return false;
    }

    int minTicketLength = totalTime + 1;
    int cost[MAX_LENGTH_TRIP] = {};
    std::string possibility[MAX_LENGTH_TRIP] = {};
    int howManyTickets[MAX_LENGTH_TRIP] = {};
    for (const auto &ticket : tickets) {
        auto &time = ticket.first;
        auto&[name, price] = ticket.second;
        int mappedTime = time;
        if (mappedTime >= minTicketLength) {
            mappedTime = minTicketLength;
        }
        if (price < cost[mappedTime] || cost[mappedTime] == 0) {
            cost[mappedTime] = price;
            possibility[mappedTime] = name;
            howManyTickets[mappedTime] = 1;
        }
    }
    for (int additionalTickets = 0; additionalTickets < 2; additionalTickets++) {
        for (int i = minTicketLength - 1; i >= 0; i--) {
            if (cost[i] != 0) {
                for (const auto &ticket : tickets) {
                    auto &time = ticket.first;
                    auto&[name, price] = ticket.second;
                    int mappedTime = time + i;
                    if (mappedTime >= minTicketLength) {
                        mappedTime = minTicketLength;
                    }
                    int newCost = cost[i] + price;
                    if (newCost < cost[mappedTime] || cost[mappedTime] == 0) {
                        cost[mappedTime] = newCost;
                        possibility[mappedTime] = possibility[i] + "; " + name;
                        howManyTickets[mappedTime] = howManyTickets[i] + 1;
                    }
                }
            }
        }
    }
    if (cost[minTicketLength] == 0) {
        std::cout << ":-|" << std::endl;
    } else {
        std::cout << "! " << possibility[minTicketLength] << std::endl;
        totalSold = totalSold + howManyTickets[minTicketLength];
    }
    return true;
}


bool addTicket(ticketOfficeState &state, const std::string &ticketName, long long price, long long validTime) {
    auto &previousTickets = std::get<3>(state);
    if (previousTickets.find(ticketName) != previousTickets.end()) {
        return false;
    }
    previousTickets.insert(ticketName);
    auto &tickets = std::get<1>(state);
    if (validTime > MAX_LENGTH_TRIP) {
        validTime = MAX_LENGTH_TRIP;
    }
    try {
        auto previousTicket = tickets.at(validTime);
        if (previousTicket.second > price) {//if previous ticket of this length was more expensive
            tickets[validTime] = {ticketName, price};
        }
    } catch (std::out_of_range &) {
        tickets[validTime] = {ticketName, price};
    }
    return true;
}

bool addService(ticketOfficeState &state, long long serviceNo, std::list<int> times, std::list<std::string> stopNames) {
    auto &previousServices = std::get<2>(state);
    if (previousServices.find(serviceNo) != previousServices.end()) {
        return false;
    }
    previousServices.insert(serviceNo);
    auto &arrivalTimes = std::get<0>(state);
    auto nextTimeIter = times.begin();
    auto nextStopIter = stopNames.begin();
    auto endTimes = times.end();
    auto endStops = stopNames.end();
    while (nextTimeIter != endTimes && nextStopIter != endStops) {
        arrivalTimes[make_pair(serviceNo, *nextStopIter)] = *nextTimeIter;
        std::advance(nextStopIter, 1);
        std::advance(nextTimeIter, 1);
    }
    return true;
}

void printTotalSold(ticketOfficeState &state) {
    int sold = std::get<4>(state);
    std::cout << sold << std::endl;
}