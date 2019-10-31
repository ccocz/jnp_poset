#include "poset.h"

#include <iostream>
#include <ios>
#include <string>
#include <optional>
#include <vector>
#include <stack>
#include <map>
#include <set>
#include <cassert>
#include <cstring>

namespace
{
using Node = std::pair<std::set<int>, std::set<int>>; // first: in edges, second: out edges
using Poset = std::tuple<std::map< std::string, int>, std::map<int, Node>, int>;

std::vector<std::optional<Poset>> &poset_list()
{
    static std::vector<std::optional<Poset>> list;
    return list;
}

std::stack<int> &available()
{
    static std::stack<int> stack;
    return stack;
}

void message(std::string message)
{
    static std::ios_base::Init stream;
    std::cerr << message << std::endl;
}

// Checks if arguments are correct, and prints error messages about wrong arguments
bool validArgument(unsigned int id, const char *value1, const char *value2, std::string function_name) 
{
    if (id >= poset_list().size() || !poset_list()[id].has_value()) 
    {
        message(function_name + ": poset " + std::to_string(id) + " does not exist");
        return false;
    }
    
    if (value1 == nullptr)
        message(function_name + ": invalid value1 (NULL)");

    if (value2 == nullptr)
        message(function_name + ": invalid value2 (NULL)");

    if (value1 == nullptr || value2 == nullptr)
        return false;
        
    return true;
}
}

unsigned long jnp1::poset_new(void)
{
    message("poset_new()");
    Poset empty_poset;
    int new_poset_index;
    
    if (available().empty())
    {
        new_poset_index = poset_list().size();
        poset_list().emplace_back(empty_poset);
    }
    else
    {
        new_poset_index = available().top();
        available().pop();
        poset_list()[new_poset_index] = empty_poset;
    }
    
    message("poset_new: poset " + std::to_string(new_poset_index) + " created");
    
    return new_poset_index;
}

void jnp1::poset_delete(unsigned long id)
{
    message("poset_delete(" + std::to_string(id) + ")");
    
    if (validArgument(id, "VALID", "VALID", "poset_delete"))
    {
        poset_list()[id] = {};
        available().push(id);
        message("poset_delete: poset " + std::to_string(id) + " deleted");
    }
}

std::size_t jnp1::poset_size(unsigned long id)
{
    message("poset_size(" + std::to_string(id) + ")");
    
    if (validArgument(id, "VALID", "VALID", "poset_size"))
    {
        auto &[string_to_int, graph, max_index] = poset_list()[id].value();
        std::size_t size = graph.size();
        message("poset_size: poset " + std::to_string(id) + " contains "
                + std::to_string(size) + " element(s)");
        return size;
    }
    
    return 0;
}

bool jnp1::poset_insert(unsigned long id, char const *value)
{
    std::string name((value==nullptr)?"NULL":value);
    message("poset_insert(" + std::to_string(id) + ", \"" + name + "\")");
    
    if (!validArgument(id, value, "VALID", "poset_insert")) 
        return false;
    
    auto &[string_to_int, graph, max_index] = poset_list()[id].value();
    if (string_to_int.find(name) != string_to_int.end()) 
    {
        message("poset_insert: poset " + std::to_string(id) + ", element \""
                + name + "\" already exists");
        return false;
    }
    
    graph[max_index].second.insert(max_index);
    graph[max_index].first.insert(max_index);
    string_to_int[name] = max_index++;
    
    message("poset_insert: poset " + std::to_string(id)
            + ", element \"" + name + "\" added");
    
    return true;
}

bool jnp1::poset_remove(unsigned long id, char const *value)
{
    std::string element_name((value==nullptr)?"NULL":value);
    message("poset_remove(" + std::to_string(id) + ", \"" + element_name + "\")");
    
    if (!validArgument(id, value, "VALID", "poset_remove"))
        return false;
        
    auto &[string_to_int, graph, max_index] = poset_list()[id].value();
    auto element_iterator = string_to_int.find(element_name);
    if (element_iterator == string_to_int.end())
    {
        message("poset_remove: poset " + std::to_string(id) + ", element \""
                + element_name + "\" does not exist");
        return false;
    }
    
    int index = element_iterator->second;
    auto &[in, out] = graph[index];
    for (auto iterator = in.begin(); iterator != in.end(); iterator++)
    {
        auto &[other_in, other_out] = graph[*iterator];
        other_out.erase(other_out.find(index));
    }
    
    for (auto iterator = out.begin(); iterator != out.end(); iterator++)
    {
        auto &[other_in, other_out] = graph[*iterator];
        other_in.erase(other_in.find(index)); // todo: check if other_in.find(index) == end
    }
    
    graph.erase(index);
    string_to_int.erase(element_iterator);
    
    message("poset_remove: poset " + std::to_string(id)
            + ", element \"" + element_name + "\" removed");
            
    return true;
}

bool jnp1::poset_add(unsigned long id, char const *value1, char const *value2)
{
    std::string element1_name((value1==nullptr)?"NULL":value1);
    std::string element2_name((value2==nullptr)?"NULL":value2);

    message("poset_add(" + std::to_string(id) + ", \"" + element1_name + "\", \""
            + element2_name + "\")");

    if (!validArgument(id, value1, value2, "poset_add"))
        return false;
        
    auto &[string_to_int, graph, max_index] = poset_list()[id].value();
    
    auto element1_iterator = string_to_int.find(element1_name);
    auto element2_iterator = string_to_int.find(element2_name);
    
    if (element1_iterator == string_to_int.end()
        || element2_iterator  == string_to_int.end())
    {
        message("poset_add: poset " + std::to_string(id) + ", element \""
                + element1_name + "\" or \"" + element2_name + "\" does not exist");
        return false;
    }
    
    int index1 = element1_iterator->second;
    int index2 = element2_iterator->second;
    
    if (jnp1::poset_test(id, element1_name.c_str(), element2_name.c_str())
        || jnp1::poset_test(id, element2_name.c_str(), element1_name.c_str()))
    {
        message("poset_add: poset " + std::to_string(id) + ", relation (\""
                + element1_name + "\" ,\"" + element2_name + "\") cannot be added");
        return false;
    }

    auto &[in1, out1] = graph[index1];
    auto &[in2, out2] = graph[index2];

    std::vector<int> in1_list;
    std::vector<int> out2_list;
    
    for (auto iterator = in1.begin(); iterator != in1.end(); iterator++)
        in1_list.push_back(*iterator);
    
    for (auto iterator = out2.begin(); iterator != out2.end(); iterator++)
        out2_list.push_back(*iterator);
    
    for (int a : in1_list)
    {
        auto &[in_a, out_a] = graph[a];
        for (int b : out2_list)
        {
            auto &[in_b, out_b] = graph[b];
            out_a.insert(b);
            in_b.insert(a);
        }
    }
    
    message("poset_add: poset " + std::to_string(id) + ", relation (\""
            + element1_name + "\" ,\"" + element2_name + "\") added");
            
    return true;
}

bool jnp1::poset_del(unsigned long id, char const *value1, char const *value2)
{
    std::string element1_name((value1==nullptr)?"NULL":value1);
    std::string element2_name((value2==nullptr)?"NULL":value2);
    
    message("poset_del(" + std::to_string(id) + ", \"" + element1_name + "\", \""
            + element2_name + "\")");
            
    if(!validArgument(id, value1, value2, "poset_del"))
        return false;
    
    if (!poset_test(id, value1, value2)) 
    {
        message("poset_del: poset " + std::to_string(id) + ", relation (\""
                + element1_name + "\" ,\"" + element2_name + "\") cannot be deleted");
        return false;
    }
    
    auto &[string_to_int, graph, max_index] = poset_list()[id].value();
    auto &[in, out] = graph[string_to_int[element1_name]];
    auto &[in2, out2] = graph[string_to_int[element2_name]];
    int index1 = string_to_int[value1];
    int index2 = string_to_int[value2];
    if (index1 == index2) 
    {
        message("poset_del: poset " + std::to_string(id) + ", relation (\""
                + element1_name + "\" ,\"" + element2_name + "\") cannot be deleted");
        return false;
    }
    
    for (int outgoing : out) 
    {
        if (outgoing != index1 && outgoing != index2 &&
            graph[outgoing].second.find(index2) != graph[outgoing].second.end()) 
        {
            message("poset_del: poset " + std::to_string(id) + ", relation (\""
                    + element1_name + "\" ,\"" + element2_name + "\") cannot be deleted");
            return false;
        }
    }
    
    out.erase(index2);
    in2.erase(index1);
    
    message("poset_del: poset " + std::to_string(id) + ", relation (\""
            + element1_name + "\" ,\"" + element2_name + "\") deleted");
            
    return true;
}

bool jnp1::poset_test(unsigned long id, char const *value1, char const *value2)
{
    std::string name1((value1==nullptr)?"NULL":value1);
    std::string name2((value2==nullptr)?"NULL":value2);
    
    message("poset_test(" + std::to_string(id) + ", \"" + name1 + "\", \""
            + name2 + "\")");
    
    if (!validArgument(id, value1, value2, "poset_test")) 
        return false;
    
    auto &[string_to_int, graph, max_index] = poset_list()[id].value();
    auto element1_iterator = string_to_int.find(name1);
    auto element2_iterator = string_to_int.find(name2);
    if (element1_iterator == string_to_int.end() ||
        element2_iterator == string_to_int.end()) 
    {
        message("poset_test: poset " + std::to_string(id) + ", element \""
                + name1 + "\" or \"" + name2 + "\" does not exist");
        return false;
    }
    
    int index2 = element2_iterator -> second;
    int index1 = element1_iterator -> second;
    auto &[in, out] = graph[index1];
    
    if (out.find(index2) != out.end())
    {
        message("poset_test: poset " + std::to_string(id) + ", relation (\""
                + name1 + "\" ,\"" + name2 + "\") exists");
        return true;
    }
    else
    {
        message("poset_test: poset " + std::to_string(id) + ", relation (\""
                + name1 + "\" ,\"" + name2 + "\") does not exixt");
        return false;
    }
}

void jnp1::poset_clear(unsigned long id)
{
    message("poset_clear(" + std::to_string(id) + ")");
    
    if (!validArgument(id, "VALID", "VALID", "poset_clear")) 
        return;

    auto &[string_to_int, graph, max_index] = poset_list()[id].value();
    string_to_int.clear();
    graph.clear();
    max_index = 0;
    
    message("poset_new: poset " + std::to_string(id) + " cleared");
}
