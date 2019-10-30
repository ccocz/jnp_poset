#include "poset.h"
#include <iostream>
#include <ios>
#include <string>
#include <optional>
#include <vector>
#include <stack>
#include <map>
#include <set>

// todo: change int to unsigned int. We don't need negative values.
using Node = std::pair<std::set<int>, std::set<int>>; // first: in edges, second: out edges
using Poset = std::pair<std::map< std::string, int>, std::map<int, Node>>;

std::vector<std::optional<Poset>> poset_list;
std::stack<int> available;

void message(std::string message)
{
	static std::ios_base::Init stream;
	std::cerr << message << std::endl;
}

unsigned long jnp1::poset_new(void) 
{
	message("poset_new()");
	Poset empty_poset;
	int new_poset_index;

	if (available.empty())
	{
		new_poset_index = poset_list.size();
		poset_list.push_back(empty_poset);
	}
	else
	{
		new_poset_index = available.top();
		available.pop();
		poset_list[new_poset_index] = empty_poset;
	}

	message("poset_new: poset " + std::to_string(new_poset_index) + " created");

	return new_poset_index;
}

void jnp1::poset_delete(unsigned long id) 
{
	message("poset_delete(" + std::to_string(id) + ")");
	
	if (id < poset_list.size() && poset_list[id].has_value()) 
	{
		poset_list[id] = {};
		available.push(id);
		message("poset_delete: poset " + std::to_string(id) + " deleted");
	}
	else
	{
		message("poset_delete: poset " + std::to_string(id) + " does not exist");
	}
}

std::size_t jnp1::poset_size(unsigned long id)
{
	message("poset_size(" + std::to_string(id) + ")");
	if (id < poset_list.size() && poset_list[id].has_value()) 
	{
		std::size_t size = poset_list[id].value().first.size();
		message("poset_size: poset " + std::to_string(id) + " contains "
		        + std::to_string(size) + " element(s)");
		return size;
	}

	message("poset_size: poset " + std::to_string(id) + " does not exist");
	
	return 0;
}

bool jnp1::poset_insert(unsigned long id, char const *value)
{
	return false;
}

bool jnp1::poset_remove(unsigned long id, char const *value)
{
	std::string element_name((value==nullptr)?"NULL":value);		
		
	message("poset_remove(" + std::to_string(id) + ", \"" + element_name + "\")");
	
	if (id >= poset_list.size() || !poset_list[id].has_value())
	{
		message("poset_remove: poset" + std::to_string(id) + "does not exist");
		return false;
	}

	if (value == nullptr)
	{
		message("poset_remove: invalid value (NULL)");
		return false;
	}

	auto &[string_to_int, graph] = poset_list[id].value();

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
		other_in.erase(other_in.find(index));
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
	
	if (id >= poset_list.size() || !poset_list[id].has_value())
	{
		message("poset_add: poset" + std::to_string(id) + "does not exist");
		return false;
	}

	if (value1 == nullptr)
		message("poset_add: invalid value1 (NULL)");
		
	if (value2 == nullptr)
		message("poset_add: invalid value2 (NULL)");
		
	if (value1 == nullptr || value2 == nullptr)
		return false;
	
	auto &[string_to_int, graph] = poset_list[id].value();

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

	// todo: adding function poset_test(int id, int index1, int index2) can reduce number of map calls.
	if (jnp1::poset_test(id, element1_name.c_str(), element2_name.c_str())
		|| jnp1::poset_test(id, element2_name.c_str(), element1_name.c_str()))
	{
		message("poset_add: poset " + std::to_string(id) + ", relation (\""
		        + element1_name + "\" ,\"" + element2_name + "\") cannot be added");
		return false;
	}

	auto &[in1, out1] = graph[index1];
	auto &[in2, out2] = graph[index2];

	for (auto iterator = in1.begin(); iterator != in1.end(); iterator++)
	{
		auto &[other_in, other_out] = graph[*iterator];
		other_out.insert(index2);
		in2.insert(*iterator);
	}

	for (auto iterator = out2.begin(); iterator != out2.end(); iterator++)
	{
		auto &[other_in, other_out] = graph[*iterator];
		out1.insert(*iterator);
		other_in.insert(index1);
	}

	message("poset_add: poset " + std::to_string(id) + ", relation (\""
			+ element1_name + "\" ,\"" + element2_name + "\") added");
	return true;
}

bool jnp1::poset_del(unsigned long id, char const *value1, char const *value2)
{
	return false;
}

bool jnp1::poset_test(unsigned long id, char const *value1, char const *value2)
{
	return true;
}

void jnp1::poset_clear(unsigned long id)
{
	
}
// 
// int main()
// {
// 
// 	int index = jnp1::poset_new();
// 
// 	std::cout << index << std::endl;
// 
// 	return 0;
// }
