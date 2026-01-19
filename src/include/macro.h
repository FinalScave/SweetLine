#ifndef SWEETLINE_MACRO_H
#define SWEETLINE_MACRO_H

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define NS_SWEETLINE sweetline

template<typename T>
using SharedPtr = std::shared_ptr<T>;

template<typename T>
using UniquePtr = std::unique_ptr<T>;

template<typename T>
using WeakPtr = std::weak_ptr<T>;

template<typename T, typename... Args>
SharedPtr<T> makeSharedPtr(Args&&... args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
UniquePtr<T> makeUniquePtr(Args&&... args) {
  return std::make_unique<T>(std::forward<Args>(args)...);
}

using String = std::string;

template<typename T>
using List = std::vector<T>;
template<typename K, typename V, typename KeyHash = std::hash<K>, typename KeyEqualTo = std::equal_to<K>>
using HashMap = std::unordered_map<K, V, KeyHash, KeyEqualTo>;
template<typename T, typename Hash = std::hash<T>, typename EqualTo = std::equal_to<T>>
using HashSet = std::unordered_set<T, Hash, EqualTo>;

#endif //SWEETLINE_MACRO_H