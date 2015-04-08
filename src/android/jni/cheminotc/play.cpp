#include "cheminotc.h"
#include <sstream>
#include <fstream>

namespace cheminotc
{
    namespace play
    {
        static std::list<Vertice> vertices;

        Json::Value serializeVertice(Vertice vertice)
        {
            Json::Value json;
            json["id"] = vertice.id;
            json["name"] = vertice.name;
            json["lat"] = vertice.lat;
            json["lng"] = vertice.lng;
            return json;
        }

        Json::Value serializeVertices()
        {
            Json::Value array = Json::Value(Json::arrayValue);
            for(auto iterator = vertices.begin(), end = vertices.end(); iterator != end; ++iterator)
            {
                Vertice vertice = *iterator;
                array.append(serializeVertice(vertice));
            }
            return array;
        }

        void push(Vertice vertice)
        {
            vertices.push_back(vertice);
        }

        void serializeToFile(std::string path)
        {
            Json::Value json;
            json["vertices"] = serializeVertices();
            std::ofstream file;
            file.open(path, std::ios::out);
            file << json.toStyledString() + "\n";
            file.close();
        }
    }
}
