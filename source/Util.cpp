#include "Util.hpp"
#include "types/HtmlSafeString.hpp"
namespace slim
{
    std::string html_escape(const std::string &str)
    {
        std::string buf;
        buf.reserve(str.size());
        for (auto c : str)
        {
            switch (c)
            {
            case '&': buf += "&amp;"; break;
            case '<': buf += "&lt;"; break;
            case '>': buf += "&gt;"; break;
            case '"': buf += "&quot;"; break;
            case '\'': buf += "&#39;"; break;
            default: buf += c; break;
            }
        }
        return buf;
    }

    std::string html_escape(const Object *obj)
    {
        if (auto safe = dynamic_cast<const HtmlSafeString*>(obj))
        {
            return safe->get_value();
        }
        else
        {
            return html_escape(obj->to_string());
        }
    }
}
