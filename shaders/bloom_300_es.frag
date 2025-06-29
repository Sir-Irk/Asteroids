#version 300 es

in mediump vec2 fragTexCoord;
out mediump vec4 outColor;

uniform sampler2D texture0;
uniform sampler2D bloomTexture1;
uniform sampler2D bloomTexture2;
uniform sampler2D bloomTexture3;
uniform sampler2D bloomTexture4;

uniform bool toggle;

void main()
{
    mediump vec3 hdrColor = texture(texture0, fragTexCoord).rgb;

    mediump vec3 blurColor1 = texture(bloomTexture1, fragTexCoord).rgb;
    mediump vec3 blurColor2 = texture(bloomTexture2, fragTexCoord).rgb;
    mediump vec3 blurColor3 = texture(bloomTexture3, fragTexCoord).rgb;
    mediump vec3 blurColor4 = texture(bloomTexture4, fragTexCoord).rgb;

    mediump vec3 sharpColor = (blurColor1 + blurColor2) * 0.7;
    mediump vec3 blurryColor = (blurColor3 + blurColor4 * 0.7) * 0.7;
    hdrColor += sharpColor + blurryColor;

    // Basic tonemapping
    mediump float exposure = 1.5;
    hdrColor = vec3(1.0) - exp(-hdrColor * exposure);

    //hdrColor = texture(texture0, fragTexCoord).rgb;
    //outColor = vec4(blurColor4.rgb, 1.0f);
    //outColor = vec4(hdrColor.rgb, 1.0f);
    outColor = vec4(hdrColor.rgb, 1.0);
}
