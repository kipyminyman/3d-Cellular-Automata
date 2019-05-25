#version 330 core

in vec3 pos;
in vec3 norm;
in vec3 trans;
in uint trans_id;

out vec3 f_norm;
out vec3 f_pos;

uniform mat4 view;
uniform mat4 proj;

void main()
{
    vec3 trans_tmp = vec3(trans_id >> 20, (trans_id << 12) >> 22, (trans_id << 22) >> 22);
    trans_tmp = trans_tmp - vec3(40, 40, 40);
    //vec3 trans_tmp = vec3(trans_id, 0.0, 0.0);

    gl_Position = proj * view * vec4(pos + trans_tmp, 1.0);
    f_norm = norm;
    f_pos = pos + trans_tmp;//vec3(gl_InstanceID, 0, 0);
}
