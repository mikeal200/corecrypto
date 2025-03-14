/*
 * Copyright (c) 2016,2018 Apple Inc. All rights reserved.
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

#include <corecrypto/ccrsa_priv.h>
#include <corecrypto/ccrsa.h>
#include <corecrypto/ccdigest.h>
#include <corecrypto/ccsha1.h>
#include <corecrypto/ccrng_sequence.h>
#include <corecrypto/ccsha2.h>
#include "crypto_test_rsapkcs1v15.h"
#include "testmore.h"

struct ccrsa_verify_vector {
    const struct ccdigest_info *di;
    unsigned long modlen; // in bits
    const void *mod;
    cc_unit exp;
    unsigned long msglen; // in bytes
    const void *msg;
    unsigned long siglen; // in bytes - should be modlen/8
    const void *sig;
    bool valid; // expected result
};

static int ccrsa_test_verify_pkcs1v15_vector(const struct ccrsa_verify_vector *v)
{
    bool ok;
    int rc;
    const struct ccdigest_info *di = v->di;
    const cc_size n = ccn_nof(v->modlen);
    const size_t s = ccn_sizeof(v->modlen);
    unsigned char H[di->output_size];

    cc_unit exponent[n];
    cc_unit modulus[n];
    ccrsa_pub_ctx_decl(ccn_sizeof(v->modlen), key);
    ccrsa_ctx_n(key) = n;
    ccn_seti(n, exponent, v->exp);
    ccn_read_uint(n, modulus, s, v->mod);

    ccrsa_init_pub(key, modulus, exponent);
    ccdigest(di, v->msglen, v->msg, H);
    rc=ccrsa_verify_pkcs1v15(key, di->oid, di->output_size, H, v->siglen, v->sig, &ok);

    return ((rc==0)
        && ((ok && v->valid) || (!ok && !v->valid)))?0:1;
}


/* Nist CAVP vectors specifies the hash as strings - those are matching hashes implementations */
/* We picked the implementations that are on all platform, it does not matter since we are not testing the hash here */
#define di_SHA1 &ccsha1_eay_di
#define di_SHA224 &ccsha224_ltc_di
#define di_SHA256 &ccsha256_ltc_di
#define di_SHA384 &ccsha384_ltc_di
#define di_SHA512 &ccsha512_ltc_di

/* Nist CAVP vectors for verify specify the result as F (failed) or P (passed)
 those translate as true or false */

#define P true
#define F false

const struct ccrsa_verify_vector verify_vectors_pkcs1v15[]=
{
#include "../test_vectors/SigVer15.inc"
};


int
test_verify_pkcs1v15_known_answer_test(void)
{
    uint32_t i;
    uint32_t nb_test_passed,nb_test;
    nb_test_passed=0;
    nb_test=0;
    // Run only tests for supported hash algos
    for(i=0;i<sizeof(verify_vectors_pkcs1v15)/sizeof(struct ccrsa_verify_vector);i++)
    {
        if (verify_vectors_pkcs1v15[i].di!=NULL)
        {
            // 1 bit is set to one when the test passed
            nb_test++;
            if (ccrsa_test_verify_pkcs1v15_vector(&verify_vectors_pkcs1v15[i])==0)
            {
                nb_test_passed+=1;
            }
        }
    }

    if ((nb_test_passed==nb_test) && (nb_test>0))
    {
        return 0;
    }
    return -1;
}





