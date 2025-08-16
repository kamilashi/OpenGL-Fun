
uint hash_u32(uint x) 
{         
    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    x ^= x >> 16;
    return x;
}

uint hash_u32(uvec2 v) 
{ 
    return hash_u32(v.x ^ (0x9e3779b9u + (v.y<<6) + (v.y>>2)));
}

float rand01(uvec2 seed) 
{
    return float(hash_u32(seed)) * (1.0 / 4294967296.0); 
}