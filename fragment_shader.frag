#version 330 core

uniform vec3 light_pos = vec3(100, 303, 404);

in vec3 f_norm;
in vec3 f_pos;

out vec4 outColor;

void main()
{
    vec3 light_dir = normalize(light_pos - f_pos);
    vec3 norm = normalize(f_norm);
    float diff = (dot(norm, light_dir) + 1.0) / 2.5; // because the mag of both vecs is 1, this is just the cos of the angle between them
    vec3 color = max(diff, 0) * normalize(abs(f_pos));// - vec3(50, 50, 50)));//vec3(1, 1, 1);

    outColor = vec4(color, 1.0);
}
