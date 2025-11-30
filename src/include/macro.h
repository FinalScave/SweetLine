#ifndef SWEETLINE_MACRO_H
#define SWEETLINE_MACRO_H

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define NS_SWEETLINE sweetline
#define MAKE_PTR std::make_shared
#define MAKE_UPTR std::make_unique
#define MAKE_WPTR std::make_weak

template<typename T>
using Ptr = std::shared_ptr<T>;

template<typename T>
using UPtr = std::unique_ptr<T>;

template<typename T>
using WPtr = std::weak_ptr<T>;

using String = std::string;

template<typename T>
using List = std::vector<T>;
template<typename K, typename V, typename KeyHash = std::hash<K>, typename KeyEqualTo = std::equal_to<K>>
using HashMap = std::unordered_map<K, V, KeyHash, KeyEqualTo>;
template<typename T, typename Hash = std::hash<T>, typename EqualTo = std::equal_to<T>>
using HashSet = std::unordered_set<T, Hash, EqualTo>;

#endif //SWEETLINE_MACRO_H