#version 460
out vec3 vColor;

layout(std430, binding = 0) buffer Camera {
	mat4 u_projection;
	mat4 u_view;
};

const vec3 positions[6] = vec3[6](
    vec3(0.0, 0.0, 0.0), // X start
    vec3(1.0, 0.0, 0.0), // X end

    vec3(0.0, 0.0, 0.0), // Y start
    vec3(0.0, 1.0, 0.0), // Y end

    vec3(0.0, 0.0, 0.0), // Z start
    vec3(0.0, 0.0, 1.0)  // Z end
);

const vec3 colors[6] = vec3[6](
    vec3(1.0, 0.0, 0.0), // X red
    vec3(1.0, 0.0, 0.0),

    vec3(0.0, 1.0, 0.0), // Y green
    vec3(0.0, 1.0, 0.0),

    vec3(0.0, 0.0, 1.0), // Z blue
    vec3(0.0, 0.0, 1.0)
);

void main() {
	vec3 worldPos = positions[gl_VertexID] * 2.0;
	gl_Position = u_projection * u_view * vec4(worldPos, 1.0);
	vColor = colors[gl_VertexID];
}