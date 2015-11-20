#version 330 core
in vec2 theTextureCoord;
in vec3 theNormal;
in vec3 worldPosition;	//of a current fragment
in vec4 lightSpacePosition;	//of a current fragment
in mat3 TBN;	//for normal maps

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 brightRegions;

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

//Directional light
struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirLight dirLight;  

//Point light
struct PointLight{
	vec3 position;  
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	
    float constant;
    float linear;
    float quadratic;

	float radius;
};
uniform int activeLights = 0;
uniform PointLight pointLights[50];

//Spotlight
struct SpotLight{
	vec3 position;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	
    float constant;
    float linear;
    float quadratic;
  
    vec3  direction;
    float cutOff;
	float outerCutOff;
};
uniform SpotLight spotlight;

struct LightComponents
{
	vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

uniform vec3 viewerPos;

uniform sampler2D shadowMap;	//for directional light shadow

uniform samplerCube pointShadowMap;
uniform float pointLightFarPlane;
uniform bool pointLightShadows;

LightComponents calculateLightComponents(vec3 lightDirection, vec3 lAmbient,  vec3 lDiffuse, vec3 lSpecular, vec4 materialColor, vec3 materialSpecularity, vec3 normal)
{
	//Ambient component
	vec4 ambient = materialColor * vec4(lAmbient, 1.0);
	
	//Diffuse Component 
	float diff = max(dot(normal, lightDirection), 0.0);	//no negative values
	vec4 diffuse = diff * materialColor * vec4(lDiffuse, 1.0f);

	//Specular component	
	vec3 viewDir = normalize(viewerPos - worldPosition);
	vec3 halfwayDirection = normalize(lightDirection + viewDir);
	float spec = pow(max(dot(normal, halfwayDirection), 0.0), material.shininess);
	vec4 specular = vec4(spec  * materialSpecularity * lSpecular, 1.0f);  

	LightComponents lc;

	lc.ambient = ambient;
	lc.diffuse = diffuse;
	lc.specular = specular;

	return lc;
}

vec4 calculateDirectionalLight( DirLight light, vec4 materialColor, vec3 materialSpecularity, vec3 normal, float shadow )
{
	//Directional Light direction and fragment(vertex) normal
	vec3 lightDirection = normalize(-light.direction);
	
	LightComponents lc = calculateLightComponents(lightDirection, light.ambient, light.diffuse, light.specular, materialColor, materialSpecularity, normal);
	
	vec4 fragmentColor = lc.ambient + vec4((1.0 - shadow) * lc.diffuse.rgb, lc.diffuse.a) + vec4((1.0 - shadow) * lc.specular.rgb, lc.specular.a);

	return fragmentColor;
}

vec4 calculatePointLight( PointLight light, vec4 materialColor, vec3 materialSpecularity, vec3 normal, float shadow )
{
	//Calculate attenuation of a point light
	float distance    = length(light.position - worldPosition);
	float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));   

	//Direction from the fragment to the light source, calculated from position
	vec3 lightDirection = normalize( light.position - worldPosition );
	
	LightComponents lc = calculateLightComponents(lightDirection, light.ambient, light.diffuse, light.specular, materialColor, materialSpecularity, normal);
	
    lc.ambient  *= attenuation;  
    lc.diffuse  *= attenuation;
    lc.specular *= attenuation;   
     
	vec4 fragmentColor = lc.ambient + vec4((1.0 - shadow) * lc.diffuse.rgb, lc.diffuse.a) + vec4((1.0 - shadow) * lc.specular.rgb, lc.specular.a);
	
	return fragmentColor;
}

vec4 calculateSpotLight( SpotLight light, vec4 materialColor, vec3 materialSpecularity, vec3 normal )
{
	//Direction from the fragment to the light source, calculated from position
	vec3 lightDirection = normalize( light.position - worldPosition );
	
	//angle between the light direction, and direction from the current fragment toward the light
	float theta = dot(lightDirection, normalize(-light.direction));
    
	//Inner-outer cone intensity of light
	float epsilon   = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);  

	//Calculate attenuation of a point light
	float distance    = length(light.position - worldPosition);
	float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));   
	
	LightComponents lc = calculateLightComponents(lightDirection, light.ambient, light.diffuse, light.specular, materialColor, materialSpecularity, normal);
	
    lc.ambient  *= attenuation * intensity;  
    lc.diffuse  *= attenuation * intensity;
    lc.specular *= attenuation * intensity;   
     
	vec4 fragmentColor = lc.ambient + lc.diffuse + lc.specular;
	
	return fragmentColor;
}

float calculateShadow(sampler2D theShadowmap, vec4 fragmentLightSpacePosition, vec3 normal, vec3 lightDir)
{
	// perform perspective divide
    vec3 projectedCoords = fragmentLightSpacePosition.xyz / fragmentLightSpacePosition.w;
	//to the range [0,1]
	projectedCoords = projectedCoords * 0.5 + 0.5; 

	float closestDepth = texture(theShadowmap, projectedCoords.xy).r; 
	float currentDepth = projectedCoords.z;  
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005); 

	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(theShadowmap, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(theShadowmap, projectedCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
		}    
	}
	shadow /= 9.0;

	return shadow;
}

uniform vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
); 
float calculatePointShadow(int lightId, samplerCube theShadowmap, vec3 fragmentPosition, float pLightFarPlane)
{
	//only calculating the shadow for the first light, because only one shadow cubemap is used and prepared
	//and only when point light rendering is enabled
	if ( !pointLightShadows || lightId > 0 )
		return 0.0f;

	// Get vector between fragment position and light position
	// Use the light to fragment vector to sample from the depth map  
	vec3 fragToLight = fragmentPosition - pointLights[lightId].position; 

	// Now get current linear depth as the length between the fragment and light position
	float currentDepth = length(fragToLight);  

	float shadow = 0.0;
	float bias = 0.15;
	int samples = 20;
	float diskRadius = 0.05;
	for(int i = 0; i < samples; ++i)
	{
		float closestDepth = texture(theShadowmap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
		// It is currently in linear range between [0,1]. Re-transform back to original value
		closestDepth *= pLightFarPlane;   // Undo mapping [0;1]
		if(currentDepth - bias > closestDepth)
			shadow += 1.0;
	}
	shadow /= float(samples);  

	return shadow;
}

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
	vec2 textureCoordinates = theTextureCoord;
	if  ( material.parallaxAvailable )
	{
		textureCoordinates = parallaxMapping(theTextureCoord, normalize(viewerPos - worldPosition), material.depthMap );
		if(textureCoordinates.x > 1.0 || textureCoordinates.y > 1.0 || textureCoordinates.x < 0.0 || textureCoordinates.y < 0.0)
			 discard;
	}

	vec3 normal = normalize(theNormal);
	if  ( material.normalAvailable )	//read normals from a texture, if available
	{
		// Obtain normal from normal map in range [0,1]
		normal = texture(material.normalMap, textureCoordinates).rgb;
		// Transform normal vector to range [-1,1]
		normal = normalize(normal * 2.0 - 1.0);  
		//TODO: Might want to optimize it, and transform all the lights, viever positions, direction to TBN space in the vertex shader
		//storing the uniforms in the vertex shader, and transferring them as inputs to the fragment
		normal = normalize(TBN * normal);  
	}

	//read material colors (diffuse and ambient) from the texture
	vec4 materialColor = texture( material.diffuse, textureCoordinates );
	if(materialColor.a == 0. && material.transparent == 1)
        discard;
	vec3 materialSpecularity = vec3(texture(material.specular, textureCoordinates));
	
	float shadow = calculateShadow(shadowMap, lightSpacePosition, normal, normalize(-dirLight.direction));  //dir light shadow	

	vec4 directionalLightFragColor = calculateDirectionalLight( dirLight, materialColor, materialSpecularity, normal, shadow );

	vec4 pointLightFragColor;
	for(int i = 0; i < activeLights; i++)
	{
		//float distance = length(pointLights[i].position - worldPosition);
        //if(distance < pointLights[i].radius)
        //{
			float pointShadow = calculatePointShadow( i, pointShadowMap, worldPosition, pointLightFarPlane );
			pointLightFragColor += calculatePointLight( pointLights[i], materialColor, materialSpecularity, normal, pointShadow );	
		//}
	}

	vec4 spotLightFragColor = calculateSpotLight( spotlight, materialColor, materialSpecularity, normal );
	
    color = directionalLightFragColor + pointLightFragColor + spotLightFragColor;
	
	// Check whether fragment output is higher than threshold, if so output as brightness color
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        brightRegions = vec4(color.rgb, 1.0);
}
