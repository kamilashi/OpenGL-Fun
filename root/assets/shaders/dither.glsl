
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
    
    float castShadow = 0.0;
    float depth = 0.0;

#ifndef SHADOW_DEPTH_PASS
    float maxBias  = 0.0005;   
    float slopeScale = 0.01;    

    float bias = max(slopeScale * (1.0 - max(-dotPr, 0)), maxBias);
    vec2 castShadowAndDepth = getShadow(FragPosLightSpace, shadowMap, bias); 
    
    castShadow = castShadowAndDepth.x;
    //depth = castShadowAndDepth.y;
#endif


    vec3 color = uMainLightColor;
    if(dotPr > 0 || castShadow > 0)
    {
        vec3 ambientColor = mix(vec3(0.27, 0.3, 0.8), vec3(0.3, 0.5, 1.0), (max(dotPr, 0.0)));
        color = ambientColor;
    }
    else
    {
        vec3 colorBlend = mix(uMainLightColor, uMainLightColor + vec3(0.1, 0.1, 0.1), (max(-dotPr, 0.0)));
        color = colorBlend;
    }
    
    vec3 diffuse = multiplier * color;

    FragColor = vec4( (diffuse * (1 - depth)) * uMainColor, 1.0);