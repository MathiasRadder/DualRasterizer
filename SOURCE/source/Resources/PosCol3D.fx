//---------------------------
// Variables
//---------------------------
float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorldMatrix : WORLD;
float4x4 gViewInverse : VIEWINVERSE;
Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gGlossinessMap : GlossinessMap;
Texture2D gSpecularMap : SpecularMap;
bool gIsFlatShade : IsFlatShade;
float3 gLightDirection =  float3(0.577f, -0.577f, -0.577f);
float gPI = 3.141592653589f;
float gLightIntensity = 7.0f; //should be 7.0f
float gShininess  = 25.0f;

//---------------------------
// Texture
//---------------------------



SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Border;// or Mirror or Clamp Border
	AddressV = Clamp;// or Mirror or Clamp Border
	BorderColor = float4(0.0f,0.0f,1.0f,1.0f);
};

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Border;// or Mirror or Clamp Border
	AddressV = Clamp;// or Mirror or Clamp Border
	BorderColor = float4(0.0f,0.0f,1.0f,1.0f);
};

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	AddressU = Border;// or Mirror or Clamp Border
	AddressV = Clamp;// or Mirror or Clamp Border
	BorderColor = float4(0.0f,0.0f,1.0f,1.0f);
};

//---------------------------
// Input/Output Structs
//---------------------------
struct VS_INPUT
{
	float3 Position : POSITION;
	float4 WorldPosition : COLOR;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 Texcoord : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 WorldPosition : COLOR;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 Texcoord : TEXCOORD;
};

//---------------------------
// Vertex Shader
//---------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = mul(float4(input.Position,1.f),gWorldViewProj);
	output.Tangent = mul( normalize(input.Tangent),(float3x3)gWorldMatrix);
	output.Normal = mul( normalize(input.Normal),(float3x3)gWorldMatrix);
	output.Texcoord = input.Texcoord;
	output.WorldPosition = mul(input.WorldPosition,gWorldMatrix);
	return output;
}

//---------------------------
// Pixel Shader
//---------------------------

//MaxToOne
float3 MaxToOne(float3 colour)
{
	float biggestValue = 0.0f;
	for(int i = 0; i < 3; i++)
	{
		if(colour[i] > 1 && colour[i] > biggestValue)
		{
			biggestValue = colour[i];
		}
	}

	if(biggestValue < 1.0f)
	{
		return colour;
	}
	
	colour /= biggestValue;
	return colour;

}
//Phong
float3 Phong(float3 spc, float phongExponnent, float3 viewDrection, float3 inputNormal)
{
	float reflectance = saturate(dot(reflect(-gLightDirection,inputNormal),viewDrection));
	return  spc * pow(reflectance,phongExponnent);
}
//Shading
float3 Shading(VS_OUTPUT input,SamplerState samState)
{
	
	float3 lightColor = float3(1.0f,1.0f,1.0f); 
	float3 ambient = float3(0.025f,0.025f,0.025f); 
	//variable samples
	float3 diff = gDiffuseMap.Sample(samState,input.Texcoord).rgb;
	float3 norm = gNormalMap.Sample(samState,input.Texcoord).rgb;
	float3 gloss = gGlossinessMap.Sample(samState,input.Texcoord).rgb;
	float3 spec = gSpecularMap.Sample(samState,input.Texcoord).rgb;
	
	
	//making the color from 0 to 255 to -1 to 1
	norm *= 2.f;
	norm.r -=1.f;
	norm.g -=1.f;
	norm.b -=1.f;
	
	//making the observer area
	float3 binormal =  cross(input.Tangent, input.Normal);
	float3x3 tangentSpaceAxis =  float3x3(input.Tangent, binormal, input.Normal);
	float3 newNormal = normalize(mul(norm, tangentSpaceAxis));
	//making the observerdArea
	float observerdArea = saturate(float(dot(newNormal,-gLightDirection)));
	observerdArea/= gPI;
	float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInverse[3].xyz);
	
	float3 phongcolor = diff +(Phong(spec, float(gloss.r * gShininess), viewDirection, input.Normal)+ambient);
	
	float3 finalColor = (lightColor* gLightIntensity * phongcolor * observerdArea);
	
	finalColor = MaxToOne(finalColor);
	return finalColor;
}
//FlatShading
float4 FlatShading(VS_OUTPUT input,SamplerState samState)
{
	//variable
	float3 lightColor = float3(1.0f,1.0f,1.0f); 
	float4 diff = gDiffuseMap.Sample(samState,input.Texcoord);
	
	//making the observerdArea
	float observerdArea = saturate(float(dot(-input.Normal,gLightDirection)));
	observerdArea/= gPI;
	float3 finalColor = (lightColor* gLightIntensity * diff.rgb + observerdArea );
	
	finalColor = MaxToOne(finalColor);
	return float4(finalColor,diff.a);
}
float4 PS(VS_OUTPUT input) : SV_TARGET
{
	if(gIsFlatShade)
	{
		return (FlatShading(input,samPoint));
	}
	return float4(Shading(input,samPoint),1.f);
}
//Ps linear
float4 PSLin(VS_OUTPUT input) : SV_TARGET
{
	if(gIsFlatShade)
	{
		return (FlatShading(input,samLinear));
	}
	return float4(Shading(input,samLinear),1.f);
}
//ps Anisotropic
float4 PSAnis(VS_OUTPUT input) : SV_TARGET
{
	if(gIsFlatShade)
	{
		return (FlatShading(input,samAnisotropic));
	}
	return float4(Shading(input,samAnisotropic),1.f);
}


//---------------------------
// Techniques
//---------------------------
//point technique
technique11 DefaultTechnique
{
	pass P0
	{
	
		SetVertexShader( CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader( CompileShader(ps_5_0, PS()));
	}
}
//AnisotropicTechnique
technique11 AnisotropicTechnique
{
	pass P0
	{
		
		SetVertexShader( CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader( CompileShader(ps_5_0, PSAnis()));
	}
}
//LinearTechnique
technique11 LinearTechnique
{
	pass P0
	{
		
		SetVertexShader( CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader( CompileShader(ps_5_0, PSLin()));
	}
}













