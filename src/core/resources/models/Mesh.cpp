#include "Mesh.h"

#include <cmath>

#include "mikktspace/mikktspace.h"

int getNumFaces(const SMikkTSpaceContext* context) {
    auto* mesh = static_cast<Mesh*>(context->m_pUserData);
    return static_cast<int>(mesh->getIndices().size() / 3);
}

int getNumFaceVertices(const SMikkTSpaceContext*, int) {
    return 3;
}

Vertex& getVertex(const SMikkTSpaceContext* context, const int face, const int vert) {
    const auto mesh = static_cast<Mesh*>(context->m_pUserData);

    const uint32_t index  = mesh->getIndices()[face * 3 + vert];
    Vertex&        vertex = mesh->getVertices()[index];

    return vertex;
}

void getPosition(const SMikkTSpaceContext* context, float out[3], const int face, const int vert) {
    const Vertex& vertex = getVertex(context, face, vert);

    out[0] = vertex.position.x;
    out[1] = vertex.position.y;
    out[2] = vertex.position.z;
}

void getNormal(const SMikkTSpaceContext* context, float out[3], const int face, const int vert) {
    const Vertex& vertex = getVertex(context, face, vert);

    out[0] = vertex.normal.x;
    out[1] = vertex.normal.y;
    out[2] = vertex.normal.z;
}

void getTextureCoords(const SMikkTSpaceContext* context, float out[3], const int face, const int vert) {
    const Vertex& vertex = getVertex(context, face, vert);

    out[0] = vertex.textureCoords.x;
    out[1] = vertex.textureCoords.y;
}

void setTangent(
    const SMikkTSpaceContext* context, const float tangent[3], const float sign, const int face, const int vert
) {
    Vertex& vertex = getVertex(context, face, vert);
    vertex.tangent = glm::vec4(tangent[0], tangent[1], tangent[2], sign);
}

void Mesh::generateTangents() {
    SMikkTSpaceInterface mikkt{};
    mikkt.m_getNumFaces          = getNumFaces;
    mikkt.m_getNumVerticesOfFace = getNumFaceVertices;
    mikkt.m_getPosition          = getPosition;
    mikkt.m_getNormal            = getNormal;
    mikkt.m_getTexCoord          = getTextureCoords;
    mikkt.m_setTSpaceBasic       = setTangent;

    SMikkTSpaceContext context{};
    context.m_pInterface = &mikkt;
    context.m_pUserData  = this;

    genTangSpaceDefault(&context);
}

void Mesh::generateSmoothNormals(
    const size_t vertexStart,
    const size_t vertexEnd,
    const size_t indexStart,
    const size_t indexEnd
) {
    for (size_t i = indexStart; i < indexEnd; i += 3) {
        const uint32_t i0 = _indices[i + 0];
        const uint32_t i1 = _indices[i + 1];
        const uint32_t i2 = _indices[i + 2];

        if (i0 < vertexStart || i0 >= vertexEnd) continue;
        if (i1 < vertexStart || i1 >= vertexEnd) continue;
        if (i2 < vertexStart || i2 >= vertexEnd) continue;

        const glm::vec3 edge1 = _vertices[i1].position - _vertices[i0].position;
        const glm::vec3 edge2 = _vertices[i2].position - _vertices[i0].position;

        const glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

        _vertices[i0].normal += faceNormal;
        _vertices[i1].normal += faceNormal;
        _vertices[i2].normal += faceNormal;
    }

    for (uint32_t i = vertexStart; i < vertexEnd; i++) {
        _vertices[i].normal = glm::normalize(_vertices[i].normal);
    }
}

void Mesh::generateSmoothNormals() {
    generateSmoothNormals(0, _vertices.size(), 0, _indices.size());
}
