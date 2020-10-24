#include "assets/MeshAsset.h"

#include "graphics/VulkanBuffer.h"
#include "graphics/VulkanInstance.h"
#include "log/log.h"

MeshAsset::MeshAsset(std::string meshName, VulkanInstance* vulkanInstance, aiMesh* mesh)
 : meshName(meshName),
   vulkanInstance(vulkanInstance)
{
    log_dbg("Loading mesh asset with name:");
    log_dbg(meshName.c_str());

    if((mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) != aiPrimitiveType_TRIANGLE) {
        log_ftl("Mesh topology is not exclusively triangles.");
    }

    if(!mesh->HasPositions()) {
        log_ftl("Mesh vertices have no positions.");
    }

    if(!mesh->HasNormals()) {
        log_ftl("Mesh vertices have no normals.");
    }

    if(!mesh->HasTextureCoords(0)) {
        log_ftl("Mesh vertices have no texture coordinates.");
    }

    std::vector<MeshVertex> vertices(mesh->mNumVertices);

    for(uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; vertexIndex++) {
        aiVector3D position = mesh->mVertices[vertexIndex];
        aiVector3D normal = mesh->mNormals[vertexIndex];
        aiVector3D texCoord = mesh->mTextureCoords[0][vertexIndex];

        vertices[vertexIndex].position = glm::vec3(position.x, position.y, position.z);
        vertices[vertexIndex].normal = glm::vec3(normal.x, normal.y, normal.z);
        // Take only the UV coordinates
        // Invert the Y coordinate into range [0, 1]
        vertices[vertexIndex].texCoord = glm::vec2(texCoord.x, -texCoord.y);
    }

    // Three indices per triangle face
    indexCount = mesh->mNumFaces * 3;
    std::vector<MeshIndex> indices(indexCount);

    // lol
    uint32_t indexIndex = 0;
    for(uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++) {
        aiFace& face = mesh->mFaces[faceIndex];

        for(uint32_t faceCornerIndex = 0; faceCornerIndex < face.mNumIndices; faceCornerIndex++) {
            indices[indexIndex] = face.mIndices[faceCornerIndex];
            indexIndex++;
        }
    }

    size_t vertexSize = sizeof(MeshVertex) * vertices.size();
    vertexBuffer = new VulkanBuffer(vulkanInstance, vertexSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    vertexBuffer->writeData(vertices.data());

    size_t indexSize = sizeof(indices[0]) * indices.size();
    indexBuffer = new VulkanBuffer(vulkanInstance, indexSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    indexBuffer->writeData(indices.data());
}

MeshAsset::~MeshAsset()
{
    log_dbg("Destroying mesh asset:");
    log_dbg(meshName.c_str());
    
    if(vertexBuffer != nullptr) delete vertexBuffer;
    if(indexBuffer != nullptr) delete indexBuffer;
}