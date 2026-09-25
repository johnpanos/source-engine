//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Fast low quality noise suitable for real time use
//
//=====================================================================================//

#include <math.h>
#include <float.h>	// Needed for FLT_EPSILON
#include "basetypes.h"
#include <memory.h>
#include "tier0/dbg.h"
#include "mathlib/mathlib.h"
#include "mathlib/vector.h"
#include "mathlib/ssemath.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
#include "noisedata.h"


#define MAGIC_NUMBER (1<<15)								// gives 8 bits of fraction

static fltx4 Four_MagicNumbers = { MAGIC_NUMBER, MAGIC_NUMBER, MAGIC_NUMBER, MAGIC_NUMBER };


static ALIGN16 int32 idx_mask[4]= {0xffff, 0xffff, 0xffff, 0xffff};

#define MASK255 (*((fltx4 *)(& idx_mask )))

// returns 0..1
static inline float GetLatticePointValue( int idx_x, int idx_y, int idx_z )
{
	NOTE_UNUSED(perm_d);
	NOTE_UNUSED(impulse_ycoords);
	NOTE_UNUSED(impulse_zcoords);

	int ret_idx = perm_a[idx_x & 0xff];
	ret_idx = perm_b[( idx_y + ret_idx ) & 0xff];
	ret_idx = perm_c[( idx_z + ret_idx ) & 0xff];
	return impulse_xcoords[ret_idx];

}

fltx4 NoiseSIMD( const fltx4 & x, const fltx4 & y, const fltx4 & z )
{
	// use magic to convert to integer index
	fltx4 x_idx = AndSIMD( MASK255, AddSIMD( x, Four_MagicNumbers ) );
	fltx4 y_idx = AndSIMD( MASK255, AddSIMD( y, Four_MagicNumbers ) );
	fltx4 z_idx = AndSIMD( MASK255, AddSIMD( z, Four_MagicNumbers ) );

	fltx4 lattice000 = Four_Zeros, lattice001 = Four_Zeros, lattice010 = Four_Zeros, lattice011 = Four_Zeros;
	fltx4 lattice100 = Four_Zeros, lattice101 = Four_Zeros, lattice110 = Four_Zeros, lattice111 = Four_Zeros;

	// FIXME: Converting the input vectors to int indices will cause load-hit-stores (48 bytes)
	//        Converting the indexed noise values back to vectors will cause more (128 bytes)
	//        The noise table could store vectors if we chunked it into 2x2x2 blocks.
	fltx4 xfrac = Four_Zeros, yfrac = Four_Zeros, zfrac = Four_Zeros;
#define DOPASS(i)															\
    {	unsigned int xi = SubInt( x_idx, i );								\
		unsigned int yi = SubInt( y_idx, i );								\
		unsigned int zi = SubInt( z_idx, i );								\
		SubFloat( xfrac, i ) = (xi & 0xff)*(1.0/256.0);						\
		SubFloat( yfrac, i ) = (yi & 0xff)*(1.0/256.0);						\
		SubFloat( zfrac, i ) = (zi & 0xff)*(1.0/256.0);						\
		xi>>=8;																\
		yi>>=8;																\
		zi>>=8;																\
																			\
		SubFloat( lattice000, i ) = GetLatticePointValue( xi,yi,zi );		\
		SubFloat( lattice001, i ) = GetLatticePointValue( xi,yi,zi+1 );		\
		SubFloat( lattice010, i ) = GetLatticePointValue( xi,yi+1,zi );		\
		SubFloat( lattice011, i ) = GetLatticePointValue( xi,yi+1,zi+1 );	\
		SubFloat( lattice100, i ) = GetLatticePointValue( xi+1,yi,zi );		\
		SubFloat( lattice101, i ) = GetLatticePointValue( xi+1,yi,zi+1 );	\
		SubFloat( lattice110, i ) = GetLatticePointValue( xi+1,yi+1,zi );	\
		SubFloat( lattice111, i ) = GetLatticePointValue( xi+1,yi+1,zi+1 );	\
    }

	DOPASS( 0 );
	DOPASS( 1 );
	DOPASS( 2 );
	DOPASS( 3 );

	// now, we have 8 lattice values for each of four points as m128s, and interpolant values for
	// each axis in m128 form in [xyz]frac. Perfom the trilinear interpolation as SIMD ops

	// first, do x interpolation
	fltx4 l2d00 = AddSIMD( lattice000, MulSIMD( xfrac, SubSIMD( lattice100, lattice000 ) ) );
	fltx4 l2d01 = AddSIMD( lattice001, MulSIMD( xfrac, SubSIMD( lattice101, lattice001 ) ) );
	fltx4 l2d10 = AddSIMD( lattice010, MulSIMD( xfrac, SubSIMD( lattice110, lattice010 ) ) );
	fltx4 l2d11 = AddSIMD( lattice011, MulSIMD( xfrac, SubSIMD( lattice111, lattice011 ) ) );

	// now, do y interpolation
	fltx4 l1d0 = AddSIMD( l2d00, MulSIMD( yfrac, SubSIMD( l2d10, l2d00 ) ) );
	fltx4 l1d1 = AddSIMD( l2d01, MulSIMD( yfrac, SubSIMD( l2d11, l2d01 ) ) );

	// final z interpolation
	fltx4 rslt = AddSIMD( l1d0, MulSIMD( zfrac, SubSIMD( l1d1, l1d0 ) ) );

	// map to 0..1
	return MulSIMD( Four_Twos, SubSIMD( rslt, Four_PointFives ) );


}

fltx4 NoiseSIMD( FourVectors const &pos )
{
	return NoiseSIMD( pos.x, pos.y, pos.z );
}

//-----------------------------------------------------------------------------
// Vector noise (DNoiseSIMD) and curl noise, declared in ssemath.h. Ported from
// the Portal 2 / CS:GO mathlib for the particle "turbulent force" operator.
//-----------------------------------------------------------------------------
static float s_randomGradients[] = { -0.460087, -0.887463, -0.058594, -0.458151, 0.861646,
    -0.430176, -0.930437, 0.316048, -0.195496, -0.883558, -0.393287, -0.276550, 0.171025, -0.983455,
    -0.329712, -0.033573, -0.941867, -0.994995, -0.476492, 0.014764, 0.879150, 0.834786, -0.454571,
    0.348755, -0.585801, -0.782531, -0.338745, 0.973990, -0.023774, 0.225403, -0.989659, -0.011313,
    -0.143005, 0.507109, -0.838016, -0.369141, -0.609995, -0.766277, 0.314087, 0.429987, 0.599850,
    -0.843323, 0.089587, -0.904071, -0.977783, -0.306997, -0.901432, 0.705078, 0.031606, 0.994782,
    -0.950806, 0.797663, -0.161508, -0.588806, 0.811569, -0.505360, 0.339783, 0.936130, -0.114223,
    0.334778, 0.217280, -0.970264, 0.440674, 0.600976, -0.712375, -0.516418, 0.197935, 0.979260,
    0.213501, 0.002956, 0.999995, -0.268127, -0.912763, 0.084651, -0.401062, -0.193271, -0.945607,
    -0.804382, 0.662480, 0.640156, -0.506348, 0.363459, -0.884439, 0.627197, -0.433415, 0.685363,
    0.803589, -0.721652, 0.416952, -0.607971, 0.647676, 0.296700, 0.734863, 0.723040, -0.444294,
    0.590454, -0.716318, -0.420435, -0.613770, -0.039076, -0.996459, 0.885437, 0.175225, -0.969092,
    0.703918, 0.116952, -0.991832, -0.399048, -0.504674, -0.013997, 0.863281, -0.436364, -0.817916,
    0.651733, 0.098030, -0.995090, 0.137573, 0.637157, -0.766031, -0.132263, -0.594718, 0.583153,
    -0.681213, -0.625632, 0.419913, -0.724426, -0.607341, -0.394521, 0.750427, -0.312161, 0.698925,
    0.899719, 0.101228, -0.927363, -0.962708, -0.934241, 0.041214, -0.354553, -0.826005, -0.284775,
    -0.507446, -0.363751, -0.929287, -0.173584, -0.141266, 0.983869, -0.613525, -0.436139,
    -0.074329, 0.899292, -0.875355, -0.480839, 0.057556, 0.250714, 0.071270, 0.967896, 0.182131,
    0.811467, 0.950195, -0.687696, -0.668570, -0.380554, 0.785175, -0.540171, -0.359863, 0.399774,
    0.848526, 0.655151, -0.412243, -0.004602, 0.911072, -0.132187, -0.990485, 0.278198, 0.212421,
    0.764179, 0.944214, -0.694878, 0.234042, -0.699402, 0.404273, 0.904644, -0.316406, 0.358393,
    0.087135, 0.933044, -0.473398, 0.820774, -0.559692, 0.044667, -0.997938, 0.718201, 0.603896,
    -0.046386, 0.796570, -0.968822, 0.180966, 0.172058, -0.458206, 0.886932, -0.126221, -0.656709,
    -0.410319, 0.693848, 0.999495, -0.018023, 0.026184, -0.486069, -0.740178, -0.690979, 0.942399,
    -0.333819, 0.022461, -0.294545, 0.867619, 0.805664, 0.886791, -0.416081, -0.221252, -0.797187,
    0.587661, -0.171021, -0.617708, -0.762817, -0.295654, 0.449351, -0.853660, -0.505615, 0.065153,
    -0.995535, 0.723572, 0.996518, 0.000000, 0.083374, 0.263346, 0.088663, -0.964417, -0.221316,
    -0.970864, 0.383423, -0.512560, 0.718804, 0.675598, 0.588859, 0.406293, -0.764648, -0.803841,
    -0.592769, -0.061646, 0.860199, 0.492898, -0.150330, -0.351871, 0.858024, 0.728455, 0.515724,
    -0.815149, 0.455322, -0.122322, -0.960484, 0.898254, -0.529020, 0.844443, -0.156799, 0.530671,
    -0.725304, 0.637024, -0.748915, -0.248928, -0.634094, -0.188099, 0.584087, 0.972778, 0.974165,
    0.222094, -0.041992, 0.595326, -0.701663, -0.549438, -0.060279, -0.998047, -0.262451, -0.191682,
    -0.782292, -0.951477, 0.528851, -0.596315, 0.752319, 0.612134, 0.639567, -0.604919, 0.882803,
    0.200541, 0.433594, -0.936278, -0.039490, 0.349304, 0.940848, -0.121649, 0.318604, -0.115022,
    0.048685, -0.993347, -0.324162, -0.935726, -0.394226, -0.937457, -0.294685, 0.193909, 0.894463,
    -0.437237, 0.104065, -0.861852, -0.165102, -0.486206, -0.980480, -0.139899, 0.139526, -0.024496,
    0.960750, -0.996094, -0.699760, 0.714256, -0.018860, 0.538575, -0.792107, 0.470581, 0.309926,
    -0.943720, 0.349182, 0.525671, -0.772280, 0.561523, -0.793079, 0.268745, 0.567505, 0.697504,
    -0.421131, 0.639221, -0.737871, 0.672553, -0.076660, -0.390769, -0.894942, -0.482666, -0.593469,
    0.191892, 0.796448, 0.439379, -0.896646, 0.123108, 0.337698, -0.703709, -0.879822, -0.654687,
    0.749517, 0.148071, -0.482070, -0.700569, 0.737305, 0.626971, 0.761948, -0.250610, 0.616585,
    0.015339, -0.787231, -0.175877, -0.982000, 0.364624, 0.891483, -0.324585, -0.334167, 0.858029,
    0.438272, -0.297913, 0.949369, 0.258757, 0.184448, 0.105948, -0.901183, 0.969666, -0.261581,
    0.943276, -0.615845, -0.682063, -0.528339, -0.595520, -0.810856, 0.514103, -0.326050, -0.163757,
    0.986118, 0.165527, -0.595927, -0.221907, 0.791504, -0.160374, -0.977354, 0.652405, -0.428837,
    0.641628, -0.829102, -0.634149, -0.486378, -0.687927, -0.093271, -0.995222, -0.295654, 0.988659,
    -0.150144, -0.003357, 0.730821, -0.497396, -0.538818, -0.781913, -0.621260, -0.065674,
    -0.655884, -0.753313, -0.073486, 0.845542, -0.409094, 0.375977, -0.630041, -0.514925, -0.678101,
    0.205571, 0.978634, -0.019531, 0.582841, 0.763684, -0.430054, 0.685084, -0.728464, 0.000000,
    -0.241437, -0.958430, -0.532898, 0.741884, 0.020899, -0.670349, 0.740273, -0.318412, 0.624634,
    -0.738068, -0.539041, 0.481812, -0.965798, -0.034508, -0.257141, 0.495184, 0.805372, 0.549683,
    -0.572524, 0.809558, -0.221008, -0.537181, 0.834652, 0.220825, -0.899741, 0.097826, -0.427368,
    -0.370148, 0.494066, 0.904846, 0.711387, 0.577688, 0.490356, 0.183324, -0.722791, -0.964172,
    0.552815, -0.807753, -0.347351, -0.096050, 0.994565, -0.386047, -0.884907, 0.369536, 0.305115,
    -0.832976, -0.551898, 0.047363, 0.338883, 0.641922, 0.897034, 0.805354, 0.506187, 0.357727,
    -0.040128, 0.998805, -0.570923, 0.466918, -0.602455, 0.811035, 0.139166, -0.983697, 0.633362,
    -0.253765, -0.340498, -0.962891, -0.448806, 0.843929, 0.547791, -0.859087, -0.434649, -0.300110,
    0.287570, 0.957661, 0.047729, 0.379100, 0.795023, 0.780640, 0.154245, -0.987903, -0.103088,
    -0.538067, 0.794791, -0.462524, -0.466455, -0.180966, 0.880371, -0.175736, -0.983766, 0.202576,
    -0.891655, 0.192080, -0.417725, -0.688716, -0.619004, 0.480652, 0.120790, -0.987844, -0.629456,
    -0.075080, 0.983385, 0.910461, 0.147032, -0.960431, -0.849304, 0.732309, 0.671559, 0.152283,
    0.804657, 0.273913, -0.547729, 0.391462, -0.913976, 0.263184, -0.567300, 0.783128, 0.409607,
    0.214917, 0.167182, -0.975952, 0.367428, -0.789995, -0.800537, -0.320112, 0.912727, -0.621399,
    0.659247, -0.647346, -0.501892, 0.222842, -0.696452, -0.950562, -0.697513, -0.576278, 0.521118,
    0.602260, -0.756081, 0.391418, -0.116043, 0.992942, 0.206665, 0.220693, -0.968855, -0.453552,
    0.737991, 0.670137, 0.106812, 0.198419, -0.696590, 0.960999, -0.391866, -0.883543, 0.547668,
    0.082067, -0.996213, 0.330200, -0.806059, 0.491897, -0.377991, -0.992265, 0.120698, 0.029236,
    0.406622, -0.867524, 0.575928, 0.789945, 0.608406, 0.096191, -0.531904, -0.004218, -0.846802,
    0.558298, -0.089427, 0.828125, -0.783155, 0.363828, -0.541382, 0.981706, -0.183228, 0.052673,
    -0.388642, 0.920618, -0.096497, -0.506403, -0.044662, -0.862000, -0.512421, -0.852059,
    -0.204163, 0.559542, 0.339777, 0.803772, 0.527502, -0.846389, 0.137573, -0.184315, -0.952725,
    0.794983, 0.125024, -0.977110, -0.809082, -0.643507, 0.678632, 0.482056, -0.277474, 0.954056,
    0.377380, -0.622333, -0.717603, 0.448914, 0.366846, -0.110794, -0.929382, 0.120402, 0.992596,
    0.131653, -0.982921, 0.103550, -0.152954, -0.058333, -0.997913, -0.428894, 0.132631, 0.979299,
    0.755432, 0.326398, 0.937806, 0.340637, 0.211720, 0.976659, 0.168640, 0.957557, -0.019174,
    -0.287659, -0.016554, 0.999650, 0.780090, -0.271222, 0.827292, -0.875732, 0.850790, -0.448069,
    0.307129, 0.115949, 0.600003, -0.989441, 0.285877, -0.940896, -0.536255, -0.321317, -0.278336,
    -0.942383, -0.422133, 0.754447, 0.765747, 0.669674, -0.741852, -0.051514, 0.213604, -0.949888,
    0.730103, 0.619681, -0.751798, -0.341797, -0.223762, 0.438616, -0.968506, -0.302925, -0.945732,
    0.361877, 0.121093, -0.977151, -0.821838, 0.127125, 0.758710, -0.980774, 0.691682, 0.695626,
    0.270203, 0.241114, 0.967463, -0.303040, -0.829705, 0.422869, 0.402100, -0.484170, -0.741723,
    0.692017, -0.431259, -0.777492, -0.727844, 0.835756, -0.211986, 0.518311, 0.297724, 0.932993,
    0.561829, 0.633475, -0.764920, -0.181091, -0.833849, -0.453546, -0.353027, -0.369433, 0.839581,
    -0.733154, 0.555847, 0.392934, -0.796631, -0.856065, 0.028375, 0.516296, 0.067161, 0.997565,
    0.269409, -0.962279, -0.051749, 0.267456, -0.738893, 0.080065, -0.671204, -0.764325, 0.462240,
    0.507019, 0.148758, 0.751545, 0.974243, -0.153430, -0.318230, 0.986816, -0.439372, 0.776405,
    0.716919 };

static inline void GetVectorLatticePointValue(
    int idx, fltx4 &x, fltx4 &y, fltx4 &z, int idx_x, int idx_y, int idx_z )
{
	int ret_idx = perm_a[idx_x & 0xff];
	ret_idx = perm_b[( idx_y + ret_idx ) & 0xff];
	ret_idx = perm_c[( idx_z + ret_idx ) & 0xff];
	float const *pData = s_randomGradients + ret_idx * 3;
	SubFloat( x, idx ) = pData[0];
	SubFloat( y, idx ) = pData[1];
	SubFloat( z, idx ) = pData[2];
}

FourVectors DNoiseSIMD( const fltx4 &x, const fltx4 &y, const fltx4 &z )
{
	// use magic to convert to integer index
	fltx4 x_idx = AndSIMD( MASK255, AddSIMD( x, Four_MagicNumbers ) );
	fltx4 y_idx = AndSIMD( MASK255, AddSIMD( y, Four_MagicNumbers ) );
	fltx4 z_idx = AndSIMD( MASK255, AddSIMD( z, Four_MagicNumbers ) );

	fltx4 xlattice000 = Four_Zeros, xlattice001 = Four_Zeros, xlattice010 = Four_Zeros,
	      xlattice011 = Four_Zeros;
	fltx4 xlattice100 = Four_Zeros, xlattice101 = Four_Zeros, xlattice110 = Four_Zeros,
	      xlattice111 = Four_Zeros;
	fltx4 ylattice000 = Four_Zeros, ylattice001 = Four_Zeros, ylattice010 = Four_Zeros,
	      ylattice011 = Four_Zeros;
	fltx4 ylattice100 = Four_Zeros, ylattice101 = Four_Zeros, ylattice110 = Four_Zeros,
	      ylattice111 = Four_Zeros;
	fltx4 zlattice000 = Four_Zeros, zlattice001 = Four_Zeros, zlattice010 = Four_Zeros,
	      zlattice011 = Four_Zeros;
	fltx4 zlattice100 = Four_Zeros, zlattice101 = Four_Zeros, zlattice110 = Four_Zeros,
	      zlattice111 = Four_Zeros;

	// FIXME: Converting the input vectors to int indices will cause load-hit-stores (48 bytes)
	//        Converting the indexed noise values back to vectors will cause more (128 bytes)
	//        The noise table could store vectors if we chunked it into 2x2x2 blocks.
	fltx4 xfrac = Four_Zeros, yfrac = Four_Zeros, zfrac = Four_Zeros;
#define DODPASS( i )                                                                               \
	{                                                                                              \
		unsigned int xi = SubInt( x_idx, i );                                                      \
		unsigned int yi = SubInt( y_idx, i );                                                      \
		unsigned int zi = SubInt( z_idx, i );                                                      \
		SubFloat( xfrac, i ) = ( xi & 0xff ) * ( 1.0 / 256.0 );                                    \
		SubFloat( yfrac, i ) = ( yi & 0xff ) * ( 1.0 / 256.0 );                                    \
		SubFloat( zfrac, i ) = ( zi & 0xff ) * ( 1.0 / 256.0 );                                    \
		xi >>= 8;                                                                                  \
		yi >>= 8;                                                                                  \
		zi >>= 8;                                                                                  \
                                                                                                   \
		GetVectorLatticePointValue( i, xlattice000, ylattice000, zlattice000, xi, yi, zi );        \
		GetVectorLatticePointValue( i, xlattice001, ylattice001, zlattice001, xi, yi, zi + 1 );    \
		GetVectorLatticePointValue( i, xlattice010, ylattice010, zlattice010, xi, yi + 1, zi );    \
		GetVectorLatticePointValue(                                                                \
		    i, xlattice011, ylattice011, zlattice011, xi, yi + 1, zi + 1 );                        \
		GetVectorLatticePointValue( i, xlattice100, ylattice100, zlattice100, xi + 1, yi, zi );    \
		GetVectorLatticePointValue(                                                                \
		    i, xlattice101, ylattice101, zlattice101, xi + 1, yi, zi + 1 );                        \
		GetVectorLatticePointValue(                                                                \
		    i, xlattice110, ylattice110, zlattice110, xi + 1, yi + 1, zi );                        \
		GetVectorLatticePointValue(                                                                \
		    i, xlattice111, ylattice111, zlattice111, xi + 1, yi + 1, zi + 1 );                    \
	}

	DODPASS( 0 );
	DODPASS( 1 );
	DODPASS( 2 );
	DODPASS( 3 );

	// now, we have 8 lattice values for each of four points as m128s, and interpolant values for
	// each axis in m128 form in [xyz]frac. Perfom the trilinear interpolation as SIMD ops

	// first, do x interpolation
	fltx4 xl2d00 = AddSIMD( xlattice000, MulSIMD( xfrac, SubSIMD( xlattice100, xlattice000 ) ) );
	fltx4 xl2d01 = AddSIMD( xlattice001, MulSIMD( xfrac, SubSIMD( xlattice101, xlattice001 ) ) );
	fltx4 xl2d10 = AddSIMD( xlattice010, MulSIMD( xfrac, SubSIMD( xlattice110, xlattice010 ) ) );
	fltx4 xl2d11 = AddSIMD( xlattice011, MulSIMD( xfrac, SubSIMD( xlattice111, xlattice011 ) ) );

	// now, do y interpolation
	fltx4 xl1d0 = AddSIMD( xl2d00, MulSIMD( yfrac, SubSIMD( xl2d10, xl2d00 ) ) );
	fltx4 xl1d1 = AddSIMD( xl2d01, MulSIMD( yfrac, SubSIMD( xl2d11, xl2d01 ) ) );

	// final z interpolation
	FourVectors rslt;
	rslt.x = AddSIMD( xl1d0, MulSIMD( zfrac, SubSIMD( xl1d1, xl1d0 ) ) );

	fltx4 yl2d00 = AddSIMD( ylattice000, MulSIMD( xfrac, SubSIMD( ylattice100, ylattice000 ) ) );
	fltx4 yl2d01 = AddSIMD( ylattice001, MulSIMD( xfrac, SubSIMD( ylattice101, ylattice001 ) ) );
	fltx4 yl2d10 = AddSIMD( ylattice010, MulSIMD( xfrac, SubSIMD( ylattice110, ylattice010 ) ) );
	fltx4 yl2d11 = AddSIMD( ylattice011, MulSIMD( xfrac, SubSIMD( ylattice111, ylattice011 ) ) );

	// now, do y interpolation
	fltx4 yl1d0 = AddSIMD( yl2d00, MulSIMD( yfrac, SubSIMD( yl2d10, yl2d00 ) ) );
	fltx4 yl1d1 = AddSIMD( yl2d01, MulSIMD( yfrac, SubSIMD( yl2d11, yl2d01 ) ) );

	// final z interpolation
	rslt.y = AddSIMD( yl1d0, MulSIMD( zfrac, SubSIMD( yl1d1, yl1d0 ) ) );

	fltx4 zl2d00 = AddSIMD( zlattice000, MulSIMD( xfrac, SubSIMD( zlattice100, zlattice000 ) ) );
	fltx4 zl2d01 = AddSIMD( zlattice001, MulSIMD( xfrac, SubSIMD( zlattice101, zlattice001 ) ) );
	fltx4 zl2d10 = AddSIMD( zlattice010, MulSIMD( xfrac, SubSIMD( zlattice110, zlattice010 ) ) );
	fltx4 zl2d11 = AddSIMD( zlattice011, MulSIMD( xfrac, SubSIMD( zlattice111, zlattice011 ) ) );

	// now, do y interpolation
	fltx4 zl1d0 = AddSIMD( zl2d00, MulSIMD( yfrac, SubSIMD( zl2d10, zl2d00 ) ) );
	fltx4 zl1d1 = AddSIMD( zl2d01, MulSIMD( yfrac, SubSIMD( zl2d11, zl2d01 ) ) );

	// final z interpolation
	rslt.z = AddSIMD( zl1d0, MulSIMD( zfrac, SubSIMD( zl1d1, zl1d0 ) ) );

	return rslt;
}

FourVectors DNoiseSIMD( FourVectors const &pos )
{
	return DNoiseSIMD( pos.x, pos.y, pos.z );
}

FourVectors CurlNoiseSIMD( FourVectors const &pos )
{
	FourVectors fl4Comp1 = DNoiseSIMD( pos );
	FourVectors fl4Pos = pos;
	fl4Pos.x = AddSIMD( fl4Pos.x, ReplicateX4( 43.256 ) );
	fl4Pos.y = AddSIMD( fl4Pos.y, ReplicateX4( -67.89 ) );
	fl4Pos.z = AddSIMD( fl4Pos.z, ReplicateX4( 1338.2 ) );
	FourVectors fl4Comp2 = DNoiseSIMD( fl4Pos );
	fl4Pos.x = AddSIMD( fl4Pos.x, ReplicateX4( -129.856 ) );
	fl4Pos.y = AddSIMD( fl4Pos.y, ReplicateX4( -967.23 ) );
	fl4Pos.z = AddSIMD( fl4Pos.z, ReplicateX4( 2338.98 ) );
	FourVectors fl4Comp3 = DNoiseSIMD( fl4Pos );

	// now we have the 3 derivatives of a vector valued field. return the curl of the field.
	FourVectors fl4Ret;
	fl4Ret.x = SubSIMD( fl4Comp3.y, fl4Comp2.z );
	fl4Ret.y = SubSIMD( fl4Comp1.z, fl4Comp3.x );
	fl4Ret.z = SubSIMD( fl4Comp2.x, fl4Comp1.y );
	return fl4Ret;
}
