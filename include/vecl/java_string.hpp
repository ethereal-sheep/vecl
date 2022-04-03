#ifndef VECL_JAVA_STRING_HPP
#define VECL_JAVA_STRING_HPP

#include "config/config.h"
#include "type_traits.hpp" 
#include <unordered_set> // unordered_set
#include <unordered_map> // unordered_map
#include <string> // string
#include <string_view> // string_view
#include <iostream> // ostream

using std::literals::string_view_literals::operator""sv;
using std::literals::string_literals::operator""s;

namespace vecl
{
    using js_conversion_list = type_list<const char*, std::string&&, const std::string&, std::string_view>;

    template<typename T>
    concept js_convertible = contains_v<T, js_conversion_list>;

    class java_string
    {
        struct string_hash {
            using is_transparent = void;

            VECL_NODISCARD size_t operator()(const char *txt) const 
            {
                return std::hash<std::string_view>{}(txt);
            }
            VECL_NODISCARD size_t operator()(std::string_view txt) const 
            {
                return std::hash<std::string_view>{}(txt);
            }
            VECL_NODISCARD size_t operator()(const std::string &txt) const 
            {
                return std::hash<std::string>{}(txt);
            }
        };

        class js_view
        {
            friend java_string;

            const char* _begin;
            const char* _end;

            js_view(const char* begin, const char* end) 
                : _begin(begin), _end(end) {} 

            js_view(const std::string& str) 
                : _begin(str.c_str()), _end(str.c_str() + str.size()) {} 

        public:

            using iterator = const char*;

            operator std::string_view() const { return std::string_view(_begin, _end); }

            VECL_NODISCARD auto begin() const { return _begin; }
            VECL_NODISCARD auto end() const { return _end; }
            VECL_NODISCARD auto cbegin() const { return _begin; }
            VECL_NODISCARD auto cend() const { return _end; }
            VECL_NODISCARD auto size() const { return _end - _begin; }
            VECL_NODISCARD auto c_str() const { return _begin; }
            VECL_NODISCARD auto empty() const { return _begin == _end; }

            auto operator==(js_view str) const { return _begin == str._begin; }
        };

        struct js_view_hash {
            using is_transparent = void;

            VECL_NODISCARD size_t operator()(const js_view& str) const 
            {
                return std::hash<const char*>{}(str.begin());
            }
            VECL_NODISCARD size_t operator()(const char* str) const 
            {
                return std::hash<const char*>{}(str);
            }
        };

        struct js_library
        {
            std::unordered_set<std::string, string_hash, std::equal_to<>> _set;
            // std::unordered_map<const char*, const std::string*, js_view_hash> _map;

            const std::string& try_emplace_and_get_str(std::string&& str)
            {
                // auto [it, b] = _set.insert(std::move(str));
                // if(b) _map.insert(make_pair(it->c_str(), &*it));
                return *_set.insert(std::move(str)).first;
            }

            const std::string& try_emplace_and_get_str(std::string_view sv)
            {
#ifdef __cpp_lib_generic_unordered_lookup
                if (!_set.contains(sv))
                {
                    // auto [it, b] = _set.emplace(sv);
                    //_map.insert(make_pair(it->c_str(), &*it));
                    return *_set.emplace(sv).first;
                }
                return *_set.find(sv);
#else
                std::string str(sv.begin(), sv.end());
                return try_emplace_and_get_str(str);
#endif
            }

            auto try_emplace_and_get_js_view(std::string_view str)
            {
                return js_view(try_emplace_and_get_str(str));
            }

            auto try_emplace_and_get_js_view(const std::string& str)
            {
                return js_view(try_emplace_and_get_str(str));
            }

            auto try_emplace_and_get_js_view(std::string&& str)
            {
                return js_view(try_emplace_and_get_str(std::move(str)));
            }

            auto try_concat_and_get_js_view(std::string_view lhs, std::string_view rhs)
            {
                auto& left_str = try_emplace_and_get_str(lhs);
                auto new_str = left_str + rhs.data();

                return try_emplace_and_get_js_view(std::move(new_str));
            }

        };

        static auto& get_library()
        {
            static js_library s_library;
            return s_library;
        }

        js_view _view;

    public:


        java_string()
            : _view{get_library().try_emplace_and_get_js_view(""sv)}
        {
        }

        
        java_string(js_view str)
            : _view{str}
        {
        }

        java_string(std::string_view str)
            : _view{get_library().try_emplace_and_get_js_view(str)}
        {
        }

        java_string(const char* str)
            : java_string(std::string_view(str))
        {
        }

        java_string(const std::string& str)
            : _view{get_library().try_emplace_and_get_js_view(str)}
        {
        }

        java_string(std::string&& str)
            : _view{get_library().try_emplace_and_get_js_view(std::move(str))}
        {
        }

        explicit operator std::string_view() const
        {
            return std::string_view(_view.begin());
        }

        explicit operator std::string() const
        {
            return std::string(_view.begin());
        }

        const char* c_str() const
        {
            return _view.begin();
        }
        auto view() const
        {
            return std::string_view(_view.begin());
        }

        auto string() const
        {
            return std::string(_view.begin());
        }

        bool operator==(const java_string& rhs) const
        {
            return _view == rhs._view;
        }

        bool contains(std::string_view )
        {
            return false;
        }

        java_string& operator+=(std::string_view str)
        {
            return *this = concat(str);
        }

        java_string& operator+=(const char* str)
        {
            return *this = concat(str);
        }

        char operator[](size_t i) const
        {
            return _view.begin()[i];
        }

        size_t size() const
        {
            return _view.size();
        }

        java_string concat(std::string_view str) const
        {
            return java_string(get_library().try_concat_and_get_js_view(view(), str));
        }

        java_string concat(const char* str) const
        {
            return java_string(get_library().try_concat_and_get_js_view(view(), std::string_view(str)));
        }

        friend java_string operator+(java_string lhs, std::string_view rhs)
        {
            return lhs.concat(rhs);
        }

        friend java_string operator+(java_string lhs, const char* rhs)
        {
            return lhs.concat(std::string_view(rhs));
        }

        friend std::ostream& operator<<(std::ostream& os, java_string lhs)
        {
            return os << lhs._view.begin();
        }

    };

} // namespace vecl



#endif