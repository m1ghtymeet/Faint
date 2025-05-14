#pragma once
//#include "Physics2/Physics.h"
#include "Common/Types.h"
#include <fstream>
#include <string>
#include <filesystem>
#include <format>
#include <vector>
#include <iostream>
#include <assimp/matrix3x3.h>
#include <assimp/matrix4x4.h>

namespace Util {

    // Math
    inline float FInterpTo(float current, float target, float deltaTime, float interpSpeed) {
        if (interpSpeed < 0.0f)
            return target;
        const float dist = target - current;
        if (dist * dist < SMALL_NUMBER)
            return target;
        const float deltaMove = dist * glm::clamp(deltaTime * interpSpeed, 0.0f, 1.0f);
        return current + deltaMove;
    }

    // Raycasting
    //CubeRayResult CastCubeRay(const glm::vec3& rayOrigin, const glm::vec3 rayDir, std::vector<Transform>& cubeTransforms, float maxDistance = 99999);
    glm::vec3 GetMouseRayDir(glm::mat4 projection, glm::mat4 view, int windowWidth, int windowHeight, int mouseX, int mouseY);
    bool RayIntersectsTriangle(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, float& t);

    // Mesh
    std::vector<Vertex> GenerateSphereVertices(float radius, int segments);
    std::vector<Vertex> GenerateRingVertices(float sphereRadius, float ringThickness, int ringSegments, int thicknessSegments);
    std::vector<Vertex> GenerateConeVertices(float radius, float height, int segments);
    std::vector<Vertex> GenerateCylinderVertices(float radius, float height, int subdivisions);
    std::vector<Vertex> GenerateCubeVertices();
    std::vector<uint32_t> GenerateSphereIndices(int segments);
    std::vector<uint32_t> GenerateRingIndices(int segments, int thicknessSegments);
    std::vector<uint32_t> GenerateConeIndices(int segments);
    std::vector<uint32_t> GenerateCylinderIndices(int subdivisions);
    std::vector<uint32_t> GenerateCubeIndices();

    // Text
    std::string Lowercase(std::string& str);
    bool StrCmp(const char* queryA, const char* queryB);
    const char* CopyConstChar(const char* text);
    std::string ReadTextFromFile(std::string path);
    std::string FloatToString(float value, int precision = 2);
    std::string BoolToString(bool value);

    // File
    inline bool FileExists(const std::string_view name) {
        struct stat buffer;
        return (stat(name.data(), &buffer) == 0);
    }

    inline const std::string GetFilename(const std::string& filename) {
        std::string result = filename.substr(filename.rfind("/") + 1);
        result = result.substr(0, result.length() - 4);
        return result;
    }

    // Convert
    //glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4& from) {
    //    glm::mat4 to;
    //    //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
    //    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    //    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    //    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    //    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    //    return to;
    //}

    /*inline PxMat44 GlmMat4ToPxMat44(glm::mat4 mat4) {
        PxMat44 matrix;
        for (int x = 0; x < 4; x++)
            for (int y = 0; y < 4; y++)
                matrix[x][y] = mat4[x][y];
        return matrix;
    }

    inline glm::mat4 PxMat44ToGlmMat4(PxMat44 pxMatrix) {
        glm::mat4 matrix;
        for (int x = 0; x < 4; x++)
            for (int y = 0; y < 4; y++)
                matrix[x][y] = pxMatrix[x][y];
        return matrix;
    }

    inline glm::vec3 PxVec3toGlmVec3(PxVec3 vec) {
        return { vec.x, vec.y, vec.z };
    }

    inline glm::vec3 PxVec3toGlmVec3(PxExtendedVec3 vec) {
        return { vec.x, vec.y, vec.z };
    }*/

    inline float Lerp(float a, float b, float f) {
        return a + f * (b - a);
    }

    inline glm::ivec2 CalculateScreenSpaceCoordinates(const glm::vec3& worldPos, const glm::mat4& mvpMatrix, int screenWidth, int screenHeight, bool flipY = false) {
        glm::vec4 clipCoords = mvpMatrix * glm::vec4(worldPos, 1.0f);
        glm::vec3 ndcCoords = glm::vec3(clipCoords) / clipCoords.w;
        glm::ivec2 screenCoords;
        screenCoords.x = (ndcCoords.x + 1.0f) * 0.5f * screenWidth;
        if (flipY) {
            screenCoords.y = screenHeight - (1.0f - ndcCoords.y) * 0.5f * screenHeight;
        }
        else {
            screenCoords.y = (1.0f - ndcCoords.y) * 0.5f * screenHeight;
        }
        return screenCoords;
    }

    inline glm::vec3 Vec3Min(const glm::vec3& a, const glm::vec3& b) {
        return glm::vec3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
    }

    inline glm::vec3 Vec3Max(const glm::vec3& a, const glm::vec3& b) {
        return glm::vec3(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
    }

    /*inline FileInfo GetFileInfo(std::string filepath) {
        // isolate name
        std::string filename = filepath.substr(filepath.rfind("/") + 1);
        filename = filename.substr(0, filename.length() - 4);
        // isolate filetype
        std::string filetype = filepath.substr(filepath.length() - 3);
        // isolate directory
        std::string directory = filepath.substr(0, filepath.rfind("/") + 1);
        // material name
        std::string materialType = "NONE";
        if (filename.length() > 5) {
            std::string query = filename.substr(filename.length() - 3);
            if (query == "ALB" || query == "RMA" || query == "NRM")
                materialType = query;
        }
        // RETURN IT
        FileInfo info;
        info.fullpath = filepath;
        info.filename = filename;
        info.filetype = filetype;
        info.directory = directory;
        info.materialType = materialType;
        return info;
    }

    inline FileInfo GetFileInfo(const std::filesystem::directory_entry& filepath) {
        const auto& path{ filepath.path() };

        static const auto get_material_type{ [](std::string_view filename) {
            if (filename.size() > 5) {
                filename.remove_prefix(filename.size() - 3);
                if (filename == "ALB" || filename == "RMA" || filename == "NRM") {
                    return std::string{ filename };
                }
            }
            return std::string{ "NONE" };
        } };

        const auto stem{ path.has_stem() ? path.stem().string() : "" };
        std::string filetype = path.has_extension() ? path.extension().string().substr(1) : "";

        return FileInfo{
            path.string(),
            path.parent_path().string(),
            stem,
            Lowercase(filetype),
            get_material_type(stem)
        };
    }*/

    inline glm::vec3 GetTranslationFromMatrix(glm::mat4 matrix) {
        return glm::vec3(matrix[3][0], matrix[3][0], matrix[3][2]);
    }

    inline void SetNormalsAndTangentsFromVertices(Vertex* vert0, Vertex* vert1, Vertex* vert2) {
        // Shortcuts for UVs
        glm::vec3& v0 = vert0->position;
        glm::vec3& v1 = vert1->position;
        glm::vec3& v2 = vert2->position;
        glm::vec2& uv0 = vert0->uv;
        glm::vec2& uv1 = vert1->uv;
        glm::vec2& uv2 = vert2->uv;
        // Edges of the triangle : position delta. UV delta
        glm::vec3 deltaPos1 = v1 - v0;
        glm::vec3 deltaPos2 = v2 - v0;
        glm::vec2 deltaUV1 = uv1 - uv0;
        glm::vec2 deltaUV2 = uv2 - uv0;
        float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
        glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
        glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;
        glm::vec3 normal = glm::normalize(glm::cross(deltaPos1, deltaPos2));
        vert0->normal = normal;
        vert1->normal = normal;
        vert2->normal = normal;
        vert0->tangent = tangent;
        vert1->tangent = tangent;
        vert2->tangent = tangent;
    }

    /*inline void SetNormalsAndTangentsFromVertices(CSGVertex* vert0, CSGVertex* vert1, CSGVertex* vert2) {
        // Shortcuts for UVs
        glm::vec3& v0 = vert0->position;
        glm::vec3& v1 = vert1->position;
        glm::vec3& v2 = vert2->position;
        glm::vec2& uv0 = vert0->texcoord;
        glm::vec2& uv1 = vert1->texcoord;
        glm::vec2& uv2 = vert2->texcoord;
        // Edges of the triangle : position delta. UV delta
        glm::vec3 deltaPos1 = v1 - v0;
        glm::vec3 deltaPos2 = v2 - v0;
        glm::vec2 deltaUV1 = uv1 - uv0;
        glm::vec2 deltaUV2 = uv2 - uv0;
        float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
        glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
        glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;
        glm::vec3 normal = glm::normalize(glm::cross(deltaPos1, deltaPos2));
        vert0->normal = normal;
        vert1->normal = normal;
        vert2->normal = normal;
        vert0->tangent = tangent;
        vert1->tangent = tangent;
        vert2->tangent = tangent;
    }*/

    inline bool AreNormalsAligned(const glm::vec3& normal1, const glm::vec3& normal2, float threshold = 0.999f) {
        float dotProduct = glm::dot(normal1, normal2);
        return dotProduct * dotProduct > threshold * threshold;
    }

    /*inline glm::vec3 PxVec3toGlmVec3(PxVec3 vec) {
        return { vec.x, vec.y, vec.z };
    }

    inline glm::vec3 PxVec3toGlmVec3(PxExtendedVec3 vec) {
        return { vec.x, vec.y, vec.z };
    }

    inline PxQuat GlmQuatToPxQuat(glm::quat quat) {
        return { quat.x, quat.y, quat.z, quat.w };
    }*/

    inline glm::vec3 GetMouseRay(glm::mat4 projection, glm::mat4 view, int windowWidth, int windowHeight, int mouseX, int mouseY) {
        float x = (2.0f * mouseX) / (float)windowWidth - 1.0f;
        float y = 1.0f - (2.0f * mouseY) / (float)windowHeight;
        float z = 1.0f;
        glm::vec3 ray_nds = glm::vec3(x, y, z);
        glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, ray_nds.z, 1.0f);
        glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
        ray_eye = glm::vec4(ray_eye.x, ray_eye.y, ray_eye.z, 0.0f);
        glm::vec4 inv_ray_wor = (inverse(view) * ray_eye);
        glm::vec3 ray_wor = glm::vec3(inv_ray_wor.x, inv_ray_wor.y, inv_ray_wor.z);
        ray_wor = normalize(ray_wor);
        return ray_wor;
    }

    /*inline PhysXRayResult CastPhysXRay(glm::vec3 rayOrigin, glm::vec3 rayDirection, float rayLength, PxU32 collisionFlags, bool cullBackFacing = false) {

        PxScene* scene = Physics::GetScene();

        PxVec3 origin = PxVec3(rayOrigin.x, rayOrigin.y, rayOrigin.z);
        PxVec3 unitDir = PxVec3(rayDirection.x, rayDirection.y, rayDirection.z);
        PxReal maxDistance = rayLength;
        PxRaycastBuffer hit;
        // [in] Define what parts of PxRaycastHit we're interested in
     //   const PxHitFlags outputFlags = PxHitFlag::ePOSITION | PxHitFlag::eNORMAL | ~PxHitFlag::eMESH_BOTH_SIDES;

        PxHitFlags outputFlags = PxHitFlag::ePOSITION | PxHitFlag::eNORMAL | PxHitFlag::eMESH_BOTH_SIDES;
        if (cullBackFacing) {
            outputFlags = PxHitFlag::ePOSITION | PxHitFlag::eNORMAL;
        }

        // Only ray cast against objects with the GROUP_RAYCAST flag
        PxQueryFilterData filterData = PxQueryFilterData();
        filterData.data.word0 = collisionFlags;

        // Defaults
        PhysXRayResult result;
        result.hitObjectName = "NO_HIT";
        result.hitPosition = glm::vec3(0, 0, 0);
        result.surfaceNormal = glm::vec3(0, 0, 0);
        result.rayDirection = rayDirection;
        result.hitFound = false;
        result.hitActor = nullptr;
        result.parent = nullptr;

        // Cast the ray
        bool status = scene->raycast(origin, unitDir, maxDistance, hit, outputFlags, filterData);

        if (status) {
            if (hit.block.actor->getName()) {
                result.hitObjectName = hit.block.actor->getName();
            }
            else
                result.hitObjectName = "HIT OBJECT HAS NO ACTOR NAME";

            result.hitPosition = glm::vec3(hit.block.position.x, hit.block.position.y, hit.block.position.z);
            result.surfaceNormal = glm::vec3(hit.block.normal.x, hit.block.normal.y, hit.block.normal.z);
            result.hitFound = true;
            result.hitActor = hit.block.actor;

            if (hit.block.actor->userData) {
                PhysicsObjectData* physicsObjectData = (PhysicsObjectData*)hit.block.actor->userData;
                result.physicsObjectType = physicsObjectData->type;
                result.parent = physicsObjectData->parent;
            }
            else {
                result.physicsObjectType = ObjectType::UNDEFINED;
                result.hitFound = false;
                std::cout << "no user data found on ray hit\n";
            }

            /*EntityData* hitEntityData = (EntityData*)hit.block.actor->userData;
             m_parent = hitEntityData->parent;
             mg_physicsObjectType = hitEntityData->type;*/
             /*

             }
             return result;
         }*/

    inline glm::mat4 Mat4InitScaleTransform(float scaleX, float scaleY, float scaleZ) {
        return glm::scale(glm::mat4(1.0f), glm::vec3(scaleX, scaleY, scaleZ));
    }

    inline glm::mat4 Mat4InitTranslationTransform(float x, float y, float z) {
        return glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
    }

    /*inline glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4& from) {
        glm::mat4 to;
        to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
        to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
        to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
        to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
        return to;
    }

    inline PxMat44 GlmMat4ToPxMat44(glm::mat4 mat4) {
        PxMat44 matrix;
        for (int x = 0; x < 4; x++)
            for (int y = 0; y < 4; y++)
                matrix[x][y] = mat4[x][y];
        return matrix;
    }

    inline glm::mat4 PxMat44ToGlmMat4(PxMat44 pxMatrix) {
        glm::mat4 matrix;
        for (int x = 0; x < 4; x++)
            for (int y = 0; y < 4; y++)
                matrix[x][y] = pxMatrix[x][y];
        return matrix;
    }*/

    inline std::string Vec3ToString(glm::vec3 v) {
        return std::string("[" + std::format("{:.2f}", v.x) + ", " + std::format("{:.2f}", v.y) + ", " + std::format("{:.2f}", v.z) + "]");
    }

    inline std::string QuatToString(glm::quat q) {
        return std::string("[" + std::format("{:.2f}", q.x) + ", " + std::format("{:.2f}", q.y) + ", " + std::format("{:.2f}", q.z) + "]");
    }

    inline float DistanceSquared(glm::vec3 A, glm::vec3 B) {
        glm::vec3 C = A - B;
        return glm::dot(C, C);
    }

    inline float RandomFloat(float min, float max) {
        return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
    }

    inline bool IsPointInTriangle2D(const glm::vec2& pt, const glm::vec2& v0, const glm::vec2& v1, const glm::vec2& v2) {
        glm::vec2 v0v1 = v1 - v0;
        glm::vec2 v0v2 = v2 - v0;
        glm::vec2 v0pt = pt - v0;
        // Precompute the determinant to avoid division in each barycentric calculation
        float denom = v0v1.x * v0v2.y - v0v1.y * v0v2.x;
        // Early out if the denominator is zero (i.e., the triangle is degenerate)
        if (denom == 0.0f) return false;
        float invDenom = 1.0f / denom;
        // Compute the barycentric coordinates (u, v, w) directly
        float v = (v0pt.x * v0v2.y - v0pt.y * v0v2.x) * invDenom;
        float w = (v0v1.x * v0pt.y - v0v1.y * v0pt.x) * invDenom;
        float u = 1.0f - v - w;
        // Check if point is inside the triangle
        return (u >= 0.0f) && (v >= 0.0f) && (w >= 0.0f);
    }

    inline void InterpolateQuaternion(glm::quat& Out, const glm::quat& Start, const glm::quat& End, float pFactor) {
        // Calc cosine theta
        float cosom = Start.x * End.x + Start.y * End.y + Start.z * End.z + Start.w * End.w;
        // adjust signs (if necessay)
        glm::quat end = End;
        if (cosom < static_cast<float>(0.0)) {
            cosom = -cosom;
            end.x = -end.x; // Reverse all signs
            end.y = -end.y;
            end.z = -end.z;
            end.w = -end.w;
        }
        // Calculate coefficients
        float sclp, sclq;
        if ((static_cast<float>(1.0) - cosom) > static_cast<float>(0.0001)) {
            // Standard case (slerp)
            float omega, sinom;
            omega = std::acos(cosom);
            sinom = std::sin(omega);
            sclp = std::sin((static_cast<float>(1.0) - pFactor) * omega) / sinom;
            sclq = std::sin(pFactor * omega) / sinom;
        }
        else {
            // Very close, do linear interp (because it's faster)
            sclp = static_cast<float>(1.0) - pFactor;
            sclq = pFactor;
        }
        Out.x = sclp * Start.x + sclq * end.x;
        Out.y = sclp * Start.y + sclq * end.y;
        Out.z = sclp * Start.z + sclq * end.z;
        Out.w = sclp * Start.w + sclq * end.w;
    }

    //inline std::string Vec3ToString(glm::vec3 v) {
    //    return std::string("[" + std::format("{:.2f}", v.x) + ", " + std::format("{:.2f}", v.y) + ", " + std::format("{:.2f}", v.z) + "]");
    //}
    //
    //inline std::string QuatToString(glm::quat q) {
    //    return std::string("[" + std::format("{:.2f}", q.x) + ", " + std::format("{:.2f}", q.y) + ", " + std::format("{:.2f}", q.z) + "]");
    //}
    //
    //inline std::string Mat4ToString(glm::mat4 m) {
    //    std::string result;
    //    result += std::format("{:.2f}", m[0][0]) + ", " + std::format("{:.2f}", m[1][0]) + ", " + std::format("{:.2f}", m[2][0]) + ", " + std::format("{:.2f}", m[3][0]);
    //    result += std::format("{:.2f}", m[0][1]) + ", " + std::format("{:.2f}", m[1][1]) + ", " + std::format("{:.2f}", m[2][1]) + ", " + std::format("{:.2f}", m[3][1]);
    //    result += std::format("{:.2f}", m[0][2]) + ", " + std::format("{:.2f}", m[1][2]) + ", " + std::format("{:.2f}", m[2][2]) + ", " + std::format("{:.2f}", m[3][2]);
    //    result += std::format("{:.2f}", m[0][3]) + ", " + std::format("{:.2f}", m[1][3]) + ", " + std::format("{:.2f}", m[2][3]) + ", " + std::format("{:.2f}", m[3][3]);
    //    return result;
    //}

    inline float MapRange(float inValue, float minInRange, float maxInRange, float minOutRange, float maxOutRange) {
        float x = (inValue - minInRange) / (maxInRange - minInRange);
        return minOutRange + (maxOutRange - minOutRange) * x;
    }

    /*inline std::vector<Vertex> GetAABBVertices(AABB& aabb, glm::vec3 color) {
        std::vector<Vertex> vertices;
        glm::vec4 frontTopLeft = glm::vec4(aabb.boundsMin.x, aabb.boundsMax.y, aabb.boundsMax.z, 1.0f);
        glm::vec4 frontTopRight = glm::vec4(aabb.boundsMax.x, aabb.boundsMax.y, aabb.boundsMax.z, 1.0f);
        glm::vec4 frontBottomLeft = glm::vec4(aabb.boundsMin.x, aabb.boundsMin.y, aabb.boundsMax.z, 1.0f);
        glm::vec4 frontBottomRight = glm::vec4(aabb.boundsMax.x, aabb.boundsMin.y, aabb.boundsMax.z, 1.0f);
        glm::vec4 backTopLeft = glm::vec4(aabb.boundsMin.x, aabb.boundsMax.y, aabb.boundsMin.z, 1.0f);
        glm::vec4 backTopRight = glm::vec4(aabb.boundsMax.x, aabb.boundsMax.y, aabb.boundsMin.z, 1.0f);
        glm::vec4 backBottomLeft = glm::vec4(aabb.boundsMin.x, aabb.boundsMin.y, aabb.boundsMin.z, 1.0f);
        glm::vec4 backBottomRight = glm::vec4(aabb.boundsMax.x, aabb.boundsMin.y, aabb.boundsMin.z, 1.0f);
        vertices.push_back(Vertex(glm::vec3(frontBottomLeft), color));
        vertices.push_back(Vertex(glm::vec3(frontBottomRight), color));
        vertices.push_back(Vertex(glm::vec3(frontTopLeft), color));
        vertices.push_back(Vertex(glm::vec3(frontTopRight), color));
        vertices.push_back(Vertex(glm::vec3(frontBottomLeft), color));
        vertices.push_back(Vertex(glm::vec3(frontTopLeft), color));
        vertices.push_back(Vertex(glm::vec3(frontBottomRight), color));
        vertices.push_back(Vertex(glm::vec3(frontTopRight), color));
        vertices.push_back(Vertex(glm::vec3(backBottomLeft), color));
        vertices.push_back(Vertex(glm::vec3(backBottomRight), color));
        vertices.push_back(Vertex(glm::vec3(backTopLeft), color));
        vertices.push_back(Vertex(glm::vec3(backTopRight), color));
        vertices.push_back(Vertex(glm::vec3(backBottomLeft), color));
        vertices.push_back(Vertex(glm::vec3(backTopLeft), color));
        vertices.push_back(Vertex(glm::vec3(backBottomRight), color));
        vertices.push_back(Vertex(glm::vec3(backTopRight), color));
        vertices.push_back(Vertex(glm::vec3(frontBottomLeft), color));
        vertices.push_back(Vertex(glm::vec3(backBottomLeft), color));
        vertices.push_back(Vertex(glm::vec3(frontBottomRight), color));
        vertices.push_back(Vertex(glm::vec3(backBottomRight), color));
        vertices.push_back(Vertex(glm::vec3(frontTopLeft), color));
        vertices.push_back(Vertex(glm::vec3(backTopLeft), color));
        vertices.push_back(Vertex(glm::vec3(frontTopRight), color));
        vertices.push_back(Vertex(glm::vec3(backTopRight), color));
        return vertices;
    }*/

    inline void CalculateAABB(const std::vector<Vertex>& vertices, glm::vec3& aabbMin, glm::vec3& aabbMax) {
        if (vertices.empty()) {
            return;
        }
        aabbMin = vertices[0].position;
        aabbMax = vertices[0].position;
        for (const auto& vertex : vertices) {
            aabbMin = glm::min(aabbMin, vertex.position);
            aabbMax = glm::max(aabbMax, vertex.position);
        }
    }
}