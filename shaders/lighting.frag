#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform sampler2D normalMap;
uniform float shininess;
uniform vec3 viewPos;
uniform vec3 lightPos;

// Output fragment color
out vec4 finalColor;

in mat3 TBN;

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, vec2(fragTexCoord.x * 1.0f, -fragTexCoord.y * 1.0f));
    //texelColor = vec4(1.0);
    vec3 lightDot = vec3(0.0);

    //vec3 normal = normalize(fragNormal);
    vec3 normal = texture(normalMap, vec2(fragTexCoord.x * 1.0f, -fragTexCoord.y * 1.0f)).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    //normal.y = -normal.y;
    //normal = normalize(normal * TBN);
    normal = fragNormal;

    vec3 viewD = normalize(viewPos - fragPosition);
    vec3 specular = vec3(0.0);

    vec4 tint = vec4(1.0, 1.0, 1.0, 1.0);

    vec3 light = normalize(lightPos - fragPosition);

    float NdotL = max(dot(normal, light), 0.0);
    lightDot += vec3(1.0, 1.0, 1.0) * NdotL;

    float specCo = 0.0;
    if (NdotL > 0.0) specCo = pow(max(0.0, dot(viewD, reflect(-(light), normal))), 4.0); // 16 refers to shine
    specular += specCo;

    finalColor = (texelColor * ((tint + vec4(specular, 1.0)) * vec4(lightDot, 1.0)));
    finalColor += texelColor * (vec4(1.0, 1.0, 1.0, 1.0) / 40.0) * tint;
    //finalColor = vec4(lightDot, 1.0f);

    // Gamma correction
    finalColor = pow(finalColor, vec4(1.0 / 2.2));

    //finalColor = vec4(normal, 1.0);
}
