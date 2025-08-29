#version 460

// ===============================
// Compute Shader Configuration
// ===============================
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

// Output image
layout(rgba32f, binding = 0) uniform image2D outputImage;

// ===============================
// Uniforms
// ===============================
uniform vec2 resolutionVector;   // image_width, image_height
uniform vec3 cameraPos;
uniform mat4 invProjection;
uniform mat4 invView;

uniform vec3 bh_center;
uniform float Rs;                 // Schwarzschild radius

uniform vec3 starCenter;
uniform float starRadius;
uniform vec3 starEmissionColor;
uniform float starIntensity;

// ===============================
// Constants
// ===============================
const int max_phi_steps = 4000;
const float dphi = 0.002;
const float r_escape = 1e6;
const float epsilon_horizon = 1e-4;
const vec3 background_color = vec3(0.0);
const float EPSILON = 1e-12;

// ===============================
// Helper Functions
// ===============================

// Generate primary ray from NDC coordinates
void generate_primary_ray(vec2 ndc, vec3 cam_pos, mat4 inv_proj, mat4 inv_v, 
                         out vec3 origin, out vec3 direction) {
    vec4 clip = vec4(ndc.x, ndc.y, -1.0, 1.0);
    vec4 view = inv_proj * clip;
    view = vec4(view.xy, -1.0, 0.0);
    vec4 world = inv_v * view;
    direction = normalize(world.xyz);
    origin = cam_pos;
}

// Build plane basis from camera position, ray direction, and black hole center
void build_plane_basis(vec3 cam_pos, vec3 ray_dir, vec3 bh_ctr, 
                      out vec3 e1, out vec3 e2) {
    vec3 to_bh = bh_ctr - cam_pos;
    e1 = normalize(ray_dir);
    
    vec3 tmp = to_bh - e1 * dot(to_bh, e1);
    if (length(tmp) < 1e-8) {
        // Degenerate case: choose stable perpendicular
        tmp = abs(e1.y) < 0.9 ? vec3(0, 1, 0) : vec3(1, 0, 0);
        tmp = tmp - e1 * dot(tmp, e1);
    }
    e2 = normalize(tmp);
}

// Convert 3D point to plane coordinates
void to_plane_coords(vec3 point3, vec3 origin3, vec3 e1, vec3 e2, vec3 bh_ctr,
                    out float r, out float phi, out float x, out float y) {
    vec3 p = point3 - bh_ctr;
    x = dot(p, e1);
    y = dot(p, e2);
    r = sqrt(x * x + y * y);
    phi = atan(y, x);
}

// Convert plane coordinates back to 3D
vec3 from_plane_coords(float x, float y, vec3 e1, vec3 e2, vec3 bh_ctr) {
    return bh_ctr + e1 * x + e2 * y;
}

// Calculate radial and angular velocities
void radial_angular_vel(vec3 ray_dir, vec3 e1, vec3 e2, float r, float phi,
                       out float v_r, out float v_phi) {
    vec3 er = cos(phi) * e1 + sin(phi) * e2;      // radial unit vector
    vec3 ephi = -sin(phi) * e1 + cos(phi) * e2;   // angular unit vector
    
    v_r = dot(ray_dir, er);
    v_phi = dot(ray_dir, ephi) / max(r, EPSILON);
}

// RK4 ODE solver for photon motion
// y = [u, up] where u = 1/r, up = du/dphi
// The ODE: u'' = -u + 3*M*u^2
vec2 f(float phi, vec2 y, float M) {
    float u = y.x;
    float up = y.y;
    return vec2(up, -u + 3.0 * M * u * u);
}

vec2 rk4_step(float phi, vec2 y, float h, float M) {
    vec2 k1 = f(phi,           y,               M);
    vec2 k2 = f(phi + 0.5 * h, y + 0.5 * h * k1, M);
    vec2 k3 = f(phi + 0.5 * h, y + 0.5 * h * k2, M);
    vec2 k4 = f(phi + h,       y + h * k3,       M);
    
    return y + (h / 6.0) * (k1 + 2.0 * k2 + 2.0 * k3 + k4);
}

// Check if position hits the star
bool hit_star(vec3 pos3, vec3 star_ctr, float star_rad) {
    return length(pos3 - star_ctr) <= star_rad;
}

// ===============================
// Main Ray Tracing Function
// ===============================
vec3 trace_ray(vec2 pixel) {
    // 1) Map pixel to NDC coordinates
    vec2 uv = (pixel + 0.5) / resolutionVector;
    vec2 ndc = vec2(uv.x * 2.0 - 1.0, -(uv.y * 2.0 - 1.0));
    
    // 2) Generate primary ray
    vec3 ray_origin, ray_dir;
    generate_primary_ray(ndc, cameraPos, invProjection, invView, 
                        ray_origin, ray_dir);
    
    // 3) Build plane basis
    vec3 e1, e2;
    build_plane_basis(ray_origin, ray_dir, bh_center, e1, e2);
    
    // 4) Get initial polar coordinates
    float r0, phi0, x0, y0;
    to_plane_coords(ray_origin, cameraPos, e1, e2, bh_center, 
                   r0, phi0, x0, y0);
    
    // 5) Calculate initial velocities and convert to u, u'
    float vr, vphi;
    radial_angular_vel(ray_dir, e1, e2, r0, phi0, vr, vphi);
    
    float dphidl = max(vphi, EPSILON);
    float u0 = 1.0 / max(r0, EPSILON);
    float up0 = -(1.0 / (r0 * r0)) * (vr / dphidl);
    
    // 6) Integrate photon path using RK4
    vec2 y = vec2(u0, up0);
    float phi = phi0;
    float M = 0.5 * Rs;  // GM/c^2
    
    for (int i = 0; i < max_phi_steps; i++) {
        // Current position from u and phi
        float u = max(y.x, EPSILON);
        float r = 1.0 / u;
        float x = r * cos(phi);
        float y2d = r * sin(phi);
        vec3 pos3 = from_plane_coords(x, y2d, e1, e2, bh_center);
        
        // Check if absorbed by horizon
        if (r <= Rs * (1.0 + epsilon_horizon)) {
            return vec3(0.0);  // Black hole absorption
        }
        
        // Check if hit star
        if (hit_star(pos3, starCenter, starRadius)) {
            return starEmissionColor * starIntensity;
        }
        
        // Check if escaped to infinity
        if (r >= r_escape) {
            return background_color;
        }
        
        // RK4 integration step
        y = rk4_step(phi, y, dphi, M);
        phi += dphi;
    }
    
    // Max steps reached - return background
    return background_color;
}

// ===============================
// Main Compute Shader Entry Point
// ===============================
void main() {
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    
    // Check bounds
    if (pixel.x >= int(resolutionVector.x) || pixel.y >= int(resolutionVector.y)) {
        return;
    }
    
    // Trace ray for this pixel
    vec3 color = trace_ray(vec2(pixel));
    
    // Write to output image
    imageStore(outputImage, pixel, vec4(color, 1.0));
}