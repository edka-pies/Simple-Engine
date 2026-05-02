#version 410 core

#define MAX_LIGHTS 8

uniform vec4 eyePosition;

// lighting arrays
uniform int numLights;
uniform vec4 light_ambient[MAX_LIGHTS];
uniform vec4 light_diffuse[MAX_LIGHTS];
uniform vec4 light_specular[MAX_LIGHTS];
uniform vec3 light_position[MAX_LIGHTS];
uniform vec3 light_attenuation[MAX_LIGHTS];
uniform int light_type[MAX_LIGHTS];         // 0 = point, 1 = directional, 2 = spot
uniform vec3 light_direction[MAX_LIGHTS];
uniform float light_cutoff[MAX_LIGHTS];     // cos(innerAngle)
uniform float light_outerCutoff[MAX_LIGHTS]; // cos(outerAngle)

uniform vec4 materialAmbient;
uniform vec4 materialDiffuse;
uniform vec4 materialSpecular;
uniform int materialShininess;

uniform sampler2D diffuseTexture;
uniform bool useTexture; 
uniform float textureMixFactor = 1.0; 

uniform sampler2D specularMap;
uniform bool useSpecularMap;

uniform sampler2D shadowMap;
uniform mat4 lightSpaceMatrix;

in vec3 v_normal;
in vec2 UV_Coord;
in vec3 position;
in vec3 vecToEye;
in vec4 FragPosLightSpace;

out vec4 FragColor;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    // 1. If outside the shadow map box, it's definitely NOT in shadow
    if(projCoords.z > 1.0 || projCoords.x > 1.0 || projCoords.x < 0.0 || projCoords.y > 1.0 || projCoords.y < 0.0)
        return 0.0;

    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;

    // 2. DYNAMIC BIAS: This is the fix for the dark floor.
    // It adjusts based on the slope of the surface relative to the light.
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);  

    // 3. Simple check (start with this to see if it works, then add PCF back)
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
  
    return shadow;
}

void main()
{
    vec4 texColor = texture(diffuseTexture, UV_Coord);
    vec4 baseColor = useTexture ? texColor : materialDiffuse;
    if (useTexture) {
        baseColor = mix(materialDiffuse, texColor, clamp(textureMixFactor, 0.0, 1.0));
    }

    float specMask = 1.0;
    if (useSpecularMap) {
        specMask = texture(specularMap, UV_Coord).r;
    }

    vec3 N = normalize(v_normal);
    vec3 V = normalize(vecToEye);

    vec3 ambientSum = vec3(0.0);
    vec3 diffuseSum = vec3(0.0);
    vec3 specularSum = vec3(0.0);

    for (int i = 0; i < numLights; ++i)
    {
        vec3 L;
        float attenuation = 1.0;
        float shadow = 0.0; // NEW: Default no shadowtenuation = 1.0;

        if (light_type[i] == 0) {
            // Point light
            L = normalize(light_position[i] - position);
            float distance = length(light_position[i] - position);
            attenuation = 1.0 / (light_attenuation[i].x + light_attenuation[i].y * distance + light_attenuation[i].z * distance * distance);
        } else if (light_type[i] == 1) {
            // Directional light: direction is direction light points to.
            L = normalize(-light_direction[i]);
            attenuation = 1.0;
            shadow = ShadowCalculation(FragPosLightSpace, N, L);
        } else {
            // Spot light
            L = normalize(light_position[i] - position);
            float distance = length(light_position[i] - position);
            attenuation = 1.0 / (light_attenuation[i].x + light_attenuation[i].y * distance + light_attenuation[i].z * distance * distance);

            vec3 spotDir = normalize(light_direction[i]); // direction spot points to
            float theta = dot(normalize(-L), spotDir);
            float epsilon = light_cutoff[i] - light_outerCutoff[i];
            float intensity = 0.0;
            if (theta > light_outerCutoff[i]) {
                intensity = clamp((theta - light_outerCutoff[i]) / max(epsilon, 0.0001), 0.0, 1.0);
            } else {
                intensity = 0.0;
            }
            attenuation *= intensity;
        }

        float diff = max(dot(N, L), 0.0);
        vec3 diffuse = diff * (light_diffuse[i].xyz * materialDiffuse.xyz);

        vec3 H = normalize(L + V);
        float specAngle = max(dot(N, H), 0.0);
        float spec = pow(specAngle, float(materialShininess));
        vec3 specular = spec * (light_specular[i].xyz * materialSpecular.xyz);

        // FIXED: Your ambient calculation previously used specular variables.
        vec3 ambient = (light_ambient[i].xyz * materialAmbient.xyz);

        ambientSum += ambient;
        // NEW: Multiply diffuse and specular by (1.0 - shadow)
        diffuseSum += (1.0 - shadow) * diffuse * attenuation;
        specularSum += (1.0 - shadow) *  specular * attenuation;
    }

    vec3 color = ambientSum + diffuseSum + specularSum;
    FragColor = vec4(color * baseColor.xyz, baseColor.w);
}