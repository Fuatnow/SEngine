#version 430 core

/**
 * Variance Shadow Mapping
 * (Chebyshev probability prediction)
**/

in layout(location = 0) vec4 final_position;

out layout(location = 0) vec4 Depth;

void main()
{
	float d = final_position.z / final_position.w;
	d = d * 0.5 + 0.5;
	
	float moment1 = d;
	float moment2 = d * d;
	
	// Bias
	float dx = dFdx(d);
	float dy = dFdy(d);
	moment2 += 0.25 * (dx * dx + dy * dy);
	
	Depth = vec4(moment1, moment2, 0.0, 1.0);
}
