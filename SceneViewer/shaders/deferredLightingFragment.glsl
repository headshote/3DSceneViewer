#version 330 core
in vec2 TexCoords;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 brightRegions;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gTangent;
uniform sampler2D gBitangent;
uniform sampler2D gAlbedoSpec;
uniform sampler2D glightSpacePosition;

uniform float shininess = 96.0f;
uniform int transparent;

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

LightComponents calculateLightComponents(vec3 lightDirection, vec3 lAmbient,  vec3 lDiffuse, vec3 lSpecular, vec4 materialColor, vec3 materialSpecularity,
	 vec3 normal, float materialShininess, vec3 worldPosition)
{
	//Ambient component
	vec4 ambient = materialColor * vec4(lAmbient, 1.0);
	
	//Diffuse Component 
	float diff = max(dot(normal, lightDirection), 0.0);	//no negative values
	vec4 diffuse = diff * materialColor * vec4(lDiffuse, 1.0f);

	//Specular component	
	vec3 viewDir = normalize(viewerPos - worldPosition);
	vec3 halfwayDirection = normalize(lightDirection + viewDir);
	float spec = pow(max(dot(normal, halfwayDirection), 0.0), materialShininess);
	vec4 specular = vec4(spec  * materialSpecularity * lSpecular, 1.0f);  

	LightComponents lc;

	lc.ambient = ambient;
	lc.diffuse = diffuse;
	lc.specular = specular;

	return lc;
}

vec4 calculateDirectionalLight( DirLight light, vec4 materialColor, vec3 materialSpecularity, vec3 normal, float shadow, float materialShininess, vec3 worldPosition )
{
	//Directional Light direction and fragment(vertex) normal
	vec3 lightDirection = normalize(-light.direction);
	
	LightComponents lc = calculateLightComponents(lightDirection, light.ambient, light.diffuse, light.specular, materialColor, materialSpecularity, normal, materialShininess, worldPosition);
	
	vec4 fragmentColor = lc.ambient + vec4((1.0 - shadow) * lc.diffuse.rgb, lc.diffuse.a) + vec4((1.0 - shadow) * lc.specular.rgb, lc.specular.a);

	return fragmentColor;
}

vec4 calculatePointLight( PointLight light, vec4 materialColor, vec3 materialSpecularity, vec3 normal, float shadow, float materialShininess, vec3 worldPosition )
{
	//Calculate attenuation of a point light
	float distance    = length(light.position - worldPosition);
	float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));   

	//Direction from the fragment to the light source, calculated from position
	vec3 lightDirection = normalize( light.position - worldPosition );
	
	LightComponents lc = calculateLightComponents(lightDirection, light.ambient, light.diffuse, light.specular, materialColor, materialSpecularity, normal, materialShininess, worldPosition);
	
    lc.ambient  *= attenuation;  
    lc.diffuse  *= attenuation;
    lc.specular *= attenuation;   
     
	vec4 fragmentColor = lc.ambient + vec4((1.0 - shadow) * lc.diffuse.rgb, lc.diffuse.a) + vec4((1.0 - shadow) * lc.specular.rgb, lc.specular.a);
	
	return fragmentColor;
}

vec4 calculateSpotLight( SpotLight light, vec4 materialColor, vec3 materialSpecularity, vec3 normal, float materialShininess, vec3 worldPosition )
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
	
	LightComponents lc = calculateLightComponents(lightDirection, light.ambient, light.diffuse, light.specular, materialColor, materialSpecularity, normal, materialShininess, worldPosition);
	
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

void main()
{             
    // Retrieve data from G-buffer
    vec3 worldPosition = texture(gPosition, TexCoords).rgb;
    vec3 normal = texture(gNormal, TexCoords).rgb;
    vec3 tangent = texture(gTangent, TexCoords).rgb;
    vec3 bitangent = texture(gBitangent, TexCoords).rgb;
    vec3 diffuse = texture(gAlbedoSpec, TexCoords).rgb;
    float specular = texture(gAlbedoSpec, TexCoords).a;
	vec4 lightSpacePosition =  texture(glightSpacePosition, TexCoords);
    
	//TBN for bump maps, parallax maps
	mat3 TBN = mat3(tangent, bitangent, normal);
	normal = normalize(normal * TBN  );  

	//read material colors (diffuse and ambient) from the texture
	vec4 materialColor = vec4(diffuse, 1.0);

	vec3 materialSpecularity = vec3(specular);
	
	float shadow = calculateShadow(shadowMap, lightSpacePosition, normal, normalize(-dirLight.direction));  //dir light shadow	

	vec4 directionalLightFragColor = calculateDirectionalLight( dirLight, materialColor, materialSpecularity, normal, shadow, shininess, worldPosition );

	vec4 pointLightFragColor;
	for(int i = 0; i < activeLights; i++)
	{
		//float distance = length(pointLights[i].position - worldPosition);
        //if(distance < pointLights[i].radius)
        //{
			float pointShadow = calculatePointShadow( i, pointShadowMap, worldPosition, pointLightFarPlane );
			pointLightFragColor += calculatePointLight( pointLights[i], materialColor, materialSpecularity, normal, pointShadow, shininess, worldPosition );	
		//}
	}

	vec4 spotLightFragColor = calculateSpotLight( spotlight, materialColor, materialSpecularity, normal, shininess, worldPosition );
	
    color = directionalLightFragColor + pointLightFragColor + spotLightFragColor;
	
	// Check whether fragment output is higher than threshold, if so output as brightness color
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        brightRegions = vec4(color.rgb, 1.0);
} 