#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform bool horizontal;

out vec4 FragColor;

const mediump float weight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
    ivec2 tex_size = textureSize(texture0, 0);
    vec2 tex_offset = vec2(1.0 / float(tex_size.x), 1.0 / float(tex_size.y)); // gets size of single texel

    vec3 result = texture(texture0, fragTexCoord).rgb * weight[0]; // current fragment's contribution

    if (horizontal) {
        for (int i = 1; i < 5; ++i) {
            result += texture(texture0, fragTexCoord + vec2(tex_offset.x * float(i), 0.0)).rgb * weight[i];
            result += texture(texture0, fragTexCoord - vec2(tex_offset.x * float(i), 0.0)).rgb * weight[i];
        }
    } else {
        for (int i = 1; i < 5; ++i) {
            result += texture(texture0, fragTexCoord + vec2(0.0, tex_offset.y * float(i))).rgb * weight[i];
            result += texture(texture0, fragTexCoord - vec2(0.0, tex_offset.y * float(i))).rgb * weight[i];
        }
    }
    FragColor = vec4(result, 1.0);
}
