/*
 * Copyright (c) 2010,2011,2014,2015,2016,2018 Apple Inc. All rights reserved.
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

#include <corecrypto/ccec_priv.h>

int ccec_affinify(ccec_const_cp_t cp, ccec_affine_point_t r, ccec_const_projective_point_t s)
{
    int status=-1;
    if (ccn_is_zero(ccec_cp_n(cp), ccec_const_point_z(s, cp))) {
		return -1; // Point at infinity
    }

#if CCEC_DEBUG
    ccec_plprint(cp, "ccec_affinify input", s);
#endif

    cc_unit lambda[ccec_cp_n(cp)];
    cc_unit t[ccec_cp_n(cp)];

    status=cczp_inv_field(ccec_cp_zp(cp), lambda, ccec_const_point_z(s, cp));             // lambda = sz^-1
    cczp_sqr(ccec_cp_zp(cp), t, lambda);                                   // t = lambda^2
    cczp_mul(ccec_cp_zp(cp), ccec_point_x(r, cp), t, ccec_const_point_x(s, cp)); // rx = t * sx
    cczp_mul(ccec_cp_zp(cp), t, t, lambda);                                // t = lambda^3
    cczp_mul(ccec_cp_zp(cp), ccec_point_y(r, cp), t, ccec_const_point_y(s, cp)); // ry = t * sy

    // Back from Montgomery
    if (CCEC_ZP_IS_MONTGOMERY(cp)) {
        cczp_convert_from_montgomery(ccec_cp_zp(cp),ccec_point_x(r, cp), ccec_point_x(r, cp));
        cczp_convert_from_montgomery(ccec_cp_zp(cp),ccec_point_y(r, cp), ccec_point_y(r, cp));

    }

#if CCEC_DEBUG
    ccec_alprint(cp, "ccec_affinify output", r);
#endif

	return status;
}

int
ccec_affinify_x_only(ccec_const_cp_t cp, cc_unit* sx, ccec_const_projective_point_t s, int secure) {
    int status=-1;
    if (ccn_is_zero(ccec_cp_n(cp), ccec_const_point_z(s, cp))) {
        // assert(false);
		return -1;
    }

    // Allows "in place" operation => the result can be set in any of the point coordinate.
    cc_unit lambda[ccec_cp_n(cp)];
    cc_unit t[ccec_cp_n(cp)];

    cczp_sqr(ccec_cp_zp(cp), t, ccec_const_point_z(s, cp));                    // sz^2
    if (secure==0) {
        if (CCEC_ZP_IS_MONTGOMERY(cp)) {
            cczp_convert_from_montgomery(ccec_cp_zp(cp),t, t);
        }
        status=cczp_inv(ccec_cp_zp(cp),       lambda, t);   // lambda = sz^-2
    } else {
        status=cczp_inv_field(ccec_cp_zp(cp), lambda, t);   // lambda = sz^-2
        if (CCEC_ZP_IS_MONTGOMERY(cp)) {
            cczp_convert_from_montgomery(ccec_cp_zp(cp),lambda, lambda);
        }
    }
    cczp_mul(ccec_cp_zp(cp), sx, ccec_const_point_x(s, cp),lambda);   // rx = sx * lambda^2


	return status;
}
