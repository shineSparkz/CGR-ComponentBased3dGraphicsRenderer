#version 450

/// A universal light which casts light on all objects.
struct DirectionalLight
{
    vec3 direction; //!< The direction of the light.
    vec3 intensity; //!< The colour and brightness of the light.
};

/// An area light which uniformly distributes light within a range.
struct PointLight
{
    vec3    position;   //!< The world location of the light.
    vec3    intensity;  //!< The colour and brightness of the light.
	
	float   range;      //!< The maximum range of the light.
    float   aConstant;  //!< The constant co-efficient for the attenuation formula.
    float   aLinear;    //!< The linear co-efficient for the attenuation formula.
    float   aQuadratic; //!< The quadratic co-efficient for the attenuation formula.
};

/// A constricted area light which distributes light like a cone.
struct Spotlight
{
    vec3    position;       //!< The world location of the light.
	vec3    direction;      //!< The direction of the light.
	vec3    intensity;      //!< The colour and brightness of the light.
	
    float   coneAngle;      //!< The angle of the cone in degrees.
    float   range;          //!< The maximum range of the light.
    float   concentration;  //!< Effects how focused the light is and how it distributes away from the centre.
    
    float   aConstant;      //!< The constant co-efficient for the attenuation formula.
    float   aLinear;        //!< The linear co-efficient for the attenuation formula.
    float   aQuadratic;     //!< The quadratic co-efficient for the attenuation formula.
    int     viewIndex;      //!< Indicates which view transform to use, if this is -1 then the light doesn't cast a shadow.
};

layout (std140) uniform DirectionalLights
{
    #define DirectionalLightsMax 25
    
    uint                count;                          //!< How many lights exist in the scene.
    DirectionalLight    lights[DirectionalLightsMax];   //!< A collection of light data.
} directionalLights;

layout (std140) uniform PointLights
{
    #define PointLightsMax 25
    
    uint        count;                  //!< How many lights exist in the scene.
    PointLight  lights[PointLightsMax]; //!< A collection of light data.
} pointLights;

layout (std140) uniform Spotlights
{
    #define SpotlightsMax 25
    
    uint        count;                  //!< How many lights exist in the scene.
    Spotlight   lights[SpotlightsMax];  //!< A collection of light data.
} spotlights;

layout (std140) uniform LightViews
{
    #define LightViewMax 25
    
    uint    count;                      //!< How many transforms exist.
    mat4    transforms[LightViewMax];   //!< A collection of light view transforms.
} lightViews;

layout (std140) uniform Scene
{
    mat4    projection;     //!< The projection transform which establishes the perspective of the vertex.
    mat4    view;           //!< The view transform representing where the camera is looking.

    vec3    camera;         //!< Contains the position of the camera in world space.
    int     shadowMapRes;   //!< How many pixels wide/tall the shadow maps are.
    vec3    ambience;       //!< The ambient lighting in the scene.
} scene;


// Uniforms.
uniform sampler2DArrayShadow shadowMaps; //!< Contains shadow maps for every spotlight in the scene.


// Externals.
vec3 calculateReflectance (const in vec3 l, const in vec3 n, const in vec3 v, const in vec3 e);


/**
    Calculates a shadowing factor to apply to the light at the current pixel.
    http://ogldev.atspace.co.uk/www/tutorial42/tutorial42.html
*/
float spotlightShadow (const in vec3 position, const in int viewIndex)
{
    // Determine the position in light-space.
    const vec4 lightSpace   = lightViews.transforms[viewIndex] * vec4 (position, 1.0);
    const vec3 projection   = lightSpace.xyz / lightSpace.w;
    const vec3 samplePoint  = vec3 (0.5 * projection.x + 0.5, 0.5 * projection.y + 0.5, viewIndex);

    // Now we can determine the depth of the surface
    const float bias    = 0.00001;
    const float depth   = 0.5 * projection.z + 0.5 - bias;
    
    // Finally we can use percentage-closer filtering to create a shadow gradient. We use 16 sample points.
    const float offset  = 1.0 / scene.shadowMapRes;
    const int   samples = 16;
    const int   start   = -(samples / 4 - 2);
    const int   end     = -start;

    float edgeFiltering = 0.0;
    for (int y = start; y <= end; y++) 
    {
        for (int x = start; x <= end; x++) 
        {
            const vec3 offsets  = vec3 (x * offset, y * offset, 0.0);
            vec4 shadowSample   = vec4 (samplePoint + offsets, depth);
            edgeFiltering       += texture (shadowMaps, shadowSample);
        }
    }

    const float shadowStrength = 0.2;
    return shadowStrength + (edgeFiltering / (samples * 2));
}


/**
    Calculates the lighting contribution of a directional light at the given index.
*/
vec3 directionalLightContribution (const in uint index, const in vec3 normal, const in vec3 view)
{
    // Directional lights don't need attenuation.
    const DirectionalLight light = directionalLights.lights[index];
    const vec3 l = -light.direction;
    const vec3 E = light.intensity;

    return calculateReflectance (l, normal, view, E);
}


/**
    Calculates the lighting contribution of a point light at the given index.
*/
vec3 pointLightContribution (const in uint index, const in vec3 position, const in vec3 normal, const in vec3 view)
{
    // Point lights use uniform attenuation.
    const PointLight light = pointLights.lights[index];

    // We'll need the distance and direction from the light to the surface for attenuation.
    const vec3  bigL    = light.position - position;
    const float dist    = length (bigL);
    const vec3  l       = bigL / dist;

    // Point light attenuation formula is: 1 / (Kc + Kl * d + Kq * d * d).
    const float attenuation = light.range >= dist ? 
        1.0 / (light.aConstant + light.aLinear * dist + light.aQuadratic * dist * dist) :
        0.0;

    // Scale the intensity accordingly.
    const vec3 E = light.intensity * attenuation;
    
    return calculateReflectance (l, normal, view, E);
}


/**
    Calculates the lighting contribution of a spotlight at the given index.
*/
vec3 spotlightContribution (const in uint index, const in vec3 position, const in vec3 normal, const in vec3 view)
{
    // Spotlights require a special luminance attenuation and cone attenuation.
    const Spotlight light = spotlights.lights[index];

    // We'll need the distance and direction from the light to the surface for attenuation.
    const vec3  bigL    = light.position - position;
    const float dist    = length (bigL);
    const vec3  l       = bigL / dist;
    const vec3  R       = light.direction;
    const float p       = light.concentration;

    // Luminance attenuation formula is: pow (max {-R.l, 0}), p) / (Kc + kl * d + Kq * d * d).
    const float luminance = light.range >= dist ? 
        pow (max (dot (-R, l), 0.0), p) / (light.aConstant + light.aLinear * dist + light.aQuadratic * dist * dist) :
        0.0;

    // Cone attenuation is: acos ((-l.R)) > angle / 2. Attenuate using smoothstep.
    const float lightAngle  = degrees (acos (max (dot (-l, R), 0.0)));
    const float halfAngle   = light.coneAngle / 2.0;
    const float coneCutOff  = lightAngle <= halfAngle ? smoothstep (1.0, 0.75, lightAngle / halfAngle) : 0.0;

    // We need some shadow attenuation.
    const float shadowing = light.viewIndex > -1 ? spotlightShadow (position, light.viewIndex) : 1.0;

    // Scale the intensity accordingly.
    const vec3 E = light.intensity * luminance * coneCutOff * shadowing;
    
    return calculateReflectance (l, normal, view, E);
}


/**
    Calculates the lighting contribution of every directional light in the scene.
*/
vec3 directionalLightContributions (const in vec3 normal, const in vec3 view)
{
    vec3 lighting = vec3 (0.0);

    for (uint i = 0; i < directionalLights.count; ++i)
    {
        lighting += directionalLightContribution (i, normal, view);
    }

    return lighting;
}


/**
    Calculates the lighting contribution of every point light in the scene.
*/
vec3 pointLightContributions (const in vec3 position, const in vec3 normal, const in vec3 view)
{
    vec3 lighting = vec3 (0.0);

    for (uint i = 0; i < pointLights.count; ++i)
    {
        lighting += pointLightContribution (i, position, normal, view);
    }

    return lighting;
}


/**
    Calculates the lighting contribution of every spotlight in the scene.
*/
vec3 spotlightContributions (const in vec3 position, const in vec3 normal, const in vec3 view)
{
    vec3 lighting = vec3 (0.0);

    for (uint i = 0; i < spotlights.count; ++i)
    {
        lighting += spotlightContribution (i, position, normal, view);
    }

    return lighting;
}