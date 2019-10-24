#include "poset.h"

#include <iostream>
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

unsigned long jnp1::poset_new(void) 
{
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

	return new_poset_index;
}

void jnp1::poset_delete(unsigned long id) 
{
	if (id < poset_list.size() && poset_list[id].has_value()) 
	{
		poset_list[id] = {};
		available.push(id);
	}
}

std::size_t jnp1::poset_size(unsigned long id)
{
	if (id < poset_list.size() && poset_list[id].has_value()) 
	{
		return poset_list[id].value().first.size();
	}

	return 0;
}

bool jnp1::poset_remove(unsigned long id, char const *value)
{
	if (id >= poset_list.size() || !poset_list[id].has_value())
	{
		return false;
	}

	auto &[string_to_int, graph] = poset_list[id].value();

	std::string element_name(value);
	auto element_iterator = string_to_int.find(element_name);

	if (element_iterator == string_to_int.end()) 
	{
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

	return true;
}

bool jnp1::poset_add(unsigned long id, char const *value1, char const *value2)
{
	if (id >= poset_list.size() || !poset_list[id].has_value())
	{
		return false;
	}

	auto &[string_to_int, graph] = poset_list[id].value();

	std::string element1_name(value1);
	std::string element2_name(value2);

	auto element1_iterator = string_to_int.find(element1_name);
	auto element2_iterator = string_to_int.find(element2_name);

	if (element1_iterator == string_to_int.end()
		|| element2_iterator  == string_to_int.end())
	{
		return false;
	}

	int index1 = element1_iterator->second;
	int index2 = element2_iterator->second;

	// todo: adding function poset_test(int id, int index1, int index2) can reduce number of map calls.
	if (jnp1::poset_test(id, element1_name.c_str(), element2_name.c_str())
		|| jnp1::poset_test(id, element2_name.c_str(), element1_name.c_str()))
	{
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

	return true;
}

bool jnp1::poset_test(unsigned long id, char const *value1, char const *value2)
{
	return true;
}

int main()
{

	int index = jnp1::poset_new();

	std::cout << index << std::endl;

	return 0;
}