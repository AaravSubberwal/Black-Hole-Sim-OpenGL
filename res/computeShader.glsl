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
uniform ivec2 resolutionVector;   // image_width, image_height
uniform vec3 cameraPos;
uniform mat4 invProjection;
uniform mat4 invView;

uniform vec3 bh_center;
uniform float Rs;                 // Schwarzschild radius

// Accretion disk parameters
uniform float diskInnerRadius;   // typically ~3*Rs (ISCO)
uniform float diskOuterRadius;   // typically ~20*Rs
uniform vec3 diskColor;           // base color (orange/red)
uniform float diskIntensity;      // brightness multiplier
uniform vec3 diskNormal;          // disk plane normal (usually (0,1,0))
uniform float diskBetaInner;      // signed orbital speed at inner edge, in units of c
uniform float diskBetaMax;        // cap for orbital speed, in units of c
uniform float dopplerStrength;    // blends between no beaming and full beaming
uniform int enableDopplerBeaming;

// ===============================
// Constants
// ===============================
const int max_phi_steps = 4000;
const float dphi = 0.002;
const float r_escape = 1e6;
const float epsilon_horizon = 1e-4;
const vec3 background_color = vec3(0.003, 0.004, 0.008);
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

float hash13(vec3 p) {
    p = fract(p * 0.1031);
    p += dot(p, p.yzx + 33.33);
    return fract((p.x + p.y) * p.z);
}

float hash12(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

float noise2(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);

    float a = hash12(i);
    float b = hash12(i + vec2(1.0, 0.0));
    float c = hash12(i + vec2(0.0, 1.0));
    float d = hash12(i + vec2(1.0, 1.0));

    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

vec3 background_starfield(vec3 ray_dir) {
    vec3 dir = normalize(ray_dir);
    vec3 scaled = dir * 180.0;
    vec3 cell = floor(scaled);
    vec3 local = fract(scaled) - 0.5;

    float star_seed = hash13(cell);
    float star_mask = smoothstep(0.9945, 0.9995, star_seed);

    float distance_from_cell_center = length(local);
    float core = smoothstep(0.26, 0.0, distance_from_cell_center);
    float glow = smoothstep(0.45, 0.0, distance_from_cell_center);
    float brightness = star_mask * (1.4 * core + 0.35 * glow);

    float color_seed = hash13(cell + vec3(19.7, 7.3, 3.1));
    vec3 star_color = mix(vec3(1.0, 0.82, 0.72), vec3(0.72, 0.82, 1.0), color_seed);

    float band = exp(-18.0 * dir.y * dir.y);
    vec3 galactic_glow = vec3(0.025, 0.02, 0.035) * band;

    return background_color + galactic_glow + star_color * brightness;
}

// Check if position intersects the accretion disk
bool hit_disk(vec3 pos3, vec3 bh_ctr, vec3 disk_norm, float inner_r, float outer_r,
              out float disk_r, out vec3 disk_pos) {
    // Project position onto disk plane
    vec3 to_pos = pos3 - bh_ctr;
    float height = dot(to_pos, disk_norm);
    
    // Check if close to disk plane (thin disk approximation)
    if (abs(height) > 0.01 * outer_r) {
        return false;
    }
    
    // Position in disk plane
    disk_pos = pos3 - height * disk_norm;
    vec3 radial = disk_pos - bh_ctr;
    disk_r = length(radial);
    
    // Check if within disk radii
    return disk_r >= inner_r && disk_r <= outer_r;
}

// Calculate disk emission based on radius (simple temperature profile)
vec3 disk_emission(vec3 disk_pos, vec3 bh_ctr, vec3 disk_norm,
                   float r, float inner_r, float outer_r,
                   vec3 base_color, float intensity) {
    // Simple temperature profile: T ~ r^(-3/4) for thin disk
    float t = (r - inner_r) / (outer_r - inner_r);
    float temp_factor = pow(1.0 - t, 0.75);  // hotter near inner edge

    vec3 normal_dir = normalize(disk_norm);
    vec3 basis_x = normalize(abs(normal_dir.y) < 0.9 ? cross(normal_dir, vec3(0.0, 1.0, 0.0))
                                                     : cross(normal_dir, vec3(1.0, 0.0, 0.0)));
    vec3 basis_z = normalize(cross(normal_dir, basis_x));
    vec3 radial = disk_pos - bh_ctr;
    vec2 local_uv = vec2(dot(radial, basis_x), dot(radial, basis_z));
    local_uv /= outer_r;

    float radial_coord = (r - inner_r) / max(outer_r - inner_r, EPSILON);
    float turbulence = noise2(local_uv * 12.0);
    turbulence += 0.5 * noise2(local_uv * 24.0);
    turbulence /= 1.5;
    float warped_radius = radial_coord + (turbulence * 0.06);

    float banding = 0.5 + 0.5 * sin(warped_radius * 45.0);
    banding *= 0.5 + 0.5 * sin(warped_radius * 15.0 + 0.8);
    banding = pow(banding, 1.2);

    float variation = mix(0.65, 1.4, banding) * mix(0.8, 1.2, turbulence);

    vec3 hot_color = vec3(1.0, 0.95, 0.8);
    float heat = clamp(temp_factor + banding * 0.25, 0.0, 1.0);
    vec3 color = mix(base_color, hot_color, heat);
    
    return color * intensity * temp_factor * variation;
}

float disk_beaming_factor(vec3 disk_pos, vec3 bh_ctr, vec3 disk_norm, float disk_r) {
    vec3 radial = disk_pos - bh_ctr;
    float radial_len = length(radial);
    if (radial_len <= EPSILON) {
        return 1.0;
    }

    vec3 radial_dir = radial / radial_len;
    vec3 normal_dir = normalize(disk_norm);
    vec3 tangential_dir = cross(normal_dir, radial_dir);
    float tangential_len = length(tangential_dir);
    if (tangential_len <= EPSILON) {
        return 1.0;
    }

    tangential_dir /= tangential_len;

    float beta = clamp(abs(diskBetaInner) * sqrt(diskInnerRadius / max(disk_r, diskInnerRadius)),
                       0.0,
                       diskBetaMax);
    tangential_dir *= sign(diskBetaInner == 0.0 ? 1.0 : diskBetaInner);

    vec3 view_dir = normalize(cameraPos - disk_pos);
    float cos_theta = clamp(dot(tangential_dir, view_dir), -0.999, 0.999);
    float numerator = sqrt(max(1.0 - beta * beta, 1e-4));
    float denominator = max(1.0 - beta * cos_theta, 0.05);
    float doppler_factor = numerator / denominator;
    float beaming = pow(max(doppler_factor, 0.0), 3.0);

    return mix(1.0, min(beaming, 8.0), clamp(dopplerStrength, 0.0, 1.0));
}

// ===============================
// Main Ray Tracing Function
// ===============================
vec3 trace_ray(vec2 pixel) {
    // 1) Map pixel to NDC coordinates
    vec2 uv = (vec2(pixel) + 0.5) / vec2(resolutionVector);
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
        
        // Check if hit accretion disk
        float disk_r;
        vec3 disk_pos;
        if (hit_disk(pos3, bh_center, diskNormal, diskInnerRadius, diskOuterRadius, 
                     disk_r, disk_pos)) {
            vec3 emission = disk_emission(disk_pos, bh_center, diskNormal,
                                          disk_r, diskInnerRadius, diskOuterRadius, 
                                          diskColor, diskIntensity);
            if (enableDopplerBeaming != 0) {
                emission *= disk_beaming_factor(disk_pos, bh_center, diskNormal, disk_r);
            }
            return emission;
        }
        
        // Check if escaped to infinity
        if (r >= r_escape) {
            return background_starfield(pos3 - bh_center);
        }
        
        // RK4 integration step
        y = rk4_step(phi, y, dphi, M);
        phi += dphi;
    }
    
    // Max steps reached - return background
    return background_starfield(ray_dir);
}

// ===============================
// Main Compute Shader Entry Point
// ===============================
void main() {
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    
    // Check bounds
    if (pixel.x >= (resolutionVector.x) || pixel.y >= (resolutionVector.y)) {
        return;
    }
    
    // Trace ray for this pixel
    vec3 color = trace_ray(vec2(pixel));
    
    // Write to output image
    imageStore(outputImage, pixel, vec4(color, 1.0));
}
