#version 330

in vec2 fragTexCoord;
out vec4 outColor;

uniform sampler2D texture0;
uniform sampler2D bloomTexture1;
uniform sampler2D bloomTexture2;
uniform sampler2D bloomTexture3;
uniform sampler2D bloomTexture4;

uniform bool toggle;

void main()
{
    vec3 hdrColor = texture(texture0, fragTexCoord).rgb;

    vec3 blurColor1 = texture(bloomTexture1, fragTexCoord).rgb;
    vec3 blurColor2 = texture(bloomTexture2, fragTexCoord).rgb;
    vec3 blurColor3 = texture(bloomTexture3, fragTexCoord).rgb;
    vec3 blurColor4 = texture(bloomTexture4, fragTexCoord).rgb;

    vec3 sharpColor = (blurColor1 * 1.0 + blurColor2) * 0.75;
    vec3 blurryColor = (blurColor3 + blurColor4 * 1.4) * 0.6;
    hdrColor += sharpColor + blurryColor;

    // Basic tonemapping
    float exposure = 1.5;
    hdrColor = vec3(1.0) - exp(-hdrColor * exposure);

    //hdrColor = texture(texture0, fragTexCoord).rgb;
    //outColor = vec4(blurColor4.rgb, 1.0f);
    //outColor = vec4(hdrColor.rgb, 1.0f);
    outColor = vec4(hdrColor.rgb, 1.0);
}
