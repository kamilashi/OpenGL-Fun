
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
        vec3 ambientColor = mix(vec3(0.17, 0.1, 0.7), vec3(0.2, 0.4, 1.0), (max(dotPr, 0.0)));
        color = ambientColor;
    }
    
    vec3 diffuse = multiplier * color;

    FragColor = vec4( diffuse * uMainColor, 1.0);