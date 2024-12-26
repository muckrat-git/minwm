#version 460

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Input uniform values
uniform mat4 mvp;

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;
out vec4 fragColor;
out vec2 screenPosition;
out vec4 glPosition;

// NOTE: Add here your custom variables

void main() {
    // Send vertex attributes to fragment shader
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    // Calculate final vertex position
    gl_Position = mvp * vec4(vertexPosition, 1.0);
    glPosition = gl_Position;
    screenPosition = gl_Position.xy / 2.0 + vec2(0.5, 0.5);
}