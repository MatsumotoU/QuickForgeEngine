#include "ModelManager.h"
#include <fstream>
#include <sstream>
#include <cassert>

ModelData Modelmanager::LoadObjFile(const std::string& directoryPath, const std::string& filename, CoordinateSystem coordinateSystem) {
    ModelData modelData;
    std::vector<Vector4> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> texcoords;
    std::string line;
    //VertexData triangle[3]{};

    std::ifstream file(directoryPath + "/" + filename);
    assert(file.is_open());

    while (std::getline(file, line))
    {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;

        if (identifier == "v") {
            Vector4 position;
            s >> position.x >> position.y >> position.z;
            position.w = 1.0f;
            positions.push_back(position);
        } else if (identifier == "vt") {
            Vector2 texcoord{};
            s >> texcoord.x >> texcoord.y;
            texcoords.push_back(texcoord);
        } else if (identifier == "vn") {
            Vector3 normal;
            s >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        } else if (identifier == "f") {
            // 全ての頂点を探索(扇)
            std::vector<VertexData> faceVertices;
            std::string vertexDefinition;
            while (s >> vertexDefinition)
            {
                std::istringstream v(vertexDefinition);
                uint32_t elementIndices[3];
                for (int32_t element = 0; element < 3; ++element) {
                    std::string index;
                    std::getline(v, index, '/');
                    elementIndices[element] = std::stoi(index);
                }
                Vector4 position = positions[elementIndices[0] - 1];
                Vector2 texcoord = texcoords[elementIndices[1] - 1];
                Vector3 normal = normals[elementIndices[2] - 1];

                if (coordinateSystem == COORDINATESYSTEM_HAND_RIGHT) {
                    position.x *= -1.0f;
                    normal.x *= -1.0f;
                    texcoord.y = 1.0f - texcoord.y;
                }

                VertexData triangle{ position, texcoord, normal };
                faceVertices.push_back(triangle);
            }

            // 凸な面を分割(n-2)
            for (size_t i = 1; i < faceVertices.size() - 1; ++i) {
                if (coordinateSystem == COORDINATESYSTEM_HAND_RIGHT) {
                    modelData.vertices.push_back(faceVertices[i + 1]);
                    modelData.vertices.push_back(faceVertices[i]);
                    modelData.vertices.push_back(faceVertices[0]);
                } else {
                    modelData.vertices.push_back(faceVertices[0]);
                    modelData.vertices.push_back(faceVertices[i]);
                    modelData.vertices.push_back(faceVertices[i + 1]);
                }
            }

        } else if (identifier == "mtllib") {
            std::string materialFilename;
            s >> materialFilename;
            modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
        }
    }
    return modelData;
}

MaterialData Modelmanager::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
    MaterialData materialData;
    std::string line;
    std::ifstream file(directoryPath + "/" + filename);
    assert(file.is_open());

    while (std::getline(file, line))
    {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier;

        if (identifier == "map_Kd") {
            std::string textureFilename;
            s >> textureFilename;
            materialData.textureFilePath = directoryPath + "/" + textureFilename;
        }
    }

    return materialData;
}
