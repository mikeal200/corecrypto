/*
 * Copyright (c) 2012,2014,2015,2016,2018 Apple Inc. All rights reserved.
 *
 * corecrypto Internal Use License Agreement
 *
 * IMPORTANT:  This Apple corecrypto software is supplied to you by Apple Inc. ("Apple")
 * in consideration of your agreement to the following terms, and your download or use
 * of this Apple software constitutes acceptance of these terms.  If you do not agree
 * with these terms, please do not download or use this Apple software.
 *
 * 1.    As used in this Agreement, the term "Apple Software" collectively means and
 * includes all of the Apple corecrypto materials provided by Apple here, including
 * but not limited to the Apple corecrypto software, frameworks, libraries, documentation
 * and other Apple-created materials. In consideration of your agreement to abide by the
 * following terms, conditioned upon your compliance with these terms and subject to
 * these terms, Apple grants you, for a period of ninety (90) days from the date you
 * download the Apple Software, a limited, non-exclusive, non-sublicensable license
 * under Apple’s copyrights in the Apple Software to make a reasonable number of copies
 * of, compile, and run the Apple Software internally within your organization only on
 * devices and computers you own or control, for the sole purpose of verifying the
 * security characteristics and correct functioning of the Apple Software; provided
 * that you must retain this notice and the following text and disclaimers in all
 * copies of the Apple Software that you make. You may not, directly or indirectly,
 * redistribute the Apple Software or any portions thereof. The Apple Software is only
 * licensed and intended for use as expressly stated above and may not be used for other
 * purposes or in other contexts without Apple's prior written permission.  Except as
 * expressly stated in this notice, no other rights or licenses, express or implied, are
 * granted by Apple herein.
 *
 * 2.    The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES
 * OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING
 * THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS,
 * SYSTEMS, OR SERVICES. APPLE DOES NOT WARRANT THAT THE APPLE SOFTWARE WILL MEET YOUR
 * REQUIREMENTS, THAT THE OPERATION OF THE APPLE SOFTWARE WILL BE UNINTERRUPTED OR
 * ERROR-FREE, THAT DEFECTS IN THE APPLE SOFTWARE WILL BE CORRECTED, OR THAT THE APPLE
 * SOFTWARE WILL BE COMPATIBLE WITH FUTURE APPLE PRODUCTS, SOFTWARE OR SERVICES. NO ORAL
 * OR WRITTEN INFORMATION OR ADVICE GIVEN BY APPLE OR AN APPLE AUTHORIZED REPRESENTATIVE
 * WILL CREATE A WARRANTY.
 *
 * 3.    IN NO EVENT SHALL APPLE BE LIABLE FOR ANY DIRECT, SPECIAL, INDIRECT, INCIDENTAL
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING
 * IN ANY WAY OUT OF THE USE, REPRODUCTION, COMPILATION OR OPERATION OF THE APPLE
 * SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING
 * NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * 4.    This Agreement is effective until terminated. Your rights under this Agreement will
 * terminate automatically without notice from Apple if you fail to comply with any term(s)
 * of this Agreement.  Upon termination, you agree to cease all use of the Apple Software
 * and destroy all copies, full or partial, of the Apple Software. This Agreement will be
 * governed and construed in accordance with the laws of the State of California, without
 * regard to its choice of law rules.
 *
 * You may report security issues about Apple products to product-security@apple.com,
 * as described here:  https://www.apple.com/support/security/.  Non-security bugs and
 * enhancement requests can be made via https://bugreport.apple.com as described
 * here: https://developer.apple.com/bug-reporting/
 *
 * EA1350
 * 10/5/15
 */

#ifndef corecrypto_crypto_test_ec_h
#define corecrypto_crypto_test_ec_h

#include "cc_debug.h"
#include <corecrypto/ccrng.h>
#include <corecrypto/ccec.h>
#include <corecrypto/ccec_priv.h>
#include <corecrypto/ccrng.h>
#include <corecrypto/ccrng_ecfips_test.h>
#include <corecrypto/cc_macros.h>

#define cczp_bitlen(zp) ccn_bitlen(cczp_n(zp), cczp_prime(zp))

struct ccecdsa_vector {
    const struct ccdigest_info *di;	// digest
    ccec_const_cp_t (*curve)(void); // curve
    const char *priv_key; // private key
    const char *qx;       // public key
    const char *qy;       // public key
    int        hex_msg;    // ==1 if the message a hex string and need to be treated as hex rather than a string 
    const char *msg;	  // message for signature
    const char *k;		  // random used in ECDSA
    const char *r;		  // Signature r
    const char *s;		  // Signature s
};

struct ccecdh_vector {
    ccec_const_cp_t (*curve)(void); // curve
    const char *QCAVSx;     // CAVS public key Q, x coordinate
    const char *QCAVSy;     // CAVS public key Q, y coordinate
    const char *dIUT;		// CAVS private key
    const char *QIUTx;		// IUT public key Q, x coordinate
    const char *QIUTy;		// IUT public key Q, x coordinate
    const char *ZIUT;       // IUT shared secret
    const int status;
};

#define di_SHA1   &ccsha1_eay_di
#define di_SHA224 &ccsha224_ltc_di
#define di_SHA256 &ccsha256_ltc_di
#define di_SHA384 &ccsha384_ltc_di
#define di_SHA512 &ccsha512_ltc_di

int
ecdsa_known_answer_tests(void);

int
ecdsa_negative_tests(void);

int
ecdh_known_answer_tests(void);

int
ecdh_negative_tests(void);

int
ecwrapping_tests(void);

int
eckeygen_tests(void);

static inline // Get a full key from the raw scalar
int ccec_recover_full_key(ccec_const_cp_t cp,size_t length, uint8_t *data,ccec_full_ctx_t key)
{
    int result=-1;
    struct ccrng_ecfips_test_state rng;

    cc_require(ccrng_ecfips_test_init(&rng, length, data) == 0,errOut);
    cc_require(ccec_generate_key_internal_fips(cp, (struct ccrng_state *)&rng, key) == 0,errOut);

    // No problem
    result=0;
errOut:
    return result;
}

#endif

