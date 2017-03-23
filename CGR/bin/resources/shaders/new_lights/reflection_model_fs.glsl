#version 450

/// Contains the properties of the material to be applied to the current fragment.
struct Material
{
    float   roughness;      //!< Effects the distribution of specular light over the surface.
    float   reflectance;    //!< Effects the fresnel effect of dieletric surfaces.
    float   conductivity;   //!< Conductive surfaces absorb incoming light, causing them to be fully specular.
    float   transparency;   //!< How transparent the surface is.
    
    vec3    albedo;         //!< The base colour of the material.
    vec3    normalMap;      //!< The normal map of the material.
} material;


// Globals.
const float pi              = 3.14159;
const float maxShininess    = 1024.f;


// Externals.
Material fetchMaterialProperties (const in vec2 uvCoordinates, const in int materialID);


// Forward declarations.
vec3 lambertDiffuse (const in float lDotN);
vec3 blinnPhongSpecular (const in vec3 l, const in vec3 n, const in vec3 v);

vec3 disneyDiffuse (const in float lDotN, const in float vDotN, const in float hDotV);
vec3 microfacetSpecular (const in vec3 l, const in vec3 n, const in vec3 h,
    const in float lDotN, const in float vDotN, const in float hDotV);
    
vec3 fresnelReflectance (const in vec3 albedo, const in float lDotH);
float geometricAttenuation (const in float dotProduct);
float distribution (const in float hDotN);

vec3 halfVector (const in vec3 l, const in vec3 v);


/**
    Sets the global instance of the material data used by reflection models.
*/
void setFragmentMaterial (const in vec2 uvCoordinates, const in int materialID)
{
    // Set our global material to the value returned by the material fetcher.
    material = fetchMaterialProperties (uvCoordinates, materialID);
}


/**
    Calculates the diffuse and specular component of a light with the given parameters.
    
    Params:
        l = The surface to light direction.
        n = The surface normal of the fragment.
        v = The surface to view/eye direction.
        e = The intensity of the light.
*/
vec3 calculateReflectance (const in vec3 l, const in vec3 n, const in vec3 v, const in vec3 e)
{
    // Determine whether we can actually light the surface.
    const float lDotN = max (dot (l, n), 0.0);

    if (lDotN == 0.0)
    {
        return vec3 (0.0);
    }

    // Support physically based and non-physically based shading techniques.
    #ifdef PHYSICALLY_BASED_SHADING
        
        // Conductive surfaces absorb light so no diffuse reflection occurs.
        const float diffuseContribution = 1.0 - material.conductivity;

        // We need the half vector for reflectance calculations.
        const vec3  h       = halfVector (l, v);
        const float hDotV   = dot (h, v);
        const float vDotN   = max (dot (v, n), 0.0001);

        // Calculate and scale diffuse and specular reflectance.
        const vec3 diffuse = diffuseContribution > 0.0 ? 
            disneyDiffuse (lDotN, vDotN, hDotV) * diffuseContribution : 
            vec3 (0.0);

        const vec3 specular = microfacetSpecular (l, n, h, lDotN, vDotN, hDotV);
        return e * (diffuse + specular);

    #else
        
        return e * (lambertDiffuse (lDotN) + blinnPhongSpecular (l, n, v));

    #endif
}


/**
    The most basic diffuse lighting model. Lambertian diffuse causes a relatively uniform reflectance by disregarding
    surface roughness.
*/
vec3 lambertDiffuse (const in float lDotN)
{
    return material.albedo * lDotN;
}


/**
    An inexpensive specular model which attempts to approximate shininess and specular colour from PBS parameters.
*/
vec3 blinnPhongSpecular (const in vec3 l, const in vec3 n, const in vec3 v)
{
    // First we need to interpret PBS parameters for shading.
    const vec3  albedo          = material.albedo;
    const float luminosity      = albedo.r * 0.2126f + albedo.g * 0.7151f + albedo.b * 0.0722f;
    const vec3  specularColour  = mix (vec3 (material.reflectance), vec3 (luminosity), material.conductivity);

    const float shininess       = ((2.0 / pow (material.roughness, 2.0)) - 2.0) * 4.0;

    // Using the half vector we can calculate the specularity of a surface.
    return shininess > 0.0 ? 
        specularColour * pow (max (dot (halfVector (l, v), n), 0.0), shininess) 
        : vec3 (0.0);
}


/**
    A state-of-the-art diffuse model from Disney as presented here (slide 17): 
    http://blog.selfshadow.com/publications/s2016-shading-course/hoffman/s2016_pbs_recent_advances_v2.pdf
*/
vec3 disneyDiffuse (const in float lDotN, const in float vDotN, const in float hDotV)
{
    // The base colour is simply the albedo.
    const vec3 baseColour = material.albedo;

    // Calculate fresnel weightings FL and FV.
    const float fresnelL = pow (1.0 - lDotN, 5.0);
    const float fresnelV = pow (1.0 - vDotN, 5.0);

    // Now we need a retro-reflection weighting RR.
    const float roughReflection = 2.0 * material.roughness * pow (hDotV, 2.0);

    // We can start putting the formula together now. 
    // fLambert = baseColour / PI.
    const vec3 lambert = baseColour / pi;

    // fretro-reflection: fLambert * RR * (FL + FV + FL * FV * (RR - 1)).
    const vec3 retroReflection = lambert * roughReflection * 
        (fresnelL + fresnelV + fresnelL * fresnelV * (roughReflection - 1.0));
    
    // fd = fLambert * (1 - 0.5 * FL) * (1 - 0.5 * FV) + fretro-reflection.
    return lambert * (1.0 - 0.5 * fresnelL) * (1.0 - 0.5 * fresnelV) + retroReflection;
}


/**
    Using a similar structure to the Cook-Torrance model, calculates the specular lighting of the fragment using
    fresnel, geometry and distribution components:
    http://blog.selfshadow.com/publications/s2016-shading-course/hoffman/s2016_pbs_recent_advances_v2.pdf
*/
vec3 microfacetSpecular (const in vec3 l, const in vec3 n, const in vec3 h,
    const in float lDotN, const in float vDotN, const in float hDotV)
{
    // Conductive materials reflect using their albedo, everything else reflects using white.
    const vec3 albedo = mix (vec3 (material.reflectance), material.albedo, material.conductivity);

    // Perform the required dot products.
    const float lDotH = dot (l, h);
    const float hDotN = dot (h, n);

    // Calculate the three attenuation components.
    const vec3  f = fresnelReflectance (albedo, lDotH);
    const float g = geometricAttenuation (lDotN) * geometricAttenuation (vDotN);
    const float d = distribution (hDotN);

    // Calculate the denominator.
    const float denominator = 4.0 * lDotN * vDotN;
    
    // Return the specular effect.
    return (f * g * d) / denominator;
}


/**
    Calcualtes the fresnel effect for specular lighting based on Schlick's approximation.
*/
vec3 fresnelReflectance (const in vec3 albedo, const in float lDotH)
{
    // F(0) = F0 + (1 - F0) * pow (1 - cos(0), 5).
    return albedo + (1.0 - albedo) * pow (1.0 - lDotH, 5.0);
}


/**
    Calculates an attenuation factor representing self-shadowing based on the Smith function.
    http://jcgt.org/published/0003/02/03/
    http://blog.selfshadow.com/publications/s2013-shading-course/rad/s2013_pbs_rad_notes.pdf
*/
float geometricAttenuation (const in float dotProduct)
{
    const float roughSqr    = material.roughness * material.roughness;
    const float dotSqr      = dotProduct * dotProduct;

    const float numerator   = 2.0 * dotProduct;
    const float denominator = dotProduct + sqrt (roughSqr + (1.0 - roughSqr) * dotSqr);

    return numerator / denominator;
}


/**
    Calculate a distribution attenuation factor using either GGX, Blinn-Phong or Beckmann:
    http://blog.selfshadow.com/publications/s2013-shading-course/rad/s2013_pbs_rad_notes.pdf
    http://blog.selfshadow.com/publications/s2013-shading-course/lazarov/s2013_pbs_black_ops_2_notes.pdf
*/
float distribution (const in float hDotN)
{
    // Values required for multiple distribution functions.
    const float hDotNSqr = hDotN * hDotN;
    const float roughSqr = material.roughness * material.roughness;

    // GGX.
    const float ggxNumerator    = roughSqr;
    const float ggxDenominator  = pi * pow (hDotNSqr * (roughSqr - 1.0) + 1.0, 2.0);

    return ggxNumerator / ggxDenominator;

    // Blinn-Phong.
    //return ((material.roughness + 2.0) / (pi * 2.0)) * pow (hDotN, material.roughness * maxShininess);

    // Beckmann.
    /*const float tanNumerator    = 1.0 - hDotNSqr;
    const float tanDenominator  = hDotNSqr * roughSqr;
    const float tangent         = tanNumerator / tanDenominator;

    const float exponential = exp (-tangent);
    const float denominator = pi * roughSqr * (hDotNSqr * hDotNSqr);
    return exponential / denominator;*/
}


/**
    Calculates the vector half way between the given surface-to-light and surface-to-viewer directions.
*/
vec3 halfVector (const in vec3 l, const in vec3 v)
{
    return normalize (l + v);
}