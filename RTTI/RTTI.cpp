#include <iostream>
#include <map>
#include <set>
#include <stdarg.h>
#include <vector>

std::map<std::string, int> IDs;
std::map<int, std::set<std::pair<int,int>>> children;
std::map<int, std::vector<int>> parents;
std::map<int, int> sizes;
std::map<int, std::string> class_names;

void RTTI_constructor(int ID, std::string class_name, int num_base_classes, ...) {
  IDs[class_name] = ID;
  class_names[ID] = class_name;
  va_list args;
  va_start(args, num_base_classes);
  for (int i = 0; i < num_base_classes; ++i) {
      std::string base_class = va_arg(args, char*);
      int base_class_id = IDs[base_class];
      if (parents[ID].size() < num_base_classes) {
          parents[ID].push_back(base_class_id);
      }
      children[base_class_id].insert(std::make_pair(ID, i));
  }
}

#define TURN_ON_RTTI(class_name, num_base_classes, ...) \
int ID = __COUNTER__; \
class_name() { \
  RTTI_constructor(ID, #class_name, num_base_classes, __VA_ARGS__); \
  sizes[ID] = sizeof(class_name); \
} \

int get_class_id(char *ptr) {
  return *(int*)ptr;
}

// check if there is such a class with the given ID in the given memory buffer
// that starts from the pointer to the class that has ind_in_par_list in ID's class' parents list
bool check_obj_ID(void *obj_ptr, int ID, int ind_in_par_list) {
  int offset = 0;
  for (int i = ind_in_par_list; i < parents[ID].size(); ++i) {
    offset += sizes[parents[ID][i]];
  }
  return get_class_id((char*)obj_ptr + offset) == ID;
}

int get_bottom_child(void *&obj_ptr, int ID) {
  for (auto child : children[ID]) {
    if (check_obj_ID(obj_ptr, child.first, child.second)) {
      int offset = 0;
      for (int i = 0; i < child.second; ++i) {
            offset += sizes[parents[child.first][i]];
      }
      obj_ptr = (char*)obj_ptr - offset;
      return get_bottom_child(obj_ptr, child.first);
    }
  }
  return ID;
}

int find_offset_to_target(int obj_ID, int target_ID) {
  if (obj_ID == target_ID) {
    return 0;
  }
  if (parents[obj_ID].size() == 0) {
    return -1;
  }
  int offset = 0;
  for (auto parent : parents[obj_ID]) {
    int return_val = find_offset_to_target(parent, target_ID);
    if (return_val >= 0) {
      return return_val + offset;
    }
    offset += sizes[parent];
  }
  return -1;
}

void* dynamic_cast_fun(void *obj_ptr, std::string to_class) {
  int to_class_ID = IDs[to_class];
  int obj_ID = get_class_id((char*)obj_ptr);
  int bottom_child_ID = get_bottom_child(obj_ptr, obj_ID);
  int offset = find_offset_to_target(bottom_child_ID, to_class_ID);
  if (offset == -1) {
    return nullptr;
  }
  return (char*)obj_ptr + offset;
}

#define DYNAMIC_CAST(obj_ptr, to_class) \
((to_class*)dynamic_cast_fun((obj_ptr), #to_class))


struct TypeInfo {
  TypeInfo(int ID, std::string name): ID(ID), name(name) {
    hash = std::hash<std::string>()(name);
  }
  std::string name;
  int hash;
  int ID;
  bool operator==(const TypeInfo &other) {
    return (ID == other.ID);
  }
};

TypeInfo get_type_id(void *obj_ptr) {
  int obj_ID = get_class_id((char*)obj_ptr);
  int bottom_child_ID = get_bottom_child(obj_ptr, obj_ID);
  std::string bottom_child_name = class_names[bottom_child_ID];
  return TypeInfo(bottom_child_ID, bottom_child_name);
}

#define TYPEID(obj_ptr) \
get_type_id(obj_ptr)


struct A {
  TURN_ON_RTTI(A, 0)
    int a = 1;
   void greet() {
    std::cout << "I am struct A! a = " << a << std::endl;
  }
};

struct B {
  TURN_ON_RTTI(B, 0)
    int b = 2;
   void greet() {
    std::cout << "I am struct B! b = " << b << std::endl;
  }
};

struct C : public A, public B {
  TURN_ON_RTTI(C, 2, "A", "B")
    int c = 3;
  void greet() {
    std::cout << "I am struct C! c = " << c << std::endl;
  }
};


int main() {
  A *a = new A();
  B *b = new B();
  C *c = new C();
  std::cout << (DYNAMIC_CAST(a, B) == nullptr) << std::endl;  // 1
  std::cout << (DYNAMIC_CAST(a, C) == nullptr) << std::endl;  // 1
  DYNAMIC_CAST(c, B)->greet();  // I am struct B! b = 2
  DYNAMIC_CAST(c, A)->greet();  // I am struct A! a = 1

  B *bb = new C();
  DYNAMIC_CAST(bb, A)->greet();  // I am struct A! a = 1

  std::cout << (TYPEID(bb) == TYPEID(c)) << std::endl;  // 1
  return 0;
}
