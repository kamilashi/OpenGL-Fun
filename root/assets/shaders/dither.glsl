
// dither

    float dotPr = dot(norm, uMainLightDirection);

    vec2 seed = genSeed(WorldPos, TexCoord);
    
    float random = GradientNoise01(seed, 10.0); 

    float multiplier = 0.0;
    
    // total light             // total shade
    if(random > (1.0 + dotPr)  ||  random > (1.0 - dotPr))
    {
        multiplier = 1.0;
    }

    vec3 color = uMainLightColor;
    if(dotPr > 0)
    {
        vec3 ambientColor = mix(vec3(0.27, 0.3, 0.8), vec3(0.3, 0.5, 1.0), (max(dotPr, 0.0)));
        color = ambientColor;
    }
    
    vec3 diffuse = multiplier * color;

    
    float shadow = 0.0;
    float depth = 0.0;

#ifndef SHADOW_DEPTH_PASS
    float bias = max(0.05 * (1.0 - dotPr), 0.005);
    vec2 shadowAndDepth = getShadow(FragPosLightSpace, shadowMap, bias); 
    
    shadow = shadowAndDepth.x;
    depth = shadowAndDepth.y;
#endif

    FragColor = vec4( (1 - shadow) * (diffuse * (1 - depth)) * uMainColor, 1.0);