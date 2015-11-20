#version 330 core
in vec2 theTextureCoord;
in vec3 theNormal;
in vec3 worldPosition;
in vec4 lightSpacePosition;
in mat3 TBN;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gTangent;
layout (location = 3) out vec3 gBitangent;
layout (location = 4) out vec4 gAlbedoSpec;
layout (location = 5) out vec4 glightSpacePosition;

struct Material {
	sampler2D diffuse;
	sampler2D specular;
    float shininess;

	int transparent;

	//for normal maps
	sampler2D normalMap;  
	bool normalAvailable;
	
	//for parallax maps
	sampler2D depthMap;
	bool parallaxAvailable;
};   
uniform Material material;

uniform vec3 viewerPos;

vec2 parallaxMapping(vec2 texCoords, vec3 viewDir, sampler2D depthMap)
{ 
	viewDir = TBN * viewDir;

    // number of depth layers (the bigger the angle of surface viewing - the more)
    const float minLayers = 10;
    const float maxLayers = 20;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * 0.1; 
    vec2 deltaTexCoords = P / numLayers;
  
    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(depthMap, currentTexCoords).r;
      
    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(depthMap, currentTexCoords).r;  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }
    
	// get texture coordinates before collision (reverse operations)
	vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

	// get depth after and before collision for linear interpolation
	float afterDepth  = currentDepthMapValue - currentLayerDepth;
	float beforeDepth = texture(depthMap, prevTexCoords).r - currentLayerDepth + layerDepth;
 
	// interpolation of texture coordinates
	float weight = afterDepth / (afterDepth - beforeDepth);
	vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

	return finalTexCoords;  
}

void main()
{    
    // Store the fragment position vector in the first gbuffer texture
    gPosition = worldPosition;
	
	vec2 textureCoordinates = theTextureCoord;
	if  ( material.parallaxAvailable )
	{
		textureCoordinates = parallaxMapping(textureCoordinates, normalize(viewerPos - worldPosition), material.depthMap );
		if(textureCoordinates.x > 1.0 || textureCoordinates.y > 1.0 || textureCoordinates.x < 0.0 || textureCoordinates.y < 0.0)
			 discard;
	}

    // Also store the per-fragment normals into the gbuffer
    gNormal = normalize(theNormal);
	if  ( material.normalAvailable )	//read normals from a texture, if available
	{
		// Obtain normal from normal map in range [0,1]
		gNormal = texture(material.normalMap, textureCoordinates).rgb;
		// Transform normal vector to range [-1,1]
		gNormal = normalize(gNormal * 2.0 - 1.0);  
	}

	gTangent = TBN[0].xyz;
	gBitangent = TBN[1].xyz;

    // And the diffuse per-fragment color
    gAlbedoSpec.rgb = texture(material.diffuse, textureCoordinates).rgb;
	
	if(texture(material.diffuse, textureCoordinates).a == 0. && material.transparent == 1)
    	discard;

    // Store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = texture(material.specular, textureCoordinates).r;

	glightSpacePosition = lightSpacePosition;
}  