
/*============================================================================

This C source file is part of the SoftFloat IEC/IEEE Floating-point Arithmetic
Package, Release 2b.

Written by John R. Hauser.  This work was made possible in part by the
International Computer Science Institute, located at Suite 600, 1947 Center
Street, Berkeley, California 94704.  Funding was partially provided by the
National Science Foundation under grant MIP-9311980.  The original version
of this code was written as part of a project to build a fixed-point vector
processor in collaboration with the University of California at Berkeley,
overseen by Profs. Nelson Morgan and John Wawrzynek.  More information
is available through the Web page `http://www.cs.berkeley.edu/~jhauser/
arithmetic/SoftFloat.html'.

THIS SOFTWARE IS DISTRIBUTED AS IS, FOR FREE.  Although reasonable effort has
been made to avoid it, THIS SOFTWARE MAY CONTAIN FAULTS THAT WILL AT TIMES
RESULT IN INCORRECT BEHAVIOR.  USE OF THIS SOFTWARE IS RESTRICTED TO PERSONS
AND ORGANIZATIONS WHO CAN AND WILL TAKE FULL RESPONSIBILITY FOR ALL LOSSES,
COSTS, OR OTHER PROBLEMS THEY INCUR DUE TO THE SOFTWARE, AND WHO FURTHERMORE
EFFECTIVELY INDEMNIFY JOHN HAUSER AND THE INTERNATIONAL COMPUTER SCIENCE
INSTITUTE (possibly via similar legal warning) AGAINST ALL LOSSES, COSTS, OR
OTHER PROBLEMS INCURRED BY THEIR CUSTOMERS AND CLIENTS DUE TO THE SOFTWARE.

Derivative works are acceptable, even for commercial purposes, so long as
(1) the source code for the derivative work includes prominent notice that
the work is derivative, and (2) the source code includes prominent notice with
these four paragraphs for those parts of this code that are retained.

=============================================================================*/

#include "milieu.h"
#include "softfloat.h"

#if 0 // moved to struct float_ctrl
/*----------------------------------------------------------------------------
| Floating-point rounding mode, extended double-precision rounding precision,
| and exception flags.
*----------------------------------------------------------------------------*/
int8 float_exception_flags = 0;
#ifdef SOFTFLOAT_I860
int8 float_exception_flags2 = 0;
#endif
#ifdef FLOATX80
int8 floatx80_rounding_precision = 80;
#endif

int8 float_rounding_mode = float_round_nearest_even;
#ifdef SOFTFLOAT_I860
int8 float_rounding_mode2 = float_round_nearest_even;
#endif

/*----------------------------------------------------------------------------
 | Variables for storing sign, exponent and significand of internal extended
 | double-precision floating-point value for external use.
 *----------------------------------------------------------------------------*/
flag floatx80_internal_sign = 0;
int32 floatx80_internal_exp = 0;
bits64 floatx80_internal_sig0 = 0;
bits64 floatx80_internal_sig1 = 0;
int8 floatx80_internal_precision = 80;
int8 floatx80_internal_mode = float_round_nearest_even;
#endif

void float_init( float_ctrl* c )
{
    c->float_detect_tininess = float_tininess_before_rounding;
    c->float_exception_flags = 0;
    c->float_rounding_mode = float_round_nearest_even;
#ifdef FLOATX80
    c->floatx80_rounding_precision = 80;
    c->floatx80_special_flags = 0;
    c->floatx80_internal_sign = 0;
    c->floatx80_internal_exp = 0;
    c->floatx80_internal_sig0 = 0;
    c->floatx80_internal_sig1 = 0;
    c->floatx80_internal_precision = 80;
    c->floatx80_internal_mode = float_round_nearest_even;
#endif
}

int8 get_float_rounding_mode( float_ctrl* c )
{
    return c->float_rounding_mode;
}
void set_float_rounding_mode( int8 mode, float_ctrl* c )
{
    c->float_rounding_mode = mode;
}

int8 get_float_rounding_precision( float_ctrl* c )
{
    return c->floatx80_rounding_precision;
}
void set_float_rounding_precision( int8 precision, float_ctrl* c )
{
    c->floatx80_rounding_precision = precision;
}

int8 get_float_exception_flags( float_ctrl* c )
{
    return c->float_exception_flags;
}
void set_float_exception_flags( int8 flags, float_ctrl* c )
{
    c->float_exception_flags = flags;
}

int8 get_float_detect_tininess( float_ctrl* c )
{
    return c->float_detect_tininess;
}
void set_float_detect_tininess( int8 mode, float_ctrl* c )
{
    c->float_detect_tininess = mode;
}

void set_special_flags( int8 flags, float_ctrl* c)
{
    c->floatx80_special_flags = flags;
}
int8 fcmp_signed_nan( float_ctrl* c )
{
    return c->floatx80_special_flags&cmp_signed_nan;
}
int8 faddsub_swap_inf( float_ctrl* c )
{
    return c->floatx80_special_flags&addsub_swap_inf;
}
int8 inf_clear_intbit( float_ctrl* c )
{
    return c->floatx80_special_flags&infinity_clear_intbit;
}


/*----------------------------------------------------------------------------
 | Functions for storing sign, exponent and significand of extended
 | double-precision floating-point intermediate result for external use.
 *----------------------------------------------------------------------------*/
static void saveFloatx80Internal( int8 prec, flag zSign, int32 zExp, bits64 zSig0, bits64 zSig1, float_ctrl* c )
{
    c->floatx80_internal_sign = zSign;
    c->floatx80_internal_exp = zExp;
    c->floatx80_internal_sig0 = zSig0;
    c->floatx80_internal_sig1 = zSig1;
    c->floatx80_internal_precision = prec;
    c->floatx80_internal_mode = get_float_rounding_mode( c );
    
}

static void saveFloat64Internal( flag zSign, int16 zExp, bits64 zSig, float_ctrl* c )
{
    c->floatx80_internal_sign = zSign;
    c->floatx80_internal_exp = zExp + 0x3C01;
    c->floatx80_internal_sig0 = zSig<<1;
    c->floatx80_internal_sig1 = 0;
    c->floatx80_internal_precision = 64;
    c->floatx80_internal_mode = get_float_rounding_mode( c );

}

static void saveFloat32Internal( flag zSign, int16 zExp, bits32 zSig, float_ctrl* c )
{
    c->floatx80_internal_sign = zSign;
    c->floatx80_internal_exp = zExp + 0x3F81;
    c->floatx80_internal_sig0 = ( (bits64) zSig )<<33;
    c->floatx80_internal_sig1 = 0;
    c->floatx80_internal_precision = 32;
    c->floatx80_internal_mode = get_float_rounding_mode( c );

}


/*----------------------------------------------------------------------------
 | Functions for returning sign, exponent and significand of extended
 | double-precision floating-point intermediate result for external use.
 *----------------------------------------------------------------------------*/

static void getRoundedFloatInternal( int8 roundingPrecision, flag *pzSign, int32 *pzExp, bits64 *pzSig, float_ctrl* c )
{
    int64 roundIncrement, roundMask, roundBits;
    flag increment;

    flag zSign = c->floatx80_internal_sign;
    int32 zExp = c->floatx80_internal_exp;
    bits64 zSig0 = c->floatx80_internal_sig0;
    bits64 zSig1 = c->floatx80_internal_sig1;
    
    if ( roundingPrecision == 80 ) {
        goto precision80;
    } else if ( roundingPrecision == 64 ) {
        roundIncrement = LIT64( 0x0000000000000400 );
        roundMask = LIT64( 0x00000000000007FF );
    } else if ( roundingPrecision == 32 ) {
        roundIncrement = LIT64( 0x0000008000000000 );
        roundMask = LIT64( 0x000000FFFFFFFFFF );
    } else {
        goto precision80;
    }
    
    zSig0 |= ( zSig1 != 0 );
    if ( c->floatx80_internal_mode != float_round_nearest_even ) {
        if ( c->floatx80_internal_mode == float_round_to_zero ) {
            roundIncrement = 0;
        } else {
            roundIncrement = roundMask;
            if ( zSign ) {
                if ( c->floatx80_internal_mode == float_round_up ) roundIncrement = 0;
            } else {
                if ( c->floatx80_internal_mode == float_round_down ) roundIncrement = 0;
            }
        }
    }
    
    roundBits = zSig0 & roundMask;
    
    zSig0 += roundIncrement;
    if ( zSig0 < (bits64)roundIncrement ) {
        ++zExp;
        zSig0 = LIT64( 0x8000000000000000 );
    }
    roundIncrement = roundMask + 1;
    if ( c->floatx80_internal_mode == float_round_nearest_even && ( roundBits<<1 == roundIncrement ) ) {
        roundMask |= roundIncrement;
    }
    zSig0 &= ~ roundMask;
    if ( zSig0 == 0 ) zExp = 0;
    
    *pzSign = zSign;
    *pzExp = zExp;
    *pzSig = zSig0;
    return;
    
precision80:
    increment = ( (sbits64) zSig1 < 0 );
    if ( c->floatx80_internal_mode != float_round_nearest_even ) {
        if ( c->floatx80_internal_mode == float_round_to_zero ) {
            increment = 0;
        } else {
            if ( zSign ) {
                increment = ( c->floatx80_internal_mode == float_round_down ) && zSig1;
            } else {
                increment = ( c->floatx80_internal_mode == float_round_up ) && zSig1;
            }
        }
    }
    if ( increment ) {
        ++zSig0;
        if ( zSig0 == 0 ) {
            ++zExp;
            zSig0 = LIT64( 0x8000000000000000 );
        } else {
            zSig0 &= ~ ( ( (bits64) ( zSig1<<1 ) == 0 ) & ( c->floatx80_internal_mode == float_round_nearest_even ) );
        }
    } else {
        if ( zSig0 == 0 ) zExp = 0;
    }
    
    *pzSign = zSign;
    *pzExp = zExp;
    *pzSig = zSig0;
    return;
    
}

floatx80 getFloatInternalOverflow( float_ctrl* c )
{
    flag zSign;
    int32 zExp;
    bits64 zSig;
    
    getRoundedFloatInternal( c->floatx80_internal_precision, &zSign, &zExp, &zSig, c );
    
    if (zExp > (0x7fff + 0x6000)) { // catastrophic
        zExp = 0;
    } else {
        zExp -= 0x6000;
    }

    return packFloatx80( zSign, zExp, zSig );
    
}

floatx80 getFloatInternalUnderflow( float_ctrl* c )
{
    flag zSign;
    int32 zExp;
    bits64 zSig;
    
    getRoundedFloatInternal( c->floatx80_internal_precision, &zSign, &zExp, &zSig, c );
    
    if (zExp < (0x0000 - 0x6000)) { // catastrophic
        zExp = 0;
    } else {
        zExp += 0x6000;
    }
    
    return packFloatx80( zSign, zExp, zSig );
    
}

floatx80 getFloatInternalRoundedAll( float_ctrl* c )
{
    flag zSign;
    int32 zExp;
    bits64 zSig, zSig32, zSig64, zSig80;
    
    if (c->floatx80_internal_precision == 80) {
        getRoundedFloatInternal( 80, &zSign, &zExp, &zSig80, c );
        zSig = zSig80;
    } else if (c->floatx80_internal_precision == 64) {
        getRoundedFloatInternal( 80, &zSign, &zExp, &zSig80, c );
        getRoundedFloatInternal( 64, &zSign, &zExp, &zSig64, c );
        zSig = zSig64;
        zSig |= zSig80 & LIT64( 0x00000000000007FF );
    } else {
        getRoundedFloatInternal( 80, &zSign, &zExp, &zSig80, c );
        getRoundedFloatInternal( 64, &zSign, &zExp, &zSig64, c );
        getRoundedFloatInternal( 32, &zSign, &zExp, &zSig32, c );
        zSig = zSig32;
        zSig |= zSig64 & LIT64( 0x000000FFFFFFFFFF );
        zSig |= zSig80 & LIT64( 0x00000000000007FF );
    }

    return packFloatx80( zSign, zExp & 0x7FFF, zSig );

}

floatx80 getFloatInternalRoundedSome( float_ctrl* c )
{
    flag zSign;
    int32 zExp;
    bits64 zSig, zSig32, zSig64, zSig80;
    
    if (c->floatx80_internal_precision == 80) {
        getRoundedFloatInternal( 80, &zSign, &zExp, &zSig80, c );
        zSig = zSig80;
    } else if (c->floatx80_internal_precision == 64) {
        getRoundedFloatInternal( 64, &zSign, &zExp, &zSig64, c );
        zSig80 = c->floatx80_internal_sig0;
        if (zSig64 != (zSig80 & LIT64( 0xFFFFFFFFFFFFF800 ))) {
            zSig80++;
        }
        zSig = zSig64;
        zSig |= zSig80 & LIT64( 0x00000000000007FF );
    } else {
        getRoundedFloatInternal( 32, &zSign, &zExp, &zSig32, c );
        zSig80 = c->floatx80_internal_sig0;
        if (zSig32 != (zSig80 & LIT64( 0xFFFFFF0000000000 ))) {
           zSig80++;
        }
        zSig = zSig32;
        zSig |= zSig80 & LIT64( 0x000000FFFFFFFFFF );
    }
    
    return packFloatx80( zSign, zExp & 0x7FFF, zSig );
    
}

floatx80 getFloatInternalFloatx80( float_ctrl* c )
{
    flag zSign;
    int32 zExp;
    bits64 zSig;
    
    getRoundedFloatInternal( 80, &zSign, &zExp, &zSig, c );
    
    return packFloatx80( zSign, zExp & 0x7FFF, zSig );
    
}

floatx80 getFloatInternalUnrounded( float_ctrl* c )
{
    flag zSign = c->floatx80_internal_sign;
    int32 zExp = c->floatx80_internal_exp;
    bits64 zSig = c->floatx80_internal_sig0;
    
    return packFloatx80( zSign, zExp & 0x7FFF, zSig );
    
}

bits64 getFloatInternalGRS( float_ctrl* c )
{
#if 1
    if (c->floatx80_internal_sig1)
        return 5;
    
    if (c->floatx80_internal_precision == 64 &&
        c->floatx80_internal_sig0 & LIT64( 0x00000000000007FF )) {
        return 1;
    }
    if (c->floatx80_internal_precision == 32 &&
        c->floatx80_internal_sig0 & LIT64( 0x000000FFFFFFFFFF )) {
        return 1;
    }
    
    return 0;
#else
    bits64 roundbits;
    shift64RightJamming(floatx80_internal_sig1, 61, &roundbits);

    return roundbits;
#endif
    
}


/*----------------------------------------------------------------------------
| Functions and definitions to determine:  (1) whether tininess for underflow
| is detected before or after rounding by default, (2) what (if anything)
| happens when exceptions are raised, (3) how signaling NaNs are distinguished
| from quiet NaNs, (4) the default generated quiet NaNs, and (5) how NaNs
| are propagated from function inputs to output.  These details are target-
| specific.
*----------------------------------------------------------------------------*/
#include "softfloat-specialize.h"

/*----------------------------------------------------------------------------
| Takes a 64-bit fixed-point value `absZ' with binary point between bits 6
| and 7, and returns the properly rounded 32-bit integer corresponding to the
| input.  If `zSign' is 1, the input is negated before being converted to an
| integer.  Bit 63 of `absZ' must be zero.  Ordinarily, the fixed-point input
| is simply rounded to an integer, with the inexact exception raised if the
| input cannot be represented exactly as an integer.  However, if the fixed-
| point input is too large, the invalid exception is raised and the largest
| positive or negative integer is returned.
*----------------------------------------------------------------------------*/

static int32 roundAndPackInt32( flag zSign, bits64 absZ, float_ctrl* c )
{
	int8 roundingMode;
	flag roundNearestEven;
	int8 roundIncrement, roundBits;
	int32 z;

	roundingMode = get_float_rounding_mode( c );
	roundNearestEven = ( roundingMode == float_round_nearest_even );
	roundIncrement = 0x40;
	if ( ! roundNearestEven ) {
		if ( roundingMode == float_round_to_zero ) {
			roundIncrement = 0;
		}
		else {
			roundIncrement = 0x7F;
			if ( zSign ) {
				if ( roundingMode == float_round_up ) roundIncrement = 0;
			}
			else {
				if ( roundingMode == float_round_down ) roundIncrement = 0;
			}
		}
	}
	roundBits = absZ & 0x7F;
	absZ = ( absZ + roundIncrement )>>7;
	absZ &= ~ ( ( ( roundBits ^ 0x40 ) == 0 ) & roundNearestEven );
	z = absZ;
	if ( zSign ) z = - z;
    z = (sbits32) z;
	if ( ( absZ>>32 ) || ( z && ( ( z < 0 ) ^ zSign ) ) ) {
		float_raise( float_flag_invalid, c );
		return zSign ? (sbits32) 0x80000000 : 0x7FFFFFFF;
	}
	if ( roundBits ) float_raise( float_flag_inexact, c );
	return z;

}

#ifdef SOFTFLOAT_68K // 30-01-2017: Added for Previous
static int16 roundAndPackInt16( flag zSign, bits64 absZ, float_ctrl* c )
{
    int8 roundingMode;
    flag roundNearestEven;
    int8 roundIncrement, roundBits;
    int16 z;
    
    roundingMode = get_float_rounding_mode( c );
    roundNearestEven = ( roundingMode == float_round_nearest_even );
    roundIncrement = 0x40;
    if ( ! roundNearestEven ) {
        if ( roundingMode == float_round_to_zero ) {
            roundIncrement = 0;
        }
        else {
            roundIncrement = 0x7F;
            if ( zSign ) {
                if ( roundingMode == float_round_up ) roundIncrement = 0;
            }
            else {
                if ( roundingMode == float_round_down ) roundIncrement = 0;
            }
        }
    }
    roundBits = absZ & 0x7F;
    absZ = ( absZ + roundIncrement )>>7;
    absZ &= ~ ( ( ( roundBits ^ 0x40 ) == 0 ) & roundNearestEven );
    z = absZ;
    if ( zSign ) z = - z;
    z = (sbits16) z;
    if ( ( absZ>>16 ) || ( z && ( ( z < 0 ) ^ zSign ) ) ) {
        float_raise( float_flag_invalid, c );
        return zSign ? (sbits16) 0x8000 : 0x7FFF;
    }
    if ( roundBits ) float_raise( float_flag_inexact, c );
    return z;
    
}

static int8 roundAndPackInt8( flag zSign, bits64 absZ, float_ctrl* c )
{
    int8 roundingMode;
    flag roundNearestEven;
    int8 roundIncrement, roundBits;
    int8 z;
    
    roundingMode = get_float_rounding_mode( c );
    roundNearestEven = ( roundingMode == float_round_nearest_even );
    roundIncrement = 0x40;
    if ( ! roundNearestEven ) {
        if ( roundingMode == float_round_to_zero ) {
            roundIncrement = 0;
        }
        else {
            roundIncrement = 0x7F;
            if ( zSign ) {
                if ( roundingMode == float_round_up ) roundIncrement = 0;
            }
            else {
                if ( roundingMode == float_round_down ) roundIncrement = 0;
            }
        }
    }
    roundBits = absZ & 0x7F;
    absZ = ( absZ + roundIncrement )>>7;
    absZ &= ~ ( ( ( roundBits ^ 0x40 ) == 0 ) & roundNearestEven );
    z = absZ;
    if ( zSign ) z = - z;
    z = (sbits8) z;
    if ( ( absZ>>8 ) || ( z && ( ( z < 0 ) ^ zSign ) ) ) {
        float_raise( float_flag_invalid, c );
        return zSign ? (sbits8) 0x80 : 0x7F;
    }
    if ( roundBits ) float_raise( float_flag_inexact, c );
    return z;
    
}
#endif // End of addition for Previous

#ifdef SOFTFLOAT_I860 // 29-04-2017: Added for Previous
static int32 roundAndPackInt32_2( flag zSign, bits64 absZ, float_ctrl* c )
{
    int8 roundingMode;
    flag roundNearestEven;
    int8 roundIncrement, roundBits;
    int32 z;
    
    roundingMode = get_float_rounding_mode( c );
    roundNearestEven = ( roundingMode == float_round_nearest_even );
    roundIncrement = 0x40;
    if ( ! roundNearestEven ) {
        if ( roundingMode == float_round_to_zero ) {
            roundIncrement = 0;
        }
        else {
            roundIncrement = 0x7F;
            if ( zSign ) {
                if ( roundingMode == float_round_up ) roundIncrement = 0;
            }
            else {
                if ( roundingMode == float_round_down ) roundIncrement = 0;
            }
        }
    }
    roundBits = absZ & 0x7F;
    absZ = ( absZ + roundIncrement )>>7;
    absZ &= ~ ( ( ( roundBits ^ 0x40 ) == 0 ) & roundNearestEven );
    z = absZ;
    if ( zSign ) z = - z;
    z = (sbits32) z;
    if ( ( absZ>>32 ) || ( z && ( ( z < 0 ) ^ zSign ) ) ) {
        float_raise( float_flag_invalid, c );
        return zSign ? (sbits32) 0x80000000 : 0x7FFFFFFF;
    }
    if ( roundBits ) float_raise( float_flag_inexact, c );
    return z;
    
}
#endif // End of addition for Previous

/*----------------------------------------------------------------------------
| Takes the 128-bit fixed-point value formed by concatenating `absZ0' and
| `absZ1', with binary point between bits 63 and 64 (between the input words),
| and returns the properly rounded 64-bit integer corresponding to the input.
| If `zSign' is 1, the input is negated before being converted to an integer.
| Ordinarily, the fixed-point input is simply rounded to an integer, with
| the inexact exception raised if the input cannot be represented exactly as
| an integer.  However, if the fixed-point input is too large, the invalid
| exception is raised and the largest positive or negative integer is
| returned.
*----------------------------------------------------------------------------*/

static int64 roundAndPackInt64( flag zSign, bits64 absZ0, bits64 absZ1, float_ctrl* c )
{
	int8 roundingMode;
	flag roundNearestEven, increment;
	int64 z;

	roundingMode = get_float_rounding_mode( c );
	roundNearestEven = ( roundingMode == float_round_nearest_even );
	increment = ( (sbits64) absZ1 < 0 );
	if ( ! roundNearestEven ) {
		if ( roundingMode == float_round_to_zero ) {
			increment = 0;
		}
		else {
			if ( zSign ) {
				increment = ( roundingMode == float_round_down ) && absZ1;
			}
			else {
				increment = ( roundingMode == float_round_up ) && absZ1;
			}
		}
	}
	if ( increment ) {
		++absZ0;
		if ( absZ0 == 0 ) goto overflow;
		absZ0 &= ~ ( ( (bits64) ( absZ1<<1 ) == 0 ) & roundNearestEven );
	}
	z = absZ0;
	if ( zSign ) z = - z;
    z = (sbits64) z;
	if ( z && ( ( z < 0 ) ^ zSign ) ) {
	overflow:
		float_raise( float_flag_invalid, c );
		return
				zSign ? (sbits64) LIT64( 0x8000000000000000 )
			: LIT64( 0x7FFFFFFFFFFFFFFF );
	}
	if ( absZ1 ) float_raise( float_flag_inexact, c );
	return z;

}

/*----------------------------------------------------------------------------
| Returns the fraction bits of the single-precision floating-point value `a'.
*----------------------------------------------------------------------------*/

INLINE bits32 extractFloat32Frac( float32 a )
{
	return a & 0x007FFFFF;

}

/*----------------------------------------------------------------------------
| Returns the exponent bits of the single-precision floating-point value `a'.
*----------------------------------------------------------------------------*/

INLINE int16 extractFloat32Exp( float32 a )
{
	return ( a>>23 ) & 0xFF;

}

/*----------------------------------------------------------------------------
| Returns the sign bit of the single-precision floating-point value `a'.
*----------------------------------------------------------------------------*/

INLINE flag extractFloat32Sign( float32 a )
{
	return a>>31;

}

/*----------------------------------------------------------------------------
| Normalizes the subnormal single-precision floating-point value represented
| by the denormalized significand `aSig'.  The normalized exponent and
| significand are stored at the locations pointed to by `zExpPtr' and
| `zSigPtr', respectively.
*----------------------------------------------------------------------------*/

static void
	normalizeFloat32Subnormal( bits32 aSig, int16 *zExpPtr, bits32 *zSigPtr )
{
	int8 shiftCount;

	shiftCount = countLeadingZeros32( aSig ) - 8;
	*zSigPtr = aSig<<shiftCount;
	*zExpPtr = 1 - shiftCount;

}

/*----------------------------------------------------------------------------
| Packs the sign `zSign', exponent `zExp', and significand `zSig' into a
| single-precision floating-point value, returning the result.  After being
| shifted into the proper positions, the three fields are simply added
| together to form the result.  This means that any integer portion of `zSig'
| will be added into the exponent.  Since a properly normalized significand
| will have an integer portion equal to 1, the `zExp' input should be 1 less
| than the desired result exponent whenever `zSig' is a complete, normalized
| significand.
*----------------------------------------------------------------------------*/

INLINE float32 packFloat32( flag zSign, int16 zExp, bits32 zSig )
{
	return ( ( (bits32) zSign )<<31 ) + ( ( (bits32) zExp )<<23 ) + zSig;

}

/*----------------------------------------------------------------------------
| Takes an abstract floating-point value having sign `zSign', exponent `zExp',
| and significand `zSig', and returns the proper single-precision floating-
| point value corresponding to the abstract input.  Ordinarily, the abstract
| value is simply rounded and packed into the single-precision format, with
| the inexact exception raised if the abstract input cannot be represented
| exactly.  However, if the abstract value is too large, the overflow and
| inexact exceptions are raised and an infinity or maximal finite value is
| returned.  If the abstract value is too small, the input value is rounded to
| a subnormal number, and the underflow and inexact exceptions are raised if
| the abstract input cannot be represented exactly as a subnormal single-
| precision floating-point number.
|     The input significand `zSig' has its binary point between bits 30
| and 29, which is 7 bits to the left of the usual location.  This shifted
| significand must be normalized or smaller.  If `zSig' is not normalized,
| `zExp' must be 0; in that case, the result returned is a subnormal number,
| and it must not require rounding.  In the usual case that `zSig' is
| normalized, `zExp' must be 1 less than the ``true'' floating-point exponent.
| The handling of underflow and overflow follows the IEC/IEEE Standard for
| Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

static float32 roundAndPackFloat32( flag zSign, int16 zExp, bits32 zSig, float_ctrl* c )
{
	int8 roundingMode;
	flag roundNearestEven;
	int8 roundIncrement, roundBits;
	flag isTiny;

	roundingMode = get_float_rounding_mode( c );
	roundNearestEven = ( roundingMode == float_round_nearest_even );
	roundIncrement = 0x40;
	if ( ! roundNearestEven ) {
		if ( roundingMode == float_round_to_zero ) {
			roundIncrement = 0;
		}
		else {
			roundIncrement = 0x7F;
			if ( zSign ) {
				if ( roundingMode == float_round_up ) roundIncrement = 0;
			}
			else {
				if ( roundingMode == float_round_down ) roundIncrement = 0;
			}
		}
	}
	roundBits = zSig & 0x7F;
	if ( 0xFD <= (bits16) zExp ) {
		if (    ( 0xFD < zExp )
				|| (    ( zExp == 0xFD )
					&& ( (sbits32) ( zSig + roundIncrement ) < 0 ) )
			) {
#ifdef SOFTFLOAT_68K
            float_raise( float_flag_overflow, c );
            saveFloat32Internal( zSign, zExp, zSig, c );
            if ( roundBits ) float_raise( float_flag_inexact, c );
#else
			float_raise( float_flag_overflow | float_flag_inexact );
#endif
			return packFloat32( zSign, 0xFF, 0 ) - ( roundIncrement == 0 );
		}
		if ( zExp < 0 ) {
			isTiny =
					( get_float_detect_tininess(c) == float_tininess_before_rounding )
				|| ( zExp < -1 )
				|| ( zSig + roundIncrement < 0x80000000 );
#ifdef SOFTFLOAT_68K
            if ( isTiny ) {
                float_raise( float_flag_underflow, c );
                saveFloat32Internal( zSign, zExp, zSig, c );
            }
#endif
			shift32RightJamming( zSig, - zExp, &zSig );
			zExp = 0;
			roundBits = zSig & 0x7F;
#ifndef SOFTFLOAT_68K
			if ( isTiny && roundBits ) float_raise( float_flag_underflow );
#endif
		}
	}
	if ( roundBits ) float_raise( float_flag_inexact, c );
	zSig = ( zSig + roundIncrement )>>7;
	zSig &= ~ ( ( ( roundBits ^ 0x40 ) == 0 ) & roundNearestEven );
	if ( zSig == 0 ) zExp = 0;
	return packFloat32( zSign, zExp, zSig );

}

#ifdef SOFTFLOAT_I860 // 29-04-2017: Added for Previous
static float32 roundAndPackFloat32_2( flag zSign, int16 zExp, bits32 zSig, float_ctrl* c )
{
    int8 roundingMode;
    flag roundNearestEven;
    int8 roundIncrement, roundBits;
    flag isTiny;
    
    roundingMode = get_float_rounding_mode( c );
    roundNearestEven = ( roundingMode == float_round_nearest_even );
    roundIncrement = 0x40;
    if ( ! roundNearestEven ) {
        if ( roundingMode == float_round_to_zero ) {
            roundIncrement = 0;
        }
        else {
            roundIncrement = 0x7F;
            if ( zSign ) {
                if ( roundingMode == float_round_up ) roundIncrement = 0;
            }
            else {
                if ( roundingMode == float_round_down ) roundIncrement = 0;
            }
        }
    }
    roundBits = zSig & 0x7F;
    if ( 0xFD <= (bits16) zExp ) {
        if (    ( 0xFD < zExp )
            || (    ( zExp == 0xFD )
                && ( (sbits32) ( zSig + roundIncrement ) < 0 ) )
            ) {
            float_raise( float_flag_overflow | float_flag_inexact, c );
            return packFloat32( zSign, 0xFF, 0 ) - ( roundIncrement == 0 );
        }
        if ( zExp < 0 ) {
            isTiny =
#ifndef SOFTFLOAT_I860
            ( get_float_detect_tininess(c) == float_tininess_before_rounding ) ||
#endif
            ( zExp < -1 ) ||
            ( zSig + roundIncrement < 0x80000000 );
            shift32RightJamming( zSig, - zExp, &zSig );
            zExp = 0;
            roundBits = zSig & 0x7F;
            if ( isTiny && roundBits ) float_raise( float_flag_underflow, c );
        }
    }
    if ( roundBits ) float_raise( float_flag_inexact, c );
    zSig = ( zSig + roundIncrement )>>7;
    zSig &= ~ ( ( ( roundBits ^ 0x40 ) == 0 ) & roundNearestEven );
    if ( zSig == 0 ) zExp = 0;
    return packFloat32( zSign, zExp, zSig );
    
}
#endif // end of addition for Previous

/*----------------------------------------------------------------------------
| Takes an abstract floating-point value having sign `zSign', exponent `zExp',
| and significand `zSig', and returns the proper single-precision floating-
| point value corresponding to the abstract input.  This routine is just like
| `roundAndPackFloat32' except that `zSig' does not have to be normalized.
| Bit 31 of `zSig' must be zero, and `zExp' must be 1 less than the ``true''
| floating-point exponent.
*----------------------------------------------------------------------------*/

static float32
	normalizeRoundAndPackFloat32( flag zSign, int16 zExp, bits32 zSig, float_ctrl* c )
{
	int8 shiftCount;

	shiftCount = countLeadingZeros32( zSig ) - 1;
	return roundAndPackFloat32( zSign, zExp - shiftCount, zSig<<shiftCount, c );

}

/*----------------------------------------------------------------------------
| Returns the fraction bits of the double-precision floating-point value `a'.
*----------------------------------------------------------------------------*/

INLINE bits64 extractFloat64Frac( float64 a )
{
	return a & LIT64( 0x000FFFFFFFFFFFFF );

}

/*----------------------------------------------------------------------------
| Returns the exponent bits of the double-precision floating-point value `a'.
*----------------------------------------------------------------------------*/

INLINE int16 extractFloat64Exp( float64 a )
{
	return ( a>>52 ) & 0x7FF;

}

/*----------------------------------------------------------------------------
| Returns the sign bit of the double-precision floating-point value `a'.
*----------------------------------------------------------------------------*/

INLINE flag extractFloat64Sign( float64 a )
{
	return a>>63;

}

/*----------------------------------------------------------------------------
| Normalizes the subnormal double-precision floating-point value represented
| by the denormalized significand `aSig'.  The normalized exponent and
| significand are stored at the locations pointed to by `zExpPtr' and
| `zSigPtr', respectively.
*----------------------------------------------------------------------------*/

static void
	normalizeFloat64Subnormal( bits64 aSig, int16 *zExpPtr, bits64 *zSigPtr )
{
	int8 shiftCount;

	shiftCount = countLeadingZeros64( aSig ) - 11;
	*zSigPtr = aSig<<shiftCount;
	*zExpPtr = 1 - shiftCount;

}

/*----------------------------------------------------------------------------
| Packs the sign `zSign', exponent `zExp', and significand `zSig' into a
| double-precision floating-point value, returning the result.  After being
| shifted into the proper positions, the three fields are simply added
| together to form the result.  This means that any integer portion of `zSig'
| will be added into the exponent.  Since a properly normalized significand
| will have an integer portion equal to 1, the `zExp' input should be 1 less
| than the desired result exponent whenever `zSig' is a complete, normalized
| significand.
*----------------------------------------------------------------------------*/

INLINE float64 packFloat64( flag zSign, int16 zExp, bits64 zSig )
{
	return ( ( (bits64) zSign )<<63 ) + ( ( (bits64) zExp )<<52 ) + zSig;

}

/*----------------------------------------------------------------------------
| Takes an abstract floating-point value having sign `zSign', exponent `zExp',
| and significand `zSig', and returns the proper double-precision floating-
| point value corresponding to the abstract input.  Ordinarily, the abstract
| value is simply rounded and packed into the double-precision format, with
| the inexact exception raised if the abstract input cannot be represented
| exactly.  However, if the abstract value is too large, the overflow and
| inexact exceptions are raised and an infinity or maximal finite value is
| returned.  If the abstract value is too small, the input value is rounded
| to a subnormal number, and the underflow and inexact exceptions are raised
| if the abstract input cannot be represented exactly as a subnormal double-
| precision floating-point number.
|     The input significand `zSig' has its binary point between bits 62
| and 61, which is 10 bits to the left of the usual location.  This shifted
| significand must be normalized or smaller.  If `zSig' is not normalized,
| `zExp' must be 0; in that case, the result returned is a subnormal number,
| and it must not require rounding.  In the usual case that `zSig' is
| normalized, `zExp' must be 1 less than the ``true'' floating-point exponent.
| The handling of underflow and overflow follows the IEC/IEEE Standard for
| Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

static float64 roundAndPackFloat64( flag zSign, int16 zExp, bits64 zSig, float_ctrl* c )
{
	int8 roundingMode;
	flag roundNearestEven;
	int16 roundIncrement, roundBits;
	flag isTiny;

	roundingMode = get_float_rounding_mode( c );
	roundNearestEven = ( roundingMode == float_round_nearest_even );
	roundIncrement = 0x200;
	if ( ! roundNearestEven ) {
		if ( roundingMode == float_round_to_zero ) {
			roundIncrement = 0;
		}
		else {
			roundIncrement = 0x3FF;
			if ( zSign ) {
				if ( roundingMode == float_round_up ) roundIncrement = 0;
			}
			else {
				if ( roundingMode == float_round_down ) roundIncrement = 0;
			}
		}
	}
	roundBits = zSig & 0x3FF;
	if ( 0x7FD <= (bits16) zExp ) {
		if (    ( 0x7FD < zExp )
				|| (    ( zExp == 0x7FD )
					&& ( (sbits64) ( zSig + roundIncrement ) < 0 ) )
			) {
#ifdef SOFTFLOAT_68K
			float_raise( float_flag_overflow, c );
            saveFloat64Internal( zSign, zExp, zSig, c );
            if ( roundBits ) float_raise( float_flag_inexact, c );
#else
            float_raise( float_flag_overflow | float_flag_inexact );
#endif
			return packFloat64( zSign, 0x7FF, 0 ) - ( roundIncrement == 0 );
		}
		if ( zExp < 0 ) {
			isTiny =
					( get_float_detect_tininess(c) == float_tininess_before_rounding )
				|| ( zExp < -1 )
				|| ( zSig + roundIncrement < LIT64( 0x8000000000000000 ) );
#ifdef SOFTFLOAT_68K
            if ( isTiny ) {
                float_raise( float_flag_underflow, c );
                saveFloat64Internal( zSign, zExp, zSig, c );
            }
#endif
			shift64RightJamming( zSig, - zExp, &zSig );
			zExp = 0;
			roundBits = zSig & 0x3FF;
#ifndef SOFTFLOAT_68K
			if ( isTiny && roundBits ) float_raise( float_flag_underflow );
#endif
		}
	}
	if ( roundBits ) float_raise( float_flag_inexact, c );
	zSig = ( zSig + roundIncrement )>>10;
	zSig &= ~ ( ( ( roundBits ^ 0x200 ) == 0 ) & roundNearestEven );
	if ( zSig == 0 ) zExp = 0;
	return packFloat64( zSign, zExp, zSig );

}

#ifdef SOFTFLOAT_I860 // 29-04-2017: Added for Previous
static float64 roundAndPackFloat64_2( flag zSign, int16 zExp, bits64 zSig, float_ctrl* c )
{
    int8 roundingMode;
    flag roundNearestEven;
    int16 roundIncrement, roundBits;
    flag isTiny;
    
    roundingMode = get_float_rounding_mode( c );
    roundNearestEven = ( roundingMode == float_round_nearest_even );
    roundIncrement = 0x200;
    if ( ! roundNearestEven ) {
        if ( roundingMode == float_round_to_zero ) {
            roundIncrement = 0;
        }
        else {
            roundIncrement = 0x3FF;
            if ( zSign ) {
                if ( roundingMode == float_round_up ) roundIncrement = 0;
            }
            else {
                if ( roundingMode == float_round_down ) roundIncrement = 0;
            }
        }
    }
    roundBits = zSig & 0x3FF;
    if ( 0x7FD <= (bits16) zExp ) {
        if (    ( 0x7FD < zExp )
            || (    ( zExp == 0x7FD )
                && ( (sbits64) ( zSig + roundIncrement ) < 0 ) )
            ) {
            float_raise( float_flag_overflow | float_flag_inexact, c );
            return packFloat64( zSign, 0x7FF, 0 ) - ( roundIncrement == 0 );
        }
        if ( zExp < 0 ) {
            isTiny =
#ifndef SOFTFLOAT_I860
            ( get_float_detect_tininess(c) == float_tininess_before_rounding ) ||
#endif
            ( zExp < -1 ) ||
            ( zSig + roundIncrement < LIT64( 0x8000000000000000 ) );
            shift64RightJamming( zSig, - zExp, &zSig );
            zExp = 0;
            roundBits = zSig & 0x3FF;
            if ( isTiny && roundBits ) float_raise( float_flag_underflow, c );
        }
    }
    if ( roundBits ) float_raise( float_flag_inexact, c );
    zSig = ( zSig + roundIncrement )>>10;
    zSig &= ~ ( ( ( roundBits ^ 0x200 ) == 0 ) & roundNearestEven );
    if ( zSig == 0 ) zExp = 0;
    return packFloat64( zSign, zExp, zSig );
    
}
#endif // End of addition for Previous

/*----------------------------------------------------------------------------
| Takes an abstract floating-point value having sign `zSign', exponent `zExp',
| and significand `zSig', and returns the proper double-precision floating-
| point value corresponding to the abstract input.  This routine is just like
| `roundAndPackFloat64' except that `zSig' does not have to be normalized.
| Bit 63 of `zSig' must be zero, and `zExp' must be 1 less than the ``true''
| floating-point exponent.
*----------------------------------------------------------------------------*/

static float64
	normalizeRoundAndPackFloat64( flag zSign, int16 zExp, bits64 zSig, float_ctrl* c )
{
	int8 shiftCount;

	shiftCount = countLeadingZeros64( zSig ) - 1;
	return roundAndPackFloat64( zSign, zExp - shiftCount, zSig<<shiftCount, c );

}

#ifdef FLOATX80

/*----------------------------------------------------------------------------
| Normalizes the subnormal extended double-precision floating-point value
| represented by the denormalized significand `aSig'.  The normalized exponent
| and significand are stored at the locations pointed to by `zExpPtr' and
| `zSigPtr', respectively.
*----------------------------------------------------------------------------*/

/* static */ void
	normalizeFloatx80Subnormal( bits64 aSig, int32 *zExpPtr, bits64 *zSigPtr )
{
	int8 shiftCount;

	shiftCount = countLeadingZeros64( aSig );
	*zSigPtr = aSig<<shiftCount;
#ifdef SOFTFLOAT_68K
	*zExpPtr = -shiftCount;
#else
	*zExpPtr = 1 - shiftCount;
#endif

}

/*----------------------------------------------------------------------------
 | Packs the sign `zSign', exponent `zExp', and significand `zSig' into an
 | extended double-precision floating-point value, returning the result.
 *----------------------------------------------------------------------------*/

floatx80 packFloatx80( flag zSign, int32 zExp, bits64 zSig )
{
    floatx80 z;
    
    z.low = zSig;
    z.high = ( ( (bits16) zSign )<<15 ) + zExp;
    return z;
    
}

/*----------------------------------------------------------------------------
| Takes an abstract floating-point value having sign `zSign', exponent `zExp',
| and extended significand formed by the concatenation of `zSig0' and `zSig1',
| and returns the proper extended double-precision floating-point value
| corresponding to the abstract input.  Ordinarily, the abstract value is
| rounded and packed into the extended double-precision format, with the
| inexact exception raised if the abstract input cannot be represented
| exactly.  However, if the abstract value is too large, the overflow and
| inexact exceptions are raised and an infinity or maximal finite value is
| returned.  If the abstract value is too small, the input value is rounded to
| a subnormal number, and the underflow and inexact exceptions are raised if
| the abstract input cannot be represented exactly as a subnormal extended
| double-precision floating-point number.
|     If `roundingPrecision' is 32 or 64, the result is rounded to the same
| number of bits as single or double precision, respectively.  Otherwise, the
| result is rounded to the full precision of the extended double-precision
| format.
|     The input significand must be normalized or smaller.  If the input
| significand is not normalized, `zExp' must be 0; in that case, the result
| returned is a subnormal number, and it must not require rounding.  The
| handling of underflow and overflow follows the IEC/IEEE Standard for Binary
| Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

// roundAndPackFloatx80 is now also used in fyl2x.c
#ifndef SOFTFLOAT_68K
/* static */ floatx80
	roundAndPackFloatx80(
		int8 roundingPrecision, flag zSign, int32 zExp, bits64 zSig0, bits64 zSig1
	)
{
	int8 roundingMode;
	flag roundNearestEven, increment, isTiny;
	int64 roundIncrement, roundMask, roundBits;

	roundingMode = float_rounding_mode;
	roundNearestEven = ( roundingMode == float_round_nearest_even );
	if ( roundingPrecision == 80 ) goto precision80;
	if ( roundingPrecision == 64 ) {
		roundIncrement = LIT64( 0x0000000000000400 );
		roundMask = LIT64( 0x00000000000007FF );
	}
	else if ( roundingPrecision == 32 ) {
		roundIncrement = LIT64( 0x0000008000000000 );
		roundMask = LIT64( 0x000000FFFFFFFFFF );
	}
	else {
		goto precision80;
	}
	zSig0 |= ( zSig1 != 0 );
	if ( ! roundNearestEven ) {
		if ( roundingMode == float_round_to_zero ) {
			roundIncrement = 0;
		}
		else {
			roundIncrement = roundMask;
			if ( zSign ) {
				if ( roundingMode == float_round_up ) roundIncrement = 0;
			}
			else {
				if ( roundingMode == float_round_down ) roundIncrement = 0;
			}
		}
	}
	roundBits = zSig0 & roundMask;
#ifdef SOFTFLOAT_68K
	if ( 0x7FFE <= (bits32) zExp ) {
#else
	if ( 0x7FFD <= (bits32) ( zExp - 1 ) ) {
#endif
		if (    ( 0x7FFE < zExp )
				|| ( ( zExp == 0x7FFE ) && ( zSig0 + roundIncrement < zSig0 ) )
			) {
			goto overflow;
		}
#ifdef SOFTFLOAT_68K
        if ( zExp < 0 ) {
#else
		if ( zExp <= 0 ) {
#endif
			isTiny =
					( get_float_detect_tininess(c) == float_tininess_before_rounding )
#ifdef SOFTFLOAT_68K
				|| ( zExp < -1 )
#else
				|| ( zExp < 0 )
#endif
				|| ( zSig0 <= zSig0 + roundIncrement );
#ifdef SOFTFLOAT_68K
            if ( isTiny ) {
                float_raise( float_flag_underflow );
                saveFloatx80Internal( zSign, zExp, zSig0, zSig1 );
            }
			shift64RightJamming( zSig0, -zExp, &zSig0 );
#else
			shift64RightJamming( zSig0, 1 - zExp, &zSig0 );
#endif
			zExp = 0;
			roundBits = zSig0 & roundMask;
#ifndef SOFTFLOAT_68K
			if ( isTiny && roundBits ) float_raise( float_flag_underflow );
#endif
			if ( roundBits ) float_raise( float_flag_inexact, c );
			zSig0 += roundIncrement;
#ifndef SOFTFLOAT_68K
			if ( (sbits64) zSig0 < 0 ) zExp = 1;
#endif
			roundIncrement = roundMask + 1;
			if ( roundNearestEven && ( roundBits<<1 == roundIncrement ) ) {
				roundMask |= roundIncrement;
			}
			zSig0 &= ~ roundMask;
			return packFloatx80( zSign, zExp, zSig0 );
		}
	}
	if ( roundBits ) float_raise( float_flag_inexact, c );
	zSig0 += roundIncrement;
	if ( zSig0 < roundIncrement ) {
		++zExp;
		zSig0 = LIT64( 0x8000000000000000 );
	}
	roundIncrement = roundMask + 1;
	if ( roundNearestEven && ( roundBits<<1 == roundIncrement ) ) {
		roundMask |= roundIncrement;
	}
	zSig0 &= ~ roundMask;
	if ( zSig0 == 0 ) zExp = 0;
	return packFloatx80( zSign, zExp, zSig0 );
	precision80:
	increment = ( (sbits64) zSig1 < 0 );
	if ( ! roundNearestEven ) {
		if ( roundingMode == float_round_to_zero ) {
			increment = 0;
		}
		else {
			if ( zSign ) {
				increment = ( roundingMode == float_round_down ) && zSig1;
			}
			else {
				increment = ( roundingMode == float_round_up ) && zSig1;
			}
		}
	}
#ifdef SOFTFLOAT_68K
	if ( 0x7FFE <= (bits32) zExp ) {
#else
	if ( 0x7FFD <= (bits32) ( zExp - 1 ) ) {
#endif
		if (    ( 0x7FFE < zExp )
				|| (    ( zExp == 0x7FFE )
					&& ( zSig0 == LIT64( 0xFFFFFFFFFFFFFFFF ) )
					&& increment
				)
			) {
			roundMask = 0;
	overflow:
#ifndef SOFTFLOAT_68K
			float_raise( float_flag_overflow | float_flag_inexact );
#else
            float_raise( float_flag_overflow );
            saveFloatx80Internal( zSign, zExp, zSig0, zSig1 );
            if ( ( zSig0 & roundMask ) || zSig1 ) float_raise( float_flag_inexact );
#endif
			if (    ( roundingMode == float_round_to_zero )
					|| ( zSign && ( roundingMode == float_round_up ) )
					|| ( ! zSign && ( roundingMode == float_round_down ) )
				) {
				return packFloatx80( zSign, 0x7FFE, ~ roundMask );
			}
			return packFloatx80( zSign, floatx80_default_infinity_high, floatx80_default_infinity_low );
		}
#ifdef SOFTFLOAT_68K
		if ( zExp < 0 ) {
#else
		if ( zExp <= 0 ) {
#endif
			isTiny =
					( get_float_detect_tininess(c) == float_tininess_before_rounding )
#ifdef SOFTFLOAT_68K
				|| ( zExp < -1 )
#else
				|| ( zExp < 0 )
#endif
				|| ! increment
				|| ( zSig0 < LIT64( 0xFFFFFFFFFFFFFFFF ) );
#ifdef SOFTFLOAT_68K
            if ( isTiny ) {
                float_raise( float_flag_underflow );
                saveFloatx80Internal( zSign, zExp, zSig0, zSig1 );
            }
            shift64ExtraRightJamming( zSig0, zSig1, -zExp, &zSig0, &zSig1 );
#else
			shift64ExtraRightJamming( zSig0, zSig1, 1 - zExp, &zSig0, &zSig1 );
#endif
			zExp = 0;
#ifndef SOFTFLOAT_68K
			if ( isTiny && zSig1 ) float_raise( float_flag_underflow );
#endif
			if ( zSig1 ) float_raise( float_flag_inexact, c );
			if ( roundNearestEven ) {
				increment = ( (sbits64) zSig1 < 0 );
			}
			else {
				if ( zSign ) {
					increment = ( roundingMode == float_round_down ) && zSig1;
				}
				else {
					increment = ( roundingMode == float_round_up ) && zSig1;
				}
			}
			if ( increment ) {
				++zSig0;
				zSig0 &=
					~ ( ( (bits64) ( zSig1<<1 ) == 0 ) & roundNearestEven );
#ifndef SOFTFLOAT_68K
				if ( (sbits64) zSig0 < 0 ) zExp = 1;
#endif
			}
			return packFloatx80( zSign, zExp, zSig0 );
		}
	}
	if ( zSig1 ) float_raise( float_flag_inexact, c );
	if ( increment ) {
		++zSig0;
		if ( zSig0 == 0 ) {
			++zExp;
			zSig0 = LIT64( 0x8000000000000000 );
		}
		else {
			zSig0 &= ~ ( ( (bits64) ( zSig1<<1 ) == 0 ) & roundNearestEven );
		}
	}
	else {
		if ( zSig0 == 0 ) zExp = 0;
	}
	return packFloatx80( zSign, zExp, zSig0 );

}
#else // SOFTFLOAT_68K
floatx80 roundAndPackFloatx80( int8 roundingPrecision, flag zSign, int32 zExp, bits64 zSig0, bits64 zSig1, float_ctrl* c )
{
    int8 roundingMode;
    flag roundNearestEven, increment;
    int64 roundIncrement, roundMask, roundBits;
    int32 expOffset;
    
    roundingMode = get_float_rounding_mode( c );
    roundNearestEven = ( roundingMode == float_round_nearest_even );
    if ( roundingPrecision == 80 ) goto precision80;
    if ( roundingPrecision == 64 ) {
        roundIncrement = LIT64( 0x0000000000000400 );
        roundMask = LIT64( 0x00000000000007FF );
        expOffset = 0x3C00;
    } else if ( roundingPrecision == 32 ) {
        roundIncrement = LIT64( 0x0000008000000000 );
        roundMask = LIT64( 0x000000FFFFFFFFFF );
        expOffset = 0x3F80;
    } else {
        goto precision80;
    }
    zSig0 |= ( zSig1 != 0 );
    if ( ! roundNearestEven ) {
        if ( roundingMode == float_round_to_zero ) {
            roundIncrement = 0;
        } else {
            roundIncrement = roundMask;
            if ( zSign ) {
                if ( roundingMode == float_round_up ) roundIncrement = 0;
            } else {
                if ( roundingMode == float_round_down ) roundIncrement = 0;
            }
        }
    }
    roundBits = zSig0 & roundMask;
    if ( ( ( 0x7FFE - expOffset ) < zExp ) ||
        ( ( zExp == ( 0x7FFE - expOffset ) ) && ( zSig0 + roundIncrement < zSig0 ) ) ) {
        float_raise( float_flag_overflow, c );
        saveFloatx80Internal( roundingPrecision, zSign, zExp, zSig0, zSig1, c );
        if ( zSig0 & roundMask ) float_raise( float_flag_inexact, c );
        if (    ( roundingMode == float_round_to_zero )
            || ( zSign && ( roundingMode == float_round_up ) )
            || ( ! zSign && ( roundingMode == float_round_down ) )
            ) {
            return packFloatx80( zSign, 0x7FFE - expOffset, ~ roundMask );
        }
        return packFloatx80( zSign, floatx80_default_infinity_high, floatx80_default_infinity_low );
    }
    if ( zExp < ( expOffset + 1 ) ) {
        float_raise( float_flag_underflow, c );
        saveFloatx80Internal( roundingPrecision, zSign, zExp, zSig0, zSig1, c );
        shift64RightJamming( zSig0, -( zExp - ( expOffset + 1 ) ), &zSig0 );
        zExp = expOffset + 1;
        roundBits = zSig0 & roundMask;
        if ( roundBits ) float_raise( float_flag_inexact, c );
        zSig0 += roundIncrement;
        roundIncrement = roundMask + 1;
        if ( roundNearestEven && ( roundBits<<1 == roundIncrement ) ) {
            roundMask |= roundIncrement;
        }
        zSig0 &= ~ roundMask;
        return packFloatx80( zSign, zExp, zSig0 );
    }
    if ( roundBits ) {
        float_raise( float_flag_inexact, c );
        saveFloatx80Internal( roundingPrecision, zSign, zExp, zSig0, zSig1, c );
    }
    zSig0 += roundIncrement;
    if ( zSig0 < (bits64)roundIncrement ) {
        ++zExp;
        zSig0 = LIT64( 0x8000000000000000 );
    }
    roundIncrement = roundMask + 1;
    if ( roundNearestEven && ( roundBits<<1 == roundIncrement ) ) {
        roundMask |= roundIncrement;
    }
    zSig0 &= ~ roundMask;
    if ( zSig0 == 0 ) zExp = 0;
    return packFloatx80( zSign, zExp, zSig0 );
precision80:
    increment = ( (sbits64) zSig1 < 0 );
    if ( ! roundNearestEven ) {
        if ( roundingMode == float_round_to_zero ) {
            increment = 0;
        } else {
            if ( zSign ) {
                increment = ( roundingMode == float_round_down ) && zSig1;
            } else {
                increment = ( roundingMode == float_round_up ) && zSig1;
            }
        }
    }
    if ( 0x7FFE <= (bits32) zExp ) {
        if ( ( 0x7FFE < zExp ) ||
            ( ( zExp == 0x7FFE ) && ( zSig0 == LIT64( 0xFFFFFFFFFFFFFFFF ) ) && increment )
            ) {
            roundMask = 0;
            float_raise( float_flag_overflow, c );
            saveFloatx80Internal( roundingPrecision, zSign, zExp, zSig0, zSig1, c );
            if ( ( zSig0 & roundMask ) || zSig1 ) float_raise( float_flag_inexact, c );
            if (    ( roundingMode == float_round_to_zero )
                || ( zSign && ( roundingMode == float_round_up ) )
                || ( ! zSign && ( roundingMode == float_round_down ) )
                ) {
                return packFloatx80( zSign, 0x7FFE, ~ roundMask );
            }
            return packFloatx80( zSign, floatx80_default_infinity_high, floatx80_default_infinity_low );
        }
        if ( zExp < 0 ) {
            float_raise( float_flag_underflow, c );
            saveFloatx80Internal( roundingPrecision, zSign, zExp, zSig0, zSig1, c );
            shift64ExtraRightJamming( zSig0, zSig1, -zExp, &zSig0, &zSig1 );
            zExp = 0;
            if ( zSig1 ) float_raise( float_flag_inexact, c );
            if ( roundNearestEven ) {
                increment = ( (sbits64) zSig1 < 0 );
            } else {
                if ( zSign ) {
                    increment = ( roundingMode == float_round_down ) && zSig1;
                } else {
                    increment = ( roundingMode == float_round_up ) && zSig1;
                }
            }
            if ( increment ) {
                ++zSig0;
                zSig0 &=
                ~ ( ( (bits64) ( zSig1<<1 ) == 0 ) & roundNearestEven );
            }
            return packFloatx80( zSign, zExp, zSig0 );
        }
    }
    if ( zSig1 ) {
        float_raise( float_flag_inexact, c );
        saveFloatx80Internal( roundingPrecision, zSign, zExp, zSig0, zSig1, c );
    }
    if ( increment ) {
        ++zSig0;
        if ( zSig0 == 0 ) {
            ++zExp;
            zSig0 = LIT64( 0x8000000000000000 );
        } else {
            zSig0 &= ~ ( ( (bits64) ( zSig1<<1 ) == 0 ) & roundNearestEven );
        }
    } else {
        if ( zSig0 == 0 ) zExp = 0;
    }
    return packFloatx80( zSign, zExp, zSig0 );
    
}
#endif

#ifdef SOFTFLOAT_68K // 21-01-2017: Added for Previous
static floatx80 roundSigAndPackFloatx80( int8 roundingPrecision, flag zSign, int32 zExp, bits64 zSig0, bits64 zSig1, float_ctrl* c )
{
    int8 roundingMode;
    flag roundNearestEven, isTiny;
    int64 roundIncrement, roundMask, roundBits;
    
    roundingMode = get_float_rounding_mode( c );
    roundNearestEven = ( roundingMode == float_round_nearest_even );
    if ( roundingPrecision == 32 ) {
        roundIncrement = LIT64( 0x0000008000000000 );
        roundMask = LIT64( 0x000000FFFFFFFFFF );
    } else if ( roundingPrecision == 64 ) {
        roundIncrement = LIT64( 0x0000000000000400 );
        roundMask = LIT64( 0x00000000000007FF );
    } else {
        return roundAndPackFloatx80( 80, zSign, zExp, zSig0, zSig1, c );
    }
    zSig0 |= ( zSig1 != 0 );
    if ( ! roundNearestEven ) {
        if ( roundingMode == float_round_to_zero ) {
            roundIncrement = 0;
        }
        else {
            roundIncrement = roundMask;
            if ( zSign ) {
                if ( roundingMode == float_round_up ) roundIncrement = 0;
            }
            else {
                if ( roundingMode == float_round_down ) roundIncrement = 0;
            }
        }
    }
    roundBits = zSig0 & roundMask;
    
    if ( 0x7FFE <= (bits32) zExp ) {
        if (    ( 0x7FFE < zExp )
            || ( ( zExp == 0x7FFE ) && ( zSig0 + roundIncrement < zSig0 ) )
            ) {
            float_raise( float_flag_overflow, c );
            saveFloatx80Internal( roundingPrecision, zSign, zExp, zSig0, zSig1, c);
            if ( zSig0 & roundMask ) float_raise( float_flag_inexact, c );
            if (    ( roundingMode == float_round_to_zero )
                || ( zSign && ( roundingMode == float_round_up ) )
                || ( ! zSign && ( roundingMode == float_round_down ) )
                ) {
                return packFloatx80( zSign, 0x7FFE, LIT64( 0xFFFFFFFFFFFFFFFF ) );
            }
            return packFloatx80( zSign, floatx80_default_infinity_high, floatx80_default_infinity_low );
        }
        
        if ( zExp < 0 ) {
            isTiny =
            ( get_float_detect_tininess(c) == float_tininess_before_rounding )
            || ( zExp < -1 )
            || ( zSig0 <= zSig0 + roundIncrement );
            if ( isTiny ) {
                float_raise( float_flag_underflow, c );
                saveFloatx80Internal( roundingPrecision, zSign, zExp, zSig0, zSig1, c );
            }
            shift64RightJamming( zSig0, -zExp, &zSig0 );
            zExp = 0;
            roundBits = zSig0 & roundMask;
            if ( roundBits ) float_raise( float_flag_inexact, c );
            zSig0 += roundIncrement;
            if ( roundNearestEven && ( roundBits == roundIncrement ) ) {
                roundMask |= roundIncrement<<1;
            }
            zSig0 &= ~ roundMask;
            return packFloatx80( zSign, zExp, zSig0 );
        }
    }
    if ( roundBits ) {
        float_raise( float_flag_inexact, c );
        saveFloatx80Internal( roundingPrecision, zSign, zExp, zSig0, zSig1, c );
    }
    zSig0 += roundIncrement;
    if ( zSig0 < (bits64)roundIncrement ) {
        ++zExp;
        zSig0 = LIT64( 0x8000000000000000 );
    }
    roundIncrement = roundMask + 1;
    if ( roundNearestEven && ( roundBits<<1 == roundIncrement ) ) {
        roundMask |= roundIncrement;
    }
    zSig0 &= ~ roundMask;
    if ( zSig0 == 0 ) zExp = 0;
    return packFloatx80( zSign, zExp, zSig0 );
    
}
#endif // End of Addition for Previous
/*----------------------------------------------------------------------------
| Takes an abstract floating-point value having sign `zSign', exponent
| `zExp', and significand formed by the concatenation of `zSig0' and `zSig1',
| and returns the proper extended double-precision floating-point value
| corresponding to the abstract input.  This routine is just like
| `roundAndPackFloatx80' except that the input significand does not have to be
| normalized.
*----------------------------------------------------------------------------*/

static floatx80 normalizeRoundAndPackFloatx80( int8 roundingPrecision, flag zSign, int32 zExp, bits64 zSig0, bits64 zSig1, float_ctrl* c )
{
	int8 shiftCount;

	if ( zSig0 == 0 ) {
		zSig0 = zSig1;
		zSig1 = 0;
		zExp -= 64;
	}
	shiftCount = countLeadingZeros64( zSig0 );
	shortShift128Left( zSig0, zSig1, shiftCount, &zSig0, &zSig1 );
	zExp -= shiftCount;
	return
		roundAndPackFloatx80( roundingPrecision, zSign, zExp, zSig0, zSig1, c );

}

#endif

#ifdef FLOAT128

/*----------------------------------------------------------------------------
| Returns the least-significant 64 fraction bits of the quadruple-precision
| floating-point value `a'.
*----------------------------------------------------------------------------*/

INLINE bits64 extractFloat128Frac1( float128 a )
{
	return a.low;

}

/*----------------------------------------------------------------------------
| Returns the most-significant 48 fraction bits of the quadruple-precision
| floating-point value `a'.
*----------------------------------------------------------------------------*/

INLINE bits64 extractFloat128Frac0( float128 a )
{
	return a.high & LIT64( 0x0000FFFFFFFFFFFF );

}

/*----------------------------------------------------------------------------
| Returns the exponent bits of the quadruple-precision floating-point value
| `a'.
*----------------------------------------------------------------------------*/

INLINE int32 extractFloat128Exp( float128 a )
{
	return ( a.high>>48 ) & 0x7FFF;

}

/*----------------------------------------------------------------------------
| Returns the sign bit of the quadruple-precision floating-point value `a'.
*----------------------------------------------------------------------------*/

INLINE flag extractFloat128Sign( float128 a )
{
	return a.high>>63;

}

/*----------------------------------------------------------------------------
| Normalizes the subnormal quadruple-precision floating-point value
| represented by the denormalized significand formed by the concatenation of
| `aSig0' and `aSig1'.  The normalized exponent is stored at the location
| pointed to by `zExpPtr'.  The most significant 49 bits of the normalized
| significand are stored at the location pointed to by `zSig0Ptr', and the
| least significant 64 bits of the normalized significand are stored at the
| location pointed to by `zSig1Ptr'.
*----------------------------------------------------------------------------*/

static void
	normalizeFloat128Subnormal(
		bits64 aSig0,
		bits64 aSig1,
		int32 *zExpPtr,
		bits64 *zSig0Ptr,
		bits64 *zSig1Ptr
	)
{
	int8 shiftCount;

	if ( aSig0 == 0 ) {
		shiftCount = countLeadingZeros64( aSig1 ) - 15;
		if ( shiftCount < 0 ) {
			*zSig0Ptr = aSig1>>( - shiftCount );
			*zSig1Ptr = aSig1<<( shiftCount & 63 );
		}
		else {
			*zSig0Ptr = aSig1<<shiftCount;
			*zSig1Ptr = 0;
		}
		*zExpPtr = - shiftCount - 63;
	}
	else {
		shiftCount = countLeadingZeros64( aSig0 ) - 15;
		shortShift128Left( aSig0, aSig1, shiftCount, zSig0Ptr, zSig1Ptr );
		*zExpPtr = 1 - shiftCount;
	}

}
        
/*----------------------------------------------------------------------------
 | Packs the sign `zSign', the exponent `zExp', and the significand formed
 | by the concatenation of `zSig0' and `zSig1' into a quadruple-precision
 | floating-point value, returning the result.  After being shifted into the
 | proper positions, the three fields `zSign', `zExp', and `zSig0' are simply
 | added together to form the most significant 32 bits of the result.  This
 | means that any integer portion of `zSig0' will be added into the exponent.
 | Since a properly normalized significand will have an integer portion equal
 | to 1, the `zExp' input should be 1 less than the desired result exponent
 | whenever `zSig0' and `zSig1' concatenated form a complete, normalized
 | significand.
 *----------------------------------------------------------------------------*/
        
float128 packFloat128( flag zSign, int32 zExp, bits64 zSig0, bits64 zSig1 )
{
    float128 z;
    
    z.low = zSig1;
    z.high = ( ( (bits64) zSign )<<63 ) + ( ( (bits64) zExp )<<48 ) + zSig0;
    return z;
    
}
        
/*----------------------------------------------------------------------------
 | Takes an abstract floating-point value having sign `zSign', exponent `zExp',
 | and extended significand formed by the concatenation of `zSig0', `zSig1',
 | and `zSig2', and returns the proper quadruple-precision floating-point value
 | corresponding to the abstract input.  Ordinarily, the abstract value is
 | simply rounded and packed into the quadruple-precision format, with the
 | inexact exception raised if the abstract input cannot be represented
 | exactly.  However, if the abstract value is too large, the overflow and
 | inexact exceptions are raised and an infinity or maximal finite value is
 | returned.  If the abstract value is too small, the input value is rounded to
 | a subnormal number, and the underflow and inexact exceptions are raised if
 | the abstract input cannot be represented exactly as a subnormal quadruple-
 | precision floating-point number.
 |     The input significand must be normalized or smaller.  If the input
 | significand is not normalized, `zExp' must be 0; in that case, the result
 | returned is a subnormal number, and it must not require rounding.  In the
 | usual case that the input significand is normalized, `zExp' must be 1 less
 | than the ``true'' floating-point exponent.  The handling of underflow and
 | overflow follows the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
 *----------------------------------------------------------------------------*/
        
float128 roundAndPackFloat128( flag zSign, int32 zExp, bits64 zSig0, bits64 zSig1, bits64 zSig2, float_ctrl* c )
{
    int8 roundingMode;
    flag roundNearestEven, increment, isTiny;
    
    roundingMode = get_float_rounding_mode( c );
    roundNearestEven = ( roundingMode == float_round_nearest_even );
    increment = ( (sbits64) zSig2 < 0 );
    if ( ! roundNearestEven ) {
        if ( roundingMode == float_round_to_zero ) {
            increment = 0;
        }
        else {
            if ( zSign ) {
                increment = ( roundingMode == float_round_down ) && zSig2;
            }
            else {
                increment = ( roundingMode == float_round_up ) && zSig2;
            }
        }
    }
    if ( 0x7FFD <= (bits32) zExp ) {
        if (    ( 0x7FFD < zExp )
            || (    ( zExp == 0x7FFD )
                && eq128(
                         LIT64( 0x0001FFFFFFFFFFFF ),
                         LIT64( 0xFFFFFFFFFFFFFFFF ),
                         zSig0,
                         zSig1
                         )
                && increment
                )
            ) {
#ifdef SOFTFLOAT_68K
            float_raise( float_flag_overflow, c );
            if ( zSig2 ) float_raise( float_flag_inexact, c );
#else
            float_raise( float_flag_overflow | float_flag_inexact );
#endif
            if (    ( roundingMode == float_round_to_zero )
                || ( zSign && ( roundingMode == float_round_up ) )
                || ( ! zSign && ( roundingMode == float_round_down ) )
                ) {
                return
                packFloat128(
                             zSign,
                             0x7FFE,
                             LIT64( 0x0000FFFFFFFFFFFF ),
                             LIT64( 0xFFFFFFFFFFFFFFFF )
                             );
            }
            return packFloat128( zSign, 0x7FFF, 0, 0 );
        }
        if ( zExp < 0 ) {
            isTiny =
            ( get_float_detect_tininess(c) == float_tininess_before_rounding )
            || ( zExp < -1 )
            || ! increment
            || lt128(
                     zSig0,
                     zSig1,
                     LIT64( 0x0001FFFFFFFFFFFF ),
                     LIT64( 0xFFFFFFFFFFFFFFFF )
                     );
            shift128ExtraRightJamming(
                                      zSig0, zSig1, zSig2, - zExp, &zSig0, &zSig1, &zSig2 );
            zExp = 0;
#ifdef SOFTFLOAT_68K
            if ( isTiny ) float_raise( float_flag_underflow, c );
#else
            if ( isTiny && zSig2 ) float_raise( float_flag_underflow );
#endif
            if ( roundNearestEven ) {
                increment = ( (sbits64) zSig2 < 0 );
            }
            else {
                if ( zSign ) {
                    increment = ( roundingMode == float_round_down ) && zSig2;
                }
                else {
                    increment = ( roundingMode == float_round_up ) && zSig2;
                }
            }
        }
    }
    if ( zSig2 ) float_raise( float_flag_inexact, c );
    if ( increment ) {
        add128( zSig0, zSig1, 0, 1, &zSig0, &zSig1 );
        zSig1 &= ~ ( ( zSig2 + zSig2 == 0 ) & roundNearestEven );
    }
    else {
        if ( ( zSig0 | zSig1 ) == 0 ) zExp = 0;
    }
    return packFloat128( zSign, zExp, zSig0, zSig1 );
    
}
        
/*----------------------------------------------------------------------------
 | Takes an abstract floating-point value having sign `zSign', exponent `zExp',
 | and significand formed by the concatenation of `zSig0' and `zSig1', and
 | returns the proper quadruple-precision floating-point value corresponding
 | to the abstract input.  This routine is just like `roundAndPackFloat128'
 | except that the input significand has fewer bits and does not have to be
 | normalized.  In all cases, `zExp' must be 1 less than the ``true'' floating-
 | point exponent.
 *----------------------------------------------------------------------------*/
        
float128 normalizeRoundAndPackFloat128( flag zSign, int32 zExp, bits64 zSig0, bits64 zSig1, float_ctrl* c )
{
    int8 shiftCount;
    bits64 zSig2;
    
    if ( zSig0 == 0 ) {
        zSig0 = zSig1;
        zSig1 = 0;
        zExp -= 64;
    }
    shiftCount = countLeadingZeros64( zSig0 ) - 15;
    if ( 0 <= shiftCount ) {
        zSig2 = 0;
        shortShift128Left( zSig0, zSig1, shiftCount, &zSig0, &zSig1 );
    }
    else {
        shift128ExtraRightJamming(
                                  zSig0, zSig1, 0, - shiftCount, &zSig0, &zSig1, &zSig2 );
    }
    zExp -= shiftCount;
    return roundAndPackFloat128( zSign, zExp, zSig0, zSig1, zSig2, c );
    
}

#endif

/*----------------------------------------------------------------------------
| Returns the result of converting the 32-bit two's complement integer `a'
| to the single-precision floating-point format.  The conversion is performed
| according to the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float32 int32_to_float32( int32 a, float_ctrl* c )
{
	flag zSign;

	if ( a == 0 ) return 0;
	if ( a == (sbits32) 0x80000000 ) return packFloat32( 1, 0x9E, 0 );
	zSign = ( a < 0 );
	return normalizeRoundAndPackFloat32( zSign, 0x9C, zSign ? - a : a, c );

}

/*----------------------------------------------------------------------------
| Returns the result of converting the 32-bit two's complement integer `a'
| to the double-precision floating-point format.  The conversion is performed
| according to the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float64 int32_to_float64( int32 a )
{
	flag zSign;
	uint32 absA;
	int8 shiftCount;
	bits64 zSig;

	if ( a == 0 ) return 0;
	zSign = ( a < 0 );
	absA = zSign ? - a : a;
	shiftCount = countLeadingZeros32( absA ) + 21;
	zSig = absA;
	return packFloat64( zSign, 0x432 - shiftCount, zSig<<shiftCount );

}

#ifdef FLOATX80

/*----------------------------------------------------------------------------
| Returns the result of converting the 32-bit two's complement integer `a'
| to the extended double-precision floating-point format.  The conversion
| is performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic.
*----------------------------------------------------------------------------*/

floatx80 int32_to_floatx80( int32 a )
{
	flag zSign;
	uint32 absA;
	int8 shiftCount;
	bits64 zSig;

	if ( a == 0 ) return packFloatx80( 0, 0, 0 );
	zSign = ( a < 0 );
	absA = zSign ? - a : a;
	shiftCount = countLeadingZeros32( absA ) + 32;
	zSig = absA;
	return packFloatx80( zSign, 0x403E - shiftCount, zSig<<shiftCount );

}

#endif

#ifdef FLOAT128

/*----------------------------------------------------------------------------
| Returns the result of converting the 32-bit two's complement integer `a' to
| the quadruple-precision floating-point format.  The conversion is performed
| according to the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float128 int32_to_float128( int32 a )
{
	flag zSign;
	uint32 absA;
	int8 shiftCount;
	bits64 zSig0;

	if ( a == 0 ) return packFloat128( 0, 0, 0, 0 );
	zSign = ( a < 0 );
	absA = zSign ? - a : a;
	shiftCount = countLeadingZeros32( absA ) + 17;
	zSig0 = absA;
	return packFloat128( zSign, 0x402E - shiftCount, zSig0<<shiftCount, 0 );

}

#endif

/*----------------------------------------------------------------------------
| Returns the result of converting the 64-bit two's complement integer `a'
| to the single-precision floating-point format.  The conversion is performed
| according to the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float32 int64_to_float32( int64 a, float_ctrl* c )
{
	flag zSign;
	uint64 absA;
	int8 shiftCount;
//    bits32 zSig;

	if ( a == 0 ) return 0;
	zSign = ( a < 0 );
	absA = zSign ? - a : a;
	shiftCount = countLeadingZeros64( absA ) - 40;
	if ( 0 <= shiftCount ) {
		return packFloat32( zSign, 0x95 - shiftCount, absA<<shiftCount );
	}
	else {
		shiftCount += 7;
		if ( shiftCount < 0 ) {
			shift64RightJamming( absA, - shiftCount, &absA );
		}
		else {
			absA <<= shiftCount;
		}
		return roundAndPackFloat32( zSign, 0x9C - shiftCount, absA, c );
	}

}

/*----------------------------------------------------------------------------
| Returns the result of converting the 64-bit two's complement integer `a'
| to the double-precision floating-point format.  The conversion is performed
| according to the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float64 int64_to_float64( int64 a, float_ctrl* c )
{
	flag zSign;

	if ( a == 0 ) return 0;
	if ( a == (sbits64) LIT64( 0x8000000000000000 ) ) {
		return packFloat64( 1, 0x43E, 0 );
	}
	zSign = ( a < 0 );
	return normalizeRoundAndPackFloat64( zSign, 0x43C, zSign ? - a : a, c );

}

#ifdef FLOATX80

/*----------------------------------------------------------------------------
| Returns the result of converting the 64-bit two's complement integer `a'
| to the extended double-precision floating-point format.  The conversion
| is performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic.
*----------------------------------------------------------------------------*/

floatx80 int64_to_floatx80( int64 a )
{
	flag zSign;
	uint64 absA;
	int8 shiftCount;

	if ( a == 0 ) return packFloatx80( 0, 0, 0 );
	zSign = ( a < 0 );
	absA = zSign ? - a : a;
	shiftCount = countLeadingZeros64( absA );
	return packFloatx80( zSign, 0x403E - shiftCount, absA<<shiftCount );

}

#endif

#ifdef FLOAT128

/*----------------------------------------------------------------------------
| Returns the result of converting the 64-bit two's complement integer `a' to
| the quadruple-precision floating-point format.  The conversion is performed
| according to the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float128 int64_to_float128( int64 a )
{
	flag zSign;
	uint64 absA;
	int8 shiftCount;
	int32 zExp;
	bits64 zSig0, zSig1;

	if ( a == 0 ) return packFloat128( 0, 0, 0, 0 );
	zSign = ( a < 0 );
	absA = zSign ? - a : a;
	shiftCount = countLeadingZeros64( absA ) + 49;
	zExp = 0x406E - shiftCount;
	if ( 64 <= shiftCount ) {
		zSig1 = 0;
		zSig0 = absA;
		shiftCount -= 64;
	}
	else {
		zSig1 = absA;
		zSig0 = 0;
	}
	shortShift128Left( zSig0, zSig1, shiftCount, &zSig0, &zSig1 );
	return packFloat128( zSign, zExp, zSig0, zSig1 );

}

#endif

/*----------------------------------------------------------------------------
| Returns the result of converting the single-precision floating-point value
| `a' to the 32-bit two's complement integer format.  The conversion is
| performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic---which means in particular that the conversion is rounded
| according to the current rounding mode.  If `a' is a NaN, the largest
| positive integer is returned.  Otherwise, if the conversion overflows, the
| largest integer with the same sign as `a' is returned.
*----------------------------------------------------------------------------*/

int32 float32_to_int32( float32 a, float_ctrl* c )
{
	flag aSign;
	int16 aExp, shiftCount;
	bits32 aSig;
	bits64 aSig64;

	aSig = extractFloat32Frac( a );
	aExp = extractFloat32Exp( a );
	aSign = extractFloat32Sign( a );
	if ( ( aExp == 0xFF ) && aSig ) aSign = 0;
	if ( aExp ) aSig |= 0x00800000;
	shiftCount = 0xAF - aExp;
	aSig64 = aSig;
	aSig64 <<= 32;
	if ( 0 < shiftCount ) shift64RightJamming( aSig64, shiftCount, &aSig64 );
#ifdef SOFTFLOAT_I860
	return roundAndPackInt32_2( aSign, aSig64, c );
#else
	return roundAndPackInt32( aSign, aSig64 );
#endif

}

/*----------------------------------------------------------------------------
| Returns the result of converting the single-precision floating-point value
| `a' to the 32-bit two's complement integer format.  The conversion is
| performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic, except that the conversion is always rounded toward zero.
| If `a' is a NaN, the largest positive integer is returned.  Otherwise, if
| the conversion overflows, the largest integer with the same sign as `a' is
| returned.
*----------------------------------------------------------------------------*/

int32 float32_to_int32_round_to_zero( float32 a, float_ctrl* c )
{
	flag aSign;
	int16 aExp, shiftCount;
	bits32 aSig;
	int32 z;

	aSig = extractFloat32Frac( a );
	aExp = extractFloat32Exp( a );
	aSign = extractFloat32Sign( a );
	shiftCount = aExp - 0x9E;
	if ( 0 <= shiftCount ) {
		if ( a != 0xCF000000 ) {
#ifdef SOFTFLOAT_I860
            float_raise( float_flag_invalid, c );
#else
			float_raise( float_flag_invalid );
#endif
			if ( ! aSign || ( ( aExp == 0xFF ) && aSig ) ) return 0x7FFFFFFF;
		}
		return (sbits32) 0x80000000;
	}
	else if ( aExp <= 0x7E ) {
#ifdef SOFTFLOAT_I860
        if ( aExp | aSig ) float_raise( float_flag_inexact, c );
#else
		if ( aExp | aSig ) float_raise( float_flag_inexact, c );
#endif
		return 0;
	}
	aSig = ( aSig | 0x00800000 )<<8;
	z = aSig>>( - shiftCount );
	if ( (bits32) ( aSig<<( shiftCount & 31 ) ) ) {
#ifdef SOFTFLOAT_I860
        float_raise( float_flag_inexact, c );
#else
		float_exception_flags |= float_flag_inexact;
#endif
	}
	if ( aSign ) z = - z;
	return z;

}

/*----------------------------------------------------------------------------
| Returns the result of converting the single-precision floating-point value
| `a' to the 64-bit two's complement integer format.  The conversion is
| performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic---which means in particular that the conversion is rounded
| according to the current rounding mode.  If `a' is a NaN, the largest
| positive integer is returned.  Otherwise, if the conversion overflows, the
| largest integer with the same sign as `a' is returned.
*----------------------------------------------------------------------------*/

int64 float32_to_int64( float32 a, float_ctrl* c )
{
	flag aSign;
	int16 aExp, shiftCount;
	bits32 aSig;
	bits64 aSig64, aSigExtra;

	aSig = extractFloat32Frac( a );
	aExp = extractFloat32Exp( a );
	aSign = extractFloat32Sign( a );
	shiftCount = 0xBE - aExp;
	if ( shiftCount < 0 ) {
		float_raise( float_flag_invalid, c );
		if ( ! aSign || ( ( aExp == 0xFF ) && aSig ) ) {
			return LIT64( 0x7FFFFFFFFFFFFFFF );
		}
		return (sbits64) LIT64( 0x8000000000000000 );
	}
	if ( aExp ) aSig |= 0x00800000;
	aSig64 = aSig;
	aSig64 <<= 40;
	shift64ExtraRightJamming( aSig64, 0, shiftCount, &aSig64, &aSigExtra );
	return roundAndPackInt64( aSign, aSig64, aSigExtra, c );

}

/*----------------------------------------------------------------------------
| Returns the result of converting the single-precision floating-point value
| `a' to the 64-bit two's complement integer format.  The conversion is
| performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic, except that the conversion is always rounded toward zero.  If
| `a' is a NaN, the largest positive integer is returned.  Otherwise, if the
| conversion overflows, the largest integer with the same sign as `a' is
| returned.
*----------------------------------------------------------------------------*/

int64 float32_to_int64_round_to_zero( float32 a, float_ctrl* c )
{
	flag aSign;
	int16 aExp, shiftCount;
	bits32 aSig;
	bits64 aSig64;
	int64 z;

	aSig = extractFloat32Frac( a );
	aExp = extractFloat32Exp( a );
	aSign = extractFloat32Sign( a );
	shiftCount = aExp - 0xBE;
	if ( 0 <= shiftCount ) {
		if ( a != 0xDF000000 ) {
			float_raise( float_flag_invalid, c );
			if ( ! aSign || ( ( aExp == 0xFF ) && aSig ) ) {
				return LIT64( 0x7FFFFFFFFFFFFFFF );
			}
		}
		return (sbits64) LIT64( 0x8000000000000000 );
	}
	else if ( aExp <= 0x7E ) {
		if ( aExp | aSig ) float_raise( float_flag_inexact, c );
		return 0;
	}
	aSig64 = aSig | 0x00800000;
	aSig64 <<= 40;
	z = aSig64>>( - shiftCount );
	if ( (bits64) ( aSig64<<( shiftCount & 63 ) ) ) {
		float_raise( float_flag_inexact, c );
	}
	if ( aSign ) z = - z;
	return z;

}

/*----------------------------------------------------------------------------
| Returns the result of converting the single-precision floating-point value
| `a' to the double-precision floating-point format.  The conversion is
| performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic.
*----------------------------------------------------------------------------*/

float64 float32_to_float64( float32 a, float_ctrl* c )
{
	flag aSign;
	int16 aExp;
	bits32 aSig;

	aSig = extractFloat32Frac( a );
	aExp = extractFloat32Exp( a );
	aSign = extractFloat32Sign( a );
	if ( aExp == 0xFF ) {
		if ( aSig ) return commonNaNToFloat64( float32ToCommonNaN( a, c ) );
		return packFloat64( aSign, 0x7FF, 0 );
	}
	if ( aExp == 0 ) {
		if ( aSig == 0 ) return packFloat64( aSign, 0, 0 );
		normalizeFloat32Subnormal( aSig, &aExp, &aSig );
		--aExp;
	}
	return packFloat64( aSign, aExp + 0x380, ( (bits64) aSig )<<29 );

}

#ifdef FLOATX80

/*----------------------------------------------------------------------------
| Returns the result of converting the single-precision floating-point value
| `a' to the extended double-precision floating-point format.  The conversion
| is performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic.
*----------------------------------------------------------------------------*/

floatx80 float32_to_floatx80( float32 a, float_ctrl* c )
{
	flag aSign;
	int16 aExp;
	bits32 aSig;

	aSig = extractFloat32Frac( a );
	aExp = extractFloat32Exp( a );
	aSign = extractFloat32Sign( a );
	if ( aExp == 0xFF ) {
		if ( aSig ) return commonNaNToFloatx80( float32ToCommonNaN( a, c ) );
		return packFloatx80( aSign, floatx80_default_infinity_high, floatx80_default_infinity_low );
	}
	if ( aExp == 0 ) {
		if ( aSig == 0 ) return packFloatx80( aSign, 0, 0 );
		normalizeFloat32Subnormal( aSig, &aExp, &aSig );
	}
	aSig |= 0x00800000;
	return packFloatx80( aSign, aExp + 0x3F80, ( (bits64) aSig )<<40 );

}

#ifdef SOFTFLOAT_68K // 31-12-2016: Added for Previous
floatx80 float32_to_floatx80_allowunnormal( float32 a )
{
    flag aSign;
    int16 aExp;
    bits32 aSig;
    
    aSig = extractFloat32Frac( a );
    aExp = extractFloat32Exp( a );
    aSign = extractFloat32Sign( a );
    if ( aExp == 0xFF ) {
        return packFloatx80( aSign, 0x7FFF, ( (bits64) aSig )<<40 );
    }
    if ( aExp == 0 ) {
        if ( aSig == 0 ) return packFloatx80( aSign, 0, 0 );
        return packFloatx80( aSign, 0x3F81, ( (bits64) aSig )<<40 );
    }
    aSig |= 0x00800000;
    return packFloatx80( aSign, aExp + 0x3F80, ( (bits64) aSig )<<40 );
    
}
#endif // end of addition for Previous

#endif

#ifdef FLOAT128

/*----------------------------------------------------------------------------
| Returns the result of converting the single-precision floating-point value
| `a' to the double-precision floating-point format.  The conversion is
| performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic.
*----------------------------------------------------------------------------*/

float128 float32_to_float128( float32 a, float_ctrl* c )
{
	flag aSign;
	int16 aExp;
	bits32 aSig;

	aSig = extractFloat32Frac( a );
	aExp = extractFloat32Exp( a );
	aSign = extractFloat32Sign( a );
	if ( aExp == 0xFF ) {
		if ( aSig ) return commonNaNToFloat128( float32ToCommonNaN( a, c ) );
		return packFloat128( aSign, 0x7FFF, 0, 0 );
	}
	if ( aExp == 0 ) {
		if ( aSig == 0 ) return packFloat128( aSign, 0, 0, 0 );
		normalizeFloat32Subnormal( aSig, &aExp, &aSig );
		--aExp;
	}
	return packFloat128( aSign, aExp + 0x3F80, ( (bits64) aSig )<<25, 0 );

}

#endif

/*----------------------------------------------------------------------------
| Rounds the single-precision floating-point value `a' to an integer, and
| returns the result as a single-precision floating-point value.  The
| operation is performed according to the IEC/IEEE Standard for Binary
| Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float32 float32_round_to_int( float32 a, float_ctrl* c )
{
	flag aSign;
	int16 aExp;
	bits32 lastBitMask, roundBitsMask;
	int8 roundingMode;
	float32 z;

    roundingMode = get_float_rounding_mode( c );
	aExp = extractFloat32Exp( a );
	if ( 0x96 <= aExp ) {
		if ( ( aExp == 0xFF ) && extractFloat32Frac( a ) ) {
			return propagateFloat32NaN( a, a, c );
		}
		return a;
	}
	if ( aExp <= 0x7E ) {
		if ( (bits32) ( a<<1 ) == 0 ) return a;
		float_raise( float_flag_inexact, c );
		aSign = extractFloat32Sign( a );
		switch ( roundingMode ) {
			case float_round_nearest_even:
			if ( ( aExp == 0x7E ) && extractFloat32Frac( a ) ) {
				return packFloat32( aSign, 0x7F, 0 );
			}
			break;
			case float_round_down:
			return aSign ? 0xBF800000 : 0;
			case float_round_up:
			return aSign ? 0x80000000 : 0x3F800000;
		}
		return packFloat32( aSign, 0, 0 );
	}
	lastBitMask = 1;
	lastBitMask <<= 0x96 - aExp;
	roundBitsMask = lastBitMask - 1;
	z = a;
	if ( roundingMode == float_round_nearest_even ) {
		z += lastBitMask>>1;
		if ( ( z & roundBitsMask ) == 0 ) z &= ~ lastBitMask;
	}
	else if ( roundingMode != float_round_to_zero ) {
		if ( extractFloat32Sign( z ) ^ ( roundingMode == float_round_up ) ) {
			z += roundBitsMask;
		}
	}
	z &= ~ roundBitsMask;
	if ( z != a ) float_raise( float_flag_inexact, c );
	return z;

}

/*----------------------------------------------------------------------------
| Returns the result of adding the absolute values of the single-precision
| floating-point values `a' and `b'.  If `zSign' is 1, the sum is negated
| before being returned.  `zSign' is ignored if the result is a NaN.
| The addition is performed according to the IEC/IEEE Standard for Binary
| Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

static float32 addFloat32Sigs( float32 a, float32 b, flag zSign, float_ctrl* c )
{
	int16 aExp, bExp, zExp;
	bits32 aSig, bSig, zSig;
	int16 expDiff;

	aSig = extractFloat32Frac( a );
	aExp = extractFloat32Exp( a );
	bSig = extractFloat32Frac( b );
	bExp = extractFloat32Exp( b );
	expDiff = aExp - bExp;
	aSig <<= 6;
	bSig <<= 6;
	if ( 0 < expDiff ) {
		if ( aExp == 0xFF ) {
			if ( aSig ) return propagateFloat32NaN( a, b, c );
			return a;
		}
		if ( bExp == 0 ) {
			--expDiff;
		}
		else {
			bSig |= 0x20000000;
		}
		shift32RightJamming( bSig, expDiff, &bSig );
		zExp = aExp;
	}
	else if ( expDiff < 0 ) {
		if ( bExp == 0xFF ) {
			if ( bSig ) return propagateFloat32NaN( a, b, c );
			return packFloat32( zSign, 0xFF, 0 );
		}
		if ( aExp == 0 ) {
			++expDiff;
		}
		else {
			aSig |= 0x20000000;
		}
		shift32RightJamming( aSig, - expDiff, &aSig );
		zExp = bExp;
	}
	else {
		if ( aExp == 0xFF ) {
			if ( aSig | bSig ) return propagateFloat32NaN( a, b, c );
			return a;
		}
		if ( aExp == 0 ) return packFloat32( zSign, 0, ( aSig + bSig )>>6 );
		zSig = 0x40000000 + aSig + bSig;
		zExp = aExp;
		goto roundAndPack;
	}
	aSig |= 0x20000000;
	zSig = ( aSig + bSig )<<1;
	--zExp;
	if ( (sbits32) zSig < 0 ) {
		zSig = aSig + bSig;
		++zExp;
	}
	roundAndPack:
#ifdef SOFTFLOAT_I860
	return roundAndPackFloat32_2( zSign, zExp, zSig, c );
#else
	return roundAndPackFloat32( zSign, zExp, zSig );
#endif

}

/*----------------------------------------------------------------------------
| Returns the result of subtracting the absolute values of the single-
| precision floating-point values `a' and `b'.  If `zSign' is 1, the
| difference is negated before being returned.  `zSign' is ignored if the
| result is a NaN.  The subtraction is performed according to the IEC/IEEE
| Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

static float32 subFloat32Sigs( float32 a, float32 b, flag zSign, float_ctrl* c )
{
	int16 aExp, bExp, zExp;
	bits32 aSig, bSig, zSig;
	int16 expDiff;
#ifdef SOFTFLOAT_I860
    int8 shiftCount;
#endif

	aSig = extractFloat32Frac( a );
	aExp = extractFloat32Exp( a );
	bSig = extractFloat32Frac( b );
	bExp = extractFloat32Exp( b );
	expDiff = aExp - bExp;
	aSig <<= 7;
	bSig <<= 7;
	if ( 0 < expDiff ) goto aExpBigger;
	if ( expDiff < 0 ) goto bExpBigger;
	if ( aExp == 0xFF ) {
		if ( aSig | bSig ) return propagateFloat32NaN( a, b, c );
#ifdef SOFTFLOAT_I860
        float_raise( float_flag_invalid, c );
#else
		float_raise( float_flag_invalid );
#endif
		return float32_default_nan;
	}
	if ( aExp == 0 ) {
		aExp = 1;
		bExp = 1;
	}
	if ( bSig < aSig ) goto aBigger;
	if ( aSig < bSig ) goto bBigger;
#ifdef SOFTFLOAT_I860
    return packFloat32( get_float_rounding_mode( c ) == float_round_down, 0, 0 );
#else
	return packFloat32( float_rounding_mode == float_round_down, 0, 0 );
#endif
	bExpBigger:
	if ( bExp == 0xFF ) {
		if ( bSig ) return propagateFloat32NaN( a, b, c );
		return packFloat32( zSign ^ 1, 0xFF, 0 );
	}
	if ( aExp == 0 ) {
		++expDiff;
	}
	else {
		aSig |= 0x40000000;
	}
	shift32RightJamming( aSig, - expDiff, &aSig );
	bSig |= 0x40000000;
	bBigger:
	zSig = bSig - aSig;
	zExp = bExp;
	zSign ^= 1;
	goto normalizeRoundAndPack;
	aExpBigger:
	if ( aExp == 0xFF ) {
		if ( aSig ) return propagateFloat32NaN( a, b, c );
		return a;
	}
	if ( bExp == 0 ) {
		--expDiff;
	}
	else {
		bSig |= 0x40000000;
	}
	shift32RightJamming( bSig, expDiff, &bSig );
	aSig |= 0x40000000;
	aBigger:
	zSig = aSig - bSig;
	zExp = aExp;
	normalizeRoundAndPack:
	--zExp;
#ifdef SOFTFLOAT_I860
    shiftCount = countLeadingZeros32( zSig ) - 1;
    return roundAndPackFloat32_2( zSign, zExp - shiftCount, zSig<<shiftCount, c );
#else
	return normalizeRoundAndPackFloat32( zSign, zExp, zSig );
#endif

}

/*----------------------------------------------------------------------------
| Returns the result of adding the single-precision floating-point values `a'
| and `b'.  The operation is performed according to the IEC/IEEE Standard for
| Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float32 float32_add( float32 a, float32 b, float_ctrl* c )
{
	flag aSign, bSign;

	aSign = extractFloat32Sign( a );
	bSign = extractFloat32Sign( b );
	if ( aSign == bSign ) {
		return addFloat32Sigs( a, b, aSign, c );
	}
	else {
		return subFloat32Sigs( a, b, aSign, c );
	}

}

/*----------------------------------------------------------------------------
| Returns the result of subtracting the single-precision floating-point values
| `a' and `b'.  The operation is performed according to the IEC/IEEE Standard
| for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float32 float32_sub( float32 a, float32 b, float_ctrl* c )
{
	flag aSign, bSign;

	aSign = extractFloat32Sign( a );
	bSign = extractFloat32Sign( b );
	if ( aSign == bSign ) {
		return subFloat32Sigs( a, b, aSign, c );
	}
	else {
		return addFloat32Sigs( a, b, aSign, c );
	}

}

/*----------------------------------------------------------------------------
| Returns the result of multiplying the single-precision floating-point values
| `a' and `b'.  The operation is performed according to the IEC/IEEE Standard
| for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float32 float32_mul( float32 a, float32 b, float_ctrl* c )
{
	flag aSign, bSign, zSign;
	int16 aExp, bExp, zExp;
	bits32 aSig, bSig;
	bits64 zSig64;
	bits32 zSig;

	aSig = extractFloat32Frac( a );
	aExp = extractFloat32Exp( a );
	aSign = extractFloat32Sign( a );
	bSig = extractFloat32Frac( b );
	bExp = extractFloat32Exp( b );
	bSign = extractFloat32Sign( b );
	zSign = aSign ^ bSign;
	if ( aExp == 0xFF ) {
		if ( aSig || ( ( bExp == 0xFF ) && bSig ) ) {
			return propagateFloat32NaN( a, b, c );
		}
		if ( ( bExp | bSig ) == 0 ) {
#ifdef SOFTFLOAT_I860
			float_raise( float_flag_invalid, c );
#else
			float_raise( float_flag_invalid );
#endif
			return float32_default_nan;
		}
		return packFloat32( zSign, 0xFF, 0 );
	}
	if ( bExp == 0xFF ) {
		if ( bSig ) return propagateFloat32NaN( a, b, c );
		if ( ( aExp | aSig ) == 0 ) {
#ifdef SOFTFLOAT_I860
            float_raise( float_flag_invalid, c );
#else
			float_raise( float_flag_invalid );
#endif
			return float32_default_nan;
		}
		return packFloat32( zSign, 0xFF, 0 );
	}
	if ( aExp == 0 ) {
		if ( aSig == 0 ) return packFloat32( zSign, 0, 0 );
		normalizeFloat32Subnormal( aSig, &aExp, &aSig );
	}
	if ( bExp == 0 ) {
		if ( bSig == 0 ) return packFloat32( zSign, 0, 0 );
		normalizeFloat32Subnormal( bSig, &bExp, &bSig );
	}
	zExp = aExp + bExp - 0x7F;
	aSig = ( aSig | 0x00800000 )<<7;
	bSig = ( bSig | 0x00800000 )<<8;
	shift64RightJamming( ( (bits64) aSig ) * bSig, 32, &zSig64 );
	zSig = zSig64;
	if ( 0 <= (sbits32) ( zSig<<1 ) ) {
		zSig <<= 1;
		--zExp;
	}
#ifdef SOFTFLOAT_I860
	return roundAndPackFloat32_2( zSign, zExp, zSig, c );
#else
	return roundAndPackFloat32( zSign, zExp, zSig );
#endif

}

/*----------------------------------------------------------------------------
| Returns the result of dividing the single-precision floating-point value `a'
| by the corresponding value `b'.  The operation is performed according to the
| IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float32 float32_div( float32 a, float32 b, float_ctrl* c )
{
	flag aSign, bSign, zSign;
	int16 aExp, bExp, zExp;
	bits32 aSig, bSig, zSig;

	aSig = extractFloat32Frac( a );
	aExp = extractFloat32Exp( a );
	aSign = extractFloat32Sign( a );
	bSig = extractFloat32Frac( b );
	bExp = extractFloat32Exp( b );
	bSign = extractFloat32Sign( b );
	zSign = aSign ^ bSign;
	if ( aExp == 0xFF ) {
		if ( aSig ) return propagateFloat32NaN( a, b, c );
		if ( bExp == 0xFF ) {
			if ( bSig ) return propagateFloat32NaN( a, b, c );
#ifdef SOFTFLOAT_I860
			float_raise( float_flag_invalid, c );
#else
			float_raise( float_flag_invalid );
#endif
			return float32_default_nan;
		}
		return packFloat32( zSign, 0xFF, 0 );
	}
	if ( bExp == 0xFF ) {
		if ( bSig ) return propagateFloat32NaN( a, b, c );
		return packFloat32( zSign, 0, 0 );
	}
	if ( bExp == 0 ) {
		if ( bSig == 0 ) {
			if ( ( aExp | aSig ) == 0 ) {
#ifdef SOFTFLOAT_I860
                float_raise( float_flag_invalid, c );
#else
                float_raise( float_flag_invalid );
#endif
				return float32_default_nan;
			}
#ifdef SOFTFLOAT_I860
			float_raise( float_flag_divbyzero, c );
#else
			float_raise( float_flag_divbyzero );
#endif
			return packFloat32( zSign, 0xFF, 0 );
		}
		normalizeFloat32Subnormal( bSig, &bExp, &bSig );
	}
	if ( aExp == 0 ) {
		if ( aSig == 0 ) return packFloat32( zSign, 0, 0 );
		normalizeFloat32Subnormal( aSig, &aExp, &aSig );
	}
	zExp = aExp - bExp + 0x7D;
	aSig = ( aSig | 0x00800000 )<<7;
	bSig = ( bSig | 0x00800000 )<<8;
	if ( bSig <= ( aSig + aSig ) ) {
		aSig >>= 1;
		++zExp;
	}
	zSig = ( ( (bits64) aSig )<<32 ) / bSig;
	if ( ( zSig & 0x3F ) == 0 ) {
		zSig |= ( (bits64) bSig * zSig != ( (bits64) aSig )<<32 );
	}
#ifdef SOFTFLOAT_I860
    return roundAndPackFloat32_2( zSign, zExp, zSig, c );
#else
	return roundAndPackFloat32( zSign, zExp, zSig );
#endif

}

/*----------------------------------------------------------------------------
| Returns the remainder of the single-precision floating-point value `a'
| with respect to the corresponding value `b'.  The operation is performed
| according to the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float32 float32_rem( float32 a, float32 b, float_ctrl* c )
{
	flag aSign, zSign;
	int16 aExp, bExp, expDiff;
	bits32 aSig, bSig;
	bits32 q;
	bits64 aSig64, bSig64, q64;
	bits32 alternateASig;
	sbits32 sigMean;

	aSig = extractFloat32Frac( a );
	aExp = extractFloat32Exp( a );
	aSign = extractFloat32Sign( a );
	bSig = extractFloat32Frac( b );
	bExp = extractFloat32Exp( b );
//    bSign = extractFloat32Sign( b );
	if ( aExp == 0xFF ) {
		if ( aSig || ( ( bExp == 0xFF ) && bSig ) ) {
			return propagateFloat32NaN( a, b, c );
		}
		float_raise( float_flag_invalid, c );
		return float32_default_nan;
	}
	if ( bExp == 0xFF ) {
		if ( bSig ) return propagateFloat32NaN( a, b, c );
		return a;
	}
	if ( bExp == 0 ) {
		if ( bSig == 0 ) {
			float_raise( float_flag_invalid, c );
			return float32_default_nan;
		}
		normalizeFloat32Subnormal( bSig, &bExp, &bSig );
	}
	if ( aExp == 0 ) {
		if ( aSig == 0 ) return a;
		normalizeFloat32Subnormal( aSig, &aExp, &aSig );
	}
	expDiff = aExp - bExp;
	aSig |= 0x00800000;
	bSig |= 0x00800000;
	if ( expDiff < 32 ) {
		aSig <<= 8;
		bSig <<= 8;
		if ( expDiff < 0 ) {
			if ( expDiff < -1 ) return a;
			aSig >>= 1;
		}
		q = ( bSig <= aSig );
		if ( q ) aSig -= bSig;
		if ( 0 < expDiff ) {
			q = ( ( (bits64) aSig )<<32 ) / bSig;
			q >>= 32 - expDiff;
			bSig >>= 2;
			aSig = ( ( aSig>>1 )<<( expDiff - 1 ) ) - bSig * q;
		}
		else {
			aSig >>= 2;
			bSig >>= 2;
		}
	}
	else {
		if ( bSig <= aSig ) aSig -= bSig;
		aSig64 = ( (bits64) aSig )<<40;
		bSig64 = ( (bits64) bSig )<<40;
		expDiff -= 64;
		while ( 0 < expDiff ) {
			q64 = estimateDiv128To64( aSig64, 0, bSig64 );
			q64 = ( 2 < q64 ) ? q64 - 2 : 0;
			aSig64 = - ( ( bSig * q64 )<<38 );
			expDiff -= 62;
		}
		expDiff += 64;
		q64 = estimateDiv128To64( aSig64, 0, bSig64 );
		q64 = ( 2 < q64 ) ? q64 - 2 : 0;
		q = q64>>( 64 - expDiff );
		bSig <<= 6;
		aSig = ( ( aSig64>>33 )<<( expDiff - 1 ) ) - bSig * q;
	}
	do {
		alternateASig = aSig;
		++q;
		aSig -= bSig;
	} while ( 0 <= (sbits32) aSig );
	sigMean = aSig + alternateASig;
	if ( ( sigMean < 0 ) || ( ( sigMean == 0 ) && ( q & 1 ) ) ) {
		aSig = alternateASig;
	}
	zSign = ( (sbits32) aSig < 0 );
	if ( zSign ) aSig = - aSig;
	return normalizeRoundAndPackFloat32( aSign ^ zSign, bExp, aSig, c );

}

/*----------------------------------------------------------------------------
| Returns the square root of the single-precision floating-point value `a'.
| The operation is performed according to the IEC/IEEE Standard for Binary
| Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float32 float32_sqrt( float32 a, float_ctrl* c )
{
	flag aSign;
	int16 aExp, zExp;
	bits32 aSig, zSig;
	bits64 rem, term;

	aSig = extractFloat32Frac( a );
	aExp = extractFloat32Exp( a );
	aSign = extractFloat32Sign( a );
	if ( aExp == 0xFF ) {
		if ( aSig ) return propagateFloat32NaN( a, 0, c );
		if ( ! aSign ) return a;
#ifdef SOFTFLOAT_I860
        float_raise( float_flag_invalid, c );
#else
		float_raise( float_flag_invalid );
#endif
		return float32_default_nan;
	}
	if ( aSign ) {
		if ( ( aExp | aSig ) == 0 ) return a;
#ifdef SOFTFLOAT_I860
		float_raise( float_flag_invalid, c );
#else
		float_raise( float_flag_invalid );
#endif
		return float32_default_nan;
	}
	if ( aExp == 0 ) {
		if ( aSig == 0 ) return 0;
		normalizeFloat32Subnormal( aSig, &aExp, &aSig );
	}
	zExp = ( ( aExp - 0x7F )>>1 ) + 0x7E;
	aSig = ( aSig | 0x00800000 )<<8;
	zSig = estimateSqrt32( aExp, aSig ) + 2;
	if ( ( zSig & 0x7F ) <= 5 ) {
		if ( zSig < 2 ) {
			zSig = 0x7FFFFFFF;
			goto roundAndPack;
		}
		aSig >>= aExp & 1;
		term = ( (bits64) zSig ) * zSig;
		rem = ( ( (bits64) aSig )<<32 ) - term;
		while ( (sbits64) rem < 0 ) {
			--zSig;
			rem += ( ( (bits64) zSig )<<1 ) | 1;
		}
		zSig |= ( rem != 0 );
	}
	shift32RightJamming( zSig, 1, &zSig );
	roundAndPack:
#ifdef SOFTFLOAT_I860
    return roundAndPackFloat32_2( 0, zExp, zSig, c );
#else
	return roundAndPackFloat32( 0, zExp, zSig );
#endif

}

/*----------------------------------------------------------------------------
| Returns 1 if the single-precision floating-point value `a' is equal to
| the corresponding value `b', and 0 otherwise.  The comparison is performed
| according to the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

flag float32_eq( float32 a, float32 b, float_ctrl* c )
{
	if (    ( ( extractFloat32Exp( a ) == 0xFF ) && extractFloat32Frac( a ) )
			|| ( ( extractFloat32Exp( b ) == 0xFF ) && extractFloat32Frac( b ) )
		) {
		if ( float32_is_signaling_nan( a ) || float32_is_signaling_nan( b ) ) {
#ifdef SOFTFLOAT_I860
            float_raise( float_flag_invalid, c );
#else
			float_raise( float_flag_invalid );
#endif
		}
		return 0;
	}
	return ( a == b ) || ( (bits32) ( ( a | b )<<1 ) == 0 );

}

/*----------------------------------------------------------------------------
| Returns 1 if the single-precision floating-point value `a' is less than
| or equal to the corresponding value `b', and 0 otherwise.  The comparison
| is performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic.
*----------------------------------------------------------------------------*/

flag float32_le( float32 a, float32 b, float_ctrl* c )
{
	flag aSign, bSign;

	if (    ( ( extractFloat32Exp( a ) == 0xFF ) && extractFloat32Frac( a ) )
			|| ( ( extractFloat32Exp( b ) == 0xFF ) && extractFloat32Frac( b ) )
		) {
#ifdef SOFTFLOAT_I860
        float_raise( float_flag_invalid, c );
#else
		float_raise( float_flag_invalid );
#endif
		return 0;
	}
	aSign = extractFloat32Sign( a );
	bSign = extractFloat32Sign( b );
	if ( aSign != bSign ) return aSign || ( (bits32) ( ( a | b )<<1 ) == 0 );
	return ( a == b ) || ( aSign ^ ( a < b ) );

}

/*----------------------------------------------------------------------------
| Returns 1 if the single-precision floating-point value `a' is less than
| the corresponding value `b', and 0 otherwise.  The comparison is performed
| according to the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

flag float32_lt( float32 a, float32 b, float_ctrl* c )
{
	flag aSign, bSign;

	if (    ( ( extractFloat32Exp( a ) == 0xFF ) && extractFloat32Frac( a ) )
			|| ( ( extractFloat32Exp( b ) == 0xFF ) && extractFloat32Frac( b ) )
		) {
#ifdef SOFTFLOAT_I860
        float_raise( float_flag_invalid, c );
#else
		float_raise( float_flag_invalid );
#endif
		return 0;
	}
	aSign = extractFloat32Sign( a );
	bSign = extractFloat32Sign( b );
	if ( aSign != bSign ) return aSign && ( (bits32) ( ( a | b )<<1 ) != 0 );
	return ( a != b ) && ( aSign ^ ( a < b ) );

}

#ifdef SOFTFLOAT_I860 // 29-04-2017: Added for Previous
/*----------------------------------------------------------------------------
| Returns 1 if the single-precision floating-point value `a' is greater than
| the corresponding value `b', and 0 otherwise.  The comparison is performed
| according to the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

flag float32_gt( float32 a, float32 b, float_ctrl* c )
{
    flag aSign, bSign;
    
    if (   ( ( extractFloat32Exp( a ) == 0xFF ) && extractFloat32Frac( a ) )
        || ( ( extractFloat32Exp( b ) == 0xFF ) && extractFloat32Frac( b ) )
        ) {
#ifdef SOFTFLOAT_I860
        float_raise( float_flag_invalid, c );
#else
        float_raise( float_flag_invalid );
#endif
        return 0;
    }
    aSign = extractFloat32Sign( a );
    bSign = extractFloat32Sign( b );
    if ( aSign != bSign ) return bSign && ( (bits32) ( ( a | b )<<1 ) != 0 );
    return ( a != b ) && ( bSign ^ ( a > b ) );
    
}
#endif // End of addition for Previous

/*----------------------------------------------------------------------------
| Returns 1 if the single-precision floating-point value `a' is equal to
| the corresponding value `b', and 0 otherwise.  The invalid exception is
| raised if either operand is a NaN.  Otherwise, the comparison is performed
| according to the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

flag float32_eq_signaling( float32 a, float32 b, float_ctrl* c )
{
	if (    ( ( extractFloat32Exp( a ) == 0xFF ) && extractFloat32Frac( a ) )
			|| ( ( extractFloat32Exp( b ) == 0xFF ) && extractFloat32Frac( b ) )
		) {
		float_raise( float_flag_invalid, c );
		return 0;
	}
	return ( a == b ) || ( (bits32) ( ( a | b )<<1 ) == 0 );

}

/*----------------------------------------------------------------------------
| Returns 1 if the single-precision floating-point value `a' is less than or
| equal to the corresponding value `b', and 0 otherwise.  Quiet NaNs do not
| cause an exception.  Otherwise, the comparison is performed according to the
| IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

flag float32_le_quiet( float32 a, float32 b, float_ctrl* c )
{
	flag aSign, bSign;
//    int16 aExp, bExp;

	if (    ( ( extractFloat32Exp( a ) == 0xFF ) && extractFloat32Frac( a ) )
			|| ( ( extractFloat32Exp( b ) == 0xFF ) && extractFloat32Frac( b ) )
		) {
		if ( float32_is_signaling_nan( a ) || float32_is_signaling_nan( b ) ) {
			float_raise( float_flag_invalid, c );
		}
		return 0;
	}
	aSign = extractFloat32Sign( a );
	bSign = extractFloat32Sign( b );
	if ( aSign != bSign ) return aSign || ( (bits32) ( ( a | b )<<1 ) == 0 );
	return ( a == b ) || ( aSign ^ ( a < b ) );

}

/*----------------------------------------------------------------------------
| Returns 1 if the single-precision floating-point value `a' is less than
| the corresponding value `b', and 0 otherwise.  Quiet NaNs do not cause an
| exception.  Otherwise, the comparison is performed according to the IEC/IEEE
| Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

flag float32_lt_quiet( float32 a, float32 b, float_ctrl* c )
{
	flag aSign, bSign;

	if (    ( ( extractFloat32Exp( a ) == 0xFF ) && extractFloat32Frac( a ) )
			|| ( ( extractFloat32Exp( b ) == 0xFF ) && extractFloat32Frac( b ) )
		) {
		if ( float32_is_signaling_nan( a ) || float32_is_signaling_nan( b ) ) {
			float_raise( float_flag_invalid, c );
		}
		return 0;
	}
	aSign = extractFloat32Sign( a );
	bSign = extractFloat32Sign( b );
	if ( aSign != bSign ) return aSign && ( (bits32) ( ( a | b )<<1 ) != 0 );
	return ( a != b ) && ( aSign ^ ( a < b ) );

}

/*----------------------------------------------------------------------------
| Returns the result of converting the double-precision floating-point value
| `a' to the 32-bit two's complement integer format.  The conversion is
| performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic---which means in particular that the conversion is rounded
| according to the current rounding mode.  If `a' is a NaN, the largest
| positive integer is returned.  Otherwise, if the conversion overflows, the
| largest integer with the same sign as `a' is returned.
*----------------------------------------------------------------------------*/

int32 float64_to_int32( float64 a, float_ctrl* c )
{
	flag aSign;
	int16 aExp, shiftCount;
	bits64 aSig;

	aSig = extractFloat64Frac( a );
	aExp = extractFloat64Exp( a );
	aSign = extractFloat64Sign( a );
	if ( ( aExp == 0x7FF ) && aSig ) aSign = 0;
	if ( aExp ) aSig |= LIT64( 0x0010000000000000 );
	shiftCount = 0x42C - aExp;
	if ( 0 < shiftCount ) shift64RightJamming( aSig, shiftCount, &aSig );
#ifdef SOFTFLOAT_I860
    return roundAndPackInt32_2( aSign, aSig, c );
#else
	return roundAndPackInt32( aSign, aSig );
#endif

}

/*----------------------------------------------------------------------------
| Returns the result of converting the double-precision floating-point value
| `a' to the 32-bit two's complement integer format.  The conversion is
| performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic, except that the conversion is always rounded toward zero.
| If `a' is a NaN, the largest positive integer is returned.  Otherwise, if
| the conversion overflows, the largest integer with the same sign as `a' is
| returned.
*----------------------------------------------------------------------------*/

int32 float64_to_int32_round_to_zero( float64 a, float_ctrl* c )
{
	flag aSign;
	int16 aExp, shiftCount;
	bits64 aSig, savedASig;
	int32 z;

	aSig = extractFloat64Frac( a );
	aExp = extractFloat64Exp( a );
	aSign = extractFloat64Sign( a );
	if ( 0x41E < aExp ) {
		if ( ( aExp == 0x7FF ) && aSig ) aSign = 0;
		goto invalid;
	}
	else if ( aExp < 0x3FF ) {
#ifdef SOFTFLOAT_I860
        if ( aExp || aSig ) float_raise( float_flag_inexact, c );
#else
		if ( aExp || aSig ) float_exception_flags |= float_flag_inexact;
#endif
		return 0;
	}
	aSig |= LIT64( 0x0010000000000000 );
	shiftCount = 0x433 - aExp;
	savedASig = aSig;
	aSig >>= shiftCount;
	z = aSig;
	if ( aSign ) z = - z;
    z = (sbits32) z;
	if ( ( z < 0 ) ^ aSign ) {
	invalid:
#ifdef SOFTFLOAT_I860
        float_raise( float_flag_invalid, c );
#else
		float_raise( float_flag_invalid );
#endif
		return aSign ? (sbits32) 0x80000000 : 0x7FFFFFFF;
	}
	if ( ( aSig<<shiftCount ) != savedASig ) {
#ifdef SOFTFLOAT_I860
        float_raise( float_flag_inexact, c );
#else
		float_exception_flags |= float_flag_inexact;
#endif
	}
	return z;

}

/*----------------------------------------------------------------------------
| Returns the result of converting the double-precision floating-point value
| `a' to the 64-bit two's complement integer format.  The conversion is
| performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic---which means in particular that the conversion is rounded
| according to the current rounding mode.  If `a' is a NaN, the largest
| positive integer is returned.  Otherwise, if the conversion overflows, the
| largest integer with the same sign as `a' is returned.
*----------------------------------------------------------------------------*/

int64 float64_to_int64( float64 a, float_ctrl* c )
{
	flag aSign;
	int16 aExp, shiftCount;
	bits64 aSig, aSigExtra;

	aSig = extractFloat64Frac( a );
	aExp = extractFloat64Exp( a );
	aSign = extractFloat64Sign( a );
	if ( aExp ) aSig |= LIT64( 0x0010000000000000 );
	shiftCount = 0x433 - aExp;
	if ( shiftCount <= 0 ) {
		if ( 0x43E < aExp ) {
			float_raise( float_flag_invalid, c );
			if (    ! aSign
					|| (    ( aExp == 0x7FF )
						&& ( aSig != LIT64( 0x0010000000000000 ) ) )
				) {
				return LIT64( 0x7FFFFFFFFFFFFFFF );
			}
			return (sbits64) LIT64( 0x8000000000000000 );
		}
		aSigExtra = 0;
		aSig <<= - shiftCount;
	}
	else {
		shift64ExtraRightJamming( aSig, 0, shiftCount, &aSig, &aSigExtra );
	}
	return roundAndPackInt64( aSign, aSig, aSigExtra, c );

}

/*----------------------------------------------------------------------------
| Returns the result of converting the double-precision floating-point value
| `a' to the 64-bit two's complement integer format.  The conversion is
| performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic, except that the conversion is always rounded toward zero.
| If `a' is a NaN, the largest positive integer is returned.  Otherwise, if
| the conversion overflows, the largest integer with the same sign as `a' is
| returned.
*----------------------------------------------------------------------------*/

int64 float64_to_int64_round_to_zero( float64 a, float_ctrl* c )
{
	flag aSign;
	int16 aExp, shiftCount;
	bits64 aSig;
	int64 z;

	aSig = extractFloat64Frac( a );
	aExp = extractFloat64Exp( a );
	aSign = extractFloat64Sign( a );
	if ( aExp ) aSig |= LIT64( 0x0010000000000000 );
	shiftCount = aExp - 0x433;
	if ( 0 <= shiftCount ) {
		if ( 0x43E <= aExp ) {
			if ( a != LIT64( 0xC3E0000000000000 ) ) {
				float_raise( float_flag_invalid, c );
				if (    ! aSign
						|| (    ( aExp == 0x7FF )
							&& ( aSig != LIT64( 0x0010000000000000 ) ) )
					) {
					return LIT64( 0x7FFFFFFFFFFFFFFF );
				}
			}
			return (sbits64) LIT64( 0x8000000000000000 );
		}
		z = aSig<<shiftCount;
	}
	else {
		if ( aExp < 0x3FE ) {
			if ( aExp | aSig ) float_raise( float_flag_inexact, c );
			return 0;
		}
		z = aSig>>( - shiftCount );
		if ( (bits64) ( aSig<<( shiftCount & 63 ) ) ) {
			float_raise( float_flag_inexact, c );
		}
	}
	if ( aSign ) z = - z;
	return z;

}

/*----------------------------------------------------------------------------
| Returns the result of converting the double-precision floating-point value
| `a' to the single-precision floating-point format.  The conversion is
| performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic.
*----------------------------------------------------------------------------*/

float32 float64_to_float32( float64 a, float_ctrl* c )
{
	flag aSign;
	int16 aExp;
	bits64 aSig;
	bits32 zSig;

	aSig = extractFloat64Frac( a );
	aExp = extractFloat64Exp( a );
	aSign = extractFloat64Sign( a );
	if ( aExp == 0x7FF ) {
		if ( aSig ) return commonNaNToFloat32( float64ToCommonNaN( a, c ) );
		return packFloat32( aSign, 0xFF, 0 );
	}
	shift64RightJamming( aSig, 22, &aSig );
	zSig = aSig;
	if ( aExp || zSig ) {
		zSig |= 0x40000000;
		aExp -= 0x381;
	}
#ifdef SOFTFLOAT_I860
    return roundAndPackFloat32_2( aSign, aExp, zSig, c );
#else
	return roundAndPackFloat32( aSign, aExp, zSig );
#endif

}

#ifdef FLOATX80

/*----------------------------------------------------------------------------
| Returns the result of converting the double-precision floating-point value
| `a' to the extended double-precision floating-point format.  The conversion
| is performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic.
*----------------------------------------------------------------------------*/

floatx80 float64_to_floatx80( float64 a, float_ctrl* c )
{
	flag aSign;
	int16 aExp;
	bits64 aSig;

	aSig = extractFloat64Frac( a );
	aExp = extractFloat64Exp( a );
	aSign = extractFloat64Sign( a );
	if ( aExp == 0x7FF ) {
		if ( aSig ) return commonNaNToFloatx80( float64ToCommonNaN( a, c ) );
		return packFloatx80( aSign, floatx80_default_infinity_high, floatx80_default_infinity_low );
	}
	if ( aExp == 0 ) {
		if ( aSig == 0 ) return packFloatx80( aSign, 0, 0 );
		normalizeFloat64Subnormal( aSig, &aExp, &aSig );
	}
	return
		packFloatx80(
			aSign, aExp + 0x3C00, ( aSig | LIT64( 0x0010000000000000 ) )<<11 );

}

#ifdef SOFTFLOAT_68K // 31-12-2016: Added for Previous
floatx80 float64_to_floatx80_allowunnormal( float64 a )
{
    flag aSign;
    int16 aExp;
    bits64 aSig;
    
    aSig = extractFloat64Frac( a );
    aExp = extractFloat64Exp( a );
    aSign = extractFloat64Sign( a );
    if ( aExp == 0x7FF ) {
        return packFloatx80( aSign, 0x7FFF, aSig<<11 );
    }
    if ( aExp == 0 ) {
        if ( aSig == 0 ) return packFloatx80( aSign, 0, 0 );
        return packFloatx80( aSign, 0x3C01, aSig<<11 );
    }
    return
    packFloatx80(
                 aSign, aExp + 0x3C00, ( aSig | LIT64( 0x0010000000000000 ) )<<11 );
    
}
#endif // end of addition for Previous

#endif

#ifdef FLOAT128

/*----------------------------------------------------------------------------
| Returns the result of converting the double-precision floating-point value
| `a' to the quadruple-precision floating-point format.  The conversion is
| performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic.
*----------------------------------------------------------------------------*/

float128 float64_to_float128( float64 a, float_ctrl* c )
{
	flag aSign;
	int16 aExp;
	bits64 aSig, zSig0, zSig1;

	aSig = extractFloat64Frac( a );
	aExp = extractFloat64Exp( a );
	aSign = extractFloat64Sign( a );
	if ( aExp == 0x7FF ) {
		if ( aSig ) return commonNaNToFloat128( float64ToCommonNaN( a, c ) );
		return packFloat128( aSign, 0x7FFF, 0, 0 );
	}
	if ( aExp == 0 ) {
		if ( aSig == 0 ) return packFloat128( aSign, 0, 0, 0 );
		normalizeFloat64Subnormal( aSig, &aExp, &aSig );
		--aExp;
	}
	shift128Right( aSig, 0, 4, &zSig0, &zSig1 );
	return packFloat128( aSign, aExp + 0x3C00, zSig0, zSig1 );

}

#endif

/*----------------------------------------------------------------------------
| Rounds the double-precision floating-point value `a' to an integer, and
| returns the result as a double-precision floating-point value.  The
| operation is performed according to the IEC/IEEE Standard for Binary
| Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float64 float64_round_to_int( float64 a, float_ctrl* c )
{
	flag aSign;
	int16 aExp;
	bits64 lastBitMask, roundBitsMask;
	int8 roundingMode;
	float64 z;

	aExp = extractFloat64Exp( a );
	if ( 0x433 <= aExp ) {
		if ( ( aExp == 0x7FF ) && extractFloat64Frac( a ) ) {
			return propagateFloat64NaN( a, a, c );
		}
		return a;
	}
	if ( aExp < 0x3FF ) {
		if ( (bits64) ( a<<1 ) == 0 ) return a;
		float_raise( float_flag_inexact, c );
		aSign = extractFloat64Sign( a );
		switch ( get_float_rounding_mode( c ) ) {
			case float_round_nearest_even:
			if ( ( aExp == 0x3FE ) && extractFloat64Frac( a ) ) {
				return packFloat64( aSign, 0x3FF, 0 );
			}
			break;
			case float_round_down:
			return aSign ? LIT64( 0xBFF0000000000000 ) : 0;
			case float_round_up:
			return
			aSign ? LIT64( 0x8000000000000000 ) : LIT64( 0x3FF0000000000000 );
		}
		return packFloat64( aSign, 0, 0 );
	}
	lastBitMask = 1;
	lastBitMask <<= 0x433 - aExp;
	roundBitsMask = lastBitMask - 1;
	z = a;
	roundingMode = get_float_rounding_mode( c );
	if ( roundingMode == float_round_nearest_even ) {
		z += lastBitMask>>1;
		if ( ( z & roundBitsMask ) == 0 ) z &= ~ lastBitMask;
	}
	else if ( roundingMode != float_round_to_zero ) {
		if ( extractFloat64Sign( z ) ^ ( roundingMode == float_round_up ) ) {
			z += roundBitsMask;
		}
	}
	z &= ~ roundBitsMask;
	if ( z != a ) float_raise( float_flag_inexact, c );
	return z;

}

/*----------------------------------------------------------------------------
| Returns the result of adding the absolute values of the double-precision
| floating-point values `a' and `b'.  If `zSign' is 1, the sum is negated
| before being returned.  `zSign' is ignored if the result is a NaN.
| The addition is performed according to the IEC/IEEE Standard for Binary
| Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

static float64 addFloat64Sigs( float64 a, float64 b, flag zSign, float_ctrl* c )
{
	int16 aExp, bExp, zExp;
	bits64 aSig, bSig, zSig;
	int16 expDiff;

	aSig = extractFloat64Frac( a );
	aExp = extractFloat64Exp( a );
	bSig = extractFloat64Frac( b );
	bExp = extractFloat64Exp( b );
	expDiff = aExp - bExp;
	aSig <<= 9;
	bSig <<= 9;
	if ( 0 < expDiff ) {
		if ( aExp == 0x7FF ) {
			if ( aSig ) return propagateFloat64NaN( a, b, c );
			return a;
		}
		if ( bExp == 0 ) {
			--expDiff;
		}
		else {
			bSig |= LIT64( 0x2000000000000000 );
		}
		shift64RightJamming( bSig, expDiff, &bSig );
		zExp = aExp;
	}
	else if ( expDiff < 0 ) {
		if ( bExp == 0x7FF ) {
			if ( bSig ) return propagateFloat64NaN( a, b, c );
			return packFloat64( zSign, 0x7FF, 0 );
		}
		if ( aExp == 0 ) {
			++expDiff;
		}
		else {
			aSig |= LIT64( 0x2000000000000000 );
		}
		shift64RightJamming( aSig, - expDiff, &aSig );
		zExp = bExp;
	}
	else {
		if ( aExp == 0x7FF ) {
			if ( aSig | bSig ) return propagateFloat64NaN( a, b, c );
			return a;
		}
		if ( aExp == 0 ) return packFloat64( zSign, 0, ( aSig + bSig )>>9 );
		zSig = LIT64( 0x4000000000000000 ) + aSig + bSig;
		zExp = aExp;
		goto roundAndPack;
	}
	aSig |= LIT64( 0x2000000000000000 );
	zSig = ( aSig + bSig )<<1;
	--zExp;
	if ( (sbits64) zSig < 0 ) {
		zSig = aSig + bSig;
		++zExp;
	}
	roundAndPack:
#ifdef SOFTFLOAT_I860
    return roundAndPackFloat64_2( zSign, zExp, zSig, c );
#else
	return roundAndPackFloat64( zSign, zExp, zSig );
#endif

}

/*----------------------------------------------------------------------------
| Returns the result of subtracting the absolute values of the double-
| precision floating-point values `a' and `b'.  If `zSign' is 1, the
| difference is negated before being returned.  `zSign' is ignored if the
| result is a NaN.  The subtraction is performed according to the IEC/IEEE
| Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

static float64 subFloat64Sigs( float64 a, float64 b, flag zSign, float_ctrl* c )
{
	int16 aExp, bExp, zExp;
	bits64 aSig, bSig, zSig;
	int16 expDiff;
#ifdef SOFTFLOAT_I860
    int8 shiftCount;
#endif

	aSig = extractFloat64Frac( a );
	aExp = extractFloat64Exp( a );
	bSig = extractFloat64Frac( b );
	bExp = extractFloat64Exp( b );
	expDiff = aExp - bExp;
	aSig <<= 10;
	bSig <<= 10;
	if ( 0 < expDiff ) goto aExpBigger;
	if ( expDiff < 0 ) goto bExpBigger;
	if ( aExp == 0x7FF ) {
		if ( aSig | bSig ) return propagateFloat64NaN( a, b, c );
#ifdef SOFTFLOAT_I860
        float_raise( float_flag_invalid, c );
#else
		float_raise( float_flag_invalid );
#endif
		return float64_default_nan;
	}
	if ( aExp == 0 ) {
		aExp = 1;
		bExp = 1;
	}
	if ( bSig < aSig ) goto aBigger;
	if ( aSig < bSig ) goto bBigger;
#ifdef SOFTFLOAT_I860
    return packFloat64( get_float_rounding_mode( c ) == float_round_down, 0, 0 );
#else
	return packFloat64( float_rounding_mode == float_round_down, 0, 0 );
#endif
	bExpBigger:
	if ( bExp == 0x7FF ) {
		if ( bSig ) return propagateFloat64NaN( a, b, c );
		return packFloat64( zSign ^ 1, 0x7FF, 0 );
	}
	if ( aExp == 0 ) {
		++expDiff;
	}
	else {
		aSig |= LIT64( 0x4000000000000000 );
	}
	shift64RightJamming( aSig, - expDiff, &aSig );
	bSig |= LIT64( 0x4000000000000000 );
	bBigger:
	zSig = bSig - aSig;
	zExp = bExp;
	zSign ^= 1;
	goto normalizeRoundAndPack;
	aExpBigger:
	if ( aExp == 0x7FF ) {
		if ( aSig ) return propagateFloat64NaN( a, b, c );
		return a;
	}
	if ( bExp == 0 ) {
		--expDiff;
	}
	else {
		bSig |= LIT64( 0x4000000000000000 );
	}
	shift64RightJamming( bSig, expDiff, &bSig );
	aSig |= LIT64( 0x4000000000000000 );
	aBigger:
	zSig = aSig - bSig;
	zExp = aExp;
	normalizeRoundAndPack:
	--zExp;
#ifdef SOFTFLOAT_I860
    shiftCount = countLeadingZeros64( zSig ) - 1;
    return roundAndPackFloat64_2( zSign, zExp - shiftCount, zSig<<shiftCount, c );
#else
	return normalizeRoundAndPackFloat64( zSign, zExp, zSig );
#endif

}

/*----------------------------------------------------------------------------
| Returns the result of adding the double-precision floating-point values `a'
| and `b'.  The operation is performed according to the IEC/IEEE Standard for
| Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float64 float64_add( float64 a, float64 b, float_ctrl* c )
{
	flag aSign, bSign;

	aSign = extractFloat64Sign( a );
	bSign = extractFloat64Sign( b );
	if ( aSign == bSign ) {
		return addFloat64Sigs( a, b, aSign, c );
	}
	else {
		return subFloat64Sigs( a, b, aSign, c );
	}

}

/*----------------------------------------------------------------------------
| Returns the result of subtracting the double-precision floating-point values
| `a' and `b'.  The operation is performed according to the IEC/IEEE Standard
| for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float64 float64_sub( float64 a, float64 b, float_ctrl* c )
{
	flag aSign, bSign;

	aSign = extractFloat64Sign( a );
	bSign = extractFloat64Sign( b );
	if ( aSign == bSign ) {
		return subFloat64Sigs( a, b, aSign, c );
	}
	else {
		return addFloat64Sigs( a, b, aSign, c );
	}

}

/*----------------------------------------------------------------------------
| Returns the result of multiplying the double-precision floating-point values
| `a' and `b'.  The operation is performed according to the IEC/IEEE Standard
| for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float64 float64_mul( float64 a, float64 b, float_ctrl* c )
{
	flag aSign, bSign, zSign;
	int16 aExp, bExp, zExp;
	bits64 aSig, bSig, zSig0, zSig1;

	aSig = extractFloat64Frac( a );
	aExp = extractFloat64Exp( a );
	aSign = extractFloat64Sign( a );
	bSig = extractFloat64Frac( b );
	bExp = extractFloat64Exp( b );
	bSign = extractFloat64Sign( b );
	zSign = aSign ^ bSign;
	if ( aExp == 0x7FF ) {
		if ( aSig || ( ( bExp == 0x7FF ) && bSig ) ) {
			return propagateFloat64NaN( a, b, c );
		}
		if ( ( bExp | bSig ) == 0 ) {
#ifdef SOFTFLOAT_I860
            float_raise( float_flag_invalid, c );
#else
			float_raise( float_flag_invalid );
#endif
			return float64_default_nan;
		}
		return packFloat64( zSign, 0x7FF, 0 );
	}
	if ( bExp == 0x7FF ) {
		if ( bSig ) return propagateFloat64NaN( a, b, c );
		if ( ( aExp | aSig ) == 0 ) {
#ifdef SOFTFLOAT_I860
            float_raise( float_flag_invalid, c );
#else
			float_raise( float_flag_invalid );
#endif
			return float64_default_nan;
		}
		return packFloat64( zSign, 0x7FF, 0 );
	}
	if ( aExp == 0 ) {
		if ( aSig == 0 ) return packFloat64( zSign, 0, 0 );
		normalizeFloat64Subnormal( aSig, &aExp, &aSig );
	}
	if ( bExp == 0 ) {
		if ( bSig == 0 ) return packFloat64( zSign, 0, 0 );
		normalizeFloat64Subnormal( bSig, &bExp, &bSig );
	}
	zExp = aExp + bExp - 0x3FF;
	aSig = ( aSig | LIT64( 0x0010000000000000 ) )<<10;
	bSig = ( bSig | LIT64( 0x0010000000000000 ) )<<11;
	mul64To128( aSig, bSig, &zSig0, &zSig1 );
	zSig0 |= ( zSig1 != 0 );
	if ( 0 <= (sbits64) ( zSig0<<1 ) ) {
		zSig0 <<= 1;
		--zExp;
	}
#ifdef SOFTFLOAT_I860
    return roundAndPackFloat64_2( zSign, zExp, zSig0, c );
#else
	return roundAndPackFloat64( zSign, zExp, zSig0 );
#endif

}

/*----------------------------------------------------------------------------
| Returns the result of dividing the double-precision floating-point value `a'
| by the corresponding value `b'.  The operation is performed according to
| the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float64 float64_div( float64 a, float64 b, float_ctrl* c )
{
	flag aSign, bSign, zSign;
	int16 aExp, bExp, zExp;
	bits64 aSig, bSig, zSig;
	bits64 rem0, rem1;
	bits64 term0, term1;

	aSig = extractFloat64Frac( a );
	aExp = extractFloat64Exp( a );
	aSign = extractFloat64Sign( a );
	bSig = extractFloat64Frac( b );
	bExp = extractFloat64Exp( b );
	bSign = extractFloat64Sign( b );
	zSign = aSign ^ bSign;
	if ( aExp == 0x7FF ) {
		if ( aSig ) return propagateFloat64NaN( a, b, c );
		if ( bExp == 0x7FF ) {
			if ( bSig ) return propagateFloat64NaN( a, b, c );
#ifdef SOFTFLOAT_I860
            float_raise( float_flag_invalid, c );
#else
			float_raise( float_flag_invalid );
#endif
			return float64_default_nan;
		}
		return packFloat64( zSign, 0x7FF, 0 );
	}
	if ( bExp == 0x7FF ) {
		if ( bSig ) return propagateFloat64NaN( a, b, c );
		return packFloat64( zSign, 0, 0 );
	}
	if ( bExp == 0 ) {
		if ( bSig == 0 ) {
			if ( ( aExp | aSig ) == 0 ) {
#ifdef SOFTFLOAT_I860
                float_raise( float_flag_invalid, c );
#else
				float_raise( float_flag_invalid );
#endif
				return float64_default_nan;
			}
#ifdef SOFTFLOAT_I860
            float_raise( float_flag_divbyzero, c );
#else
			float_raise( float_flag_divbyzero );
#endif
			return packFloat64( zSign, 0x7FF, 0 );
		}
		normalizeFloat64Subnormal( bSig, &bExp, &bSig );
	}
	if ( aExp == 0 ) {
		if ( aSig == 0 ) return packFloat64( zSign, 0, 0 );
		normalizeFloat64Subnormal( aSig, &aExp, &aSig );
	}
	zExp = aExp - bExp + 0x3FD;
	aSig = ( aSig | LIT64( 0x0010000000000000 ) )<<10;
	bSig = ( bSig | LIT64( 0x0010000000000000 ) )<<11;
	if ( bSig <= ( aSig + aSig ) ) {
		aSig >>= 1;
		++zExp;
	}
	zSig = estimateDiv128To64( aSig, 0, bSig );
	if ( ( zSig & 0x1FF ) <= 2 ) {
		mul64To128( bSig, zSig, &term0, &term1 );
		sub128( aSig, 0, term0, term1, &rem0, &rem1 );
		while ( (sbits64) rem0 < 0 ) {
			--zSig;
			add128( rem0, rem1, 0, bSig, &rem0, &rem1 );
		}
		zSig |= ( rem1 != 0 );
	}
#ifdef SOFTFLOAT_I860
    return roundAndPackFloat64_2( zSign, zExp, zSig, c );
#else
	return roundAndPackFloat64( zSign, zExp, zSig );
#endif

}

/*----------------------------------------------------------------------------
| Returns the remainder of the double-precision floating-point value `a'
| with respect to the corresponding value `b'.  The operation is performed
| according to the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float64 float64_rem( float64 a, float64 b, float_ctrl* c )
{
	flag aSign, zSign;
	int16 aExp, bExp, expDiff;
	bits64 aSig, bSig;
	bits64 q, alternateASig;
	sbits64 sigMean;

	aSig = extractFloat64Frac( a );
	aExp = extractFloat64Exp( a );
	aSign = extractFloat64Sign( a );
	bSig = extractFloat64Frac( b );
	bExp = extractFloat64Exp( b );
//    bSign = extractFloat64Sign( b );
	if ( aExp == 0x7FF ) {
		if ( aSig || ( ( bExp == 0x7FF ) && bSig ) ) {
			return propagateFloat64NaN( a, b, c );
		}
		float_raise( float_flag_invalid, c );
		return float64_default_nan;
	}
	if ( bExp == 0x7FF ) {
		if ( bSig ) return propagateFloat64NaN( a, b, c );
		return a;
	}
	if ( bExp == 0 ) {
		if ( bSig == 0 ) {
			float_raise( float_flag_invalid, c );
			return float64_default_nan;
		}
		normalizeFloat64Subnormal( bSig, &bExp, &bSig );
	}
	if ( aExp == 0 ) {
		if ( aSig == 0 ) return a;
		normalizeFloat64Subnormal( aSig, &aExp, &aSig );
	}
	expDiff = aExp - bExp;
	aSig = ( aSig | LIT64( 0x0010000000000000 ) )<<11;
	bSig = ( bSig | LIT64( 0x0010000000000000 ) )<<11;
	if ( expDiff < 0 ) {
		if ( expDiff < -1 ) return a;
		aSig >>= 1;
	}
	q = ( bSig <= aSig );
	if ( q ) aSig -= bSig;
	expDiff -= 64;
	while ( 0 < expDiff ) {
		q = estimateDiv128To64( aSig, 0, bSig );
		q = ( 2 < q ) ? q - 2 : 0;
		aSig = - ( ( bSig>>2 ) * q );
		expDiff -= 62;
	}
	expDiff += 64;
	if ( 0 < expDiff ) {
		q = estimateDiv128To64( aSig, 0, bSig );
		q = ( 2 < q ) ? q - 2 : 0;
		q >>= 64 - expDiff;
		bSig >>= 2;
		aSig = ( ( aSig>>1 )<<( expDiff - 1 ) ) - bSig * q;
	}
	else {
		aSig >>= 2;
		bSig >>= 2;
	}
	do {
		alternateASig = aSig;
		++q;
		aSig -= bSig;
	} while ( 0 <= (sbits64) aSig );
	sigMean = aSig + alternateASig;
	if ( ( sigMean < 0 ) || ( ( sigMean == 0 ) && ( q & 1 ) ) ) {
		aSig = alternateASig;
	}
	zSign = ( (sbits64) aSig < 0 );
	if ( zSign ) aSig = - aSig;
	return normalizeRoundAndPackFloat64( aSign ^ zSign, bExp, aSig, c );

}

/*----------------------------------------------------------------------------
| Returns the square root of the double-precision floating-point value `a'.
| The operation is performed according to the IEC/IEEE Standard for Binary
| Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float64 float64_sqrt( float64 a, float_ctrl* c )
{
	flag aSign;
	int16 aExp, zExp;
	bits64 aSig, zSig, doubleZSig;
	bits64 rem0, rem1, term0, term1;

	aSig = extractFloat64Frac( a );
	aExp = extractFloat64Exp( a );
	aSign = extractFloat64Sign( a );
	if ( aExp == 0x7FF ) {
		if ( aSig ) return propagateFloat64NaN( a, a, c );
		if ( ! aSign ) return a;
#ifdef SOFTFLOAT_I860
        float_raise( float_flag_invalid, c );
#else
		float_raise( float_flag_invalid );
#endif
		return float64_default_nan;
	}
	if ( aSign ) {
		if ( ( aExp | aSig ) == 0 ) return a;
#ifdef SOFTFLOAT_I860
        float_raise( float_flag_invalid, c );
#else
		float_raise( float_flag_invalid );
#endif
		return float64_default_nan;
	}
	if ( aExp == 0 ) {
		if ( aSig == 0 ) return 0;
		normalizeFloat64Subnormal( aSig, &aExp, &aSig );
	}
	zExp = ( ( aExp - 0x3FF )>>1 ) + 0x3FE;
	aSig |= LIT64( 0x0010000000000000 );
	zSig = estimateSqrt32( aExp, aSig>>21 );
	aSig <<= 9 - ( aExp & 1 );
	zSig = estimateDiv128To64( aSig, 0, zSig<<32 ) + ( zSig<<30 );
	if ( ( zSig & 0x1FF ) <= 5 ) {
		doubleZSig = zSig<<1;
		mul64To128( zSig, zSig, &term0, &term1 );
		sub128( aSig, 0, term0, term1, &rem0, &rem1 );
		while ( (sbits64) rem0 < 0 ) {
			--zSig;
			doubleZSig -= 2;
			add128( rem0, rem1, zSig>>63, doubleZSig | 1, &rem0, &rem1 );
		}
		zSig |= ( ( rem0 | rem1 ) != 0 );
	}
#ifdef SOFTFLOAT_I860
    return roundAndPackFloat64_2( 0, zExp, zSig, c );
#else
	return roundAndPackFloat64( 0, zExp, zSig );
#endif

}

/*----------------------------------------------------------------------------
| Returns 1 if the double-precision floating-point value `a' is equal to the
| corresponding value `b', and 0 otherwise.  The comparison is performed
| according to the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

flag float64_eq( float64 a, float64 b, float_ctrl* c )
{
	if (    ( ( extractFloat64Exp( a ) == 0x7FF ) && extractFloat64Frac( a ) )
			|| ( ( extractFloat64Exp( b ) == 0x7FF ) && extractFloat64Frac( b ) )
		) {
		if ( float64_is_signaling_nan( a ) || float64_is_signaling_nan( b ) ) {
#ifdef SOFTFLOAT_I860
            float_raise( float_flag_invalid, c );
#else
			float_raise( float_flag_invalid );
#endif
		}
		return 0;
	}
	return ( a == b ) || ( (bits64) ( ( a | b )<<1 ) == 0 );

}

/*----------------------------------------------------------------------------
| Returns 1 if the double-precision floating-point value `a' is less than or
| equal to the corresponding value `b', and 0 otherwise.  The comparison is
| performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic.
*----------------------------------------------------------------------------*/

flag float64_le( float64 a, float64 b, float_ctrl* c )
{
	flag aSign, bSign;

	if (    ( ( extractFloat64Exp( a ) == 0x7FF ) && extractFloat64Frac( a ) )
			|| ( ( extractFloat64Exp( b ) == 0x7FF ) && extractFloat64Frac( b ) )
		) {
#ifdef SOFTFLOAT_I860
        float_raise( float_flag_invalid, c );
#else
		float_raise( float_flag_invalid );
#endif
		return 0;
	}
	aSign = extractFloat64Sign( a );
	bSign = extractFloat64Sign( b );
	if ( aSign != bSign ) return aSign || ( (bits64) ( ( a | b )<<1 ) == 0 );
	return ( a == b ) || ( aSign ^ ( a < b ) );

}

/*----------------------------------------------------------------------------
| Returns 1 if the double-precision floating-point value `a' is less than
| the corresponding value `b', and 0 otherwise.  The comparison is performed
| according to the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

flag float64_lt( float64 a, float64 b, float_ctrl* c )
{
	flag aSign, bSign;

	if (    ( ( extractFloat64Exp( a ) == 0x7FF ) && extractFloat64Frac( a ) )
			|| ( ( extractFloat64Exp( b ) == 0x7FF ) && extractFloat64Frac( b ) )
		) {
#ifdef SOFTFLOAT_I860
        float_raise( float_flag_invalid, c );
#else
		float_raise( float_flag_invalid );
#endif
		return 0;
	}
	aSign = extractFloat64Sign( a );
	bSign = extractFloat64Sign( b );
	if ( aSign != bSign ) return aSign && ( (bits64) ( ( a | b )<<1 ) != 0 );
	return ( a != b ) && ( aSign ^ ( a < b ) );

}

#ifdef SOFTFLOAT_I860 // 29-04-2017: Added for Previous
/*----------------------------------------------------------------------------
| Returns 1 if the double-precision floating-point value `a' is greater than
| the corresponding value `b', and 0 otherwise.  The comparison is performed
| according to the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/
        
flag float64_gt( float64 a, float64 b, float_ctrl* c )
{
    flag aSign, bSign;
    
    if (    ( ( extractFloat64Exp( a ) == 0x7FF ) && extractFloat64Frac( a ) )
        || ( ( extractFloat64Exp( b ) == 0x7FF ) && extractFloat64Frac( b ) )
        ) {
#ifdef SOFTFLOAT_I860
        float_raise( float_flag_invalid, c );
#else
        float_raise( float_flag_invalid );
#endif
        return 0;
    }
    aSign = extractFloat64Sign( a );
    bSign = extractFloat64Sign( b );
    if ( aSign != bSign ) return bSign && ( (bits64) ( ( a | b )<<1 ) != 0 );
    return ( a != b ) && ( bSign ^ ( a > b ) );

}
#endif // End of addition for Previous

/*----------------------------------------------------------------------------
| Returns 1 if the double-precision floating-point value `a' is equal to the
| corresponding value `b', and 0 otherwise.  The invalid exception is raised
| if either operand is a NaN.  Otherwise, the comparison is performed
| according to the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

flag float64_eq_signaling( float64 a, float64 b, float_ctrl* c )
{
	if (    ( ( extractFloat64Exp( a ) == 0x7FF ) && extractFloat64Frac( a ) )
			|| ( ( extractFloat64Exp( b ) == 0x7FF ) && extractFloat64Frac( b ) )
		) {
		float_raise( float_flag_invalid, c );
		return 0;
	}
	return ( a == b ) || ( (bits64) ( ( a | b )<<1 ) == 0 );

}

/*----------------------------------------------------------------------------
| Returns 1 if the double-precision floating-point value `a' is less than or
| equal to the corresponding value `b', and 0 otherwise.  Quiet NaNs do not
| cause an exception.  Otherwise, the comparison is performed according to the
| IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

flag float64_le_quiet( float64 a, float64 b, float_ctrl* c )
{
	flag aSign, bSign;
//    int16 aExp, bExp;

	if (    ( ( extractFloat64Exp( a ) == 0x7FF ) && extractFloat64Frac( a ) )
			|| ( ( extractFloat64Exp( b ) == 0x7FF ) && extractFloat64Frac( b ) )
		) {
		if ( float64_is_signaling_nan( a ) || float64_is_signaling_nan( b ) ) {
			float_raise( float_flag_invalid, c );
		}
		return 0;
	}
	aSign = extractFloat64Sign( a );
	bSign = extractFloat64Sign( b );
	if ( aSign != bSign ) return aSign || ( (bits64) ( ( a | b )<<1 ) == 0 );
	return ( a == b ) || ( aSign ^ ( a < b ) );

}

/*----------------------------------------------------------------------------
| Returns 1 if the double-precision floating-point value `a' is less than
| the corresponding value `b', and 0 otherwise.  Quiet NaNs do not cause an
| exception.  Otherwise, the comparison is performed according to the IEC/IEEE
| Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

flag float64_lt_quiet( float64 a, float64 b, float_ctrl* c )
{
	flag aSign, bSign;

	if (    ( ( extractFloat64Exp( a ) == 0x7FF ) && extractFloat64Frac( a ) )
			|| ( ( extractFloat64Exp( b ) == 0x7FF ) && extractFloat64Frac( b ) )
		) {
		if ( float64_is_signaling_nan( a ) || float64_is_signaling_nan( b ) ) {
			float_raise( float_flag_invalid, c );
		}
		return 0;
	}
	aSign = extractFloat64Sign( a );
	bSign = extractFloat64Sign( b );
	if ( aSign != bSign ) return aSign && ( (bits64) ( ( a | b )<<1 ) != 0 );
	return ( a != b ) && ( aSign ^ ( a < b ) );

}

#ifdef FLOATX80

/*----------------------------------------------------------------------------
| Returns the result of converting the extended double-precision floating-
| point value `a' to the 32-bit two's complement integer format.  The
| conversion is performed according to the IEC/IEEE Standard for Binary
| Floating-Point Arithmetic---which means in particular that the conversion
| is rounded according to the current rounding mode.  If `a' is a NaN, the
| largest positive integer is returned.  Otherwise, if the conversion
| overflows, the largest integer with the same sign as `a' is returned.
*----------------------------------------------------------------------------*/

int32 floatx80_to_int32( floatx80 a, float_ctrl* c )
{
	flag aSign;
	int32 aExp, shiftCount;
	bits64 aSig;

	aSig = extractFloatx80Frac( a );
	aExp = extractFloatx80Exp( a );
	aSign = extractFloatx80Sign( a );
#ifdef SOFTFLOAT_68K
    if ( aExp == 0x7FFF ) {
        if ( (bits64) ( aSig<<1 ) ) {
            a = propagateFloatx80NaNOneArg( a, c );
            if ( a.low == aSig ) float_raise( float_flag_invalid, c );
            return (sbits32)(a.low>>32);
        }
        float_raise( float_flag_invalid, c );
        return aSign ? (sbits32) 0x80000000 : 0x7FFFFFFF;
    }
#else
	if ( ( aExp == 0x7FFF ) && (bits64) ( aSig<<1 ) ) aSign = 0;
#endif
	shiftCount = 0x4037 - aExp;
	if ( shiftCount <= 0 ) shiftCount = 1;
	shift64RightJamming( aSig, shiftCount, &aSig );
	return roundAndPackInt32( aSign, aSig, c );

}
#ifdef SOFTFLOAT_68K // 30-01-2017: Addition for Previous
int16 floatx80_to_int16( floatx80 a, float_ctrl* c )
{
    flag aSign;
    int32 aExp, shiftCount;
    bits64 aSig;
    
    aSig = extractFloatx80Frac( a );
    aExp = extractFloatx80Exp( a );
    aSign = extractFloatx80Sign( a );
    if ( aExp == 0x7FFF ) {
        if ( (bits64) ( aSig<<1 ) ) {
            a = propagateFloatx80NaNOneArg( a, c );
            if ( a.low == aSig ) float_raise( float_flag_invalid, c );
            return (sbits16)(a.low>>48);
        }
        float_raise( float_flag_invalid, c );
        return aSign ? (sbits16) 0x8000 : 0x7FFF;
    }
    shiftCount = 0x4037 - aExp;
    if ( shiftCount <= 0 ) shiftCount = 1;
    shift64RightJamming( aSig, shiftCount, &aSig );
    return roundAndPackInt16( aSign, aSig, c );
    
}
int8 floatx80_to_int8( floatx80 a, float_ctrl* c )
{
    flag aSign;
    int32 aExp, shiftCount;
    bits64 aSig;
    
    aSig = extractFloatx80Frac( a );
    aExp = extractFloatx80Exp( a );
    aSign = extractFloatx80Sign( a );
    if ( aExp == 0x7FFF ) {
        if ( (bits64) ( aSig<<1 ) ) {
            a = propagateFloatx80NaNOneArg( a, c );
            if ( a.low == aSig ) float_raise( float_flag_invalid, c );
            return (sbits8)(a.low>>56);
        }
        float_raise( float_flag_invalid, c );
        return aSign ? (sbits8) 0x80 : 0x7F;
    }
    shiftCount = 0x4037 - aExp;
    if ( shiftCount <= 0 ) shiftCount = 1;
    shift64RightJamming( aSig, shiftCount, &aSig );
    return roundAndPackInt8( aSign, aSig, c );
    
}
#endif // End of addition for Previous

/*----------------------------------------------------------------------------
| Returns the result of converting the extended double-precision floating-
| point value `a' to the 32-bit two's complement integer format.  The
| conversion is performed according to the IEC/IEEE Standard for Binary
| Floating-Point Arithmetic, except that the conversion is always rounded
| toward zero.  If `a' is a NaN, the largest positive integer is returned.
| Otherwise, if the conversion overflows, the largest integer with the same
| sign as `a' is returned.
*----------------------------------------------------------------------------*/

int32 floatx80_to_int32_round_to_zero( floatx80 a, float_ctrl* c )
{
	flag aSign;
	int32 aExp, shiftCount;
	bits64 aSig, savedASig;
	int32 z;

	aSig = extractFloatx80Frac( a );
	aExp = extractFloatx80Exp( a );
	aSign = extractFloatx80Sign( a );
	if ( 0x401E < aExp ) {
		if ( ( aExp == 0x7FFF ) && (bits64) ( aSig<<1 ) ) aSign = 0;
		goto invalid;
	}
	else if ( aExp < 0x3FFF ) {
		if ( aExp || aSig ) float_raise( float_flag_inexact, c );
		return 0;
	}
	shiftCount = 0x403E - aExp;
	savedASig = aSig;
	aSig >>= shiftCount;
	z = aSig;
	if ( aSign ) z = - z;
    z = (sbits32) z;
	if ( ( z < 0 ) ^ aSign ) {
	invalid:
		float_raise( float_flag_invalid, c );
		return aSign ? (sbits32) 0x80000000 : 0x7FFFFFFF;
	}
	if ( ( aSig<<shiftCount ) != savedASig ) {
		float_raise( float_flag_inexact, c );
	}
	return z;

}

/*----------------------------------------------------------------------------
| Returns the result of converting the extended double-precision floating-
| point value `a' to the 64-bit two's complement integer format.  The
| conversion is performed according to the IEC/IEEE Standard for Binary
| Floating-Point Arithmetic---which means in particular that the conversion
| is rounded according to the current rounding mode.  If `a' is a NaN,
| the largest positive integer is returned.  Otherwise, if the conversion
| overflows, the largest integer with the same sign as `a' is returned.
*----------------------------------------------------------------------------*/

int64 floatx80_to_int64( floatx80 a, float_ctrl* c )
{
	flag aSign;
	int32 aExp, shiftCount;
	bits64 aSig, aSigExtra;

	aSig = extractFloatx80Frac( a );
	aExp = extractFloatx80Exp( a );
	aSign = extractFloatx80Sign( a );
	shiftCount = 0x403E - aExp;
	if ( shiftCount <= 0 ) {
		if ( shiftCount ) {
			float_raise( float_flag_invalid, c );
			if (    ! aSign
					|| (    ( aExp == 0x7FFF )
						&& ( (bits64) ( aSig<<1 ) ) )
				) {
				return LIT64( 0x7FFFFFFFFFFFFFFF );
			}
			return (sbits64) LIT64( 0x8000000000000000 );
		}
		aSigExtra = 0;
	}
	else {
		shift64ExtraRightJamming( aSig, 0, shiftCount, &aSig, &aSigExtra );
	}
	return roundAndPackInt64( aSign, aSig, aSigExtra, c );

}

/*----------------------------------------------------------------------------
| Returns the result of converting the extended double-precision floating-
| point value `a' to the 64-bit two's complement integer format.  The
| conversion is performed according to the IEC/IEEE Standard for Binary
| Floating-Point Arithmetic, except that the conversion is always rounded
| toward zero.  If `a' is a NaN, the largest positive integer is returned.
| Otherwise, if the conversion overflows, the largest integer with the same
| sign as `a' is returned.
*----------------------------------------------------------------------------*/

int64 floatx80_to_int64_round_to_zero( floatx80 a, float_ctrl* c )
{
	flag aSign;
	int32 aExp, shiftCount;
	bits64 aSig;
	int64 z;

	aSig = extractFloatx80Frac( a );
	aExp = extractFloatx80Exp( a );
	aSign = extractFloatx80Sign( a );
	shiftCount = aExp - 0x403E;
	if ( 0 <= shiftCount ) {
		aSig &= LIT64( 0x7FFFFFFFFFFFFFFF );
		if ( ( a.high != 0xC03E ) || aSig ) {
			float_raise( float_flag_invalid, c );
			if ( ! aSign || ( ( aExp == 0x7FFF ) && aSig ) ) {
				return LIT64( 0x7FFFFFFFFFFFFFFF );
			}
		}
		return (sbits64) LIT64( 0x8000000000000000 );
	}
	else if ( aExp < 0x3FFF ) {
		if ( aExp | aSig ) float_raise( float_flag_inexact, c );
		return 0;
	}
	z = aSig>>( - shiftCount );
	if ( (bits64) ( aSig<<( shiftCount & 63 ) ) ) {
		float_raise( float_flag_inexact, c );
	}
	if ( aSign ) z = - z;
	return z;

}

/*----------------------------------------------------------------------------
| Returns the result of converting the extended double-precision floating-
| point value `a' to the single-precision floating-point format.  The
| conversion is performed according to the IEC/IEEE Standard for Binary
| Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float32 floatx80_to_float32( floatx80 a, float_ctrl* c )
{
	flag aSign;
	int32 aExp;
	bits64 aSig;

	aSig = extractFloatx80Frac( a );
	aExp = extractFloatx80Exp( a );
	aSign = extractFloatx80Sign( a );
	if ( aExp == 0x7FFF ) {
		if ( (bits64) ( aSig<<1 ) ) {
			return commonNaNToFloat32( floatx80ToCommonNaN( a, c ) );
		}
		return packFloat32( aSign, 0xFF, 0 );
	}
#ifdef SOFTFLOAT_68K
    if ( aExp == 0 ) {
        if ( aSig == 0) return packFloat32( aSign, 0, 0 );
        normalizeFloatx80Subnormal( aSig, &aExp, &aSig );
    }
    shift64RightJamming( aSig, 33, &aSig );
    aExp -= 0x3F81;
#else
	shift64RightJamming( aSig, 33, &aSig );
	if ( aExp || aSig ) aExp -= 0x3F81;
#endif
	return roundAndPackFloat32( aSign, aExp, aSig, c );

}

/*----------------------------------------------------------------------------
| Returns the result of converting the extended double-precision floating-
| point value `a' to the double-precision floating-point format.  The
| conversion is performed according to the IEC/IEEE Standard for Binary
| Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float64 floatx80_to_float64( floatx80 a, float_ctrl* c )
{
	flag aSign;
	int32 aExp;
	bits64 aSig, zSig;

	aSig = extractFloatx80Frac( a );
	aExp = extractFloatx80Exp( a );
	aSign = extractFloatx80Sign( a );
	if ( aExp == 0x7FFF ) {
		if ( (bits64) ( aSig<<1 ) ) {
			return commonNaNToFloat64( floatx80ToCommonNaN( a, c ) );
		}
		return packFloat64( aSign, 0x7FF, 0 );
	}
#ifdef SOFTFLOAT_68K
    if ( aExp == 0 ) {
        if ( aSig == 0) return packFloat64( aSign, 0, 0 );
        normalizeFloatx80Subnormal( aSig, &aExp, &aSig );
    }
    shift64RightJamming( aSig, 1, &zSig );
    aExp -= 0x3C01;
#else
	shift64RightJamming( aSig, 1, &zSig );
	if ( aExp || aSig ) aExp -= 0x3C01;
#endif
	return roundAndPackFloat64( aSign, aExp, zSig, c );

}
        
#ifdef SOFTFLOAT_68K // 31-01-2017
/*----------------------------------------------------------------------------
 | Returns the result of converting the extended double-precision floating-
 | point value `a' to the extended double-precision floating-point format.
 | The conversion is performed according to the IEC/IEEE Standard for Binary
 | Floating-Point Arithmetic.
 *----------------------------------------------------------------------------*/
        
floatx80 floatx80_to_floatx80( floatx80 a, float_ctrl* c )
{
    flag aSign;
    int32 aExp;
    bits64 aSig;
    
    aSig = extractFloatx80Frac( a );
    aExp = extractFloatx80Exp( a );
    aSign = extractFloatx80Sign( a );
    
    if ( aExp == 0x7FFF && (bits64) ( aSig<<1 ) ) {
        return propagateFloatx80NaNOneArg( a, c );
    }
    if ( aExp == 0 && aSig != 0 ) {
        return normalizeRoundAndPackFloatx80( get_float_rounding_precision(c), aSign, aExp, aSig, 0, c );
    }
    return a;
    
}
#endif

#ifdef FLOAT128

/*----------------------------------------------------------------------------
| Returns the result of converting the extended double-precision floating-
| point value `a' to the quadruple-precision floating-point format.  The
| conversion is performed according to the IEC/IEEE Standard for Binary
| Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float128 floatx80_to_float128( floatx80 a, float_ctrl* c )
{
	flag aSign;
	int32 aExp;
	bits64 aSig, zSig0, zSig1;

	aSig = extractFloatx80Frac( a );
	aExp = extractFloatx80Exp( a );
	aSign = extractFloatx80Sign( a );
	if ( ( aExp == 0x7FFF ) && (bits64) ( aSig<<1 ) ) {
		return commonNaNToFloat128( floatx80ToCommonNaN( a, c ) );
	}
#ifdef SOFTFLOAT_68K
    if ( aExp == 0 ) {
        if ( aSig == 0 ) return packFloat128( aSign, 0, 0, 0 );
        normalizeFloatx80Subnormal( aSig, &aExp, &aSig );
    }
#endif
	shift128Right( aSig<<1, 0, 16, &zSig0, &zSig1 );
	return packFloat128( aSign, aExp, zSig0, zSig1 );

}

#endif

#ifdef SOFTFLOAT_68K // 30-01-2016: Added for Previous
floatx80 floatx80_round32( floatx80 a, float_ctrl* c )
{
    flag aSign;
    int32 aExp;
    bits64 aSig;
    
    aSig = extractFloatx80Frac( a );
    aExp = extractFloatx80Exp( a );
    aSign = extractFloatx80Sign( a );
    
    if ( aExp == 0x7FFF || aSig == 0 ) {
        return a;
    }
    if ( aExp == 0 ) {
        normalizeFloatx80Subnormal( aSig, &aExp, &aSig );
    }
    
    return roundSigAndPackFloatx80( 32, aSign, aExp, aSig, 0, c );

}

floatx80 floatx80_round64( floatx80 a, float_ctrl* c )
{
    flag aSign;
    int32 aExp;
    bits64 aSig;
    
    aSig = extractFloatx80Frac( a );
    aExp = extractFloatx80Exp( a );
    aSign = extractFloatx80Sign( a );
    
    if ( aExp == 0x7FFF || aSig == 0 ) {
        return a;
    }
    if ( aExp == 0 ) {
        normalizeFloatx80Subnormal( aSig, &aExp, &aSig );
    }
    
    return roundSigAndPackFloatx80( 64, aSign, aExp, aSig, 0, c );
    
}
        
floatx80 floatx80_round_to_float32( floatx80 a, float_ctrl* c )
{
    flag aSign;
    int32 aExp;
    bits64 aSig;
    
    aSign = extractFloatx80Sign( a );
    aSig = extractFloatx80Frac( a );
    aExp = extractFloatx80Exp( a );
    
    if ( aExp == 0x7FFF ) {
        if ( (bits64) ( aSig<<1 ) ) return propagateFloatx80NaNOneArg( a, c );
        return a;
    }
    if ( aExp == 0 ) {
        if ( aSig == 0 ) return a;
        normalizeFloatx80Subnormal( aSig, &aExp, &aSig );
    }
    
    return roundAndPackFloatx80( 32, aSign, aExp, aSig, 0, c );

}
        
floatx80 floatx80_round_to_float64( floatx80 a, float_ctrl* c )
{
    flag aSign;
    int32 aExp;
    bits64 aSig;
    
    aSign = extractFloatx80Sign( a );
    aSig = extractFloatx80Frac( a );
    aExp = extractFloatx80Exp( a );

    if ( aExp == 0x7FFF ) {
        if ( (bits64) ( aSig<<1 ) ) return propagateFloatx80NaNOneArg( a, c );
        return a;
    }
    if ( aExp == 0 ) {
        if ( aSig == 0 ) return a;
        normalizeFloatx80Subnormal( aSig, &aExp, &aSig );
    }

    return roundAndPackFloatx80( 64, aSign, aExp, aSig, 0, c );

}
        
floatx80 floatx80_normalize( floatx80 a )
{
    flag aSign;
    int16 aExp;
    bits64 aSig;
    int8 shiftCount;
    
    aSig = extractFloatx80Frac( a );
    aExp = extractFloatx80Exp( a );
    aSign = extractFloatx80Sign( a );
    
    if ( aExp == 0x7FFF || aExp == 0 ) return a;
    if ( aSig == 0 ) return packFloatx80(aSign, 0, 0);
    
    shiftCount = countLeadingZeros64( aSig );
    
    if ( shiftCount > aExp ) shiftCount = aExp;
    
    aExp -= shiftCount;
    aSig <<= shiftCount;
    
    return packFloatx80( aSign, aExp, aSig );
    
}
        
floatx80 floatx80_denormalize( floatx80 a, flag eSign)
{
    flag aSign;
    int32 aExp;
    bits64 aSig;
    int32 shiftCount;

    aSig = extractFloatx80Frac( a );
    aExp = extractFloatx80Exp( a );
    aSign = extractFloatx80Sign( a );
    
    if ( eSign ) {
        shiftCount = 0x8000 - aExp;
        aExp = 0;
        if (shiftCount > 63) {
            aSig = 0;
        } else {
            aSig >>= shiftCount;
        }
    }
    return packFloatx80(aSign, aExp, aSig);
    
}
#endif // end of addition for Previous

/*----------------------------------------------------------------------------
| Rounds the extended double-precision floating-point value `a' to an integer,
| and returns the result as an extended quadruple-precision floating-point
| value.  The operation is performed according to the IEC/IEEE Standard for
| Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

floatx80 floatx80_round_to_int( floatx80 a, float_ctrl* c )
{
	flag aSign;
	int32 aExp;
	bits64 lastBitMask, roundBitsMask;
	int8 roundingMode;
	floatx80 z;

    roundingMode = get_float_rounding_mode( c );
    aSign = extractFloatx80Sign( a );
	aExp = extractFloatx80Exp( a );
	if ( 0x403E <= aExp ) {
		if ( aExp == 0x7FFF ) {
            if ( (bits64) ( extractFloatx80Frac( a )<<1 ) ) return propagateFloatx80NaNOneArg( a, c );
            return inf_clear_intbit( c ) ? packFloatx80( aSign, aExp, 0 ) : a;
		}
		return a;
	}
	if ( aExp < 0x3FFF ) {
		if (    ( aExp == 0 )
#ifdef SOFTFLOAT_68K
				&& ( (bits64) extractFloatx80Frac( a ) == 0 ) ) {
#else
				&& ( (bits64) ( extractFloatx80Frac( a )<<1 ) == 0 ) ) {
#endif
			return a;
		}
		float_raise( float_flag_inexact, c );
		switch ( roundingMode ) {
			case float_round_nearest_even:
			if ( ( aExp == 0x3FFE ) && (bits64) ( extractFloatx80Frac( a )<<1 )
				) {
				return
					packFloatx80( aSign, 0x3FFF, LIT64( 0x8000000000000000 ) );
			}
			break;
			case float_round_down:
			return
					aSign ?
						packFloatx80( 1, 0x3FFF, LIT64( 0x8000000000000000 ) )
				: packFloatx80( 0, 0, 0 );
			case float_round_up:
			return
					aSign ? packFloatx80( 1, 0, 0 )
				: packFloatx80( 0, 0x3FFF, LIT64( 0x8000000000000000 ) );
		}
		return packFloatx80( aSign, 0, 0 );
	}
	lastBitMask = 1;
	lastBitMask <<= 0x403E - aExp;
	roundBitsMask = lastBitMask - 1;
	z = a;
	if ( roundingMode == float_round_nearest_even ) {
		z.low += lastBitMask>>1;
		if ( ( z.low & roundBitsMask ) == 0 ) z.low &= ~ lastBitMask;
	}
	else if ( roundingMode != float_round_to_zero ) {
		if ( extractFloatx80Sign( z ) ^ ( roundingMode == float_round_up ) ) {
			z.low += roundBitsMask;
		}
	}
	z.low &= ~ roundBitsMask;
	if ( z.low == 0 ) {
		++z.high;
		z.low = LIT64( 0x8000000000000000 );
	}
	if ( z.low != a.low ) float_raise( float_flag_inexact, c );
	return z;

}

#ifdef SOFTFLOAT_68K // 09-01-2017: Added for Previous
floatx80 floatx80_round_to_int_toward_zero( floatx80 a, float_ctrl* c )
{
    flag aSign;
    int32 aExp;
    bits64 lastBitMask, roundBitsMask;
    floatx80 z;
    
    aSign = extractFloatx80Sign( a );
    aExp = extractFloatx80Exp( a );
    if ( 0x403E <= aExp ) {
        if ( aExp == 0x7FFF ) {
            if ( (bits64) ( extractFloatx80Frac( a )<<1 ) ) return propagateFloatx80NaNOneArg( a, c );
            return inf_clear_intbit( c ) ? packFloatx80( aSign, aExp, 0 ) : a;
        }
        return a;
    }
    if ( aExp < 0x3FFF ) {
        if (    ( aExp == 0 )
#ifdef SOFTFLOAT_68K
            && ( (bits64) extractFloatx80Frac( a ) == 0 ) ) {
#else
            && ( (bits64) ( extractFloatx80Frac( a )<<1 ) == 0 ) ) {
#endif
            return a;
        }
        float_raise( float_flag_inexact, c );
        return packFloatx80( aSign, 0, 0 );
    }
    lastBitMask = 1;
    lastBitMask <<= 0x403E - aExp;
    roundBitsMask = lastBitMask - 1;
    z = a;
    z.low &= ~ roundBitsMask;
    if ( z.low == 0 ) {
        ++z.high;
        z.low = LIT64( 0x8000000000000000 );
    }
    if ( z.low != a.low ) float_raise( float_flag_inexact, c );
    return z;
    
}
#endif // End of addition for Previous

/*----------------------------------------------------------------------------
| Returns the result of adding the absolute values of the extended double-
| precision floating-point values `a' and `b'.  If `zSign' is 1, the sum is
| negated before being returned.  `zSign' is ignored if the result is a NaN.
| The addition is performed according to the IEC/IEEE Standard for Binary
| Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

static floatx80 addFloatx80Sigs( floatx80 a, floatx80 b, flag zSign, float_ctrl* c )
{
	int32 aExp, bExp, zExp;
	bits64 aSig, bSig, zSig0, zSig1;
	int32 expDiff;

	aSig = extractFloatx80Frac( a );
	aExp = extractFloatx80Exp( a );
	bSig = extractFloatx80Frac( b );
	bExp = extractFloatx80Exp( b );
#ifdef SOFTFLOAT_68K
	if ( aExp == 0 ) {
		normalizeFloatx80Subnormal( aSig, &aExp, &aSig );
	}
	if ( bExp == 0 ) {
		normalizeFloatx80Subnormal( bSig, &bExp, &bSig );
	}
#endif
	expDiff = aExp - bExp;
	if ( 0 < expDiff ) {
		if ( aExp == 0x7FFF ) {
			if ( (bits64) ( aSig<<1 ) ) return propagateFloatx80NaN( a, b, c );
            return inf_clear_intbit( c ) ? packFloatx80( extractFloatx80Sign( a ), aExp, 0 ) : a;
		}
#ifndef SOFTFLOAT_68K
		if ( bExp == 0 ) --expDiff;
#endif
		shift64ExtraRightJamming( bSig, 0, expDiff, &bSig, &zSig1 );
		zExp = aExp;
	}
	else if ( expDiff < 0 ) {
		if ( bExp == 0x7FFF ) {
			if ( (bits64) ( bSig<<1 ) ) return propagateFloatx80NaN( a, b, c );
            if ( inf_clear_intbit( c ) ) bSig = 0;
			return packFloatx80( zSign, bExp, bSig );
		}
#ifndef SOFTFLOAT_68K
		if ( aExp == 0 ) ++expDiff;
#endif
		shift64ExtraRightJamming( aSig, 0, - expDiff, &aSig, &zSig1 );
		zExp = bExp;
	}
	else {
		if ( aExp == 0x7FFF ) {
			if ( (bits64) ( ( aSig | bSig )<<1 ) ) {
				return propagateFloatx80NaN( a, b, c );
			}
            if ( inf_clear_intbit( c ) ) return packFloatx80( extractFloatx80Sign( a ), aExp, 0 );
            return ( faddsub_swap_inf( c ) ) ? b : a;
		}
		zSig1 = 0;
		zSig0 = aSig + bSig;
#ifndef SOFTFLOAT_68K
		if ( aExp == 0 ) {
			normalizeFloatx80Subnormal( zSig0, &zExp, &zSig0 );
			goto roundAndPack;
		}
#endif
		zExp = aExp;
#ifdef SOFTFLOAT_68K
        if ( aSig == 0 && bSig == 0 ) return packFloatx80( zSign, 0, 0 );
        if ( aSig == 0 || bSig == 0 ) goto roundAndPack;
#endif
		goto shiftRight1;
	}
	zSig0 = aSig + bSig;
	if ( (sbits64) zSig0 < 0 ) goto roundAndPack;
	shiftRight1:
	shift64ExtraRightJamming( zSig0, zSig1, 1, &zSig0, &zSig1 );
	zSig0 |= LIT64( 0x8000000000000000 );
	++zExp;
	roundAndPack:
	return
		roundAndPackFloatx80(
			get_float_rounding_precision(c), zSign, zExp, zSig0, zSig1, c );

}

/*----------------------------------------------------------------------------
| Returns the result of subtracting the absolute values of the extended
| double-precision floating-point values `a' and `b'.  If `zSign' is 1, the
| difference is negated before being returned.  `zSign' is ignored if the
| result is a NaN.  The subtraction is performed according to the IEC/IEEE
| Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

static floatx80 subFloatx80Sigs( floatx80 a, floatx80 b, flag zSign, float_ctrl* c )
{
	int32 aExp, bExp, zExp;
	bits64 aSig, bSig, zSig0, zSig1;
	int32 expDiff;

	aSig = extractFloatx80Frac( a );
	aExp = extractFloatx80Exp( a );
	bSig = extractFloatx80Frac( b );
	bExp = extractFloatx80Exp( b );
	expDiff = aExp - bExp;
	if ( 0 < expDiff ) goto aExpBigger;
	if ( expDiff < 0 ) goto bExpBigger;
	if ( aExp == 0x7FFF ) {
		if ( (bits64) ( ( aSig | bSig )<<1 ) ) {
			return propagateFloatx80NaN( a, b, c );
		}
		float_raise( float_flag_invalid, c );
		return packFloatx80( 0, floatx80_default_nan_high, floatx80_default_nan_low );
	}
#ifndef SOFTFLOAT_68K
	if ( aExp == 0 ) {
		aExp = 1;
		bExp = 1;
	}
#endif
	zSig1 = 0;
	if ( bSig < aSig ) goto aBigger;
	if ( aSig < bSig ) goto bBigger;
	return packFloatx80( get_float_rounding_mode( c ) == float_round_down, 0, 0 );
	bExpBigger:
	if ( bExp == 0x7FFF ) {
		if ( (bits64) ( bSig<<1 ) ) return propagateFloatx80NaN( a, b, c );
        if ( inf_clear_intbit( c ) ) bSig = 0;
		return packFloatx80( zSign ^ 1, bExp, bSig );
	}
#ifndef SOFTFLOAT_68K
	if ( aExp == 0 ) ++expDiff;
#endif
	shift128RightJamming( aSig, 0, - expDiff, &aSig, &zSig1 );
	bBigger:
	sub128( bSig, 0, aSig, zSig1, &zSig0, &zSig1 );
	zExp = bExp;
	zSign ^= 1;
	goto normalizeRoundAndPack;
	aExpBigger:
	if ( aExp == 0x7FFF ) {
		if ( (bits64) ( aSig<<1 ) ) return propagateFloatx80NaN( a, b, c );
        return inf_clear_intbit( c ) ? packFloatx80( extractFloatx80Sign( a ), aExp, 0 ) : a;
	}
#ifndef SOFTFLOAT_68K
	if ( bExp == 0 ) --expDiff;
#endif
	shift128RightJamming( bSig, 0, expDiff, &bSig, &zSig1 );
	aBigger:
	sub128( aSig, 0, bSig, zSig1, &zSig0, &zSig1 );
	zExp = aExp;
	normalizeRoundAndPack:
	return
		normalizeRoundAndPackFloatx80(
			get_float_rounding_precision(c), zSign, zExp, zSig0, zSig1, c );

}

/*----------------------------------------------------------------------------
| Returns the result of adding the extended double-precision floating-point
| values `a' and `b'.  The operation is performed according to the IEC/IEEE
| Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

floatx80 floatx80_add( floatx80 a, floatx80 b, float_ctrl* c )
{
	flag aSign, bSign;

	aSign = extractFloatx80Sign( a );
	bSign = extractFloatx80Sign( b );
	if ( aSign == bSign ) {
		return addFloatx80Sigs( a, b, aSign, c );
	}
	else {
		return subFloatx80Sigs( a, b, aSign, c );
	}

}

/*----------------------------------------------------------------------------
| Returns the result of subtracting the extended double-precision floating-
| point values `a' and `b'.  The operation is performed according to the
| IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

floatx80 floatx80_sub( floatx80 a, floatx80 b, float_ctrl* c )
{
	flag aSign, bSign;

	aSign = extractFloatx80Sign( a );
	bSign = extractFloatx80Sign( b );
	if ( aSign == bSign ) {
		return subFloatx80Sigs( a, b, aSign, c );
	}
	else {
		return addFloatx80Sigs( a, b, aSign, c );
	}

}

/*----------------------------------------------------------------------------
| Returns the result of multiplying the extended double-precision floating-
| point values `a' and `b'.  The operation is performed according to the
| IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

floatx80 floatx80_mul( floatx80 a, floatx80 b, float_ctrl* c )
{
	flag aSign, bSign, zSign;
	int32 aExp, bExp, zExp;
	bits64 aSig, bSig, zSig0, zSig1;

	aSig = extractFloatx80Frac( a );
	aExp = extractFloatx80Exp( a );
	aSign = extractFloatx80Sign( a );
	bSig = extractFloatx80Frac( b );
	bExp = extractFloatx80Exp( b );
	bSign = extractFloatx80Sign( b );
	zSign = aSign ^ bSign;
	if ( aExp == 0x7FFF ) {
		if (    (bits64) ( aSig<<1 )
			|| ( ( bExp == 0x7FFF ) && (bits64) ( bSig<<1 ) ) ) {
			return propagateFloatx80NaN( a, b, c );
		}
		if ( ( bExp | bSig ) == 0 ) goto invalid;
        if ( inf_clear_intbit( c ) ) aSig = 0;
		return packFloatx80( zSign, aExp, aSig );
	}
	if ( bExp == 0x7FFF ) {
		if ( (bits64) ( bSig<<1 ) ) return propagateFloatx80NaN( a, b, c );
		if ( ( aExp | aSig ) == 0 ) {
	invalid:
			float_raise( float_flag_invalid, c );
			return packFloatx80( 0, floatx80_default_nan_high, floatx80_default_nan_low );
		}
        if ( inf_clear_intbit( c ) ) bSig = 0;
		return packFloatx80( zSign, bExp, bSig );
	}
	if ( aExp == 0 ) {
		if ( aSig == 0 ) return packFloatx80( zSign, 0, 0 );
		normalizeFloatx80Subnormal( aSig, &aExp, &aSig );
	}
	if ( bExp == 0 ) {
		if ( bSig == 0 ) return packFloatx80( zSign, 0, 0 );
		normalizeFloatx80Subnormal( bSig, &bExp, &bSig );
	}
	zExp = aExp + bExp - 0x3FFE;
	mul64To128( aSig, bSig, &zSig0, &zSig1 );
	if ( 0 < (sbits64) zSig0 ) {
		shortShift128Left( zSig0, zSig1, 1, &zSig0, &zSig1 );
		--zExp;
	}
	return
		roundAndPackFloatx80(
			get_float_rounding_precision(c), zSign, zExp, zSig0, zSig1, c );

}
    
#ifdef SOFTFLOAT_68K // 21-01-2017: Added for Previous
floatx80 floatx80_sglmul( floatx80 a, floatx80 b, float_ctrl* c )
{
	flag aSign, bSign, zSign;
	int32 aExp, bExp, zExp;
	bits64 aSig, bSig, zSig0, zSig1;
	
	aSig = extractFloatx80Frac( a );
	aExp = extractFloatx80Exp( a );
	aSign = extractFloatx80Sign( a );
	bSig = extractFloatx80Frac( b );
	bExp = extractFloatx80Exp( b );
	bSign = extractFloatx80Sign( b );
	zSign = aSign ^ bSign;
	if ( aExp == 0x7FFF ) {
		if (    (bits64) ( aSig<<1 )
			|| ( ( bExp == 0x7FFF ) && (bits64) ( bSig<<1 ) ) ) {
			return propagateFloatx80NaN( a, b, c );
		}
		if ( ( bExp | bSig ) == 0 ) goto invalid;
        if ( inf_clear_intbit( c ) ) aSig = 0;
		return packFloatx80( zSign, aExp, aSig );
	}
	if ( bExp == 0x7FFF ) {
		if ( (bits64) ( bSig<<1 ) ) return propagateFloatx80NaN( a, b, c );
		if ( ( aExp | aSig ) == 0 ) {
		invalid:
			float_raise( float_flag_invalid, c );
			return packFloatx80( 0, floatx80_default_nan_high, floatx80_default_nan_low );
		}
        if ( inf_clear_intbit( c ) ) bSig = 0;
		return packFloatx80( zSign, bExp, bSig );
	}
	if ( aExp == 0 ) {
		if ( aSig == 0 ) return packFloatx80( zSign, 0, 0 );
		normalizeFloatx80Subnormal( aSig, &aExp, &aSig );
	}
	if ( bExp == 0 ) {
		if ( bSig == 0 ) return packFloatx80( zSign, 0, 0 );
		normalizeFloatx80Subnormal( bSig, &bExp, &bSig );
	}
	aSig &= LIT64( 0xFFFFFF0000000000 );
	bSig &= LIT64( 0xFFFFFF0000000000 );
	zExp = aExp + bExp - 0x3FFE;
	mul64To128( aSig, bSig, &zSig0, &zSig1 );
	if ( 0 < (sbits64) zSig0 ) {
		shortShift128Left( zSig0, zSig1, 1, &zSig0, &zSig1 );
		--zExp;
	}
	return roundSigAndPackFloatx80( 32, zSign, zExp, zSig0, zSig1, c );
        
}
#endif // End of addition for Previous

/*----------------------------------------------------------------------------
| Returns the result of dividing the extended double-precision floating-point
| value `a' by the corresponding value `b'.  The operation is performed
| according to the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

floatx80 floatx80_div( floatx80 a, floatx80 b, float_ctrl* c )
{
	flag aSign, bSign, zSign;
	int32 aExp, bExp, zExp;
	bits64 aSig, bSig, zSig0, zSig1;
	bits64 rem0, rem1, rem2, term0, term1, term2;

	aSig = extractFloatx80Frac( a );
	aExp = extractFloatx80Exp( a );
	aSign = extractFloatx80Sign( a );
	bSig = extractFloatx80Frac( b );
	bExp = extractFloatx80Exp( b );
	bSign = extractFloatx80Sign( b );
	zSign = aSign ^ bSign;
	if ( aExp == 0x7FFF ) {
		if ( (bits64) ( aSig<<1 ) ) return propagateFloatx80NaN( a, b, c );
		if ( bExp == 0x7FFF ) {
			if ( (bits64) ( bSig<<1 ) ) return propagateFloatx80NaN( a, b, c );
			goto invalid;
		}
        if ( inf_clear_intbit( c ) ) aSig = 0;
		return packFloatx80( zSign, aExp, aSig );
	}
	if ( bExp == 0x7FFF ) {
		if ( (bits64) ( bSig<<1 ) ) return propagateFloatx80NaN( a, b, c );
		return packFloatx80( zSign, 0, 0 );
	}
	if ( bExp == 0 ) {
		if ( bSig == 0 ) {
			if ( ( aExp | aSig ) == 0 ) {
	invalid:
				float_raise( float_flag_invalid, c );
				return packFloatx80( 0, floatx80_default_nan_high, floatx80_default_nan_low );
			}
			float_raise( float_flag_divbyzero, c );
			return packFloatx80( zSign, floatx80_default_infinity_high, floatx80_default_infinity_low );
		}
		normalizeFloatx80Subnormal( bSig, &bExp, &bSig );
	}
	if ( aExp == 0 ) {
		if ( aSig == 0 ) return packFloatx80( zSign, 0, 0 );
		normalizeFloatx80Subnormal( aSig, &aExp, &aSig );
	}
	zExp = aExp - bExp + 0x3FFE;
	rem1 = 0;
	if ( bSig <= aSig ) {
		shift128Right( aSig, 0, 1, &aSig, &rem1 );
		++zExp;
	}
	zSig0 = estimateDiv128To64( aSig, rem1, bSig );
	mul64To128( bSig, zSig0, &term0, &term1 );
	sub128( aSig, rem1, term0, term1, &rem0, &rem1 );
	while ( (sbits64) rem0 < 0 ) {
		--zSig0;
		add128( rem0, rem1, 0, bSig, &rem0, &rem1 );
	}
	zSig1 = estimateDiv128To64( rem1, 0, bSig );
	if ( (bits64) ( zSig1<<1 ) <= 8 ) {
		mul64To128( bSig, zSig1, &term1, &term2 );
		sub128( rem1, 0, term1, term2, &rem1, &rem2 );
		while ( (sbits64) rem1 < 0 ) {
			--zSig1;
			add128( rem1, rem2, 0, bSig, &rem1, &rem2 );
		}
		zSig1 |= ( ( rem1 | rem2 ) != 0 );
	}
	return
		roundAndPackFloatx80(
			get_float_rounding_precision(c), zSign, zExp, zSig0, zSig1, c );

}
    
#ifdef SOFTFLOAT_68K // 21-01-2017: Addition for Previous
floatx80 floatx80_sgldiv( floatx80 a, floatx80 b, float_ctrl* c )
{
	flag aSign, bSign, zSign;
	int32 aExp, bExp, zExp;
	bits64 aSig, bSig, zSig0, zSig1;
	bits64 rem0, rem1, rem2, term0, term1, term2;
	
	aSig = extractFloatx80Frac( a );
	aExp = extractFloatx80Exp( a );
	aSign = extractFloatx80Sign( a );
	bSig = extractFloatx80Frac( b );
	bExp = extractFloatx80Exp( b );
	bSign = extractFloatx80Sign( b );
	zSign = aSign ^ bSign;
	if ( aExp == 0x7FFF ) {
		if ( (bits64) ( aSig<<1 ) ) return propagateFloatx80NaN( a, b, c );
		if ( bExp == 0x7FFF ) {
			if ( (bits64) ( bSig<<1 ) ) return propagateFloatx80NaN( a, b, c );
			goto invalid;
		}
        if ( inf_clear_intbit( c ) ) aSig = 0;
		return packFloatx80( zSign, aExp, aSig );
	}
	if ( bExp == 0x7FFF ) {
		if ( (bits64) ( bSig<<1 ) ) return propagateFloatx80NaN( a, b, c );
		return packFloatx80( zSign, 0, 0 );
	}
	if ( bExp == 0 ) {
		if ( bSig == 0 ) {
			if ( ( aExp | aSig ) == 0 ) {
			invalid:
				float_raise( float_flag_invalid, c );
		        return packFloatx80( 0, floatx80_default_nan_high, floatx80_default_nan_low );
			}
			float_raise( float_flag_divbyzero, c );
			return packFloatx80( zSign, floatx80_default_infinity_high, floatx80_default_infinity_low );
		}
		normalizeFloatx80Subnormal( bSig, &bExp, &bSig );
	}
	if ( aExp == 0 ) {
		if ( aSig == 0 ) return packFloatx80( zSign, 0, 0 );
		normalizeFloatx80Subnormal( aSig, &aExp, &aSig );
	}
	zExp = aExp - bExp + 0x3FFE;
	rem1 = 0;
	if ( bSig <= aSig ) {
		shift128Right( aSig, 0, 1, &aSig, &rem1 );
		++zExp;
	}
	zSig0 = estimateDiv128To64( aSig, rem1, bSig );
	mul64To128( bSig, zSig0, &term0, &term1 );
	sub128( aSig, rem1, term0, term1, &rem0, &rem1 );
	while ( (sbits64) rem0 < 0 ) {
		--zSig0;
		add128( rem0, rem1, 0, bSig, &rem0, &rem1 );
	}
	zSig1 = estimateDiv128To64( rem1, 0, bSig );
	if ( (bits64) ( zSig1<<1 ) <= 8 ) {
		mul64To128( bSig, zSig1, &term1, &term2 );
		sub128( rem1, 0, term1, term2, &rem1, &rem2 );
		while ( (sbits64) rem1 < 0 ) {
			--zSig1;
			add128( rem1, rem2, 0, bSig, &rem1, &rem2 );
		}
		zSig1 |= ( ( rem1 | rem2 ) != 0 );
	}
	return roundSigAndPackFloatx80( 32, zSign, zExp, zSig0, zSig1, c );
        
}
#endif // End of addition for Previous
    
/*----------------------------------------------------------------------------
| Returns the remainder of the extended double-precision floating-point value
| `a' with respect to the corresponding value `b'.  The operation is performed
| according to the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/
#ifndef SOFTFLOAT_68K
floatx80 floatx80_rem( floatx80 a, floatx80 b )
{
	flag aSign, zSign;
	int32 aExp, bExp, expDiff;
	bits64 aSig0, aSig1, bSig;
	bits64 q, term0, term1, alternateASig0, alternateASig1;
	floatx80 z;

	aSig0 = extractFloatx80Frac( a );
	aExp = extractFloatx80Exp( a );
	aSign = extractFloatx80Sign( a );
	bSig = extractFloatx80Frac( b );
	bExp = extractFloatx80Exp( b );
//    bSign = extractFloatx80Sign( b );
	if ( aExp == 0x7FFF ) {
		if (    (bits64) ( aSig0<<1 )
				|| ( ( bExp == 0x7FFF ) && (bits64) ( bSig<<1 ) ) ) {
			return propagateFloatx80NaN( a, b );
		}
		goto invalid;
	}
	if ( bExp == 0x7FFF ) {
		if ( (bits64) ( bSig<<1 ) ) return propagateFloatx80NaN( a, b );
		return a;
	}
	if ( bExp == 0 ) {
		if ( bSig == 0 ) {
	invalid:
			float_raise( float_flag_invalid );
			return packFloatx80( 0, floatx80_default_nan_high, floatx80_default_nan_low );
		}
		normalizeFloatx80Subnormal( bSig, &bExp, &bSig );
	}
	if ( aExp == 0 ) {
		if ( (bits64) ( aSig0<<1 ) == 0 ) return a;
		normalizeFloatx80Subnormal( aSig0, &aExp, &aSig0 );
	}
	bSig |= LIT64( 0x8000000000000000 );
	zSign = aSign;
	expDiff = aExp - bExp;
	aSig1 = 0;
	if ( expDiff < 0 ) {
		if ( expDiff < -1 ) return a;
		shift128Right( aSig0, 0, 1, &aSig0, &aSig1 );
		expDiff = 0;
	}
	q = ( bSig <= aSig0 );
	if ( q ) aSig0 -= bSig;
	expDiff -= 64;
	while ( 0 < expDiff ) {
		q = estimateDiv128To64( aSig0, aSig1, bSig );
		q = ( 2 < q ) ? q - 2 : 0;
		mul64To128( bSig, q, &term0, &term1 );
		sub128( aSig0, aSig1, term0, term1, &aSig0, &aSig1 );
		shortShift128Left( aSig0, aSig1, 62, &aSig0, &aSig1 );
		expDiff -= 62;
	}
	expDiff += 64;
	if ( 0 < expDiff ) {
		q = estimateDiv128To64( aSig0, aSig1, bSig );
		q = ( 2 < q ) ? q - 2 : 0;
		q >>= 64 - expDiff;
		mul64To128( bSig, q<<( 64 - expDiff ), &term0, &term1 );
		sub128( aSig0, aSig1, term0, term1, &aSig0, &aSig1 );
		shortShift128Left( 0, bSig, 64 - expDiff, &term0, &term1 );
		while ( le128( term0, term1, aSig0, aSig1 ) ) {
			++q;
			sub128( aSig0, aSig1, term0, term1, &aSig0, &aSig1 );
		}
	}
	else {
		term1 = 0;
		term0 = bSig;
	}
	sub128( term0, term1, aSig0, aSig1, &alternateASig0, &alternateASig1 );
	if (    lt128( alternateASig0, alternateASig1, aSig0, aSig1 )
			|| (    eq128( alternateASig0, alternateASig1, aSig0, aSig1 )
				&& ( q & 1 ) )
		) {
		aSig0 = alternateASig0;
		aSig1 = alternateASig1;
		zSign = ! zSign;
	}
	return
		normalizeRoundAndPackFloatx80(
			80, zSign, bExp + expDiff, aSig0, aSig1 );

}
#else // 09-01-2017: Modified version for Previous
floatx80 floatx80_rem( floatx80 a, floatx80 b, bits64 *q, flag *s, float_ctrl* c )
{
    flag aSign, bSign, zSign;
    int32 aExp, bExp, expDiff;
    bits64 aSig0, aSig1, bSig;
    bits64 qTemp, term0, term1, alternateASig0, alternateASig1;
    
    aSig0 = extractFloatx80Frac( a );
    aExp = extractFloatx80Exp( a );
    aSign = extractFloatx80Sign( a );
    bSig = extractFloatx80Frac( b );
    bExp = extractFloatx80Exp( b );
    bSign = extractFloatx80Sign( b );

    if ( aExp == 0x7FFF ) {
        if (    (bits64) ( aSig0<<1 )
            || ( ( bExp == 0x7FFF ) && (bits64) ( bSig<<1 ) ) ) {
            return propagateFloatx80NaN( a, b, c );
        }
        goto invalid;
    }
    if ( bExp == 0x7FFF ) {
        if ( (bits64) ( bSig<<1 ) ) return propagateFloatx80NaN( a, b, c );
        *s = (aSign != bSign);
        *q = 0;
        return a;
    }
    if ( bExp == 0 ) {
        if ( bSig == 0 ) {
        invalid:
            float_raise( float_flag_invalid, c );
            return packFloatx80( 0, floatx80_default_nan_high, floatx80_default_nan_low );
        }
        normalizeFloatx80Subnormal( bSig, &bExp, &bSig );
    }
    if ( aExp == 0 ) {
#ifdef SOFTFLOAT_68K
        if ( aSig0 == 0 ) {
            *s = (aSign != bSign);
            *q = 0;
            return a;
        }
#else
        if ( (bits64) ( aSig0<<1 ) == 0 ) return a;
#endif
        normalizeFloatx80Subnormal( aSig0, &aExp, &aSig0 );
    }
    bSig |= LIT64( 0x8000000000000000 );
    zSign = aSign;
    expDiff = aExp - bExp;
    *s = (aSign != bSign);
    aSig1 = 0;
    if ( expDiff < 0 ) {
        if ( expDiff < -1 ) return a;
        shift128Right( aSig0, 0, 1, &aSig0, &aSig1 );
        expDiff = 0;
    }
    qTemp = ( bSig <= aSig0 );
    if ( qTemp ) aSig0 -= bSig;
    *q = ( expDiff > 63 ) ? 0 : ( qTemp<<expDiff );
    expDiff -= 64;
    while ( 0 < expDiff ) {
        qTemp = estimateDiv128To64( aSig0, aSig1, bSig );
        qTemp = ( 2 < qTemp ) ? qTemp - 2 : 0;
        mul64To128( bSig, qTemp, &term0, &term1 );
        sub128( aSig0, aSig1, term0, term1, &aSig0, &aSig1 );
        shortShift128Left( aSig0, aSig1, 62, &aSig0, &aSig1 );
        *q = ( expDiff > 63 ) ? 0 : ( qTemp<<expDiff );
        expDiff -= 62;
    }
    expDiff += 64;
    if ( 0 < expDiff ) {
        qTemp = estimateDiv128To64( aSig0, aSig1, bSig );
        qTemp = ( 2 < qTemp ) ? qTemp - 2 : 0;
        qTemp >>= 64 - expDiff;
        mul64To128( bSig, qTemp<<( 64 - expDiff ), &term0, &term1 );
        sub128( aSig0, aSig1, term0, term1, &aSig0, &aSig1 );
        shortShift128Left( 0, bSig, 64 - expDiff, &term0, &term1 );
        while ( le128( term0, term1, aSig0, aSig1 ) ) {
            ++qTemp;
            sub128( aSig0, aSig1, term0, term1, &aSig0, &aSig1 );
        }
        *q += qTemp;
    }
    else {
        term1 = 0;
        term0 = bSig;
    }
    sub128( term0, term1, aSig0, aSig1, &alternateASig0, &alternateASig1 );
    if (    lt128( alternateASig0, alternateASig1, aSig0, aSig1 )
        || (    eq128( alternateASig0, alternateASig1, aSig0, aSig1 )
            && ( qTemp & 1 ) )
        ) {
        aSig0 = alternateASig0;
        aSig1 = alternateASig1;
        zSign = ! zSign;
        ++*q;
    }
    return
    normalizeRoundAndPackFloatx80(
                                  80, zSign, bExp + expDiff, aSig0, aSig1, c );
    
}
#endif // End of modification

#ifdef SOFTFLOAT_68K // 08-01-2017: Added for Previous
/*----------------------------------------------------------------------------
 | Returns the modulo remainder of the extended double-precision floating-point
 | value `a' with respect to the corresponding value `b'.
 *----------------------------------------------------------------------------*/

floatx80 floatx80_mod( floatx80 a, floatx80 b, bits64 *q, flag *s, float_ctrl* c )
{
    flag aSign, bSign, zSign;
    int32 aExp, bExp, expDiff;
    bits64 aSig0, aSig1, bSig;
    bits64 qTemp, term0, term1;
    
    aSig0 = extractFloatx80Frac( a );
    aExp = extractFloatx80Exp( a );
    aSign = extractFloatx80Sign( a );
    bSig = extractFloatx80Frac( b );
    bExp = extractFloatx80Exp( b );
    bSign = extractFloatx80Sign( b );

    if ( aExp == 0x7FFF ) {
        if (    (bits64) ( aSig0<<1 )
            || ( ( bExp == 0x7FFF ) && (bits64) ( bSig<<1 ) ) ) {
            return propagateFloatx80NaN( a, b, c );
        }
        goto invalid;
    }
    if ( bExp == 0x7FFF ) {
        if ( (bits64) ( bSig<<1 ) ) return propagateFloatx80NaN( a, b, c );
        *s = (aSign != bSign);
        *q = 0;
        return a;
    }
    if ( bExp == 0 ) {
        if ( bSig == 0 ) {
        invalid:
            float_raise( float_flag_invalid, c );
            return packFloatx80( 0, floatx80_default_nan_high, floatx80_default_nan_low );
        }
        normalizeFloatx80Subnormal( bSig, &bExp, &bSig );
    }
    if ( aExp == 0 ) {
#ifdef SOFTFLOAT_68K
        if ( aSig0 == 0 ) {
            *s = (aSign != bSign);
            *q = 0;
            return a;
        }
#else
        if ( (bits64) ( aSig0<<1 ) == 0 ) return a;
#endif
        normalizeFloatx80Subnormal( aSig0, &aExp, &aSig0 );
    }
    bSig |= LIT64( 0x8000000000000000 );
    zSign = aSign;
    expDiff = aExp - bExp;
    *s = (aSign != bSign);
    aSig1 = 0;
    if ( expDiff < 0 ) return a;
    qTemp = ( bSig <= aSig0 );
    if ( qTemp ) aSig0 -= bSig;
    *q = ( expDiff > 63 ) ? 0 : ( qTemp<<expDiff );
    expDiff -= 64;
    while ( 0 < expDiff ) {
        qTemp = estimateDiv128To64( aSig0, aSig1, bSig );
        qTemp = ( 2 < qTemp ) ? qTemp - 2 : 0;
        mul64To128( bSig, qTemp, &term0, &term1 );
        sub128( aSig0, aSig1, term0, term1, &aSig0, &aSig1 );
        shortShift128Left( aSig0, aSig1, 62, &aSig0, &aSig1 );
        *q = ( expDiff > 63 ) ? 0 : ( qTemp<<expDiff );
        expDiff -= 62;
    }
    expDiff += 64;
    if ( 0 < expDiff ) {
        qTemp = estimateDiv128To64( aSig0, aSig1, bSig );
        qTemp = ( 2 < qTemp ) ? qTemp - 2 : 0;
        qTemp >>= 64 - expDiff;
        mul64To128( bSig, qTemp<<( 64 - expDiff ), &term0, &term1 );
        sub128( aSig0, aSig1, term0, term1, &aSig0, &aSig1 );
        shortShift128Left( 0, bSig, 64 - expDiff, &term0, &term1 );
        while ( le128( term0, term1, aSig0, aSig1 ) ) {
            ++qTemp;
            sub128( aSig0, aSig1, term0, term1, &aSig0, &aSig1 );
        }
        *q += qTemp;
    }
    return
        normalizeRoundAndPackFloatx80(
            80, zSign, bExp + expDiff, aSig0, aSig1, c );
    
}
#endif // end of addition for Previous

/*----------------------------------------------------------------------------
| Returns the square root of the extended double-precision floating-point
| value `a'.  The operation is performed according to the IEC/IEEE Standard
| for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

floatx80 floatx80_sqrt( floatx80 a, float_ctrl* c )
{
	flag aSign;
	int32 aExp, zExp;
	bits64 aSig0, aSig1, zSig0, zSig1, doubleZSig0;
	bits64 rem0, rem1, rem2, rem3, term0, term1, term2, term3;

	aSig0 = extractFloatx80Frac( a );
	aExp = extractFloatx80Exp( a );
	aSign = extractFloatx80Sign( a );
	if ( aExp == 0x7FFF ) {
		if ( (bits64) ( aSig0<<1 ) ) return propagateFloatx80NaNOneArg( a, c );
		if ( ! aSign ) return inf_clear_intbit( c ) ? packFloatx80(aSign, aExp, 0) : a;
		goto invalid;
	}
	if ( aSign ) {
		if ( ( aExp | aSig0 ) == 0 ) return a;
	invalid:
		float_raise( float_flag_invalid, c );
		return packFloatx80( 0, floatx80_default_nan_high, floatx80_default_nan_low );
	}
	if ( aExp == 0 ) {
		if ( aSig0 == 0 ) return packFloatx80( 0, 0, 0 );
		normalizeFloatx80Subnormal( aSig0, &aExp, &aSig0 );
	}
	zExp = ( ( aExp - 0x3FFF )>>1 ) + 0x3FFF;
	zSig0 = estimateSqrt32( aExp, aSig0>>32 );
	shift128Right( aSig0, 0, 2 + ( aExp & 1 ), &aSig0, &aSig1 );
	zSig0 = estimateDiv128To64( aSig0, aSig1, zSig0<<32 ) + ( zSig0<<30 );
	doubleZSig0 = zSig0<<1;
	mul64To128( zSig0, zSig0, &term0, &term1 );
	sub128( aSig0, aSig1, term0, term1, &rem0, &rem1 );
	while ( (sbits64) rem0 < 0 ) {
		--zSig0;
		doubleZSig0 -= 2;
		add128( rem0, rem1, zSig0>>63, doubleZSig0 | 1, &rem0, &rem1 );
	}
	zSig1 = estimateDiv128To64( rem1, 0, doubleZSig0 );
	if ( ( zSig1 & LIT64( 0x3FFFFFFFFFFFFFFF ) ) <= 5 ) {
		if ( zSig1 == 0 ) zSig1 = 1;
		mul64To128( doubleZSig0, zSig1, &term1, &term2 );
		sub128( rem1, 0, term1, term2, &rem1, &rem2 );
		mul64To128( zSig1, zSig1, &term2, &term3 );
		sub192( rem1, rem2, 0, 0, term2, term3, &rem1, &rem2, &rem3 );
		while ( (sbits64) rem1 < 0 ) {
			--zSig1;
			shortShift128Left( 0, zSig1, 1, &term2, &term3 );
			term3 |= 1;
			term2 |= doubleZSig0;
			add192( rem1, rem2, rem3, 0, term2, term3, &rem1, &rem2, &rem3 );
		}
		zSig1 |= ( ( rem1 | rem2 | rem3 ) != 0 );
	}
	shortShift128Left( 0, zSig1, 1, &zSig0, &zSig1 );
	zSig0 |= doubleZSig0;
	return
		roundAndPackFloatx80(
			get_float_rounding_precision(c), 0, zExp, zSig0, zSig1, c );

}

#ifdef SOFTFLOAT_68K // 07-01-2017: Added for Previous
/*----------------------------------------------------------------------------
 | Returns the mantissa of the extended double-precision floating-point
 | value `a'.
 *----------------------------------------------------------------------------*/

floatx80 floatx80_getman( floatx80 a, float_ctrl* c )
{
    flag aSign;
    int32 aExp;
    bits64 aSig;
    
    aSig = extractFloatx80Frac( a );
    aExp = extractFloatx80Exp( a );
    aSign = extractFloatx80Sign( a );
    
    if ( aExp == 0x7FFF ) {
        if ( (bits64) ( aSig<<1 ) ) return propagateFloatx80NaNOneArg( a, c );
        float_raise( float_flag_invalid, c );
        return packFloatx80( 0, floatx80_default_nan_high, floatx80_default_nan_low );
    }
    
    if ( aExp == 0 ) {
        if ( aSig == 0 ) return packFloatx80( aSign, 0, 0 );
        normalizeFloatx80Subnormal( aSig, &aExp, &aSig );
    }
    
    return roundAndPackFloatx80( get_float_rounding_precision(c), aSign, 0x3FFF, aSig, 0, c );
}

/*----------------------------------------------------------------------------
 | Returns the exponent of the extended double-precision floating-point
 | value `a' as an extended double-precision value.
 *----------------------------------------------------------------------------*/

floatx80 floatx80_getexp( floatx80 a, float_ctrl* c )
{
    flag aSign;
    int32 aExp;
    bits64 aSig;
    
    aSig = extractFloatx80Frac( a );
    aExp = extractFloatx80Exp( a );
    aSign = extractFloatx80Sign( a );
    
    if ( aExp == 0x7FFF ) {
        if ( (bits64) ( aSig<<1 ) ) return propagateFloatx80NaNOneArg( a, c );
        float_raise( float_flag_invalid, c );
        return packFloatx80( 0, floatx80_default_nan_high, floatx80_default_nan_low );
    }
    
    if ( aExp == 0 ) {
        if ( aSig == 0 ) return packFloatx80( aSign, 0, 0 );
        normalizeFloatx80Subnormal( aSig, &aExp, &aSig );
    }
    
    return int32_to_floatx80(aExp - 0x3FFF);
}

/*----------------------------------------------------------------------------
 | Scales extended double-precision floating-point value in operand `a' by
 | value `b'. The function truncates the value in the second operand 'b' to
 | an integral value and adds that value to the exponent of the operand 'a'.
 | The operation performed according to the IEC/IEEE Standard for Binary
 | Floating-Point Arithmetic.
 *----------------------------------------------------------------------------*/

floatx80 floatx80_scale( floatx80 a, floatx80 b, float_ctrl* c )
{
    flag aSign, bSign;
    int32 aExp, bExp, shiftCount;
    bits64 aSig, bSig;
    
    aSig = extractFloatx80Frac(a);
    aExp = extractFloatx80Exp(a);
    aSign = extractFloatx80Sign(a);
    bSig = extractFloatx80Frac(b);
    bExp = extractFloatx80Exp(b);
    bSign = extractFloatx80Sign(b);
    
    if ( bExp == 0x7FFF ) {
        if ( (bits64) ( bSig<<1 ) ||
            ( ( aExp == 0x7FFF ) && (bits64) ( aSig<<1 ) ) ) {
            return propagateFloatx80NaN( a, b, c );
        }
        float_raise( float_flag_invalid, c );
        return packFloatx80( 0, floatx80_default_nan_high, floatx80_default_nan_low );
    }
    if ( aExp == 0x7FFF ) {
        if ( (bits64) ( aSig<<1 ) ) return propagateFloatx80NaN( a, b, c );
        return a;
    }
    if ( aExp == 0 ) {
        if ( aSig == 0 ) return packFloatx80( aSign, 0, 0);
        if ( bExp < 0x3FFF ) return a;
        normalizeFloatx80Subnormal( aSig, &aExp, &aSig );
    }
    
    if ( bExp < 0x3FFF ) return a;
    
    if ( 0x400F < bExp ) {
        aExp = bSign ? -0x6001 : 0xE000;
        return roundAndPackFloatx80(
                    get_float_rounding_precision(c), aSign, aExp, aSig, 0, c );
    }
    
    shiftCount = 0x403E - bExp;
    bSig >>= shiftCount;
    aExp = bSign ? ( aExp - bSig ) : ( aExp + bSig );
    
    return roundAndPackFloatx80(
                get_float_rounding_precision(c), aSign, aExp, aSig, 0, c );
    
}
    
/*-----------------------------------------------------------------------------
 | Calculates the absolute value of the extended double-precision floating-point
 | value `a'.  The operation is performed according to the IEC/IEEE Standard
 | for Binary Floating-Point Arithmetic.
 *----------------------------------------------------------------------------*/
    
floatx80 floatx80_abs( floatx80 a, float_ctrl* c )
{
    int32 aExp;
    bits64 aSig;
    
    aSig = extractFloatx80Frac(a);
    aExp = extractFloatx80Exp(a);
    
    if ( aExp == 0x7FFF ) {
        if ( (bits64) ( aSig<<1 ) ) return propagateFloatx80NaNOneArg( a, c );
        if ( inf_clear_intbit( c ) ) aSig = 0;
        return packFloatx80( 0, aExp, aSig );
    }
    
    if ( aExp == 0 ) {
        if ( aSig == 0 ) return packFloatx80( 0, 0, 0 );
        normalizeFloatx80Subnormal( aSig, &aExp, &aSig );
    }

    return roundAndPackFloatx80(
                get_float_rounding_precision(c), 0, aExp, aSig, 0, c );
    
}
    
/*-----------------------------------------------------------------------------
 | Changes the sign of the extended double-precision floating-point value 'a'.
 | The operation is performed according to the IEC/IEEE Standard for Binary
 | Floating-Point Arithmetic.
 *----------------------------------------------------------------------------*/
    
floatx80 floatx80_neg( floatx80 a, float_ctrl* c )
{
    flag aSign;
    int32 aExp;
    bits64 aSig;
    
    aSig = extractFloatx80Frac(a);
    aExp = extractFloatx80Exp(a);
    aSign = extractFloatx80Sign(a);
    
    if ( aExp == 0x7FFF ) {
        if ( (bits64) ( aSig<<1 ) ) return propagateFloatx80NaNOneArg( a, c );
        if ( inf_clear_intbit( c ) ) aSig = 0;
        return packFloatx80( !aSign, aExp, aSig );
    }
    
    aSign = !aSign;
    
    if ( aExp == 0 ) {
        if ( aSig == 0 ) return packFloatx80( aSign, 0, 0 );
        normalizeFloatx80Subnormal( aSig, &aExp, &aSig );
    }
    
    return roundAndPackFloatx80(
                get_float_rounding_precision(c), aSign, aExp, aSig, 0, c );
    
}
    
/*----------------------------------------------------------------------------
 | Returns the result of comparing the extended double-precision floating-
 | point values `a' and `b'.  The result is abstracted for matching the
 | corresponding condition codes.
 *----------------------------------------------------------------------------*/
    
floatx80 floatx80_cmp( floatx80 a, floatx80 b, float_ctrl* c )
{
    flag aSign, bSign;
    int32 aExp, bExp;
    bits64 aSig, bSig;
    
    aSig = extractFloatx80Frac( a );
    aExp = extractFloatx80Exp( a );
    aSign = extractFloatx80Sign( a );
    bSig = extractFloatx80Frac( b );
    bExp = extractFloatx80Exp( b );
    bSign = extractFloatx80Sign( b );
    
    if ( ( aExp == 0x7FFF && (bits64) ( aSig<<1 ) ) ||
         ( bExp == 0x7FFF && (bits64) ( bSig<<1 ) ) ) {
        if ( fcmp_signed_nan(c) ) return propagateFloatx80NaN(a, b, c);
        return propagateFloatx80NaN( packFloatx80( 0, aExp, aSig ),
                                     packFloatx80( 0, bExp, bSig ), c );
    }
    
    if ( bExp < aExp ) return packFloatx80( aSign, 0x3FFF, LIT64( 0x8000000000000000 ) );
    if ( aExp < bExp ) return packFloatx80( bSign ^ 1, 0x3FFF, LIT64( 0x8000000000000000 ) );
    
    if ( aExp == 0x7FFF ) {
        if ( aSign == bSign ) return packFloatx80( aSign, 0, 0 );
        return packFloatx80( aSign, 0x3FFF, LIT64( 0x8000000000000000 ) );
    }
    
    if ( bSig < aSig ) return packFloatx80( aSign, 0x3FFF, LIT64( 0x8000000000000000 ) );
    if ( aSig < bSig ) return packFloatx80( bSign ^ 1, 0x3FFF, LIT64( 0x8000000000000000 ) );
    
    if ( aSig == 0 ) return packFloatx80( aSign, 0, 0 );
    
    if ( aSign == bSign ) return packFloatx80( 0, 0, 0 );
    
    return packFloatx80( aSign, 0x3FFF, LIT64( 0x8000000000000000 ) );
    
}
    
floatx80 floatx80_tst( floatx80 a, float_ctrl* c )
{
    int32 aExp;
    bits64 aSig;
    
    aSig = extractFloatx80Frac( a );
    aExp = extractFloatx80Exp( a );
    
    if ( aExp == 0x7FFF && (bits64) ( aSig<<1 ) )
        return propagateFloatx80NaNOneArg( a, c );
    
    return a;
}
    
floatx80 floatx80_move( floatx80 a, float_ctrl* c )
{
    flag aSign;
    int32 aExp;
    bits64 aSig;
    
    aSig = extractFloatx80Frac( a );
    aExp = extractFloatx80Exp( a );
    aSign = extractFloatx80Sign( a );
    
    if ( aExp == 0x7FFF ) {
        if ( (bits64) ( aSig<<1 ) ) return propagateFloatx80NaNOneArg( a, c );
        return inf_clear_intbit( c ) ? packFloatx80(aSign, aExp, 0) : a;
    }
    if ( aExp == 0 ) {
        if ( aSig == 0 ) return a;
        return normalizeRoundAndPackFloatx80( get_float_rounding_precision(c), aSign, aExp, aSig, 0, c );
    }
    return roundAndPackFloatx80( get_float_rounding_precision(c), aSign, aExp, aSig, 0, c );
    
}
    
#endif // End of addition for Previous

/*----------------------------------------------------------------------------
| Returns 1 if the extended double-precision floating-point value `a' is
| equal to the corresponding value `b', and 0 otherwise.  The comparison is
| performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic.
*----------------------------------------------------------------------------*/

flag floatx80_eq( floatx80 a, floatx80 b, float_ctrl* c )
{
	if (    (    ( extractFloatx80Exp( a ) == 0x7FFF )
				&& (bits64) ( extractFloatx80Frac( a )<<1 ) )
			|| (    ( extractFloatx80Exp( b ) == 0x7FFF )
				&& (bits64) ( extractFloatx80Frac( b )<<1 ) )
		) {
		if (    floatx80_is_signaling_nan( a )
				|| floatx80_is_signaling_nan( b ) ) {
			float_raise( float_flag_invalid, c );
		}
		return 0;
	}
	return
			( a.low == b.low )
		&& (    ( a.high == b.high )
				|| (    ( a.low == 0 )
					&& ( (bits16) ( ( a.high | b.high )<<1 ) == 0 ) )
			);

}

/*----------------------------------------------------------------------------
| Returns 1 if the extended double-precision floating-point value `a' is
| less than or equal to the corresponding value `b', and 0 otherwise.  The
| comparison is performed according to the IEC/IEEE Standard for Binary
| Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

flag floatx80_le( floatx80 a, floatx80 b, float_ctrl* c )
{
	flag aSign, bSign;

	if (    (    ( extractFloatx80Exp( a ) == 0x7FFF )
				&& (bits64) ( extractFloatx80Frac( a )<<1 ) )
			|| (    ( extractFloatx80Exp( b ) == 0x7FFF )
				&& (bits64) ( extractFloatx80Frac( b )<<1 ) )
		) {
		float_raise( float_flag_invalid, c );
		return 0;
	}
	aSign = extractFloatx80Sign( a );
	bSign = extractFloatx80Sign( b );
	if ( aSign != bSign ) {
		return
				aSign
			|| (    ( ( (bits16) ( ( a.high | b.high )<<1 ) ) | a.low | b.low )
					== 0 );
	}
	return
			aSign ? le128( b.high, b.low, a.high, a.low )
		: le128( a.high, a.low, b.high, b.low );

}

/*----------------------------------------------------------------------------
| Returns 1 if the extended double-precision floating-point value `a' is
| less than the corresponding value `b', and 0 otherwise.  The comparison
| is performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic.
*----------------------------------------------------------------------------*/

flag floatx80_lt( floatx80 a, floatx80 b, float_ctrl* c )
{
	flag aSign, bSign;

	if (    (    ( extractFloatx80Exp( a ) == 0x7FFF )
				&& (bits64) ( extractFloatx80Frac( a )<<1 ) )
			|| (    ( extractFloatx80Exp( b ) == 0x7FFF )
				&& (bits64) ( extractFloatx80Frac( b )<<1 ) )
		) {
		float_raise( float_flag_invalid, c );
		return 0;
	}
	aSign = extractFloatx80Sign( a );
	bSign = extractFloatx80Sign( b );
	if ( aSign != bSign ) {
		return
				aSign
			&& (    ( ( (bits16) ( ( a.high | b.high )<<1 ) ) | a.low | b.low )
					!= 0 );
	}
	return
			aSign ? lt128( b.high, b.low, a.high, a.low )
		: lt128( a.high, a.low, b.high, b.low );

}

/*----------------------------------------------------------------------------
| Returns 1 if the extended double-precision floating-point value `a' is equal
| to the corresponding value `b', and 0 otherwise.  The invalid exception is
| raised if either operand is a NaN.  Otherwise, the comparison is performed
| according to the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

flag floatx80_eq_signaling( floatx80 a, floatx80 b, float_ctrl* c )
{
	if (    (    ( extractFloatx80Exp( a ) == 0x7FFF )
				&& (bits64) ( extractFloatx80Frac( a )<<1 ) )
			|| (    ( extractFloatx80Exp( b ) == 0x7FFF )
				&& (bits64) ( extractFloatx80Frac( b )<<1 ) )
		) {
		float_raise( float_flag_invalid, c );
		return 0;
	}
	return
			( a.low == b.low )
		&& (    ( a.high == b.high )
				|| (    ( a.low == 0 )
					&& ( (bits16) ( ( a.high | b.high )<<1 ) == 0 ) )
			);

}

/*----------------------------------------------------------------------------
| Returns 1 if the extended double-precision floating-point value `a' is less
| than or equal to the corresponding value `b', and 0 otherwise.  Quiet NaNs
| do not cause an exception.  Otherwise, the comparison is performed according
| to the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

flag floatx80_le_quiet( floatx80 a, floatx80 b, float_ctrl* c )
{
	flag aSign, bSign;

	if (    (    ( extractFloatx80Exp( a ) == 0x7FFF )
				&& (bits64) ( extractFloatx80Frac( a )<<1 ) )
			|| (    ( extractFloatx80Exp( b ) == 0x7FFF )
				&& (bits64) ( extractFloatx80Frac( b )<<1 ) )
		) {
		if (    floatx80_is_signaling_nan( a )
				|| floatx80_is_signaling_nan( b ) ) {
			float_raise( float_flag_invalid, c );
		}
		return 0;
	}
	aSign = extractFloatx80Sign( a );
	bSign = extractFloatx80Sign( b );
	if ( aSign != bSign ) {
		return
				aSign
			|| (    ( ( (bits16) ( ( a.high | b.high )<<1 ) ) | a.low | b.low )
					== 0 );
	}
	return
			aSign ? le128( b.high, b.low, a.high, a.low )
		: le128( a.high, a.low, b.high, b.low );

}

/*----------------------------------------------------------------------------
| Returns 1 if the extended double-precision floating-point value `a' is less
| than the corresponding value `b', and 0 otherwise.  Quiet NaNs do not cause
| an exception.  Otherwise, the comparison is performed according to the
| IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

flag floatx80_lt_quiet( floatx80 a, floatx80 b, float_ctrl* c )
{
	flag aSign, bSign;

	if (    (    ( extractFloatx80Exp( a ) == 0x7FFF )
				&& (bits64) ( extractFloatx80Frac( a )<<1 ) )
			|| (    ( extractFloatx80Exp( b ) == 0x7FFF )
				&& (bits64) ( extractFloatx80Frac( b )<<1 ) )
		) {
		if (    floatx80_is_signaling_nan( a )
				|| floatx80_is_signaling_nan( b ) ) {
			float_raise( float_flag_invalid, c );
		}
		return 0;
	}
	aSign = extractFloatx80Sign( a );
	bSign = extractFloatx80Sign( b );
	if ( aSign != bSign ) {
		return
				aSign
			&& (    ( ( (bits16) ( ( a.high | b.high )<<1 ) ) | a.low | b.low )
					!= 0 );
	}
	return
			aSign ? lt128( b.high, b.low, a.high, a.low )
		: lt128( a.high, a.low, b.high, b.low );

}

#endif

#ifdef FLOAT128

/*----------------------------------------------------------------------------
| Returns the result of converting the quadruple-precision floating-point
| value `a' to the 32-bit two's complement integer format.  The conversion
| is performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic---which means in particular that the conversion is rounded
| according to the current rounding mode.  If `a' is a NaN, the largest
| positive integer is returned.  Otherwise, if the conversion overflows, the
| largest integer with the same sign as `a' is returned.
*----------------------------------------------------------------------------*/

int32 float128_to_int32( float128 a, float_ctrl* c )
{
	flag aSign;
	int32 aExp, shiftCount;
	bits64 aSig0, aSig1;

	aSig1 = extractFloat128Frac1( a );
	aSig0 = extractFloat128Frac0( a );
	aExp = extractFloat128Exp( a );
	aSign = extractFloat128Sign( a );
	if ( ( aExp == 0x7FFF ) && ( aSig0 | aSig1 ) ) aSign = 0;
	if ( aExp ) aSig0 |= LIT64( 0x0001000000000000 );
	aSig0 |= ( aSig1 != 0 );
	shiftCount = 0x4028 - aExp;
	if ( 0 < shiftCount ) shift64RightJamming( aSig0, shiftCount, &aSig0 );
	return roundAndPackInt32( aSign, aSig0, c );

}

/*----------------------------------------------------------------------------
| Returns the result of converting the quadruple-precision floating-point
| value `a' to the 32-bit two's complement integer format.  The conversion
| is performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic, except that the conversion is always rounded toward zero.  If
| `a' is a NaN, the largest positive integer is returned.  Otherwise, if the
| conversion overflows, the largest integer with the same sign as `a' is
| returned.
*----------------------------------------------------------------------------*/

int32 float128_to_int32_round_to_zero( float128 a, float_ctrl* c )
{
	flag aSign;
	int32 aExp, shiftCount;
	bits64 aSig0, aSig1, savedASig;
	int32 z;

	aSig1 = extractFloat128Frac1( a );
	aSig0 = extractFloat128Frac0( a );
	aExp = extractFloat128Exp( a );
	aSign = extractFloat128Sign( a );
	aSig0 |= ( aSig1 != 0 );
	if ( 0x401E < aExp ) {
		if ( ( aExp == 0x7FFF ) && aSig0 ) aSign = 0;
		goto invalid;
	}
	else if ( aExp < 0x3FFF ) {
		if ( aExp || aSig0 ) float_raise( float_flag_inexact, c );
		return 0;
	}
	aSig0 |= LIT64( 0x0001000000000000 );
	shiftCount = 0x402F - aExp;
	savedASig = aSig0;
	aSig0 >>= shiftCount;
	z = aSig0;
	if ( aSign ) z = - z;
    z = (sbits32) z;
	if ( ( z < 0 ) ^ aSign ) {
	invalid:
		float_raise( float_flag_invalid, c );
		return aSign ? (sbits32) 0x80000000 : 0x7FFFFFFF;
	}
	if ( ( aSig0<<shiftCount ) != savedASig ) {
		float_raise( float_flag_inexact, c );
	}
	return z;

}

/*----------------------------------------------------------------------------
| Returns the result of converting the quadruple-precision floating-point
| value `a' to the 64-bit two's complement integer format.  The conversion
| is performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic---which means in particular that the conversion is rounded
| according to the current rounding mode.  If `a' is a NaN, the largest
| positive integer is returned.  Otherwise, if the conversion overflows, the
| largest integer with the same sign as `a' is returned.
*----------------------------------------------------------------------------*/

int64 float128_to_int64( float128 a, float_ctrl* c )
{
	flag aSign;
	int32 aExp, shiftCount;
	bits64 aSig0, aSig1;

	aSig1 = extractFloat128Frac1( a );
	aSig0 = extractFloat128Frac0( a );
	aExp = extractFloat128Exp( a );
	aSign = extractFloat128Sign( a );
	if ( aExp ) aSig0 |= LIT64( 0x0001000000000000 );
	shiftCount = 0x402F - aExp;
	if ( shiftCount <= 0 ) {
		if ( 0x403E < aExp ) {
			float_raise( float_flag_invalid, c );
			if (    ! aSign
					|| (    ( aExp == 0x7FFF )
						&& ( aSig1 || ( aSig0 != LIT64( 0x0001000000000000 ) ) )
					)
				) {
				return LIT64( 0x7FFFFFFFFFFFFFFF );
			}
			return (sbits64) LIT64( 0x8000000000000000 );
		}
		shortShift128Left( aSig0, aSig1, - shiftCount, &aSig0, &aSig1 );
	}
	else {
		shift64ExtraRightJamming( aSig0, aSig1, shiftCount, &aSig0, &aSig1 );
	}
	return roundAndPackInt64( aSign, aSig0, aSig1, c );

}

/*----------------------------------------------------------------------------
| Returns the result of converting the quadruple-precision floating-point
| value `a' to the 64-bit two's complement integer format.  The conversion
| is performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic, except that the conversion is always rounded toward zero.
| If `a' is a NaN, the largest positive integer is returned.  Otherwise, if
| the conversion overflows, the largest integer with the same sign as `a' is
| returned.
*----------------------------------------------------------------------------*/

int64 float128_to_int64_round_to_zero( float128 a, float_ctrl* c )
{
	flag aSign;
	int32 aExp, shiftCount;
	bits64 aSig0, aSig1;
	int64 z;

	aSig1 = extractFloat128Frac1( a );
	aSig0 = extractFloat128Frac0( a );
	aExp = extractFloat128Exp( a );
	aSign = extractFloat128Sign( a );
	if ( aExp ) aSig0 |= LIT64( 0x0001000000000000 );
	shiftCount = aExp - 0x402F;
	if ( 0 < shiftCount ) {
		if ( 0x403E <= aExp ) {
			aSig0 &= LIT64( 0x0000FFFFFFFFFFFF );
			if (    ( a.high == LIT64( 0xC03E000000000000 ) )
					&& ( aSig1 < LIT64( 0x0002000000000000 ) ) ) {
				if ( aSig1 ) float_raise( float_flag_inexact, c );
			}
			else {
				float_raise( float_flag_invalid, c );
				if ( ! aSign || ( ( aExp == 0x7FFF ) && ( aSig0 | aSig1 ) ) ) {
					return LIT64( 0x7FFFFFFFFFFFFFFF );
				}
			}
			return (sbits64) LIT64( 0x8000000000000000 );
		}
		z = ( aSig0<<shiftCount ) | ( aSig1>>( ( - shiftCount ) & 63 ) );
		if ( (bits64) ( aSig1<<shiftCount ) ) {
			float_raise( float_flag_inexact, c );
		}
	}
	else {
		if ( aExp < 0x3FFF ) {
			if ( aExp | aSig0 | aSig1 ) {
				float_raise( float_flag_inexact, c );
			}
			return 0;
		}
		z = aSig0>>( - shiftCount );
		if (    aSig1
				|| ( shiftCount && (bits64) ( aSig0<<( shiftCount & 63 ) ) ) ) {
			float_raise( float_flag_inexact, c );
		}
	}
	if ( aSign ) z = - z;
	return z;

}

/*----------------------------------------------------------------------------
| Returns the result of converting the quadruple-precision floating-point
| value `a' to the single-precision floating-point format.  The conversion
| is performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic.
*----------------------------------------------------------------------------*/

float32 float128_to_float32( float128 a, float_ctrl* c )
{
	flag aSign;
	int32 aExp;
	bits64 aSig0, aSig1;
	bits32 zSig;

	aSig1 = extractFloat128Frac1( a );
	aSig0 = extractFloat128Frac0( a );
	aExp = extractFloat128Exp( a );
	aSign = extractFloat128Sign( a );
	if ( aExp == 0x7FFF ) {
		if ( aSig0 | aSig1 ) {
			return commonNaNToFloat32( float128ToCommonNaN( a, c ) );
		}
		return packFloat32( aSign, 0xFF, 0 );
	}
	aSig0 |= ( aSig1 != 0 );
	shift64RightJamming( aSig0, 18, &aSig0 );
	zSig = aSig0;
	if ( aExp || zSig ) {
		zSig |= 0x40000000;
		aExp -= 0x3F81;
	}
	return roundAndPackFloat32( aSign, aExp, zSig, c );

}

/*----------------------------------------------------------------------------
| Returns the result of converting the quadruple-precision floating-point
| value `a' to the double-precision floating-point format.  The conversion
| is performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic.
*----------------------------------------------------------------------------*/

float64 float128_to_float64( float128 a, float_ctrl* c )
{
	flag aSign;
	int32 aExp;
	bits64 aSig0, aSig1;

	aSig1 = extractFloat128Frac1( a );
	aSig0 = extractFloat128Frac0( a );
	aExp = extractFloat128Exp( a );
	aSign = extractFloat128Sign( a );
	if ( aExp == 0x7FFF ) {
		if ( aSig0 | aSig1 ) {
			return commonNaNToFloat64( float128ToCommonNaN( a, c ) );
		}
		return packFloat64( aSign, 0x7FF, 0 );
	}
	shortShift128Left( aSig0, aSig1, 14, &aSig0, &aSig1 );
	aSig0 |= ( aSig1 != 0 );
	if ( aExp || aSig0 ) {
		aSig0 |= LIT64( 0x4000000000000000 );
		aExp -= 0x3C01;
	}
	return roundAndPackFloat64( aSign, aExp, aSig0, c );

}

#ifdef FLOATX80

/*----------------------------------------------------------------------------
| Returns the result of converting the quadruple-precision floating-point
| value `a' to the extended double-precision floating-point format.  The
| conversion is performed according to the IEC/IEEE Standard for Binary
| Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

floatx80 float128_to_floatx80( float128 a, float_ctrl* c )
{
	flag aSign;
	int32 aExp;
	bits64 aSig0, aSig1;

	aSig1 = extractFloat128Frac1( a );
	aSig0 = extractFloat128Frac0( a );
	aExp = extractFloat128Exp( a );
	aSign = extractFloat128Sign( a );
	if ( aExp == 0x7FFF ) {
		if ( aSig0 | aSig1 ) {
			return commonNaNToFloatx80( float128ToCommonNaN( a, c ) );
		}
		return packFloatx80( aSign, floatx80_default_infinity_high, floatx80_default_infinity_low );
	}
	if ( aExp == 0 ) {
		if ( ( aSig0 | aSig1 ) == 0 ) return packFloatx80( aSign, 0, 0 );
		normalizeFloat128Subnormal( aSig0, aSig1, &aExp, &aSig0, &aSig1 );
	}
	else {
		aSig0 |= LIT64( 0x0001000000000000 );
	}
	shortShift128Left( aSig0, aSig1, 15, &aSig0, &aSig1 );
	return roundAndPackFloatx80( 80, aSign, aExp, aSig0, aSig1, c );

}

#endif

/*----------------------------------------------------------------------------
| Rounds the quadruple-precision floating-point value `a' to an integer, and
| returns the result as a quadruple-precision floating-point value.  The
| operation is performed according to the IEC/IEEE Standard for Binary
| Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float128 float128_round_to_int( float128 a, float_ctrl* c )
{
	flag aSign;
	int32 aExp;
	bits64 lastBitMask, roundBitsMask;
	int8 roundingMode;
	float128 z;

	aExp = extractFloat128Exp( a );
	if ( 0x402F <= aExp ) {
		if ( 0x406F <= aExp ) {
			if (    ( aExp == 0x7FFF )
					&& ( extractFloat128Frac0( a ) | extractFloat128Frac1( a ) )
				) {
				return propagateFloat128NaN( a, a, c );
			}
			return a;
		}
		lastBitMask = 1;
		lastBitMask = ( lastBitMask<<( 0x406E - aExp ) )<<1;
		roundBitsMask = lastBitMask - 1;
		z = a;
		roundingMode = get_float_rounding_mode( c );
		if ( roundingMode == float_round_nearest_even ) {
			if ( lastBitMask ) {
				add128( z.high, z.low, 0, lastBitMask>>1, &z.high, &z.low );
				if ( ( z.low & roundBitsMask ) == 0 ) z.low &= ~ lastBitMask;
			}
			else {
				if ( (sbits64) z.low < 0 ) {
					++z.high;
					if ( (bits64) ( z.low<<1 ) == 0 ) z.high &= ~1;
				}
			}
		}
		else if ( roundingMode != float_round_to_zero ) {
			if (   extractFloat128Sign( z )
					^ ( roundingMode == float_round_up ) ) {
				add128( z.high, z.low, 0, roundBitsMask, &z.high, &z.low );
			}
		}
		z.low &= ~ roundBitsMask;
	}
	else {
		if ( aExp < 0x3FFF ) {
			if ( ( ( (bits64) ( a.high<<1 ) ) | a.low ) == 0 ) return a;
			float_raise( float_flag_inexact, c );
			aSign = extractFloat128Sign( a );
            roundingMode = get_float_rounding_mode( c );
			switch ( roundingMode ) {
				case float_round_nearest_even:
				if (    ( aExp == 0x3FFE )
						&& (   extractFloat128Frac0( a )
							| extractFloat128Frac1( a ) )
					) {
					return packFloat128( aSign, 0x3FFF, 0, 0 );
				}
				break;
				case float_round_down:
				return
						aSign ? packFloat128( 1, 0x3FFF, 0, 0 )
					: packFloat128( 0, 0, 0, 0 );
				case float_round_up:
				return
						aSign ? packFloat128( 1, 0, 0, 0 )
					: packFloat128( 0, 0x3FFF, 0, 0 );
			}
			return packFloat128( aSign, 0, 0, 0 );
		}
		lastBitMask = 1;
		lastBitMask <<= 0x402F - aExp;
		roundBitsMask = lastBitMask - 1;
		z.low = 0;
		z.high = a.high;
		roundingMode = get_float_rounding_mode( c );
		if ( roundingMode == float_round_nearest_even ) {
			z.high += lastBitMask>>1;
			if ( ( ( z.high & roundBitsMask ) | a.low ) == 0 ) {
				z.high &= ~ lastBitMask;
			}
		}
		else if ( roundingMode != float_round_to_zero ) {
			if (   extractFloat128Sign( z )
					^ ( roundingMode == float_round_up ) ) {
				z.high |= ( a.low != 0 );
				z.high += roundBitsMask;
			}
		}
		z.high &= ~ roundBitsMask;
	}
	if ( ( z.low != a.low ) || ( z.high != a.high ) ) {
		float_raise( float_flag_inexact, c );
	}
	return z;

}

/*----------------------------------------------------------------------------
| Returns the result of adding the absolute values of the quadruple-precision
| floating-point values `a' and `b'.  If `zSign' is 1, the sum is negated
| before being returned.  `zSign' is ignored if the result is a NaN.
| The addition is performed according to the IEC/IEEE Standard for Binary
| Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

static float128 addFloat128Sigs( float128 a, float128 b, flag zSign, float_ctrl* c )
{
	int32 aExp, bExp, zExp;
	bits64 aSig0, aSig1, bSig0, bSig1, zSig0, zSig1, zSig2;
	int32 expDiff;

	aSig1 = extractFloat128Frac1( a );
	aSig0 = extractFloat128Frac0( a );
	aExp = extractFloat128Exp( a );
	bSig1 = extractFloat128Frac1( b );
	bSig0 = extractFloat128Frac0( b );
	bExp = extractFloat128Exp( b );
	expDiff = aExp - bExp;
	if ( 0 < expDiff ) {
		if ( aExp == 0x7FFF ) {
			if ( aSig0 | aSig1 ) return propagateFloat128NaN( a, b, c );
			return a;
		}
		if ( bExp == 0 ) {
			--expDiff;
		}
		else {
			bSig0 |= LIT64( 0x0001000000000000 );
		}
		shift128ExtraRightJamming(
			bSig0, bSig1, 0, expDiff, &bSig0, &bSig1, &zSig2 );
		zExp = aExp;
	}
	else if ( expDiff < 0 ) {
		if ( bExp == 0x7FFF ) {
			if ( bSig0 | bSig1 ) return propagateFloat128NaN( a, b, c );
			return packFloat128( zSign, 0x7FFF, 0, 0 );
		}
		if ( aExp == 0 ) {
			++expDiff;
		}
		else {
			aSig0 |= LIT64( 0x0001000000000000 );
		}
		shift128ExtraRightJamming(
			aSig0, aSig1, 0, - expDiff, &aSig0, &aSig1, &zSig2 );
		zExp = bExp;
	}
	else {
		if ( aExp == 0x7FFF ) {
			if ( aSig0 | aSig1 | bSig0 | bSig1 ) {
				return propagateFloat128NaN( a, b, c );
			}
			return a;
		}
		add128( aSig0, aSig1, bSig0, bSig1, &zSig0, &zSig1 );
		if ( aExp == 0 ) return packFloat128( zSign, 0, zSig0, zSig1 );
		zSig2 = 0;
		zSig0 |= LIT64( 0x0002000000000000 );
		zExp = aExp;
		goto shiftRight1;
	}
	aSig0 |= LIT64( 0x0001000000000000 );
	add128( aSig0, aSig1, bSig0, bSig1, &zSig0, &zSig1 );
	--zExp;
	if ( zSig0 < LIT64( 0x0002000000000000 ) ) goto roundAndPack;
	++zExp;
	shiftRight1:
	shift128ExtraRightJamming(
		zSig0, zSig1, zSig2, 1, &zSig0, &zSig1, &zSig2 );
	roundAndPack:
	return roundAndPackFloat128( zSign, zExp, zSig0, zSig1, zSig2, c );

}

/*----------------------------------------------------------------------------
| Returns the result of subtracting the absolute values of the quadruple-
| precision floating-point values `a' and `b'.  If `zSign' is 1, the
| difference is negated before being returned.  `zSign' is ignored if the
| result is a NaN.  The subtraction is performed according to the IEC/IEEE
| Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

static float128 subFloat128Sigs( float128 a, float128 b, flag zSign, float_ctrl* c )
{
	int32 aExp, bExp, zExp;
	bits64 aSig0, aSig1, bSig0, bSig1, zSig0, zSig1;
	int32 expDiff;
	float128 z;

	aSig1 = extractFloat128Frac1( a );
	aSig0 = extractFloat128Frac0( a );
	aExp = extractFloat128Exp( a );
	bSig1 = extractFloat128Frac1( b );
	bSig0 = extractFloat128Frac0( b );
	bExp = extractFloat128Exp( b );
	expDiff = aExp - bExp;
	shortShift128Left( aSig0, aSig1, 14, &aSig0, &aSig1 );
	shortShift128Left( bSig0, bSig1, 14, &bSig0, &bSig1 );
	if ( 0 < expDiff ) goto aExpBigger;
	if ( expDiff < 0 ) goto bExpBigger;
	if ( aExp == 0x7FFF ) {
		if ( aSig0 | aSig1 | bSig0 | bSig1 ) {
			return propagateFloat128NaN( a, b, c );
		}
		float_raise( float_flag_invalid, c );
		z.low = float128_default_nan_low;
		z.high = float128_default_nan_high;
		return z;
	}
	if ( aExp == 0 ) {
		aExp = 1;
		bExp = 1;
	}
	if ( bSig0 < aSig0 ) goto aBigger;
	if ( aSig0 < bSig0 ) goto bBigger;
	if ( bSig1 < aSig1 ) goto aBigger;
	if ( aSig1 < bSig1 ) goto bBigger;
	return packFloat128( get_float_rounding_mode( c ) == float_round_down, 0, 0, 0 );
	bExpBigger:
	if ( bExp == 0x7FFF ) {
		if ( bSig0 | bSig1 ) return propagateFloat128NaN( a, b, c );
		return packFloat128( zSign ^ 1, 0x7FFF, 0, 0 );
	}
	if ( aExp == 0 ) {
		++expDiff;
	}
	else {
		aSig0 |= LIT64( 0x4000000000000000 );
	}
	shift128RightJamming( aSig0, aSig1, - expDiff, &aSig0, &aSig1 );
	bSig0 |= LIT64( 0x4000000000000000 );
	bBigger:
	sub128( bSig0, bSig1, aSig0, aSig1, &zSig0, &zSig1 );
	zExp = bExp;
	zSign ^= 1;
	goto normalizeRoundAndPack;
	aExpBigger:
	if ( aExp == 0x7FFF ) {
		if ( aSig0 | aSig1 ) return propagateFloat128NaN( a, b, c );
		return a;
	}
	if ( bExp == 0 ) {
		--expDiff;
	}
	else {
		bSig0 |= LIT64( 0x4000000000000000 );
	}
	shift128RightJamming( bSig0, bSig1, expDiff, &bSig0, &bSig1 );
	aSig0 |= LIT64( 0x4000000000000000 );
	aBigger:
	sub128( aSig0, aSig1, bSig0, bSig1, &zSig0, &zSig1 );
	zExp = aExp;
	normalizeRoundAndPack:
	--zExp;
	return normalizeRoundAndPackFloat128( zSign, zExp - 14, zSig0, zSig1, c );

}

/*----------------------------------------------------------------------------
| Returns the result of adding the quadruple-precision floating-point values
| `a' and `b'.  The operation is performed according to the IEC/IEEE Standard
| for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float128 float128_add( float128 a, float128 b, float_ctrl* c )
{
	flag aSign, bSign;

	aSign = extractFloat128Sign( a );
	bSign = extractFloat128Sign( b );
	if ( aSign == bSign ) {
		return addFloat128Sigs( a, b, aSign, c );
	}
	else {
		return subFloat128Sigs( a, b, aSign, c );
	}

}

/*----------------------------------------------------------------------------
| Returns the result of subtracting the quadruple-precision floating-point
| values `a' and `b'.  The operation is performed according to the IEC/IEEE
| Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float128 float128_sub( float128 a, float128 b, float_ctrl* c )
{
	flag aSign, bSign;

	aSign = extractFloat128Sign( a );
	bSign = extractFloat128Sign( b );
	if ( aSign == bSign ) {
		return subFloat128Sigs( a, b, aSign, c );
	}
	else {
		return addFloat128Sigs( a, b, aSign, c );
	}

}

/*----------------------------------------------------------------------------
| Returns the result of multiplying the quadruple-precision floating-point
| values `a' and `b'.  The operation is performed according to the IEC/IEEE
| Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float128 float128_mul( float128 a, float128 b, float_ctrl* c )
{
	flag aSign, bSign, zSign;
	int32 aExp, bExp, zExp;
	bits64 aSig0, aSig1, bSig0, bSig1, zSig0, zSig1, zSig2, zSig3;
	float128 z;

	aSig1 = extractFloat128Frac1( a );
	aSig0 = extractFloat128Frac0( a );
	aExp = extractFloat128Exp( a );
	aSign = extractFloat128Sign( a );
	bSig1 = extractFloat128Frac1( b );
	bSig0 = extractFloat128Frac0( b );
	bExp = extractFloat128Exp( b );
	bSign = extractFloat128Sign( b );
	zSign = aSign ^ bSign;
	if ( aExp == 0x7FFF ) {
		if (    ( aSig0 | aSig1 )
				|| ( ( bExp == 0x7FFF ) && ( bSig0 | bSig1 ) ) ) {
			return propagateFloat128NaN( a, b, c );
		}
		if ( ( bExp | bSig0 | bSig1 ) == 0 ) goto invalid;
		return packFloat128( zSign, 0x7FFF, 0, 0 );
	}
	if ( bExp == 0x7FFF ) {
		if ( bSig0 | bSig1 ) return propagateFloat128NaN( a, b, c );
		if ( ( aExp | aSig0 | aSig1 ) == 0 ) {
	invalid:
			float_raise( float_flag_invalid, c );
			z.low = float128_default_nan_low;
			z.high = float128_default_nan_high;
			return z;
		}
		return packFloat128( zSign, 0x7FFF, 0, 0 );
	}
	if ( aExp == 0 ) {
		if ( ( aSig0 | aSig1 ) == 0 ) return packFloat128( zSign, 0, 0, 0 );
		normalizeFloat128Subnormal( aSig0, aSig1, &aExp, &aSig0, &aSig1 );
	}
	if ( bExp == 0 ) {
		if ( ( bSig0 | bSig1 ) == 0 ) return packFloat128( zSign, 0, 0, 0 );
		normalizeFloat128Subnormal( bSig0, bSig1, &bExp, &bSig0, &bSig1 );
	}
	zExp = aExp + bExp - 0x4000;
	aSig0 |= LIT64( 0x0001000000000000 );
	shortShift128Left( bSig0, bSig1, 16, &bSig0, &bSig1 );
	mul128To256( aSig0, aSig1, bSig0, bSig1, &zSig0, &zSig1, &zSig2, &zSig3 );
	add128( zSig0, zSig1, aSig0, aSig1, &zSig0, &zSig1 );
	zSig2 |= ( zSig3 != 0 );
	if ( LIT64( 0x0002000000000000 ) <= zSig0 ) {
		shift128ExtraRightJamming(
			zSig0, zSig1, zSig2, 1, &zSig0, &zSig1, &zSig2 );
		++zExp;
	}
	return roundAndPackFloat128( zSign, zExp, zSig0, zSig1, zSig2, c );

}

/*----------------------------------------------------------------------------
| Returns the result of dividing the quadruple-precision floating-point value
| `a' by the corresponding value `b'.  The operation is performed according to
| the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float128 float128_div( float128 a, float128 b, float_ctrl* c )
{
	flag aSign, bSign, zSign;
	int32 aExp, bExp, zExp;
	bits64 aSig0, aSig1, bSig0, bSig1, zSig0, zSig1, zSig2;
	bits64 rem0, rem1, rem2, rem3, term0, term1, term2, term3;
	float128 z;

	aSig1 = extractFloat128Frac1( a );
	aSig0 = extractFloat128Frac0( a );
	aExp = extractFloat128Exp( a );
	aSign = extractFloat128Sign( a );
	bSig1 = extractFloat128Frac1( b );
	bSig0 = extractFloat128Frac0( b );
	bExp = extractFloat128Exp( b );
	bSign = extractFloat128Sign( b );
	zSign = aSign ^ bSign;
	if ( aExp == 0x7FFF ) {
		if ( aSig0 | aSig1 ) return propagateFloat128NaN( a, b, c );
		if ( bExp == 0x7FFF ) {
			if ( bSig0 | bSig1 ) return propagateFloat128NaN( a, b, c );
			goto invalid;
		}
		return packFloat128( zSign, 0x7FFF, 0, 0 );
	}
	if ( bExp == 0x7FFF ) {
		if ( bSig0 | bSig1 ) return propagateFloat128NaN( a, b, c );
		return packFloat128( zSign, 0, 0, 0 );
	}
	if ( bExp == 0 ) {
		if ( ( bSig0 | bSig1 ) == 0 ) {
			if ( ( aExp | aSig0 | aSig1 ) == 0 ) {
	invalid:
				float_raise( float_flag_invalid, c );
				z.low = float128_default_nan_low;
				z.high = float128_default_nan_high;
				return z;
			}
			float_raise( float_flag_divbyzero, c );
			return packFloat128( zSign, 0x7FFF, 0, 0 );
		}
		normalizeFloat128Subnormal( bSig0, bSig1, &bExp, &bSig0, &bSig1 );
	}
	if ( aExp == 0 ) {
		if ( ( aSig0 | aSig1 ) == 0 ) return packFloat128( zSign, 0, 0, 0 );
		normalizeFloat128Subnormal( aSig0, aSig1, &aExp, &aSig0, &aSig1 );
	}
	zExp = aExp - bExp + 0x3FFD;
	shortShift128Left(
		aSig0 | LIT64( 0x0001000000000000 ), aSig1, 15, &aSig0, &aSig1 );
	shortShift128Left(
		bSig0 | LIT64( 0x0001000000000000 ), bSig1, 15, &bSig0, &bSig1 );
	if ( le128( bSig0, bSig1, aSig0, aSig1 ) ) {
		shift128Right( aSig0, aSig1, 1, &aSig0, &aSig1 );
		++zExp;
	}
	zSig0 = estimateDiv128To64( aSig0, aSig1, bSig0 );
	mul128By64To192( bSig0, bSig1, zSig0, &term0, &term1, &term2 );
	sub192( aSig0, aSig1, 0, term0, term1, term2, &rem0, &rem1, &rem2 );
	while ( (sbits64) rem0 < 0 ) {
		--zSig0;
		add192( rem0, rem1, rem2, 0, bSig0, bSig1, &rem0, &rem1, &rem2 );
	}
	zSig1 = estimateDiv128To64( rem1, rem2, bSig0 );
	if ( ( zSig1 & 0x3FFF ) <= 4 ) {
		mul128By64To192( bSig0, bSig1, zSig1, &term1, &term2, &term3 );
		sub192( rem1, rem2, 0, term1, term2, term3, &rem1, &rem2, &rem3 );
		while ( (sbits64) rem1 < 0 ) {
			--zSig1;
			add192( rem1, rem2, rem3, 0, bSig0, bSig1, &rem1, &rem2, &rem3 );
		}
		zSig1 |= ( ( rem1 | rem2 | rem3 ) != 0 );
	}
	shift128ExtraRightJamming( zSig0, zSig1, 0, 15, &zSig0, &zSig1, &zSig2 );
	return roundAndPackFloat128( zSign, zExp, zSig0, zSig1, zSig2, c );

}

/*----------------------------------------------------------------------------
| Returns the remainder of the quadruple-precision floating-point value `a'
| with respect to the corresponding value `b'.  The operation is performed
| according to the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float128 float128_rem( float128 a, float128 b, float_ctrl* c )
{
	flag aSign, zSign;
	int32 aExp, bExp, expDiff;
	bits64 aSig0, aSig1, bSig0, bSig1, q, term0, term1, term2;
	bits64 allZero, alternateASig0, alternateASig1, sigMean1;
	sbits64 sigMean0;
	float128 z;

	aSig1 = extractFloat128Frac1( a );
	aSig0 = extractFloat128Frac0( a );
	aExp = extractFloat128Exp( a );
	aSign = extractFloat128Sign( a );
	bSig1 = extractFloat128Frac1( b );
	bSig0 = extractFloat128Frac0( b );
	bExp = extractFloat128Exp( b );
//    bSign = extractFloat128Sign( b );
	if ( aExp == 0x7FFF ) {
		if (    ( aSig0 | aSig1 )
				|| ( ( bExp == 0x7FFF ) && ( bSig0 | bSig1 ) ) ) {
			return propagateFloat128NaN( a, b, c );
		}
		goto invalid;
	}
	if ( bExp == 0x7FFF ) {
		if ( bSig0 | bSig1 ) return propagateFloat128NaN( a, b, c );
		return a;
	}
	if ( bExp == 0 ) {
		if ( ( bSig0 | bSig1 ) == 0 ) {
	invalid:
			float_raise( float_flag_invalid, c );
			z.low = float128_default_nan_low;
			z.high = float128_default_nan_high;
			return z;
		}
		normalizeFloat128Subnormal( bSig0, bSig1, &bExp, &bSig0, &bSig1 );
	}
	if ( aExp == 0 ) {
		if ( ( aSig0 | aSig1 ) == 0 ) return a;
		normalizeFloat128Subnormal( aSig0, aSig1, &aExp, &aSig0, &aSig1 );
	}
	expDiff = aExp - bExp;
	if ( expDiff < -1 ) return a;
	shortShift128Left(
		aSig0 | LIT64( 0x0001000000000000 ),
		aSig1,
		15 - ( expDiff < 0 ),
		&aSig0,
		&aSig1
	);
	shortShift128Left(
		bSig0 | LIT64( 0x0001000000000000 ), bSig1, 15, &bSig0, &bSig1 );
	q = le128( bSig0, bSig1, aSig0, aSig1 );
	if ( q ) sub128( aSig0, aSig1, bSig0, bSig1, &aSig0, &aSig1 );
	expDiff -= 64;
	while ( 0 < expDiff ) {
		q = estimateDiv128To64( aSig0, aSig1, bSig0 );
		q = ( 4 < q ) ? q - 4 : 0;
		mul128By64To192( bSig0, bSig1, q, &term0, &term1, &term2 );
		shortShift192Left( term0, term1, term2, 61, &term1, &term2, &allZero );
		shortShift128Left( aSig0, aSig1, 61, &aSig0, &allZero );
		sub128( aSig0, 0, term1, term2, &aSig0, &aSig1 );
		expDiff -= 61;
	}
	if ( -64 < expDiff ) {
		q = estimateDiv128To64( aSig0, aSig1, bSig0 );
		q = ( 4 < q ) ? q - 4 : 0;
		q >>= - expDiff;
		shift128Right( bSig0, bSig1, 12, &bSig0, &bSig1 );
		expDiff += 52;
		if ( expDiff < 0 ) {
			shift128Right( aSig0, aSig1, - expDiff, &aSig0, &aSig1 );
		}
		else {
			shortShift128Left( aSig0, aSig1, expDiff, &aSig0, &aSig1 );
		}
		mul128By64To192( bSig0, bSig1, q, &term0, &term1, &term2 );
		sub128( aSig0, aSig1, term1, term2, &aSig0, &aSig1 );
	}
	else {
		shift128Right( aSig0, aSig1, 12, &aSig0, &aSig1 );
		shift128Right( bSig0, bSig1, 12, &bSig0, &bSig1 );
	}
	do {
		alternateASig0 = aSig0;
		alternateASig1 = aSig1;
		++q;
		sub128( aSig0, aSig1, bSig0, bSig1, &aSig0, &aSig1 );
	} while ( 0 <= (sbits64) aSig0 );
	add128(
		aSig0, aSig1, alternateASig0, alternateASig1, (bits64 *)&sigMean0, &sigMean1 );
	if (    ( sigMean0 < 0 )
			|| ( ( ( sigMean0 | sigMean1 ) == 0 ) && ( q & 1 ) ) ) {
		aSig0 = alternateASig0;
		aSig1 = alternateASig1;
	}
	zSign = ( (sbits64) aSig0 < 0 );
	if ( zSign ) sub128( 0, 0, aSig0, aSig1, &aSig0, &aSig1 );
	return
		normalizeRoundAndPackFloat128( aSign ^ zSign, bExp - 4, aSig0, aSig1, c );

}

/*----------------------------------------------------------------------------
| Returns the square root of the quadruple-precision floating-point value `a'.
| The operation is performed according to the IEC/IEEE Standard for Binary
| Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

float128 float128_sqrt( float128 a, float_ctrl* c )
{
	flag aSign;
	int32 aExp, zExp;
	bits64 aSig0, aSig1, zSig0, zSig1, zSig2, doubleZSig0;
	bits64 rem0, rem1, rem2, rem3, term0, term1, term2, term3;
	float128 z;

	aSig1 = extractFloat128Frac1( a );
	aSig0 = extractFloat128Frac0( a );
	aExp = extractFloat128Exp( a );
	aSign = extractFloat128Sign( a );
	if ( aExp == 0x7FFF ) {
		if ( aSig0 | aSig1 ) return propagateFloat128NaN( a, a, c );
		if ( ! aSign ) return a;
		goto invalid;
	}
	if ( aSign ) {
		if ( ( aExp | aSig0 | aSig1 ) == 0 ) return a;
	invalid:
		float_raise( float_flag_invalid, c );
		z.low = float128_default_nan_low;
		z.high = float128_default_nan_high;
		return z;
	}
	if ( aExp == 0 ) {
		if ( ( aSig0 | aSig1 ) == 0 ) return packFloat128( 0, 0, 0, 0 );
		normalizeFloat128Subnormal( aSig0, aSig1, &aExp, &aSig0, &aSig1 );
	}
	zExp = ( ( aExp - 0x3FFF )>>1 ) + 0x3FFE;
	aSig0 |= LIT64( 0x0001000000000000 );
	zSig0 = estimateSqrt32( aExp, aSig0>>17 );
	shortShift128Left( aSig0, aSig1, 13 - ( aExp & 1 ), &aSig0, &aSig1 );
	zSig0 = estimateDiv128To64( aSig0, aSig1, zSig0<<32 ) + ( zSig0<<30 );
	doubleZSig0 = zSig0<<1;
	mul64To128( zSig0, zSig0, &term0, &term1 );
	sub128( aSig0, aSig1, term0, term1, &rem0, &rem1 );
	while ( (sbits64) rem0 < 0 ) {
		--zSig0;
		doubleZSig0 -= 2;
		add128( rem0, rem1, zSig0>>63, doubleZSig0 | 1, &rem0, &rem1 );
	}
	zSig1 = estimateDiv128To64( rem1, 0, doubleZSig0 );
	if ( ( zSig1 & 0x1FFF ) <= 5 ) {
		if ( zSig1 == 0 ) zSig1 = 1;
		mul64To128( doubleZSig0, zSig1, &term1, &term2 );
		sub128( rem1, 0, term1, term2, &rem1, &rem2 );
		mul64To128( zSig1, zSig1, &term2, &term3 );
		sub192( rem1, rem2, 0, 0, term2, term3, &rem1, &rem2, &rem3 );
		while ( (sbits64) rem1 < 0 ) {
			--zSig1;
			shortShift128Left( 0, zSig1, 1, &term2, &term3 );
			term3 |= 1;
			term2 |= doubleZSig0;
			add192( rem1, rem2, rem3, 0, term2, term3, &rem1, &rem2, &rem3 );
		}
		zSig1 |= ( ( rem1 | rem2 | rem3 ) != 0 );
	}
	shift128ExtraRightJamming( zSig0, zSig1, 0, 14, &zSig0, &zSig1, &zSig2 );
	return roundAndPackFloat128( 0, zExp, zSig0, zSig1, zSig2, c );

}

/*----------------------------------------------------------------------------
| Returns 1 if the quadruple-precision floating-point value `a' is equal to
| the corresponding value `b', and 0 otherwise.  The comparison is performed
| according to the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

flag float128_eq( float128 a, float128 b, float_ctrl* c )
{
	if (    (    ( extractFloat128Exp( a ) == 0x7FFF )
				&& ( extractFloat128Frac0( a ) | extractFloat128Frac1( a ) ) )
			|| (    ( extractFloat128Exp( b ) == 0x7FFF )
				&& ( extractFloat128Frac0( b ) | extractFloat128Frac1( b ) ) )
		) {
		if (    float128_is_signaling_nan( a )
				|| float128_is_signaling_nan( b ) ) {
			float_raise( float_flag_invalid, c );
		}
		return 0;
	}
	return
			( a.low == b.low )
		&& (    ( a.high == b.high )
				|| (    ( a.low == 0 )
					&& ( (bits64) ( ( a.high | b.high )<<1 ) == 0 ) )
			);

}

/*----------------------------------------------------------------------------
| Returns 1 if the quadruple-precision floating-point value `a' is less than
| or equal to the corresponding value `b', and 0 otherwise.  The comparison
| is performed according to the IEC/IEEE Standard for Binary Floating-Point
| Arithmetic.
*----------------------------------------------------------------------------*/

flag float128_le( float128 a, float128 b, float_ctrl* c )
{
	flag aSign, bSign;

	if (    (    ( extractFloat128Exp( a ) == 0x7FFF )
				&& ( extractFloat128Frac0( a ) | extractFloat128Frac1( a ) ) )
			|| (    ( extractFloat128Exp( b ) == 0x7FFF )
				&& ( extractFloat128Frac0( b ) | extractFloat128Frac1( b ) ) )
		) {
		float_raise( float_flag_invalid, c );
		return 0;
	}
	aSign = extractFloat128Sign( a );
	bSign = extractFloat128Sign( b );
	if ( aSign != bSign ) {
		return
				aSign
			|| (    ( ( (bits64) ( ( a.high | b.high )<<1 ) ) | a.low | b.low )
					== 0 );
	}
	return
			aSign ? le128( b.high, b.low, a.high, a.low )
		: le128( a.high, a.low, b.high, b.low );

}

/*----------------------------------------------------------------------------
| Returns 1 if the quadruple-precision floating-point value `a' is less than
| the corresponding value `b', and 0 otherwise.  The comparison is performed
| according to the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

flag float128_lt( float128 a, float128 b, float_ctrl* c )
{
	flag aSign, bSign;

	if (    (    ( extractFloat128Exp( a ) == 0x7FFF )
				&& ( extractFloat128Frac0( a ) | extractFloat128Frac1( a ) ) )
			|| (    ( extractFloat128Exp( b ) == 0x7FFF )
				&& ( extractFloat128Frac0( b ) | extractFloat128Frac1( b ) ) )
		) {
		float_raise( float_flag_invalid, c );
		return 0;
	}
	aSign = extractFloat128Sign( a );
	bSign = extractFloat128Sign( b );
	if ( aSign != bSign ) {
		return
				aSign
			&& (    ( ( (bits64) ( ( a.high | b.high )<<1 ) ) | a.low | b.low )
					!= 0 );
	}
	return
			aSign ? lt128( b.high, b.low, a.high, a.low )
		: lt128( a.high, a.low, b.high, b.low );

}

/*----------------------------------------------------------------------------
| Returns 1 if the quadruple-precision floating-point value `a' is equal to
| the corresponding value `b', and 0 otherwise.  The invalid exception is
| raised if either operand is a NaN.  Otherwise, the comparison is performed
| according to the IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

flag float128_eq_signaling( float128 a, float128 b, float_ctrl* c )
{
	if (    (    ( extractFloat128Exp( a ) == 0x7FFF )
				&& ( extractFloat128Frac0( a ) | extractFloat128Frac1( a ) ) )
			|| (    ( extractFloat128Exp( b ) == 0x7FFF )
				&& ( extractFloat128Frac0( b ) | extractFloat128Frac1( b ) ) )
		) {
		float_raise( float_flag_invalid, c );
		return 0;
	}
	return
			( a.low == b.low )
		&& (    ( a.high == b.high )
				|| (    ( a.low == 0 )
					&& ( (bits64) ( ( a.high | b.high )<<1 ) == 0 ) )
			);

}

/*----------------------------------------------------------------------------
| Returns 1 if the quadruple-precision floating-point value `a' is less than
| or equal to the corresponding value `b', and 0 otherwise.  Quiet NaNs do not
| cause an exception.  Otherwise, the comparison is performed according to the
| IEC/IEEE Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

flag float128_le_quiet( float128 a, float128 b, float_ctrl* c )
{
	flag aSign, bSign;

	if (    (    ( extractFloat128Exp( a ) == 0x7FFF )
				&& ( extractFloat128Frac0( a ) | extractFloat128Frac1( a ) ) )
			|| (    ( extractFloat128Exp( b ) == 0x7FFF )
				&& ( extractFloat128Frac0( b ) | extractFloat128Frac1( b ) ) )
		) {
		if (    float128_is_signaling_nan( a )
				|| float128_is_signaling_nan( b ) ) {
			float_raise( float_flag_invalid, c );
		}
		return 0;
	}
	aSign = extractFloat128Sign( a );
	bSign = extractFloat128Sign( b );
	if ( aSign != bSign ) {
		return
				aSign
			|| (    ( ( (bits64) ( ( a.high | b.high )<<1 ) ) | a.low | b.low )
					== 0 );
	}
	return
			aSign ? le128( b.high, b.low, a.high, a.low )
		: le128( a.high, a.low, b.high, b.low );

}

/*----------------------------------------------------------------------------
| Returns 1 if the quadruple-precision floating-point value `a' is less than
| the corresponding value `b', and 0 otherwise.  Quiet NaNs do not cause an
| exception.  Otherwise, the comparison is performed according to the IEC/IEEE
| Standard for Binary Floating-Point Arithmetic.
*----------------------------------------------------------------------------*/

flag float128_lt_quiet( float128 a, float128 b, float_ctrl* c )
{
	flag aSign, bSign;

	if (    (    ( extractFloat128Exp( a ) == 0x7FFF )
				&& ( extractFloat128Frac0( a ) | extractFloat128Frac1( a ) ) )
			|| (    ( extractFloat128Exp( b ) == 0x7FFF )
				&& ( extractFloat128Frac0( b ) | extractFloat128Frac1( b ) ) )
		) {
		if (    float128_is_signaling_nan( a )
				|| float128_is_signaling_nan( b ) ) {
			float_raise( float_flag_invalid, c );
		}
		return 0;
	}
	aSign = extractFloat128Sign( a );
	bSign = extractFloat128Sign( b );
	if ( aSign != bSign ) {
		return
				aSign
			&& (    ( ( (bits64) ( ( a.high | b.high )<<1 ) ) | a.low | b.low )
					!= 0 );
	}
	return
			aSign ? lt128( b.high, b.low, a.high, a.low )
		: lt128( a.high, a.low, b.high, b.low );

}

#endif
