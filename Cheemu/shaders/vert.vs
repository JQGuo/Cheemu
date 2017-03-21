#version 450

layout(location = 0) in vec2 vertPosition;
layout(location = 1) in float vertColour;

out float fragColour;

void main() {
    gl_Position = vec4( vertPosition, 0, 1 );
    fragColour = vertColour;
}
