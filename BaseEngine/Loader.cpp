#include "Loader.h"
#include <sstream> 
#include <map>

//fan triangulation for any polygon (>=3)
void Loader::Triangulate(Face& face)
{
    const size_t n = face.vertexIndices.size();
    if (n < 3) return;

    if (n == 3)
    {
        faces.push_back(face);
        return;
    }
 
    for (size_t i = 1; i + 1 < n; ++i)
    {
        Face t;
        t.vertexIndices = { face.vertexIndices[0], face.vertexIndices[i], face.vertexIndices[i+1] };

        //size checks
        if (!face.uvIndices.empty())
        {
            if (face.uvIndices.size() >= 3)
            {
                t.uvIndices = { face.uvIndices[0], face.uvIndices[i], face.uvIndices[i + 1] };
            }
            else
            {
                t.uvIndices = { 0, 0, 0 };
                std::cout << "Warning: face uv indices missing or incomplete, using default 0\n";
            }
        }

        if (!face.normalIndices.empty())
        {
            if (face.normalIndices.size() >= 3)
            {
                t.normalIndices = { face.normalIndices[0], face.normalIndices[i], face.normalIndices[i + 1] };
            }
            else
            {
                t.normalIndices = { 0, 0, 0 };
                std::cout << "Warning: face normal indices missing or incomplete, using default 0\n";
            }
        }

        faces.push_back(t);
    }
}

void Loader::LoadModel(const std::string& filePath)
{
    // clear any previous data
    vertices.clear();
    faces.clear();
    indices.clear();

    type = LoadState::OpenFile;

    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;

    enum class IterState { OpenFile, ReadingVertices, ReadingUVs, ReadingNormals, ReadingFaces, Preparing };
    IterState iterState = IterState::OpenFile;

    const size_t BATCH_SIZE = 1000;
    size_t vertexBatchCount = 0;
    size_t uvBatchCount = 0;
    size_t normalBatchCount = 0;
    size_t faceBatchCount = 0;

    auto flushVertexBatch = [&]()
    {
        if (vertexBatchCount == 0) return;
        std::cout << "Processed vertex batch: " << vertexBatchCount << " (total vertices=" << positions.size() << ")\n";
        vertexBatchCount = 0;
        type = LoadState::OpenFile;
    };

    auto flushUVBatch = [&]()
    {
        if (uvBatchCount == 0) return;
        std::cout << "Processed uv batch: " << uvBatchCount << " (total uvs=" << uvs.size() << ")\n";
        uvBatchCount = 0;
    };

    auto flushNormalBatch = [&]()
    {
        if (normalBatchCount == 0) return;
        std::cout << "Processed normal batch: " << normalBatchCount << " (total normals=" << normals.size() << ")\n";
        normalBatchCount = 0;
    };

    auto flushFaceBatch = [&]()
    {
        if (faceBatchCount == 0) return;
        std::cout << "Processed face batch (triangles added): " << faceBatchCount << " (total faces=" << faces.size() << ")\n";
        faceBatchCount = 0;
    };

    std::string line;
    while (std::getline(file, line))
    {
        // trim leading whitespace
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        if (line[start] == '#') continue;

        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v")
        {
            // transition if needed
            if (iterState != IterState::ReadingVertices)
            {
                // leaving other stage -> flush anything outstanding
                flushUVBatch();
                flushNormalBatch();
                flushFaceBatch();
                iterState = IterState::ReadingVertices;
                type = LoadState::OpenFile;
            }

            glm::vec3 pos;
            iss >> pos.x >> pos.y >> pos.z;
            if (!iss.fail())
            {
                positions.push_back(pos);
                ++vertexBatchCount;
            }

            if (vertexBatchCount >= BATCH_SIZE)
                flushVertexBatch();
        }
        else if (prefix == "vt")
        {
            if (iterState != IterState::ReadingUVs)
            {
                flushVertexBatch();
                flushNormalBatch();
                flushFaceBatch();
                iterState = IterState::ReadingUVs;
            }

            glm::vec2 uv;
            iss >> uv.x >> uv.y;
            if (!iss.fail())
            {
                uvs.push_back(uv);
                ++uvBatchCount;
            }

            if (uvBatchCount >= BATCH_SIZE)
                flushUVBatch();
        }
        else if (prefix == "vn")
        {
            if (iterState != IterState::ReadingNormals)
            {
                flushVertexBatch();
                flushUVBatch();
                flushFaceBatch();
                iterState = IterState::ReadingNormals;
            }

            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            if (!iss.fail())
            {
                normals.push_back(normal);
                ++normalBatchCount;
            }

            if (normalBatchCount >= BATCH_SIZE)
                flushNormalBatch();
        }
        else if (prefix == "f")
        {
            if (iterState != IterState::ReadingFaces)
            {
                flushVertexBatch();
                flushUVBatch();
                flushNormalBatch();
                iterState = IterState::ReadingFaces;
            }

            std::vector<std::string> tokens;
            std::string tok;
            while (iss >> tok)
                tokens.push_back(tok);

            Face face;
            bool validFace = true;
            for (const std::string& t : tokens)
            {
                // split token by '/'
                int v = 0, vt = 0, vn = 0;
                size_t p1 = t.find('/');
                if (p1 == std::string::npos)
                {
                    // only vertex index
                    v = std::stoi(t);
                }
                else
                {
                    // v / vt / vn  (some parts may be empty)
                    std::string sV = t.substr(0, p1);
                    if (!sV.empty()) v = std::stoi(sV);

                    size_t p2 = t.find('/', p1 + 1);
                    if (p2 == std::string::npos)
                    {
                        // v/vt
                        std::string sVT = t.substr(p1 + 1);
                        if (!sVT.empty()) vt = std::stoi(sVT);
                    }
                    else
                    {
                        // v/vt/vn or v//vn
                        std::string sVT = t.substr(p1 + 1, p2 - p1 - 1);
                        if (!sVT.empty()) vt = std::stoi(sVT);

                        std::string sVN = t.substr(p2 + 1);
                        if (!sVN.empty()) vn = std::stoi(sVN);
                    }
                }

                // Convert negative (relative) indices to positive 1-based indices.
                if (v < 0) v = static_cast<int>(positions.size()) + v + 1;
                if (vt < 0) vt = static_cast<int>(uvs.size()) + vt + 1;
                if (vn < 0) vn = static_cast<int>(normals.size()) + vn + 1;

                // if vertex index is missing or zero -> invalid face (skip)
                if (v == 0)
                {
                    validFace = false;
                    break;
                }

                face.vertexIndices.push_back(static_cast<uint32_t>(v));
                face.uvIndices.push_back(static_cast<uint32_t>(vt));
                face.normalIndices.push_back(static_cast<uint32_t>(vn));
            }

            if (validFace)
            {
                Triangulate(face);
                size_t n = face.vertexIndices.size();
                if (n >= 3)
                    faceBatchCount += (n - 2);
            }
            else
            {
                std::cerr << "Warning: skipped face with missing vertex index\n";
            }

            if (faceBatchCount >= BATCH_SIZE)
                flushFaceBatch();
        }
    } // end read file

    // final flush of any remaining batches
    flushVertexBatch();
    flushUVBatch();
    flushNormalBatch();
    flushFaceBatch();

    file.close();

    //prepare for buffering
    ProcessData(positions, uvs, normals);

    // update iterative state
    iterState = IterState::Preparing;
    std::cout << "LoadModel: parsing complete, prepared data for buffering (vertices=" << vertices.size()
              << ", indices=" << indices.size() << ", faces(tris)=" << faces.size() << ")\n";
}

void Loader::ProcessData(std::vector<glm::vec3> positions, std::vector<glm::vec2> uvs, std::vector<glm::vec3> normals)
{
    std::map<Vertex, uint32_t> unique_vertices;

    for (const Face& face : faces)
    {
        for (size_t i = 0; i < face.vertexIndices.size(); ++i)
        {
            int vIndex = static_cast<int>(face.vertexIndices[i]) - 1;
            int tIndex = static_cast<int>(face.uvIndices[i]) - 1;
            int nIndex = static_cast<int>(face.normalIndices[i]) - 1;

            Vertex vertex{};
            if (vIndex >= 0 && vIndex < static_cast<int>(positions.size()))
                vertex.position = positions[vIndex];
            else
                vertex.position = glm::vec3(0.0f);

            if (tIndex >= 0 && tIndex < static_cast<int>(uvs.size()))
                vertex.textureCoord = uvs[tIndex];
            else
                vertex.textureCoord = glm::vec2(0.0f);

            if (nIndex >= 0 && nIndex < static_cast<int>(normals.size()))
                vertex.normal = normals[nIndex];
            else
                vertex.normal = glm::vec3(0.0f);

            auto it = unique_vertices.find(vertex);
            if (it != unique_vertices.end())
            {
                indices.push_back(it->second);
            }
            else
            {
                uint32_t newIndex = static_cast<uint32_t>(vertices.size());
                unique_vertices.emplace(vertex, newIndex);
                vertices.push_back(vertex);
                indices.push_back(newIndex);
            }
        }
    }

    // Recompute smooth per-vertex normals when file provided no normals
    if (normals.empty())
    {
        std::vector<glm::vec3> accumNormals(vertices.size(), glm::vec3(0.0f));

        // accumulate face normals per vertex
        for (size_t i = 0; i + 2 < indices.size(); i += 3)
        {
            uint32_t ia = indices[i];
            uint32_t ib = indices[i + 1];
            uint32_t ic = indices[i + 2];

            const glm::vec3& v0 = vertices[ia].position;
            const glm::vec3& v1 = vertices[ib].position;
            const glm::vec3& v2 = vertices[ic].position;

            glm::vec3 faceNormal = glm::cross(v1 - v0, v2 - v0);
            float len = glm::length(faceNormal);
            if (len > 1e-6f)
                faceNormal /= len;
            else
                faceNormal = glm::vec3(0.0f);

            accumNormals[ia] += faceNormal;
            accumNormals[ib] += faceNormal;
            accumNormals[ic] += faceNormal;
        }

        // normalize and write back to vertices
        for (size_t i = 0; i < vertices.size(); ++i)
        {
            glm::vec3 n = accumNormals[i];
            float len = glm::length(n);
            if (len > 1e-6f)
                vertices[i].normal = glm::normalize(n);
            else
                vertices[i].normal = glm::vec3(0.0f, 0.0f, 1.0f); // fallback normal
        }
    }

    std::cout << "TOTAL FACES: " << faces.size() << std::endl;
}
