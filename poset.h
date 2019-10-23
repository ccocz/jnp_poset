
namespace jnp1 
{
// a
unsigned long poset_new(void);

// a
void poset_delete(unsigned long id);

// a
size_t poset_size(unsigned long id);

// r
bool poset_insert(unsigned long id, char const *value);
// check if name is unique
// add node

// a
bool poset_remove(unsigned long id, char const *value);
// remove edges

// a
bool poset_add(unsigned long id, char const *value1, char const *value2);
// check if new edge is ok
// add new edge and neighbours 

// r
bool poset_del(unsigned long id, char const *value1, char const *value2);
// check if can delete edge
// delete edge

// r
bool poset_test(unsigned long id, char const *value1, char const *value2);
// easy check

// r
void poset_clear(unsigned long id);
// map clear
}