// HLSL (High-Level Shading Language) sample
// DirectX shader example

// Preprocessor directives
#define PI 3.14159265359
#define MAX_LIGHTS 8

#ifdef USE_TEXTURES
Texture2D diffuseTexture : register(t0);
Texture2D normalTexture : register(t1);
SamplerState samplerState : register(s0);
#endif

// Constant buffers
cbuffer TransformBuffer : register(b0) {
    matrix model;
    matrix view;
    matrix projection;
    matrix normalMatrix;
    float3 viewPos;
    float padding;
};

cbuffer MaterialBuffer : register(b1) {
    float3 albedo;
    float metallic;
    float roughness;
    float ao;
    float2 padding2;
};

cbuffer LightBuffer : register(b2) {
    float3 lightPositions[MAX_LIGHTS];
    float3 lightColors[MAX_LIGHTS];
    int numLights;
    float3 padding3;
};

// Structs
struct VertexInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD0;
    float3 tangent : TANGENT;
    float3 bitangent : BINORMAL;
};

struct VertexOutput {
    float4 position : SV_POSITION;
    float3 worldPos : WORLDPOS;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD0;
    float3 tangent : TANGENT;
    float3 bitangent : BINORMAL;
};

// Functions
float DistributionGGX(float3 N, float3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float3 fresnelSchlick(float cosTheta, float3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Vertex shader
VertexOutput VSMain(VertexInput input) {
    VertexOutput output;
    
    output.position = mul(projection, mul(view, mul(model, float4(input.position, 1.0))));
    output.worldPos = mul(model, float4(input.position, 1.0)).xyz;
    output.normal = mul((float3x3)normalMatrix, input.normal);
    output.texCoord = input.texCoord;
    output.tangent = mul((float3x3)normalMatrix, input.tangent);
    output.bitangent = mul((float3x3)normalMatrix, input.bitangent);
    
    return output;
}

// Pixel shader
float4 PSMain(VertexOutput input) : SV_TARGET {
    float3 N = normalize(input.normal);
    float3 V = normalize(viewPos - input.worldPos);

    // Calculate TBN matrix
    float3 T = normalize(input.tangent);
    float3 B = normalize(input.bitangent);
    float3x3 TBN = float3x3(T, B, N);

    // Normal mapping
    #ifdef USE_TEXTURES
    float3 normal = normalTexture.Sample(samplerState, input.texCoord).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    N = normalize(mul(TBN, normal));
    #endif

    // PBR lighting
    float3 Lo = float3(0.0, 0.0, 0.0);
    for (int i = 0; i < numLights; ++i) {
        float3 L = normalize(lightPositions[i] - input.worldPos);
        float3 H = normalize(V + L);
        float distance = length(lightPositions[i] - input.worldPos);
        float attenuation = 1.0 / (distance * distance);
        float3 radiance = lightColors[i] * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F = fresnelSchlick(max(dot(H, V), 0.0), float3(0.04, 0.04, 0.04));

        float3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        float3 specular = numerator / denominator;

        float3 kS = F;
        float3 kD = float3(1.0, 1.0, 1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // Ambient lighting
    float3 ambient = float3(0.03, 0.03, 0.03) * albedo * ao;
    float3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + float3(1.0, 1.0, 1.0));
    color = pow(color, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));

    return float4(color, 1.0);
}

// Geometry shader example
[maxvertexcount(3)]
void GSMain(triangle VertexOutput input[3], inout TriangleStream<VertexOutput> triStream) {
    for (int i = 0; i < 3; ++i) {
        VertexOutput output;
        output.position = input[i].position;
        output.worldPos = input[i].worldPos;
        output.normal = input[i].normal;
        output.texCoord = input[i].texCoord;
        output.tangent = input[i].tangent;
        output.bitangent = input[i].bitangent;
        triStream.Append(output);
    }
    triStream.RestartStrip();
}

// Compute shader example
[numthreads(16, 16, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID) {
    uint width, height;
    diffuseTexture.GetDimensions(width, height);
    
    float2 texCoord = float2(DTid.x / (float)width, DTid.y / (float)height);
    float4 color = diffuseTexture.Sample(samplerState, texCoord);
    
    // Apply some effect
    color.rgb = pow(color.rgb, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
    
    // Output to UAV
    // RWTexture2D<float4> outputTexture : register(u0);
    // outputTexture[DTid.xy] = color;
}

// Technique and pass
technique11 PBR {
    pass P0 {
        SetVertexShader(CompileShader(vs_5_0, VSMain()));
        SetPixelShader(CompileShader(ps_5_0, PSMain()));
    }
}

// Hull shader example
struct HullInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD0;
};

struct HullOutput {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD0;
};

struct PatchTess {
    float EdgeTess[3] : SV_TessFactor;
    float InsideTess : SV_InsideTessFactor;
};

PatchTess ConstantHS(InputPatch<HullInput, 3> patch, uint patchID : SV_PrimitiveID) {
    PatchTess pt;
    pt.EdgeTess[0] = 1.0;
    pt.EdgeTess[1] = 1.0;
    pt.EdgeTess[2] = 1.0;
    pt.InsideTess = 1.0;
    return pt;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ConstantHS")]
HullOutput HSMain(InputPatch<HullInput, 3> patch, uint i : SV_OutputControlPointID) {
    HullOutput output;
    output.position = patch[i].position;
    output.normal = patch[i].normal;
    output.texCoord = patch[i].texCoord;
    return output;
}

// Domain shader example
[domain("tri")]
VertexOutput DSMain(PatchTess pt, float3 bary : SV_DomainLocation, const OutputPatch<HullOutput, 3> patch) {
    VertexOutput output;
    output.position = float4(
        patch[0].position * bary.x + patch[1].position * bary.y + patch[2].position * bary.z, 1.0);
    output.normal = normalize(
        patch[0].normal * bary.x + patch[1].normal * bary.y + patch[2].normal * bary.z);
    output.texCoord = 
        patch[0].texCoord * bary.x + patch[1].texCoord * bary.y + patch[2].texCoord * bary.z;
    return output;
}
