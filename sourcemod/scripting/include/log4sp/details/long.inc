#if defined _log4sp_long_included
    #endinput
#endif
#define _log4sp_long_included


/**
 * Converts a string to a Long.
 *
 * @param str           String to convert.
 * @param nBase         Numerical base to use.  10 is default.
 * @param nConsumed     Number of characters consumed.
 * @return              Long conversion of string, or 0 on failure.
 */
stock Long StringToLong(const char[] str, int nBase=10, int &nConsumed=0)
{
    int bits[2];
    nConsumed = StringToInt64(str, bits, nBase);
    return Int64ToLong(bits);
}

/**
 * Converts an integer to a Long.
 *
 * @param value         Integer to convert.
 * @return              Long conversion of integer.
 */
stock Long IntToLong(int value)
{
    Long result;
#if !defined SM_INT64_SUPPORTED
    result.__bits[0] = value;
    result.__bits[1] = (value < 0) ? -1 : 0;
#else
    result.__digits = value;
#endif      // SM_INT64_SUPPORTED
    return result;
}

/**
 * Converts a 64-bit integer to a Long.
 *
 * @param num           Array containing the upper and lower
 *                      32-bits of a 64-bit integer.
 * @return              Long conversion of 64-bit integer.
 */
stock Long Int64ToLong(const int value[2])
{
    Long result;
#if !defined SM_INT64_SUPPORTED
    result.__bits[0] = value[0];
    result.__bits[1] = value[1];
#else
    int64 low  = value[0] & 4294967295;
    int64 high = value[1];
    result.__digits = (high << 32) | low;
#endif      // SM_INT64_SUPPORTED
    return result;
}

#if defined SM_INT64_SUPPORTED
/**
 * Converts an int64 value to a Long.
 *
 * @param num           int64 value to convert.
 *                      32-bits of a 64-bit integer.
 * @return              Long conversion of int64 value.
 */
stock Long I64ToLong(const int64 value)
{
    Long result;
    result.__digits = value;
    return result;
}
#endif      // SM_INT64_SUPPORTED


enum struct Long
{
#if !defined SM_INT64_SUPPORTED

    Long Add(Long value) {
        int low   = this.__bits[0] + value.__bits[0];

        int carry = ((this.__bits[0] & value.__bits[0]) |
                    ((this.__bits[0] ^ value.__bits[0]) & ~low)) >>> 31;

        int high  = this.__bits[1] + value.__bits[1] + carry;

        Long result;
        result.__bits[0] = low;
        result.__bits[1] = high;
        return result;
    }

    Long Sub(Long value) {
        return this.Add(value.Neg());
    }

    Long Mul(Long value) {
        Long result;
        for (int i = 0; i < 64; ++i) {
            if (value.TestBit(i))
                result = result.Add(this.Shl(i));
        }
        return result;
    }

    Long Div(Long value) {
        Long quotient, remainder;
        this.DivideAndRemainder(value, quotient, remainder);
        return quotient;
    }

    Long Mod(Long value) {
        Long quotient, remainder;
        this.DivideAndRemainder(value, quotient, remainder);
        return remainder;
    }

    void DivideAndRemainder(Long value, Long quotient, Long remainder) {
        if (value.IsZero())
            ThrowError("Divide by zero.");

        Long dividend, divisor, ONE = {{1, 0}};
        dividend = this.Abs();
        divisor  = value.Abs();

        quotient.__bits[0]  = 0;
        quotient.__bits[1]  = 0;
        remainder.__bits[0] = 0;
        remainder.__bits[1] = 0;

        for (int i = 63; i >= 0; --i) {
            remainder = remainder.Shl(1);
            if (dividend.TestBit(i))
                remainder = remainder.Or(ONE);

            // unsigned compare
            bool signRemainder = remainder.IsNegative();
            bool signDivisor   = divisor.IsNegative();
            int unsignedCompare;
            if (signRemainder != signDivisor)
                unsignedCompare = signDivisor ? -1 : 1;
            else
                unsignedCompare = remainder.Compare(divisor);

            if (unsignedCompare >= 0) {
                remainder = remainder.Sub(divisor);
                quotient  = quotient.Or(ONE.Shl(i));
            }
        }

        if (this.IsNegative() ^ value.IsNegative())
            quotient = quotient.Neg();

        if (this.IsNegative())
            remainder = remainder.Neg();
    }

    Long Shl(int num) {
        num &= 63;
        if (num == 0)
            return this;

        Long result;
        if (num < 32) {
            result.__bits[1] = (this.__bits[1] << num) | (this.__bits[0] >>> (32 - num));
            result.__bits[0] = (this.__bits[0] << num);
        } else {
            result.__bits[1] = (this.__bits[0] << (num - 32));
            result.__bits[0] = 0;
        }
        return result;
    }

    Long Shr(int num) {
        num &= 63;
        if (num == 0)
            return this;

        Long result;
        if (num < 32) {
            result.__bits[0] = (this.__bits[0] >>> num) | (this.__bits[1] << (32 - num));
            result.__bits[1] = (this.__bits[1] >>> num);
        } else {
            result.__bits[0] = (this.__bits[1] >>> (num - 32));
            result.__bits[1] = 0;
        }
        return result;
    }

    Long Sar(int num) {
        num &= 63;
        if (num == 0)
            return this;

        Long result;
        if (num < 32) {
            result.__bits[0] = (this.__bits[0] >>> num) | (this.__bits[1] << (32 - num));
            result.__bits[1] = (this.__bits[1] >>  num);
        } else {
            result.__bits[0] = (this.__bits[1] >> (num - 32));
            result.__bits[1] = (this.__bits[1] < 0) ? -1 : 0;
        }
        return result;
    }

    Long And(Long value) {
        Long result;
        result.__bits[0] = this.__bits[0] & value.__bits[0];
        result.__bits[1] = this.__bits[1] & value.__bits[1];
        return result;
    }

    Long Or(Long value) {
        Long result;
        result.__bits[0] = this.__bits[0] | value.__bits[0];
        result.__bits[1] = this.__bits[1] | value.__bits[1];
        return result;
    }

    Long Xor(Long value) {
        Long result;
        result.__bits[0] = this.__bits[0] ^ value.__bits[0];
        result.__bits[1] = this.__bits[1] ^ value.__bits[1];
        return result;
    }

    Long Not() {
        Long result;
        result.__bits[0] = (~this.__bits[0]);
        result.__bits[1] = (~this.__bits[1]);
        return result;
    }

    Long Abs() {
        if (this.IsNegative())
            return this.Neg();
        return this;
    }

    Long Neg() {
        // -value = value + 1
        Long result;
        result.__bits[0] = (~this.__bits[0]) + 1;
        result.__bits[1] = (~this.__bits[1]) + ((result.__bits[0] != 0) ? 0 : 1);
        return result;
    }

    bool TestBit(int num) {
        num &= 63;
        if (num < 32)
            return (this.__bits[0] & (1 << num)) != 0;
        return (this.__bits[1] & (1 << (num - 32))) != 0;
    }

    Long SetBit(int num) {
        num &= 63;
        Long result;
        if (num < 32) {
            result.__bits[0] = (this.__bits[0] | (1 << num));
            result.__bits[1] = this.__bits[1];
        } else {
            result.__bits[0] = this.__bits[0];
            result.__bits[1] = (this.__bits[1] | (1 << (num - 32)));
        }
        return result;
    }

    Long ClearBit(int num) {
        num &= 63;
        Long result;
        if (num < 32) {
            result.__bits[0] = (this.__bits[0] & ~(1 << num));
            result.__bits[1] = this.__bits[1];
        } else {
            result.__bits[0] = this.__bits[0];
            result.__bits[1] = (this.__bits[1] & ~(1 << (num - 32)));
        }
        return result;
    }

    Long FlipBit(int num) {
        num &= 63;
        Long result;
        if (num < 32) {
            result.__bits[0] = (this.__bits[0] ^ (1 << num));
            result.__bits[1] = this.__bits[1];
        } else {
            result.__bits[0] = this.__bits[0];
            result.__bits[1] = (this.__bits[1] ^ (1 << (num - 32)));
        }
        return result;
    }

    int Compare(Long value) {
        if (this.__bits[1] >  value.__bits[1]) return  1;
        if (this.__bits[1] <  value.__bits[1]) return -1;
        if (this.__bits[0] == value.__bits[0]) return  0;

        // 低位按无符号比较
        if ((this.__bits[0] & 0x80000000) == (value.__bits[0] & 0x80000000))
            return (this.__bits[0] < value.__bits[0]) ? -1 : 1;
        return (this.__bits[0] > 0) ? -1 : 1;
    }

    int Sign() {
        if (this.__bits[0] == 0 && this.__bits[1] == 0)
            return 0;
        return (this.__bits[1] < 0) ? -1 : 1;
    }

    bool Lt(Long value) {
        return this.Compare(value) < 0;
    }

    bool Le(Long value) {
        return this.Compare(value) <= 0;
    }

    bool Eq(Long value) {
        return this.Compare(value) == 0;
    }

    bool Ge(Long value) {
        return this.Compare(value) >= 0;
    }

    bool Gt(Long value) {
        return this.Compare(value) > 0;
    }

    bool IsNegative() {
        return this.__bits[1] < 0;
    }

    bool IsZero() {
        return (this.__bits[0] == 0) && (this.__bits[1] == 0);
    }

    int ToString(char[] buffer, int maxlen) {
        return Int64ToString(this.__bits, buffer, maxlen);
    }

    char[] ToStringFixed(int &bytes=0) {
        char buffer[21];
        bytes = Int64ToString(this.__bits, buffer, sizeof(buffer));
        return buffer;
    }

    int Low() {
        return this.__bits[0];
    }

    int High() {
        return this.__bits[1];
    }

    int  __bits[2];

#else

    Long Add(Long value) {
        Long result;
        result.__digits = this.__digits + value.__digits;
        return result;
    }

    Long Sub(Long value) {
        Long result;
        result.__digits = this.__digits - value.__digits;
        return result;
    }

    Long Mul(Long value) {
        Long result;
        result.__digits = this.__digits * value.__digits;
        return result;
    }

    Long Div(Long value) {
        if (value.IsZero())
            ThrowError("Divide by zero.");

        Long result;
        result.__digits = this.__digits / value.__digits;
        return result;
    }

    Long Mod(Long value) {
        if (value.IsZero())
            ThrowError("Divide by zero.");

        Long result;
        result.__digits = this.__digits % value.__digits;
        return result;
    }

    void DivideAndRemainder(Long value, Long quotient, Long remainder) {
        if (value.IsZero())
            ThrowError("Divide by zero.");

        quotient.__digits  = this.__digits / value.__digits;
        remainder.__digits = this.__digits % value.__digits;
    }

    Long Shl(int num) {
        Long result;
        result.__digits = this.__digits << num;
        return result;
    }

    Long Shr(int num) {
        Long result;
        result.__digits = this.__digits >>> num;
        return result;
    }

    Long Sar(int num) {
        Long result;
        result.__digits = this.__digits >> num;
        return result;
    }

    Long And(Long value) {
        Long result;
        result.__digits = this.__digits & value.__digits;
        return result;
    }

    Long Or(Long value) {
        Long result;
        result.__digits = this.__digits | value.__digits;
        return result;
    }

    Long Xor(Long value) {
        Long result;
        result.__digits = this.__digits ^ value.__digits;
        return result;
    }

    Long Not() {
        Long result;
        result.__digits = ~this.__digits;
        return result;
    }

    Long Abs() {
        Long result;
        result.__digits = (this.__digits < 0) ? -this.__digits : this.__digits;
        return result;
    }

    Long Neg() {
        Long result;
        result.__digits = -this.__digits;
        return result;
    }

    bool TestBit(int num) {
        num &= 63;
        // (Computes ((this & (1<<n)) != 0).)
        return ((this.__digits >>> num) & 1) != 0;
    }

    Long SetBit(int num) {
        num &= 63;
        // (Computes (this | (1<<n)).)
        Long result;
        result.__digits = this.__digits | (result.__digits + (1 << num));
        return result;
    }

    Long ClearBit(int num) {
        num &= 63;
        // (Computes (this & ~(1<<n)).)
        Long result;
        result.__digits = this.__digits & ~(result.__digits + (1 << num));
        return result;
    }

    Long FlipBit(int num) {
        num &= 63;
        // (Computes (this ^ (1<<n)).)
        Long result;
        result.__digits = this.__digits ^ (result.__digits + (1 << num));
        return result;
    }

    int Compare(Long value) {
        if (this.__digits == value.__digits)
            return 0;
        return (this.__digits < value.__digits) ? -1 : 1;
    }

    int Sign() {
        if (this.__digits == 0)
            return 0;
        return (this.__digits < 0) ? -1 : 1;
    }

    bool Lt(Long value) {
        return this.__digits < value.__digits;
    }

    bool Le(Long value) {
        return this.__digits <= value.__digits;
    }

    bool Eq(Long value) {
        return this.__digits == value.__digits;
    }

    bool Ge(Long value) {
        return this.__digits >= value.__digits;
    }

    bool Gt(Long value) {
        return this.__digits > value.__digits;
    }

    bool IsNegative() {
        return this.__digits < 0;
    }

    bool IsZero() {
        return this.__digits == 0;
    }

    int ToString(char[] buffer, int maxlen) {
        return FormatEx(buffer, maxlen, "%ld", this.__digits);
    }

    char[] ToStringFixed(int &bytes=0) {
        char buffer[21];
        bytes = FormatEx(buffer, sizeof(buffer), "%ld", this.__digits);
        return buffer;
    }

    int64 ToI64() {
        return this.__digits;
    }

    int Low() {
        return view_as<int>(this.__digits & 4294967295);
    }

    int High() {
        return view_as<int>(this.__digits >>> 32);
    }

    int64 __digits;
#endif      // SM_INT64_SUPPORTED
}

