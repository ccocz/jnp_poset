#include <iostream>
#include <vector>
#include <variant>
#include <tuple>
#include <map>
#include <set>
#include <stdexcept>
#include <regex>
#include <string>
#include <optional>
#include <functional>
#include <algorithm>
#include <limits>

using Time = unsigned int;
using Ticket = std::tuple<std::string, unsigned long long, Time>; // (name, price, duration)
using Line = std::pair<unsigned int, std::vector<std::pair<std::string, Time>>>; // (line id, stops)
using Track = std::vector<std::pair<std::string, unsigned int>>;

const unsigned long long max_price = std::numeric_limits<unsigned long long>::max();
const std::string lz_number_pattern = "0*([0]|[1-9][0-9]*)";
const std::string time_pattern = "(1?[0-9]|2[0-3]):([0-5][0-9])";
const std::string word_pattern = "([a-zA-Z^_]+)";
const std::string phrase_pattern = "([a-zA-Z ]+)";
const std::string number_pattern = "([1-9][0-9]*)";
const std::string dec_number_pattern = "([0-9]*)\\.([0-9]{2})";
const std::string empty_ticket_name = "";

/* Converts time from HH:MM format to single integer, number of minutes. */
Time make_time(unsigned int hours, unsigned int minutes) {
    return hours * 60 + minutes;
}

/* Checks if given line is correct, and adds it to time_table if it is. */
bool check_line(Line const &line, std::map<std::pair<std::string, unsigned int>, Time> &time_table,
        std::set<int> &line_id_set)
{
    auto [line_id, line_stops] = line;
    
    if (line_id_set.find(line_id) != line_id_set.end())
        return false; // Line with given id already exists.
    
    std::set<std::string> stops_visted;
    Time previous_stop_time = 0;
    for (auto [name, time] : line_stops) 
    {
        if (time < make_time(5,55) || time > make_time(21,21))
            return false; // Time out of range.
        
        if (time <= previous_stop_time)
            return false; // Time not greater than previous one.
        else
            previous_stop_time = time;
        
        if (stops_visted.find(name) != stops_visted.end())
            return false; // There is a loop in line.
        else
            stops_visted.insert(name);
    }
    
    for (auto [name, time] : line_stops)
        time_table[std::make_pair(name, line_id)] = time;
    
    line_id_set.insert(line_id);
    
    return true;
}

/* Returns travel time of given track. If track is incorrect, std::monostate is returned. 
 * If there is stop where passenger needs to wait, the name of the first such stop is returned.
 */
std::variant<Time, std::string, std::monostate> get_travel_time(Track &track, 
        std::map<std::pair<std::string, unsigned int>, Time> &time_table)
{
    Time travel_time = 0;
    std::string wait_stop_name = "";
    
    auto [first_stop_name, first_line_id] = track[0];
    auto prev_time_iterator = time_table.find(std::make_pair(first_stop_name, first_line_id));
    for (unsigned int i = 0; i < track.size() - 1; i++)
    {
        auto [name, line_id] = track[i];
        std::string next_stop_name = track[i+1].first;
        auto stop1_time_iterator = time_table.find(std::make_pair(name, line_id));
        auto stop2_time_iterator = time_table.find(std::make_pair(next_stop_name, line_id));
        
        if (stop1_time_iterator == time_table.end() 
           || stop2_time_iterator == time_table.end()
           || prev_time_iterator->second > stop1_time_iterator->second
           || stop1_time_iterator->second >= stop2_time_iterator->second)
            return std::monostate();
        else if (prev_time_iterator->second < stop1_time_iterator->second && wait_stop_name == "")
            wait_stop_name = name;
        
        prev_time_iterator = stop2_time_iterator;
        travel_time += stop2_time_iterator->second - stop1_time_iterator->second;
    }

    if (wait_stop_name != "")
        return wait_stop_name;
    else 
        return travel_time + 1;
}

/* Returns vector of tickets which is the cheapest or std::nullopt, when there
 * is no valid set.
 */
std::optional<std::vector<Ticket>> get_best_tickets(Time travel_time, std::vector<Ticket> &ticket_list)
{
    unsigned long long best_price = max_price;
    std::vector<Ticket> best_tickets;
    for (auto ticket1 : ticket_list) 
    {
        for (auto ticket2 : ticket_list) 
        {
            for (auto ticket3 : ticket_list) 
            {
                auto [name1, price1, time1] = ticket1;
                auto [name2, price2, time2] = ticket2;
                auto [name3, price3, time3] = ticket3;
                
                unsigned long long current_tickets_travel_time = time1 + time2 + time3;
                
                unsigned long long current_tickets_price = price1 + price2 + price3;
                
                if (current_tickets_travel_time >= travel_time 
                    && current_tickets_price <= best_price) 
                {
                    best_tickets.clear();
                    
                    best_price = current_tickets_price;
                    if (name1 != empty_ticket_name) 
                        best_tickets.push_back(ticket1);
                    if (name2 != empty_ticket_name) 
                        best_tickets.push_back(ticket2);
                    if (name3 != empty_ticket_name) 
                        best_tickets.push_back(ticket3);
                }
            }
        }
    }

    if (best_price != max_price) 
        return best_tickets;
    else 
        return {};
}

/* Parses sequence of consecutive patterns using given function. */
template<typename T>
std::optional<std::vector<T>> parse_sequence(std::string::const_iterator beg_it,
        std::string::const_iterator end_it, std::regex reg,
        std::function<T(std::smatch)> parse_function)
{
    auto r_beg_it = std::sregex_iterator(beg_it, end_it, reg,
            std::regex_constants::match_continuous);
    auto r_end_it = std::sregex_iterator();
    bool hit_end = false;

    std::vector<T> res;
    std::transform(r_beg_it, r_end_it, std::back_inserter(res),
        [&] (std::smatch match)
        {
            if (match[0].second == end_it)
                hit_end = true;

            return parse_function(match);
        });

    if (hit_end)
        return res;
    else
        return {};
}

/* Adds new ticket to ticket_list */
bool add_ticket(std::string name, int price, int duration, std::vector<Ticket> &ticket_list)
{
    auto exists = std::find_if(ticket_list.begin(), ticket_list.end(),
            [&name] (const Ticket &t)
            {
                return std::get<0>(t) == name;
            });

    if (exists == ticket_list.end()) 
    {
        ticket_list.push_back(std::make_tuple(name, price, duration));
        return true;
    }
    
    return false;
}

/* Parses one line of input. Returns false if line is incorrect or true otherwise */
bool perform_command(std::string text, std::vector<Ticket> &ticket_list, 
        std::map<std::pair<std::string, unsigned int>, Time> &time_table, 
        std::set<int> &line_id_set, unsigned int &ticket_count) 
{
    const std::regex lz_number(lz_number_pattern);
    const std::regex question_mark("\\?");
    const std::regex name_price_time(phrase_pattern + " " + dec_number_pattern + " " + number_pattern);
    const std::regex time_word(" " + time_pattern + " " + word_pattern);
    const std::regex word_number(" " + word_pattern + "(?: " + lz_number_pattern + ")?");

    std::smatch match;
    if (text == "") 
    {
        return true;
    }
    else if (std::regex_search(text, match, question_mark, std::regex_constants::match_continuous))
    {
        std::string::const_iterator it = match[0].second;
        try 
        {
            auto vec = parse_sequence<std::pair<std::string, unsigned int>>(it, text.end(), word_number,
                        [&](std::smatch m) 
                        {
                        return std::make_pair(
                                m[1],
                                m[2] != "" ? (std::stoi(m[2])) : (std::numeric_limits<unsigned int>::max())
                                );
                        });
            if (vec && vec.value().size() > 1)
            {
                std::variant<Time, std::string, std::monostate> res = get_travel_time(vec.value(), time_table);
                if (std::holds_alternative<std::monostate>(res))
                {
                    return false;
                }
                else if (std::holds_alternative<std::string>(res))
                {
                    std::cout << ":-( " << std::get<std::string>(res) << std::endl;
                }
                else  
                {
                    auto tickets = get_best_tickets(std::get<Time>(res), ticket_list);

                    if (tickets) 
                    {
                        ticket_count += tickets.value().size();
                        std::cout << "! ";
                        for (unsigned int i = 0; i < tickets.value().size(); ++i) 
                        {
                            std::cout << std::get<0>(tickets.value()[i]);
                            if (i != tickets.value().size() - 1)
                                std::cout << "; ";
                        }
                        std::cout << std::endl;
                    }
                    else
                    {
                        std::cout << ":-|" << std::endl;
                    }
                }

                return true;
            }
            else 
            {
                return false;
            }
        }
        catch (std::logic_error &e)
        {
            return false;
        }

    }
    else if (std::regex_search(text, match, lz_number, std::regex_constants::match_continuous))
    {
        try 
        {
            unsigned int num = std::stoi(match[1]);

            std::string::const_iterator it = match[0].second;
            auto vec =
                parse_sequence<std::pair<std::string, unsigned int>>(it, text.end(), time_word,
                        [&](std::smatch m)
                        {
                            return std::make_pair(
                                m[3],
                                make_time(std::stoi(m[1]), std::stoi(m[2]))
                                );
                        });
            
            if (vec)
                return check_line(std::make_pair(num, vec.value()), time_table, line_id_set);
            else 
                return false;
        }
        catch (std::logic_error &e)
        {
            return false;
        }
        
    }
    else if (std::regex_match(text, match, name_price_time))
    {
        try 
        {
            std::string name = match[1];
            unsigned int price = std::stoi(match[2]) * 100 + std::stoi(match[3]);
            unsigned int duration = std::stoi(match[4]);

            return add_ticket(name, price, duration, ticket_list);
        }
        catch (std::logic_error &e)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    return true;
}

int main() 
{
    unsigned int ticket_count = 0;
    std::vector<Ticket> ticket_list(1, std::make_tuple("", 0, 0));
    std::map<std::pair<std::string, unsigned int>, Time> time_table;
    std::set<int> line_id_set;

    std::string line;
    unsigned int num = 1;
    while (getline(std::cin, line)) 
    {
        if (!perform_command(line, ticket_list, time_table, line_id_set, ticket_count)) 
        {
            std::cerr << "Error in line " << num << ": " << line << std::endl;
        }
        ++num;
    }
    
    std::cout << ticket_count << std::endl;
    return 0;
}
