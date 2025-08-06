#version 330 core

in vec3 vertexColor;
in vec2 vertexUV;
in vec3 fragment_position;
in vec3 fragment_normal;
in vec4 fragment_position_light_space;

uniform sampler2D textureSampler;
uniform sampler2D shadow_map;

uniform bool useBlackKey;
uniform vec3 light_color;
uniform vec3 light_position;
uniform vec3 light_direction;
uniform vec3 view_position;

uniform vec3 object_color;

uniform float light_cutoff_outer;
uniform float light_cutoff_inner;
uniform float light_near_plane;
uniform float light_far_plane;

out vec4 result;

const float PI = 3.1415926535897932384626433832795;
const float shading_ambient_strength  = 0.1;
const float shading_diffuse_strength  = 0.6;
const float shading_specular_strength = 0.3;

vec3 ambient_color(vec3 light_color_arg) {
    return shading_ambient_strength * light_color_arg;
}

vec3 diffuse_color(vec3 light_color_arg, vec3 light_position_arg) {
    vec3 light_dir = normalize(light_position_arg - fragment_position);
    return shading_diffuse_strength * light_color_arg * max(dot(normalize(fragment_normal), light_dir), 0.0f);
}

vec3 specular_color(vec3 light_color_arg, vec3 light_position_arg) {
    vec3 light_dir = normalize(light_position_arg - fragment_position);
    vec3 view_dir = normalize(view_position - fragment_position);
    vec3 reflect_dir = reflect(-light_dir, normalize(fragment_normal));
    return shading_specular_strength * light_color_arg * pow(max(dot(reflect_dir, view_dir), 0.0f), 32);
}

float shadow_scalar() {
    vec3 ndc = fragment_position_light_space.xyz / fragment_position_light_space.w;
    ndc = ndc * 0.5 + 0.5;

    float closest_depth = texture(shadow_map, ndc.xy).r;
    float current_depth = ndc.z;
    float bias = 0.003;

    return (current_depth - bias) < closest_depth ? 1.0 : 0.0;
}
 
float spotlight_scalar() {
    float theta = dot(normalize(fragment_position - light_position), light_direction);
    
    if(theta > light_cutoff_inner) {
        return 1.0;
    } else if(theta > light_cutoff_outer) {
        return (1.0 - cos(PI * (theta - light_cutoff_outer) / (light_cutoff_inner - light_cutoff_outer))) / 2.0;
    } else {
        return 0.0;
    }
}

void main()
{
    vec4 tex = texture(textureSampler, vertexUV);
    if (useBlackKey && tex.r < 0.05 && tex.g < 0.05 && tex.b < 0.05) discard;

    float light_scalar = shadow_scalar() * spotlight_scalar();
    
    vec3 ambient  = ambient_color(light_color);
    vec3 diffuse  = light_scalar * diffuse_color(light_color, light_position);
    vec3 specular = light_scalar * specular_color(light_color, light_position);

    vec3 lighting = (ambient + diffuse + specular) * object_color;
    vec3 final_color = tex.rgb * lighting;

    result = vec4(final_color, tex.a);
}
