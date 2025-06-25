#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec3 vertexTangent;
in vec4 vertexColor;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;
uniform vec4 difColor;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec3 fragNormal;
out vec4 fragColor;
//out vec3 viewDirTangentSpace;
//out vec3 lightDirTangentSpace;
out mat3 TBN;

const float normalOffset = 0.1;

vec3 transform_basis(vec3 v, vec3 t, vec3 b, vec3 n) {
    return vec3(dot(t, v), dot(b, v), dot(n, v));
}

void main()
{
    // Compute binormal from vertex normal and tangent
    vec3 vertexBinormal = cross(vertexNormal, vertexTangent);

    // Compute fragment normal based on normal transformations
    mat3 normalMatrix = transpose(inverse(mat3(matModel)));

    // Compute fragment position based on model transformations
    fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));
    //   vec3 lightDir = normalize(lightPos - fragPosition);
    //  vec3 viewDir = normalize(viewPos - fragPosition);

    fragTexCoord = vertexTexCoord * 1.0;
    fragNormal = normalize(normalMatrix * vertexNormal);

    //vertexTangent = vec3(-1.0, 0.0, 0.0);
    vec3 fragTangent = normalize(normalMatrix * vertexTangent);
    fragTangent = normalize(fragTangent - dot(fragTangent, fragNormal) * fragNormal);

    vec3 fragBinormal = normalize(normalMatrix * vertexBinormal);
    fragBinormal = cross(fragNormal, fragTangent);

    //lightDirTangentSpace = transform_basis(lightDir, fragTangent, fragBinormal, fragNormal);
    //viewDirTangentSpace = transform_basis(viewDir, fragTangent, fragBinormal, fragNormal);

    TBN = transpose(mat3(fragTangent, fragBinormal, fragNormal));
    // TBN = transpose(mat3(vec3(1.0, 0.0, 0.0), vec3(0.0, 0.0, 1.0), vec3(0.0, 1.0, 0.0)));

    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
