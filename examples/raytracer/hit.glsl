hitAttributeEXT vec2 attribs;

// Interpolate the texture coordinate of the hit point from the triangle's vertices
vec2 hitTextureCoord()
{
    vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

    vec2 textureCoord = vec2(0.0);
    for (int i = 0; i < 3; i++) {
        uint index = indices.data[3 * gl_PrimitiveID + i];
        textureCoord += barycentrics[i] * vec2(vertices.data[5 * index + 3], vertices.data[5 * index + 4]);
    }
    return textureCoord;
}
