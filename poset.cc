#include <map>
#include <optional>

using Node = pair<set<int>, set<int>>;

using Poset = std::optional<pair<map<std::string, int>, map<int, Node>>>;

// vector<> string_to_int;
vector<Poset> poset_;

stack<int> available;

