// SweetLine WGSL sample: attributes, resource bindings, structs, aliases, and control flow.
// Reference URL: https://gpuweb.github.io/gpuweb/wgsl/

enable f16;

alias Color = vec4f;

struct CameraUniforms {
  view_proj : mat4x4f,
  exposure : f32,
  time : f32,
  frame_index : u32,
}

struct VertexInput {
  @location(0) position : vec3f,
  @location(1) normal : vec3f,
  @location(2) uv : vec2f,
}

struct VertexOutput {
  @builtin(position) position : vec4f,
  @location(0) world_normal : vec3f,
  @location(1) uv : vec2f,
}

struct ComputeData {
  values : array<f32, 64>,
  tint : vec4f,
}

@group(0) @binding(0) var<uniform> camera : CameraUniforms;
@group(0) @binding(1) var base_sampler : sampler;
@group(0) @binding(2) var base_texture : texture_2d<f32>;
@group(1) @binding(0) var<storage, read_write> compute_data : ComputeData;

const PI : f32 = 3.14159265f;
const HALF_PI : f32 = 1.57079632f;
override exposure_bias : f32 = 1.0f;
const_assert 8u > 0u;

fn saturate(value : f32) -> f32 {
  return clamp(value, 0.0f, 1.0f);
}

fn remap(value : f32, min_value : f32, max_value : f32) -> f32 {
  let span = max_value - min_value;
  if span == 0.0f {
    return 0.0f;
  }
  return saturate((value - min_value) / span);
}

fn build_basis(normal : vec3f) -> mat3x3f {
  let up = select(vec3f(0.0f, 1.0f, 0.0f), vec3f(1.0f, 0.0f, 0.0f), abs(normal.y) > 0.99f);
  let tangent = normalize(cross(up, normal));
  let bitangent = normalize(cross(normal, tangent));
  return mat3x3f(tangent, bitangent, normal);
}

@vertex
fn vs_main(input : VertexInput) -> VertexOutput {
  var output : VertexOutput;
  let animated = input.position + vec3f(sin(camera.time + input.position.x), 0.0f, 0.0f) * 0.05f;
  output.position = camera.view_proj * vec4f(animated, 1.0f);
  output.world_normal = normalize(input.normal);
  output.uv = input.uv;
  return output;
}

@fragment
fn fs_main(input : VertexOutput) -> @location(0) vec4f {
  let albedo = textureSample(base_texture, base_sampler, input.uv);
  let lit = saturate(dot(normalize(input.world_normal), normalize(vec3f(0.35f, 0.8f, 0.45f))));
  let boosted = remap(lit * camera.exposure * exposure_bias, 0.0f, 2.0f);
  let overlay : Color = vec4f(vec3f(boosted), 1.0f);
  return vec4f(albedo.rgb * overlay.rgb, albedo.a);
}

var<workgroup> local_average : array<vec4f, 64>;

fn sample_value(index : u32) -> f32 {
  return compute_data.values[index];
}

@compute @workgroup_size(8, 8, 1)
fn cs_main(
  @builtin(local_invocation_id) local_id : vec3u,
  @builtin(global_invocation_id) global_id : vec3u
) {
  let flat_index = local_id.y * 8u + local_id.x;
  let source_index = (global_id.x + global_id.y) % 64u;
  var value = sample_value(source_index);
  var weight = 1.0f;

  loop {
    if weight <= 0.125f {
      break;
    }
    value = value * weight;
    weight = weight * 0.5f;
    continuing {
      if weight < 0.2f {
        break if flat_index == 0u;
      }
    }
  }

  switch flat_index {
    case 0u, 1u: {
      local_average[flat_index] = vec4f(value, 0.0f, 0.0f, 1.0f);
    }
    case 2u: {
      local_average[flat_index] = vec4f(0.0f, value, 0.0f, 1.0f);
    }
    default: {
      local_average[flat_index] = vec4f(0.0f, 0.0f, value, 1.0f);
    }
  }

  compute_data.values[flat_index] = local_average[flat_index].x + HALF_PI;
}
