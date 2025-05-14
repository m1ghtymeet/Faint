#include "Util.h"

bool Util::RayIntersectsTriangle(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, float& t) {
	const float EPSILON = 1e-8f;
	glm::vec3 edge1 = v1 - v0;
	glm::vec3 edge2 = v2 - v0;
	glm::vec3 h = glm::cross(rayDir, edge2);
	float a = glm::dot(edge1, h);
	if (fabs(a) < EPSILON) {
		return false; // Ray is parallel to the triangle.
	}
	float f = 1.0f / a;
	glm::vec3 s = rayOrigin - v0;
	float u = f * glm::dot(s, h);
	if (u < 0.0f || u > 1.0f) {
		return false;
	}
	glm::vec3 q = glm::cross(s, edge1);
	float v = f * glm::dot(rayDir, q);
	if (v < 0.0f || u + v > 1.0f) {
		return false;
	}
	t = f * glm::dot(edge2, q); // Distance along the ray to the intersection.
	return t > EPSILON;
}

std::vector<Vertex> Util::GenerateSphereVertices(float radius, int segments) {
	std::vector<Vertex> vertices;
	// Ensure segments are reasonable
	segments = std::max(segments, 4);
	// Angular step sizes
	float thetaStep = glm::two_pi<float>() / segments; // Longitude step
	float phiStep = glm::pi<float>() / segments;       // Latitude step
	for (int i = 0; i <= segments; i++) { // Latitude loop
		float phi = i * phiStep; // Latitude angle
		for (int j = 0; j <= segments; j++) { // Longitude loop
			float theta = j * thetaStep; // Longitude angle
			// Calculate position on the sphere
			glm::vec3 position = glm::vec3(
				radius * sin(phi) * cos(theta),
				radius * cos(phi),
				radius * sin(phi) * sin(theta)
			);
			// Calculate normal (direction from sphere center)
			glm::vec3 normal = glm::normalize(position);
			// Calculate tangent (partial derivative with respect to theta)
			glm::vec3 tangent = glm::normalize(glm::vec3(
				-radius * sin(phi) * sin(theta),
				0.0f,
				radius * sin(phi) * cos(theta)
			));
			// Add vertex to the vector
			Vertex& vertex = vertices.emplace_back();
			vertex.position = position;
			vertex.normal = normal;
			vertex.tangent = tangent;
		}
	}
	return vertices;
}

std::vector<uint32_t> Util::GenerateSphereIndices(int segments) {
	std::vector<uint32_t> indices;
	for (int i = 0; i < segments; i++) { // Latitude loop
		for (int j = 0; j < segments; j++) { // Longitude loop
			int nextI = i + 1;
			int nextJ = (j + 1) % (segments + 1);
			// Indices of the quad
			uint32_t v0 = i * (segments + 1) + j;
			uint32_t v1 = nextI * (segments + 1) + j;
			uint32_t v2 = i * (segments + 1) + nextJ;
			uint32_t v3 = nextI * (segments + 1) + nextJ;
			// First triangle
			indices.push_back(v2);
			indices.push_back(v1);
			indices.push_back(v0);
			// Second triangle
			indices.push_back(v3);
			indices.push_back(v1);
			indices.push_back(v2);
		}
	}
	return indices;
}

std::vector<Vertex> Util::GenerateRingVertices(float sphereRadius, float ringThickness, int segments, int thicknessSegments) {
	std::vector<Vertex> vertices;
	for (int i = 0; i < segments; ++i) {
		float angle = glm::two_pi<float>() * i / segments;
		glm::vec3 ringCenter = glm::vec3(
			sphereRadius * cos(angle),
			sphereRadius * sin(angle),
			0.0f
		);
		for (int j = 0; j < thicknessSegments; ++j) {
			float thicknessAngle = glm::two_pi<float>() * j / thicknessSegments;
			glm::vec3 offset = glm::vec3(
				ringThickness * cos(thicknessAngle) * cos(angle),
				ringThickness * cos(thicknessAngle) * sin(angle),
				ringThickness * sin(thicknessAngle)
			);
			Vertex& vertex = vertices.emplace_back();
			vertex.position = ringCenter + offset;
			vertex.normal = glm::normalize(offset);
			vertex.tangent = glm::normalize(glm::vec3(-sin(angle), cos(angle), 0.0f));
		}
	}
	return vertices;
}

std::vector<uint32_t> Util::GenerateRingIndices(int segments, int thicknessSegments) {
	std::vector<uint32_t> indices;
	for (int i = 0; i < segments; ++i) {
		for (int j = 0; j < thicknessSegments; ++j) {
			int nextI = (i + 1) % segments;
			int nextJ = (j + 1) % thicknessSegments;
			// Indices of the quad
			uint32_t v0 = i * thicknessSegments + j;
			uint32_t v1 = nextI * thicknessSegments + j;
			uint32_t v2 = i * thicknessSegments + nextJ;
			uint32_t v3 = nextI * thicknessSegments + nextJ;
			// First triangle
			indices.push_back(v0);
			indices.push_back(v1);
			indices.push_back(v2);
			// Second triangle
			indices.push_back(v2);
			indices.push_back(v1);
			indices.push_back(v3);
		}
	}
	return indices;
}

std::vector<Vertex> Util::GenerateCylinderVertices(float radius, float height, int subdivisions) {
	std::vector<Vertex> vertices;
	const float angleStep = 2.0f * PI / subdivisions;
	// Generate vertices for the bottom cap
	vertices.push_back(Vertex(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));  // Center of the bottom cap
	for (int i = 0; i <= subdivisions; ++i) {
		float angle = i * angleStep;
		float x = radius * cos(angle);
		float z = radius * sin(angle);
		vertices.push_back(Vertex(glm::vec3(x, 0.0f, z), glm::vec3(0.0f, -1.0f, 0.0f)));
	}
	// Generate vertices for the top cap
	vertices.push_back(Vertex(glm::vec3(0.0f, height, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));  // Center of the top cap
	for (int i = 0; i <= subdivisions; ++i) {
		float angle = i * angleStep;
		float x = radius * cos(angle);
		float z = radius * sin(angle);
		vertices.push_back(Vertex(glm::vec3(x, height, z), glm::vec3(0.0f, 1.0f, 0.0f)));
	}
	// Generate vertices for the side
	for (int i = 0; i <= subdivisions; ++i) {
		float angle = i * angleStep;
		float x = radius * cos(angle);
		float z = radius * sin(angle);
		glm::vec3 normal = glm::normalize(glm::vec3(cos(angle), 0.0f, sin(angle)));
		// Bottom vertex
		vertices.push_back(Vertex(glm::vec3(x, 0.0f, z), normal));
		// Top vertex
		vertices.push_back(Vertex(glm::vec3(x, height, z), normal));
	}
	return vertices;
}

std::vector<uint32_t> Util::GenerateCylinderIndices(int subdivisions) {
	std::vector<uint32_t> indices;

	// Indices for the bottom cap
	for (int i = 1; i <= subdivisions; ++i) {
		indices.push_back(0);               // Center of the bottom cap
		indices.push_back(i);               // Current vertex
		indices.push_back(i % subdivisions + 1);  // Next vertex (wraps around)
	}

	// Indices for the top cap
	int topCenterIndex = subdivisions + 2;  // Index of the top center
	for (int i = 1; i <= subdivisions; ++i) {
		indices.push_back(topCenterIndex);                      // Center of the top cap
		indices.push_back(topCenterIndex + i);                  // Current vertex
		indices.push_back(topCenterIndex + (i % subdivisions) + 1);  // Next vertex (wraps around)
	}

	// Indices for the side
	int sideStartIndex = (subdivisions + 2) * 2;
	for (int i = 0; i < subdivisions; ++i) {
		int bottomIndex = sideStartIndex + i * 2;
		int topIndex = bottomIndex + 1;

		indices.push_back(bottomIndex);         // Bottom vertex of the quad
		indices.push_back(topIndex);            // Top vertex of the quad
		indices.push_back(bottomIndex + 2);     // Next bottom vertex of the quad

		indices.push_back(topIndex);            // Top vertex of the quad
		indices.push_back(topIndex + 2);        // Next top vertex of the quad
		indices.push_back(bottomIndex + 2);     // Next bottom vertex of the quad
	}

	return indices;
}

std::vector<Vertex> Util::GenerateCubeVertices() {
	std::vector<Vertex> vertices;
	glm::vec3 normals[] = {
		{ 0.0f,  0.0f,  1.0f}, // Front
		{ 0.0f,  0.0f, -1.0f}, // Back
		{ 1.0f,  0.0f,  0.0f}, // Right
		{-1.0f,  0.0f,  0.0f}, // Left
		{ 0.0f,  1.0f,  0.0f}, // Top
		{ 0.0f, -1.0f,  0.0f}, // Bottom
	};
	glm::vec3 positions[] = {
		{-0.5f, -0.5f,  0.5f}, {0.5f, -0.5f,  0.5f}, {0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f},   // Front face
		{-0.5f, -0.5f, -0.5f}, {-0.5f,  0.5f, -0.5f}, {0.5f,  0.5f, -0.5f}, {0.5f, -0.5f, -0.5f},   // Back face
		{0.5f, -0.5f,  0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f,  0.5f, -0.5f}, {0.5f,  0.5f,  0.5f},     // Right face
		{-0.5f, -0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f}, // Left face
		{-0.5f,  0.5f,  0.5f}, {0.5f,  0.5f,  0.5f}, {0.5f,  0.5f, -0.5f}, {-0.5f,  0.5f, -0.5f},   // Top face
		{-0.5f, -0.5f,  0.5f}, {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, -0.5f,  0.5f},   // Bottom face
	};
	for (int i = 0; i < 6; ++i) { // Each face
		for (int j = 0; j < 4; ++j) { // Each vertex per face
			vertices.push_back(Vertex(positions[i * 4 + j], normals[i]));
		}
	}
	return vertices;
}

std::vector<uint32_t> Util::GenerateCubeIndices() {
	std::vector<unsigned int> indices = {
			0, 1, 2, 2, 3, 0,       // Front face
			4, 5, 6, 6, 7, 4,       // Back face
			8, 9, 10, 10, 11, 8,    // Right face
			12, 13, 14, 14, 15, 12, // Left face
			16, 17, 18, 18, 19, 16, // Top face
			20, 21, 22, 22, 23, 20  // Bottom face
	};
	return indices;
}
