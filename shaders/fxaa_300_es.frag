#version 300 es

#define FXAA_REDUCE_MIN (1.0/128.0)
#define FXAA_REDUCE_MUL (1.0/8.0)
#define FXAA_SPAN_MAX 8.0

in mediump vec2 fragTexCoord;

out mediump vec4 fragColor;

uniform sampler2D texture0;
uniform mediump vec2 resolution;

void main()
{
    mediump vec2 inverse_resolution = vec2(1.0 / resolution.x, 1.0 / resolution.y);

    mediump vec3 rgbNW = texture(texture0, fragTexCoord.xy + (vec2(-1.0, -1.0)) * inverse_resolution).xyz;
    mediump vec3 rgbNE = texture(texture0, fragTexCoord.xy + (vec2(1.0, -1.0)) * inverse_resolution).xyz;
    mediump vec3 rgbSW = texture(texture0, fragTexCoord.xy + (vec2(-1.0, 1.0)) * inverse_resolution).xyz;
    mediump vec3 rgbSE = texture(texture0, fragTexCoord.xy + (vec2(1.0, 1.0)) * inverse_resolution).xyz;

    mediump vec3 rgbM = texture(texture0, fragTexCoord.xy).xyz;

    mediump vec3 luma = vec3(0.299, 0.587, 0.114);

    mediump float lumaNW = dot(rgbNW, luma);
    mediump float lumaNE = dot(rgbNE, luma);
    mediump float lumaSW = dot(rgbSW, luma);
    mediump float lumaSE = dot(rgbSE, luma);
    mediump float lumaM = dot(rgbM, luma);
    mediump float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    mediump float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    mediump vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    mediump float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
    mediump float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX), max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * rcpDirMin)) * inverse_resolution;

    mediump vec3 rgbA = 0.5 * (texture(texture0, fragTexCoord.xy + dir * (1.0 / 3.0 - 0.5)).xyz + texture(texture0, fragTexCoord.xy + dir * (2.0 / 3.0 - 0.5)).xyz);
    mediump vec3 rgbB = rgbA * 0.5 + 0.25 * (texture(texture0, fragTexCoord.xy + dir * -0.5).xyz + texture(texture0, fragTexCoord.xy + dir * 0.5).xyz);

    mediump float lumaB = dot(rgbB, luma);

    if ((lumaB < lumaMin) || (lumaB > lumaMax))
    {
        fragColor = vec4(rgbA, 1.0);
    }
    else
    {
        fragColor = vec4(rgbB, 1.0);
    }
}
