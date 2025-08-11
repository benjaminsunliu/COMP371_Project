#version 330 core

out vec4 FragColor;

in vec3 vertexColor;
in vec2 vertexUV;
in vec3 vertexNormal;
in vec3 crntPos;

uniform sampler2D textureSampler;
uniform vec3 camPos;

// === Sun and lamp lighting ===
#define MAX_LAMPS 16
uniform int lampCount;
uniform vec3 lampPos[MAX_LAMPS];
uniform vec3 lampColor;
uniform vec3 sunDir;
uniform vec3 sunColor;

// ==== Existing headlight spotlights ====
struct SpotLight {
    vec3 position;
    vec3 direction;
    float innerCut;
    float outerCut;
    float constant;
    float linear;
    float quadratic;
    vec3 color;
};

uniform bool  uUseHeadlights;
uniform SpotLight uHL[2];
// ======================================

vec3 spotContrib(SpotLight L, vec3 N, vec3 P, vec3 V, vec3 albedo)
{
    vec3 Ldir = normalize(L.position - P);
    float theta   = dot(Ldir, normalize(-L.direction));
    float epsilon = max(L.innerCut - L.outerCut, 1e-4);
    float spot    = clamp((theta - L.outerCut) / epsilon, 0.0, 1.0);

    float dist  = length(L.position - P);
    float atten = 1.0 / (L.constant + L.linear * dist + L.quadratic * dist * dist);

    float NdotL = max(dot(N, Ldir), 0.0);
    vec3 diffuse = albedo * NdotL;

    vec3 H = normalize(Ldir + V);
    float spec = pow(max(dot(N, H), 0.0), 32.0);

    return (diffuse + 0.2*spec) * L.color * atten * spot;
}

void main()
{
    vec3 albedo = texture(textureSampler, vertexUV).rgb;
    vec3 N = normalize(vertexNormal);
    vec3 V = normalize(camPos - crntPos);

    // Directional sunlight with simple shadowing
    vec3 Lsun = normalize(-sunDir);
    float diffSun = max(dot(N, Lsun), 0.0);
    vec3 Rsun = reflect(-Lsun, N);
    float specSun = pow(max(dot(V, Rsun), 0.0), 16.0);
    float shadow = diffSun > 0.0 ? 1.0 : 0.2;
    vec3 lighting = (0.1 * sunColor) + shadow * sunColor * (diffSun + 0.5 * specSun);

    // Lamps as point lights
    for (int i = 0; i < lampCount; ++i) {
        vec3 Ldir = normalize(lampPos[i] - crntPos);
        float dist = length(lampPos[i] - crntPos);
        float atten = 1.0 / (1.0 + 0.09 * dist + 0.032 * dist * dist);
        float diff = max(dot(N, Ldir), 0.0);
        vec3 Rlamp = reflect(-Ldir, N);
        float spec = pow(max(dot(V, Rlamp), 0.0), 32.0);
        lighting += lampColor * atten * (diff + 0.5 * spec);
    }

    vec3 color = albedo * lighting;

    // ==== Headlight spot contributions ====
    if (uUseHeadlights) {
        color += spotContrib(uHL[0], N, crntPos, V, albedo);
        color += spotContrib(uHL[1], N, crntPos, V, albedo);
    }
    // =====================================

    FragColor = vec4(color, 1.0);
}
