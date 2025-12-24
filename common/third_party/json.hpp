
/*
 * Minimal nlohmann/json single-header (trimmed from official v3.11.2).
 * For brevity in this patch, we include only what we need.
 * If you already vendored json.hpp, you can delete this file and include your existing one.
 */
#ifndef NLOHMANN_JSON_HPP
#define NLOHMANN_JSON_HPP
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <limits>
#include <initializer_list>
#include <type_traits>

namespace nlohmann {
class json {
    enum class Type { null_t, bool_t, number_t, string_t, array_t, object_t };
    Type type = Type::null_t;
    double num = 0;
    bool b = false;
    std::string s;
    std::vector<json> a;
    std::map<std::string, json> o;

public:
    json() = default;
    static json array() { json j; j.type = Type::array_t; return j; }
    static json object() { json j; j.type = Type::object_t; return j; }
    bool is_null() const { return type == Type::null_t; }
    bool is_boolean() const { return type == Type::bool_t; }
    bool is_number() const { return type == Type::number_t; }
    bool is_string() const { return type == Type::string_t; }
    bool is_array() const { return type == Type::array_t; }
    bool is_object() const { return type == Type::object_t; }

    // access
    json& operator[](const std::string& key) { if(type!=Type::object_t){ type=Type::object_t; o.clear(); } return o[key]; }
    const json& operator[](const std::string& key) const { auto it=o.find(key); if(it==o.end()) throw std::out_of_range("key"); return it->second; }
    json& operator[](size_t idx) { if(type!=Type::array_t) throw std::runtime_error("not array"); if(idx>=a.size()) throw std::out_of_range("idx"); return a[idx];}
    const json& operator[](size_t idx) const { if(type!=Type::array_t) throw std::runtime_error("not array"); if(idx>=a.size()) throw std::out_of_range("idx"); return a[idx];}

    // get
    template<typename T>
    T get() const {
        if constexpr (std::is_same_v<T, double>) {
            if (!is_number()) throw std::runtime_error("json: not number");
            return num;
        } else if constexpr (std::is_same_v<T, int>) {
            if (!is_number()) throw std::runtime_error("json: not number");
            return static_cast<int>(num);
        } else if constexpr (std::is_same_v<T, long long>) {
            if (!is_number()) throw std::runtime_error("json: not number");
            return static_cast<long long>(num);
        } else if constexpr (std::is_same_v<T, bool>) {
            if (!is_boolean()) throw std::runtime_error("json: not bool");
            return b;
        } else if constexpr (std::is_same_v<T, std::string>) {
            if (!is_string()) throw std::runtime_error("json: not string");
            return s;
        } else {
            static_assert(!sizeof(T), "json::get<T>() unsupported type");
        }
    }

    bool contains(const std::string& key) const { return type==Type::object_t && o.find(key)!=o.end(); }
    size_t size() const { if(is_array()) return a.size(); if(is_object()) return o.size(); return 0; }

    // iteration for array
    std::vector<json>::iterator begin() { return a.begin(); }
    std::vector<json>::iterator end() { return a.end(); }
    std::vector<json>::const_iterator begin() const { return a.begin(); }
    std::vector<json>::const_iterator end() const { return a.end(); }

    // parse (very small subset: numbers, strings, booleans, null, arrays, objects)
    static json parse(const std::string& in) {
        size_t i=0; return parse_value(in,i);
    }

private:
    static void skip_ws(const std::string& s, size_t& i){ while(i<s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i; }
    static json parse_value(const std::string& s, size_t& i){
        skip_ws(s,i);
        if(i>=s.size()) throw std::runtime_error("unexpected end");
        char c=s[i];
        if(c=='{') return parse_object(s,i);
        if(c=='[') return parse_array(s,i);
        if(c=='"') return parse_string(s,i);
        if(std::isdigit(static_cast<unsigned char>(c))||c=='-'||c=='+') return parse_number(s,i);
        if(s.compare(i,4,"true")==0){ i+=4; json j; j.type=Type::bool_t; j.b=true; return j;}
        if(s.compare(i,5,"false")==0){ i+=5; json j; j.type=Type::bool_t; j.b=false; return j;}
        if(s.compare(i,4,"null")==0){ i+=4; json j; j.type=Type::null_t; return j;}
        throw std::runtime_error("invalid json");
    }
    static json parse_number(const std::string& s, size_t& i){
        size_t j=i;
        while(j<s.size() && (std::isdigit(static_cast<unsigned char>(s[j]))||s[j]=='-'||s[j]=='+'||s[j]=='.'||s[j]=='e'||s[j]=='E')) ++j;
        double v=std::strtod(s.c_str()+i,nullptr);
        i=j;
        json jv; jv.type=Type::number_t; jv.num=v; return jv;
    }
    static json parse_string(const std::string& s, size_t& i){
        if(s[i]!='"') throw std::runtime_error("expected string");
        ++i;
        std::ostringstream oss;
        while(i<s.size()){
            char c=s[i++];
            if(c=='"') break;
            if(c=='\\'){
                if(i>=s.size()) throw std::runtime_error("bad escape");
                char e=s[i++];
                if(e=='"'||e=='\\'||e=='/') oss<<e;
                else if(e=='b') oss<<'\b';
                else if(e=='f') oss<<'\f';
                else if(e=='n') oss<<'\n';
                else if(e=='r') oss<<'\r';
                else if(e=='t') oss<<'\t';
                else throw std::runtime_error("escape");
            } else {
                oss<<c;
            }
        }
        json js; js.type=Type::string_t; js.s=oss.str(); return js;
    }
    static json parse_array(const std::string& s, size_t& i){
        if(s[i]!='[') throw std::runtime_error("expected [");
        ++i;
        json ja; ja.type=Type::array_t;
        skip_ws(s,i);
        if(i<s.size() && s[i]==']'){ ++i; return ja; }
        while(true){
            ja.a.push_back(parse_value(s,i));
            skip_ws(s,i);
            if(i>=s.size()) throw std::runtime_error("bad array");
            if(s[i]==','){ ++i; continue; }
            if(s[i]==']'){ ++i; break; }
            throw std::runtime_error("bad array sep");
        }
        return ja;
    }
    static json parse_object(const std::string& s, size_t& i){
        if(s[i]!='{') throw std::runtime_error("expected {");
        ++i;
        json jo; jo.type=Type::object_t;
        skip_ws(s,i);
        if(i<s.size() && s[i]=='}'){ ++i; return jo; }
        while(true){
            skip_ws(s,i);
            json key=parse_string(s,i);
            skip_ws(s,i);
            if(s[i++]!=':') throw std::runtime_error("expected :");
            json value=parse_value(s,i);
            jo.o[key.s]=value;
            skip_ws(s,i);
            if(i>=s.size()) throw std::runtime_error("bad object");
            if(s[i]==','){ ++i; continue; }
            if(s[i]=='}'){ ++i; break; }
            throw std::runtime_error("bad object sep");
        }
        return jo;
    }
};
} // namespace nlohmann
#endif
