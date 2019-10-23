#include <cfloat>
#include <string>
#include <vector>
#include <utility>
#include <unordered_map>
#include <iostream>
#include <regex>

constexpr uint TRAM_STOP_NAME= 0;
constexpr uint HOURS = 1;
constexpr uint MINUTES = 2;

constexpr uint VALIDITY = 0;
constexpr uint TRAM_NUMBER = 1;
constexpr uint INPUT_ROUTE = 2;
constexpr uint TICKET_NAME = 1;
constexpr uint TICKET_PRICE = 2;
constexpr uint TICKET_TIME = 3;
constexpr uint QUERY_ROUTE = 1;
constexpr uint START_STOP_NAME = 2;

/* route description <tramStopName, hours, minutes> */
using InputRoute = std::vector <std::tuple <std::string, uint, uint>>;

/* maps tramStopName to pair <hours, minutes> */
using MapRoute = std::unordered_map <std::string, std::pair <uint, uint>>;

/* query route without starting place <tramNumber, tramStopName> */
using QueryRoute = std::vector <std::pair <unsigned long long, std::string>>;

/* maps tramNumber to its MapRoute */
std::unordered_map <unsigned long long, MapRoute> currentRoutes;

/* maps ticket name to <ticketPrice, ticketValidityTime> */
std::unordered_map <std::string, std::pair <float, uint>> currentTickets;

std::tuple <bool, unsigned long long, InputRoute>
parseAddRoute(const std::string& str,
        const std::regex& addRouteRegex, const std::regex& stopDataRegex) {

    std::smatch matches;

    std::string newStr(str);
    unsigned long long tramNumber;
    InputRoute inputRoute;

    if (std::regex_match(str, matches, addRouteRegex)) {
        try {
            tramNumber = std::stoull(matches.str(1));
        }
        catch (std::out_of_range& e) {
            return {false, 0, inputRoute};
        }

        while (std::regex_search(newStr, matches, stopDataRegex)) {
            int hours, minutes;
            try {
                hours = std::stoi(matches.str(1));
                minutes = std::stoi(matches.str(2));
            }
            catch (std::out_of_range& e) {
                return {false, 0, inputRoute};
            }

            if (hours < 5 || hours > 21 ||
                (hours == 5 && minutes < 55) ||
                (hours == 21 && minutes > 21))
                return {false, 0, inputRoute};

            inputRoute.push_back({matches.str(3), hours, minutes});
            newStr = matches.suffix().str();
        }
        return {true, tramNumber, inputRoute};
    }
    return {false, 0, inputRoute};
}

std::tuple <bool, std::string, float, uint>
parseAddTicket(const std::string& str, const std::regex addTicketRegex) {

    std::smatch match;
    float ticketPrice;
    uint ticketTime;
    std::string ticketName;

    if (std::regex_match(str, match, addTicketRegex)) {
        try {
            ticketPrice = std::stof(match.str(2));
            ticketTime = std::stoi(match.str(3));
        }
        catch (std::out_of_range& e) {
            return {false, std::string(""), 0.0, 0};
        }
        return {true, match.str(1), ticketPrice, ticketTime};
    }
    return {false, std::string(""), 0.0, 0};
}

std::tuple <bool, QueryRoute, std::string>
parseQuery(const std::string& str, const std::regex& addQueryRegex,
        const std::regex& queryRouteRegex) {

    std::smatch matches;
    std::string newStr(str);
    std::string startStopName;
    QueryRoute queryRoute;

    if (std::regex_match(str, matches, addQueryRegex)) {
        startStopName = matches.str(1);

        while (std::regex_search(newStr, matches, queryRouteRegex)) {
            std::string busStopName = matches.str(2);
            unsigned long long tramNumber;

            try {
                tramNumber = std::stoull(matches.str(1));
            }
            catch (std::out_of_range& e) {
                return {false, queryRoute, std::string("")};
            }

            queryRoute.push_back({tramNumber, busStopName});
            newStr = matches.suffix().str();
        }
        return {true, queryRoute, startStopName};
    }
    return {false, queryRoute, startStopName};
}

void printError(uint rowNumber, const std::string& str) {
    std::cerr << "Error in line " << rowNumber << ": " << str << "\n";
}

/* check if tram stop names are unique on added route */
bool checkUniqueness(const InputRoute& route) {
    std::unordered_map <std::string, int> uniqueness;

    for (auto const& r : route) {
        ++uniqueness[std::get<TRAM_STOP_NAME>(r)];
    }

    for (auto const& el : uniqueness) {
        if (el.second > 1) return false;
    }

    return true;
}

/* check if times are increasing on added route */
bool checkIncreasing(const InputRoute& route) {

    auto lastTime = std::pair(std::get<HOURS>(route[0]), std::get<MINUTES>(route[0]));
    for (uint i = 1; i < route.size(); ++i) {
        auto currTime = std::pair(std::get<HOURS>(route[i]), std::get<MINUTES>(route[i]));
        if (currTime <= lastTime) return false;
        lastTime = currTime;
    }

    return true;
}

bool addRoute(unsigned long long tramNo, const InputRoute& route) {

    if (currentRoutes.find(tramNo) != currentRoutes.end()) {
        return false;
    }

    if (!checkUniqueness(route) || !checkIncreasing(route)) {
        return false;
    }

    MapRoute stopsForInsert;
    for (auto const& r : route) {
        stopsForInsert.emplace(
            std::get<TRAM_STOP_NAME>(r),
            std::pair(std::get<HOURS>(r), std::get<MINUTES>(r))
        );
    }

    currentRoutes.emplace(tramNo, stopsForInsert);
    return true;
}

bool addTicket(const std::string& ticketName, float ticketCost, uint ticketTime) {

    if (currentTickets.find(ticketName) != currentTickets.end()) {
        return false;
    }

    currentTickets.emplace(ticketName, std::pair(ticketCost, ticketTime));
    return true;
}

/* check if the query route is valud */
bool validateStops(const std::string& start, const QueryRoute& route) {
    auto lastStop = start;

    if (route.size() < 1) return false;
    if (currentRoutes.find(route[0].first) == currentRoutes.end()) return false;
    if (currentRoutes[route[0].first].find(start)
        == currentRoutes[route[0].first].end()) return false;

    auto lastTime = currentRoutes[route[0].first][route[0].second];
    lastStop = route[0].second;
    for (uint i = 1; i < route.size(); ++i) {
        auto tramNo = route[i].first;
        auto stop = route[i].second;
        if (currentRoutes.find(tramNo) == currentRoutes.end()) return false;
        if (currentRoutes[tramNo].find(stop) == currentRoutes[tramNo].end()) return false;
        if (currentRoutes[tramNo].find(lastStop) == currentRoutes[tramNo].end()) return false;
        if (currentRoutes[tramNo][lastStop] < lastTime) return false;
        lastTime = currentRoutes[tramNo][lastStop];
        if (currentRoutes[tramNo][stop] <= lastTime) return false;
        lastTime = currentRoutes[tramNo][stop];
        lastStop = stop;
    }

    return true;
}

std::string checkIfNotWaiting(const QueryRoute& route) {

    auto& lastTime = currentRoutes[route[0].first][route[0].second];
    for (uint i = 1; i < route.size(); ++i) {
        auto const& tramNo = route[i].first;
        auto const& stop = route[i].second;
        auto& currentTime = currentRoutes[tramNo][route[i - 1].second];
        if (currentTime == lastTime) {
            lastTime = currentRoutes[tramNo][stop];
        } else {
            return ":-( " + route[i - 1].second;
        }
    }

    return "";
}

std::pair <int, std::string> calculateTickets(uint minutes) {
    float cost = FLT_MAX;
    uint howManyTickets = 4;
    std::string result[3];
    for (auto const& firstTicket : currentTickets) {

        auto currTime = firstTicket.second.second;
        auto currCost = firstTicket.second.first;
        if (currTime > minutes && currCost <= cost) {
            howManyTickets = 1;
            cost = currCost;
            result[0] = firstTicket.first;
        }

        for (auto const& secondTicket : currentTickets) {

            currTime += secondTicket.second.second;
            currCost += secondTicket.second.first;
            if (currTime > minutes && currCost <= cost) {
                howManyTickets = 2;
                cost = currCost;
                result[0] = firstTicket.first;
                result[1] = secondTicket.first;
            }

            for (auto const& thirdTicket : currentTickets) {

                currTime += thirdTicket.second.second;
                currCost += thirdTicket.second.first;
                if (currTime > minutes && currCost <= cost) {
                    howManyTickets = 3;
                    cost = currCost;
                    result[0] = firstTicket.first;
                    result[1] = secondTicket.first;
                    result[2] = thirdTicket.first;
                }
                currTime -= thirdTicket.second.second;
                currCost -= thirdTicket.second.first;
            }

            currTime -= secondTicket.second.second;
            currCost -= secondTicket.second.first;
        }
    }

    if (cost == FLT_MAX) {
        return std::pair(0, ":-|");
    }

    std::string output = "!";
    if (howManyTickets > 0) {
        output += " " + result[0];
    }
    if (howManyTickets > 1) {
        output += "; " + result[1];
    }
    if (howManyTickets > 2) {
        output += "; " + result[2];
    }

    return std::pair(howManyTickets, output);
}

int getTimeRange(std::pair<unsigned long long, std::string> start,
                 std::pair<unsigned long long, std::string> end) {

    auto startTime = currentRoutes[start.first][start.second];
    auto endTime = currentRoutes[end.first][end.second];

    if (endTime.second < startTime.second) {
        --endTime.first;
        endTime.second += 60;
    }

    return 60 * (endTime.first - startTime.first) + endTime.second - startTime.second;
}

std::pair<bool, std::pair<int, std::string>> query(std::string& start, QueryRoute& route) {

    if (!validateStops(start, route)) {
        return std::pair(false, std::pair(0, ""));
    }

    auto tmpCheckTime = checkIfNotWaiting(route);
    if (tmpCheckTime.size() > 0) {
        return std::pair(true, std::pair(0, tmpCheckTime));
    }

    return std::pair(
                true,
                calculateTickets(
                    getTimeRange(
                        std::pair(route[0].first, start),
                        std::pair(route.back().first, route.back().second)
                    )
                )
           );

}

int main() {

    int countBoughtTickets = 0;
    uint rowNumber = 0;

    const std::regex addRouteRegex
    (R"(([0-9]+) (?:((?:1|2(?![4-9]))?\d):([0-5]\d) ([a-zA-Z^_]+)(?: (?=\d))?)+)");

    const std::regex stopDataRegex
    (R"(((?:1|2(?![4-9]))?\d):([0-5]\d) ([a-zA-Z^_]+))");

    const std::regex addTicketRegex(R"(([a-zA-Z ]+) (\d+\.\d{2}) ([1-9]\d*))");

    const std::regex addQueryRegex
    (R"(\? ([a-zA-Z_^]+) (?:(\d+) ([a-zA-Z_^]+)(?: (?=\d))?)+)");

    const std::regex queryRouteRegex(R"((\d+) ([a-zA-Z_^]+))");

    std::string line;
    while (std::getline(std::cin, line)) {
        ++rowNumber;

        if (line.empty())
            continue;

        if (std::regex_match(line, addRouteRegex)) {

            auto inputData = parseAddRoute(line, addRouteRegex, stopDataRegex);
            if (std::get<VALIDITY>(inputData)) {
                if (!addRoute(std::get<TRAM_NUMBER>(inputData),
                              std::get<INPUT_ROUTE>(inputData))) {
                    printError(rowNumber, line);
                }
            } else {
                printError(rowNumber, line);
            }
        } else if (std::regex_match(line, addTicketRegex)) {

            auto inputData = parseAddTicket(line, addTicketRegex);
            if (std::get<VALIDITY>(inputData)) {
                if (!addTicket(std::get<TICKET_NAME>(inputData),
                               std::get<TICKET_PRICE>(inputData),
                               std::get<TICKET_TIME>(inputData))) {
                    printError(rowNumber, line);
                }
            } else {
                printError(rowNumber, line);
            }

        } else if (std::regex_match(line, addQueryRegex)) {

            auto inputData = parseQuery(line, addQueryRegex, queryRouteRegex);
            if (std::get<VALIDITY>(inputData)) {

                auto queryResults = query(std::get<START_STOP_NAME>(inputData),
                                          std::get<QUERY_ROUTE>(inputData));

                if (queryResults.first) {
                    countBoughtTickets += queryResults.second.first;
                    std::cout << queryResults.second.second << "\n";
                } else {
                    printError(rowNumber, line);
                }

            } else {
                printError(rowNumber, line);
            }
        } else {
            printError(rowNumber, line);
        }
    }

    std::cout << countBoughtTickets << "\n";
}
