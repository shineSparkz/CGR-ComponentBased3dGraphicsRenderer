#version 330

out vec4 frag_colour;

in vec2 varying_texcoord;

uniform float u_GlobalTime;
uniform vec2 u_Resolution;
uniform sampler2D u_Sampler;

#define ANIM true
#define PI 3.1415927

mat3 m = mat3( 0.00,  0.80,  0.60,
              -0.80,  0.36, -0.48,
              -0.60, -0.48,  0.64 );

float hash( float n )
{
    return fract(sin(n)*43758.5453);
}

float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);

    f = f*f*(3.0-2.0*f);

    float n = p.x + p.y*57.0 + 113.0*p.z;

    float res = mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                        mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y),
                    mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                        mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
    return res;
}

float fbm( vec3 p )
{
    float f;
    f  = 0.5000*noise( p ); p = m*p*2.02;
    f += 0.2500*noise( p ); p = m*p*2.03;
    f += 0.1250*noise( p ); p = m*p*2.01;
    f += 0.0625*noise( p ); p = m*p*2.01;
    f += 0.0625*noise( p );
    return f;
}
// --- End of Created by inigo quilez

vec2 noise2_2( vec2 p )     // 2 noise channels from 2D position
{
	vec3 pos = vec3(p,.5);
	if (ANIM) pos.z += u_GlobalTime;
	pos *= m;
    float fx = noise(pos);
    float fy = noise(pos+vec3(1345.67,0,45.67));
    return vec2(fx,fy);
}

#define snoise(p)  (2.*noise(p)-1.)
#define snoise2_2(p)  (2.*noise2_2(p)-1.)
#define fbm2(p)  ((noise(p)+.5*noise(m*(p)))/1.5)

vec2 advfbm( vec2 p )
{	
    float l=1.;
	vec2 dp;	
    dp  =   snoise2_2(p+dp); l*=.5;
    dp += l*snoise2_2(p+dp); l*=.5;
    dp += l*snoise2_2(p+dp); l*=.5;
    dp += l*snoise2_2(p+dp); l*=.5;
    dp += l*snoise2_2(p+dp); 

    return dp;
}

float _h, _n,_d;
float scene(vec3 p)
{
	float d1 = length(p.xz);
	float d2 = length(p.xz-vec2(1.,0.));
	float h; 
	
	// main shape
	h =  .4*sin(5.*d1-u_GlobalTime       )/(.4+d1*d1)
	   + .4*sin(5.*d2-.71234*u_GlobalTime)/(.4+d2*d2);

	// add details
	vec3 pp = 15.*p+4.*u_GlobalTime*vec3(.5,.4,.8);
	float n;
	if (p.y-h < .2) n = fbm(pp); // details only if close
	else n = 0.; //.5*noise(pp); // detail approx
	_h = h; _n = n; _d=d1;
	h += .2*n; 
	
	return  .1*max(p.y-h,0.);
		// length(advfbm(10.*p.xy));
}

vec2 rotate(vec2 k,float t)
{   return vec2(cos(t)*k.x-sin(t)*k.y,sin(t)*k.x+cos(t)*k.y);
}

void main()
{	
    float speed = 0.;
	
	vec2 pos=(gl_FragCoord.xy / u_Resolution.y - vec2(.8,.5))*2. + (texture2D(u_Sampler, varying_texcoord).xy);
	//vec2 pos=(varying_texcoord.xy / u_Resolution.y - vec2(.8,.5))*2.;
	
	vec2 mouse = vec2(0.,.16);
	//if (iMouse.x > 0.) mouse = (iMouse.xy/iResolution.xy-.5)*2.;
    vec3 col;
	//ec3 sky = vec3(.6,.8,1.);
	vec3 sky = vec3(0.627, 0.501, 0.294);
	//vec3 light = vec3(.2,.8,-.2);
	vec3 light = vec3(.1,.1,.9);

	// camera, ray-marching & shading  inspired by rez
	// https://www.shadertoy.com/view/MsXGR2
	
	float fov = .5;
	vec3 dir=normalize(vec3(fov*pos,1.0));	// ray dir
	dir.yz=rotate(dir.yz, PI*mouse.y);		// rotation up/down
	dir.zx=rotate(dir.zx,-PI*mouse.x);		// rotation left/right
	//dir.xy=rotate(dir.xy,0.);	       		// twist
	vec3 ray=vec3(0.,2.,-4.);         		// pos along ray
	
	float l=0.,dl;
#define eps 1.e-3
	const int ray_n=128;
	for(int i=0; i<ray_n; i++)
	{
		l += dl = scene(ray+dir*l);
		if (dl<=eps) break;
	}
	if (true) // dl<=eps) 
	{
	vec3 hit = ray+dir*l;
	float H=_h, dH=_n;
		
	// shading ( -> eval normals)
	vec2 h = vec2(.005,-.005); 
	vec3 N = normalize(vec3(scene(hit+h.xyy),
						    scene(hit+h.yxx),
						    scene(hit+h.yyx)));
    //float c = texture(iChannel1,N.xzy).x;
	float c = dot(N,light);

	// fog
	float a = 1.0;// 1.-exp(-.15*l); // optical thickness

	// lava
	col = 1.0 * c * vec3(1.0, 0.3, 0.0); // shading
	col += pow(1.0 - dH, 2.0) * vec3(1.0, 0.6, 0.1) * 2.5 / (0.4 + 0.2 *_d *_d);

	float gaz = 1.5*fbm2(vec3(5.*(pos.x-PI*mouse.x),2.*pos.y-6.*u_GlobalTime,.5));
	col = a*sky*gaz+(1.-a)*col;
	}
	else
		col = sky;
	
#if 0
	vec2 disp = advfbm(15.*+mouse.x*uv);
	vec2 p = uv+mouse.x*disp;
	//col = texture(iChannel0,p).rgb;
	col = vec3(length(disp));
	//col = vec3(.5+.5*disp,.5);
#endif
	
	frag_colour = vec4(col.bgr, 1.0);
}

/*
vec3 mod289(vec3 x)
{
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x)
{
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x)
{
     return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
{ 
	const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
	const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

	// First corner
	vec3 i  = floor(v + dot(v, C.yyy) );
	vec3 x0 =   v - i + dot(i, C.xxx) ;

	// Other corners
	vec3 g = step(x0.yzx, x0.xyz);
	vec3 l = 1.0 - g;
	vec3 i1 = min( g.xyz, l.zxy );
	vec3 i2 = max( g.xyz, l.zxy );

	//   x0 = x0 - 0.0 + 0.0 * C.xxx;
	//   x1 = x0 - i1  + 1.0 * C.xxx;
	//   x2 = x0 - i2  + 2.0 * C.xxx;
	//   x3 = x0 - 1.0 + 3.0 * C.xxx;
	vec3 x1 = x0 - i1 + C.xxx;
	vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
	vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

	// Permutations
	i = mod289(i); 
	vec4 p = permute( permute( permute( 
			 i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
		   + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
		   + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

	// Gradients: 7x7 points over a square, mapped onto an octahedron.
	// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
	float n_ = 0.142857142857; // 1.0/7.0
	vec3  ns = n_ * D.wyz - D.xzx;

	vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

	vec4 x_ = floor(j * ns.z);
	vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

	vec4 x = x_ *ns.x + ns.yyyy;
	vec4 y = y_ *ns.x + ns.yyyy;
	vec4 h = 1.0 - abs(x) - abs(y);

	vec4 b0 = vec4( x.xy, y.xy );
	vec4 b1 = vec4( x.zw, y.zw );

	//vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
	//vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
	vec4 s0 = floor(b0)*2.0 + 1.0;
	vec4 s1 = floor(b1)*2.0 + 1.0;
	vec4 sh = -step(h, vec4(0.0));

	vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
	vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

	vec3 p0 = vec3(a0.xy,h.x);
	vec3 p1 = vec3(a0.zw,h.y);
	vec3 p2 = vec3(a1.xy,h.z);
	vec3 p3 = vec3(a1.zw,h.w);

	//Normalise gradients
	vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
	p0 *= norm.x;
	p1 *= norm.y;
	p2 *= norm.z;
	p3 *= norm.w;

	// Mix final noise value
	vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
	m = m * m;
	return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
								dot(p2,x2), dot(p3,x3) ) );
}

void main()
{
	float disp = 700.0;
	vec2 tex = varying_texcoord;
	
	vec2 div = vec2(disp, disp);
	vec2 uv = tex.xy / u_Resolution.xy * div.xy;

	vec3 v = vec3(
        uv.x + sin(u_GlobalTime) * 0.2, 
        uv.y + cos(u_GlobalTime) * 0.2, 
        u_GlobalTime / 10.0);

    float noise = snoise(v);
	uv = tex.xy / u_Resolution.xy * div.xy;
    vec3 v2 = vec3(uv.x , uv.y, u_GlobalTime / 5.0);
    noise = sin(noise * 3.14 * (sin(u_GlobalTime) + snoise(v2) * 2.0) * 0.75);
    
    float darkenFactor = 0.2;
    float darkenValue = darkenFactor;

    div = vec2(disp * 0.5, disp * 0.5);
    uv = tex.xy / u_Resolution.xy * div.xy;
    vec3 v3 = vec3(uv.x, uv.y, u_GlobalTime / 2.0);
    darkenValue = darkenValue * snoise(v3);
    
    vec3 v4 = vec3(uv.x * 1000.0, uv.y * 1000.0, u_GlobalTime);  

    float b = snoise(v4) * 0.1;
     
    frag_colour = vec4(
		1.0 - darkenValue + (noise * (darkenValue + 0.2)) - b,
        noise - b,
        b,
        1.0
	);
}
*/