
// dither

    float dotPr = dot(norm, uMainLightDirection);

   // float random = GradientNoise01(genSeed(WorldPos), 1000.0); 
    float random = GradientNoise01(gl_FragCoord.xy / vec2(10000, 10000), 5000.0); 
    

    float dotScaled = dotPr * 2;

    float ditherFactor = 1.0;
    // total light                 // total shade
    if(random > (1.0 + dotScaled)  ||  random > (1.0 - dotScaled))
    {
        ditherFactor = 0.0;
    }
    

    float maxBias  = 0.01;   
    float slopeScale = 0.01;    

    float bias = max(slopeScale * (1.0 - max(-dotPr, 0)), maxBias);
    vec2 castShadowAndDepth = getShadow(FragPosLightSpace, shadowMap, bias); 
    
    float castShadow = castShadowAndDepth.x;
    float depth = 0.0; // castShadowAndDepth.y;

    vec3 ditherColor = vec3(0.0, 0.00, 0.0); // vec3(0.3, 0.05, 0.1);//
    vec3 color = uMainLightColor;

    if(dotScaled > 0.0 || castShadow > 0) // shaded areas
    {
        vec3 ambientColor = mix(vec3(0.27, 0.3, 0.8), vec3(0.3, 0.5, 1.0), (max(dotScaled, 0.0)));
        color = ambientColor;
    }
    else  // lit areas
    {
        vec3 colorBlend = mix(uMainLightColor, uMainLightColor + vec3(0.1, 0.1, 0.1), (max(-dotScaled, 0.0)));
        color = colorBlend;
    }

    vec3 diffuse = mix(color, ditherColor, ditherFactor);

    FragColor = vec4( (diffuse * (1 - depth)) * uMainColor, 1.0);