#version 450

in float fragColour;
out vec4 colour;

void main() {
    colour = vec4( fragColour );
}
