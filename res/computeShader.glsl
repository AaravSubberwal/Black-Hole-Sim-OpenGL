#version 460 core

layout(local_size_x = 8, local_size_y = 8) in;
layout(rgba32f, binding = 0) uniform image2D outputImage;

uniform vec3 sphereCenter;
uniform float sphereRadius;
uniform vec3 cameraPos;
uniform mat4 invProjection;
uniform mat4 invView;

struct Ray {
    vec3 origin;
    vec3 direction;
};

Ray generateRay(vec2 ndc) {
    Ray ray;
    ray.origin = cameraPos;
    
    vec4 clipSpace = vec4(ndc, -1.0, 1.0);
    vec4 viewSpace = invProjection * clipSpace;
    viewSpace = vec4(viewSpace.xy, -1.0, 0.0);
    vec3 worldDir = normalize((invView * viewSpace).xyz);
    
    ray.direction = worldDir;
    return ray;
}

bool intersectSphere(Ray ray, vec3 center, float radius, out float t) {
    vec3 oc = ray.origin - center;
    float a = dot(ray.direction, ray.direction);
    float b = 2.0 * dot(oc, ray.direction);
    float c = dot(oc, oc) - radius * radius;
    float discriminant = b * b - 4 * a * c;
    
    if (discriminant < 0) {
        return false;
    }
    
    float t0 = (-b - sqrt(discriminant)) / (2.0 * a);
    float t1 = (-b + sqrt(discriminant)) / (2.0 * a);
    
    if (t0 > 0) {
        t = t0;
        return true;
    } else if (t1 > 0) {
        t = t1;
        return true;
    }
    
    return false;
}

vec3 calculateLighting(vec3 hitPoint, vec3 normal, vec3 viewDir) {
    vec3 lightPos = vec3(5.0, 5.0, 5.0);
    vec3 lightColor = vec3(1.0);
    vec3 objectColor = vec3(1.0, 0.5, 0.31);
    
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
    
    vec3 lightDir = normalize(lightPos - hitPoint);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    float specularStrength = 0.5;
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    
    return (ambient + diffuse + specular) * objectColor;
}

void main() {
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 imageSize = imageSize(outputImage);
    
    if (pixelCoord.x >= imageSize.x || pixelCoord.y >= imageSize.y) {
        return;
    }
    
    vec2 uv = (vec2(pixelCoord) + 0.5) / vec2(imageSize);
    vec2 ndc = uv * 2.0 - 1.0;
    ndc.y = -ndc.y;
    
    Ray ray = generateRay(ndc);
    
    vec3 color = vec3(0.1);
    
    float t;
    if (intersectSphere(ray, sphereCenter, sphereRadius, t)) {
        vec3 hitPoint = ray.origin + ray.direction * t;
        vec3 normal = normalize(hitPoint - sphereCenter);
        color = calculateLighting(hitPoint, normal, -ray.direction);
    }
    
    imageStore(outputImage, pixelCoord, vec4(color, 1.0));
}