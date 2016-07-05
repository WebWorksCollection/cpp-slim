#include "types/Hash.hpp"
#include "types/Array.hpp"
#include "Value.hpp"
#include "Function.hpp"
#include "Operators.hpp"
#include <algorithm>
#include <sstream>
#include <set>

namespace slim
{
    const std::string Hash::TYPE_NAME = "Hash";

    Hash::Hash()
        : def_value(NIL_VALUE), map(), insert_order()
    {}

    std::string Hash::inspect() const
    {
        std::stringstream ss;
        ss << '{';
        bool first = true;
        for (auto &i : insert_order)
        {
            if (first) first = false;
            else ss << ", ";
            ss << i->first->inspect() << " => " << i->second->inspect();
        }
        ss << '}';
        return ss.str();
    }
    bool Hash::eq(const Object * orhs) const
    {
        auto &rhs = ((const Hash*)orhs)->get_value();
        if (map.size() != rhs.size()) return false;
        for (auto &i : map)
        {
            auto it = rhs.find(i.first);
            if (it == rhs.end()) return false;
            if (!slim::eq(i.second.get(), it->second.get())) return false;
        }
        return true;
    }
    size_t Hash::hash() const
    {
        size_t h = 0;
        for (auto &i : map)
        {
            auto h2 = detail::hash(*i.first);
            detail::hash_combine(h2, *i.second);
            h ^= h2; //because iteration order is not defined
        }
        return h;
    }

    void Hash::set(ObjectPtr key, ObjectPtr val)
    {
        auto x = map.emplace(key, val);
        if (x.second) insert_order.push_back(x.first);
        else x.first->second = val;
    }

    ObjectPtr Hash::el_ref(const FunctionArgs &args)
    {
        if (args.size() != 1) throw InvalidArgument(this, "[]");
        auto it = map.find(args[0]);
        if (it != map.end()) return it->second;
        else return def_value;
    }

    ObjectPtr Hash::empty_q()
    {
        return make_value(map.empty());
    }

    ObjectPtr Hash::fetch(const FunctionArgs &args)
    {
        if (args.empty()) throw InvalidArgument(this, "fetch");
        auto it = map.find(args[0]);
        if (it != map.end()) return it->second;

        if (args.size() == 1) throw KeyError(args[0]);
        //TODO: Block
        return args[1];
    }
    std::shared_ptr<Array> Hash::flatten(const FunctionArgs &args)
    {
        int level = 0;
        if (args.size() == 1) level = (int)as_number(args[0]);
        else if (args.size() > 1) throw InvalidArgument(this, "flatten");

        std::vector<ObjectPtr> out;
        for (auto &i : insert_order)
        {
            out.push_back(i->first);
            out.push_back(i->second);
        }

        auto arr = make_array(std::move(out));
        if (level > 1) arr = arr->flatten({ make_value((double)level - 1) });
        return arr;
    }

    ObjectPtr Hash::has_key_q(Object * obj)
    {
        return make_value(map.find(obj->shared_from_this()) != map.end());
    }
    ObjectPtr Hash::has_value_q(const Object * obj)
    {
        for (auto &i : map) if (slim::eq(i.second.get(), obj)) return TRUE_VALUE;
        return FALSE_VALUE;
    }

    std::shared_ptr<Hash> Hash::invert()
    {
        auto out = create_object<Hash>(def_value);
        for (auto &i : insert_order)
            out->set(i->second, i->first);
        return out;
    }

    ObjectPtr Hash::key(const Object *val)
    {
        for (auto &i : map) if (slim::eq(i.second.get(), val)) return i.first;
        return NIL_VALUE;
    }

    std::shared_ptr<Array> Hash::keys()
    {
        std::vector<ObjectPtr> out;
        for (auto &i : insert_order) out.push_back(i->first);
        return make_array(std::move(out));
    }

    std::shared_ptr<Array> Hash::values()
    {
        std::vector<ObjectPtr> out;
        for (auto &i : insert_order) out.push_back(i->second);
        return make_array(std::move(out));
    }

    ObjectPtr Hash::size()
    {
        return make_value((double)map.size());
    }

    std::shared_ptr<Hash> Hash::merge(Hash *other_hash)
    {
        //TODO: Block
        auto out = create_object<Hash>(def_value);
        for (auto &i : insert_order)
            out->set(i->first, i->second);
        for (auto &i : other_hash->insert_order)
            out->set(i->first, i->second);
        return out;
    }

    std::shared_ptr<Array> Hash::to_a()
    {
        std::vector<ObjectPtr> out;
        for (auto &i : insert_order)
            out.push_back(make_array({ i->first, i->second }));
        return make_array(std::move(out));
    }
    std::shared_ptr<Hash> Hash::to_h()
    {
        return std::static_pointer_cast<Hash>(shared_from_this());
    }

    const MethodTable & Hash::method_table() const
    {
        static const MethodTable table(Object::method_table(),
        {
            { &Hash::empty_q, "empty?" },
            { &Hash::fetch, "fetch" },
            { &Hash::flatten, "flatten" },
            { &Hash::has_key_q, "has_key?" },
            { &Hash::has_key_q, "include?" },
            { &Hash::has_key_q, "key?" },
            { &Hash::has_key_q, "member?" },
            { &Hash::has_value_q, "has_value?" },
            { &Hash::has_value_q, "value?" },
            { &Hash::invert, "invert" },
            { &Hash::key, "key" },
            { &Hash::keys, "keys" },
            { &Hash::values, "values" },
            { &Hash::size, "size" },
            { &Hash::size, "length" },
            { &Hash::merge, "merge" },
            { &Hash::to_a, "to_a" },
            { &Hash::to_h, "to_h" }
        });
        return table;
    }
}