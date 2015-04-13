
/**************************************************************************
 *                                                                        *
 * This code is developed by Eugene Kuznetsov.  This software is an       *
 * implementation of a part of one or more MPEG-4 Video tools as          *
 * specified in ISO/IEC 14496-2 standard.  Those intending to use this    *
 * software module in hardware or software products are advised that its  *
 * use may infringe existing patents or copyrights, and any such use      *
 * would be at such party's own risk.  The original developer of this     *
 * software module and his/her company, and subsequent editors and their  *
 * companies (including Project Mayo), will have no liability for use of  *
 * this software or modifications or derivatives thereof.                 *
 *                                                                        *
 * Project Mayo gives users of the Codec a license to this software       *
 * module or modifications thereof for use in hardware or software        *
 * products claiming conformance to the MPEG-4 Video Standard as          *
 * described in the Open DivX license.                                    *
 *                                                                        *
 * The complete Open DivX license can be found at                         *
 * http://www.projectmayo.com/opendivx/license.php .                      *
 *                                                                        *
 **************************************************************************/

/**************************************************************************
 *
 *  fast_dct.c, C implementation of fast integer forward DCT algorithm
 *
 *  Copyright (C) 2001  Project Mayo
 *
 *  Eugene Kuznetsov
 *
 *  DivX Advance Research Center <darc@projectmayo.com>
 *
 **************************************************************************/
#include "dct.h"

const float m_c0 = 0.7071068f;			 // 2896
const float m_c1 = 0.4903926f;			 // 2009
const float m_c2 = 0.4619398f;			 // 1892
const float m_c3 = 0.4157348f;			 // 1703
const float m_c4 = 0.3535534f;			 // 1448
const float m_c5 = 0.2777851f;			 // 1138
const float m_c6 = 0.1913417f;			 // 784
const float m_c7 = 0.0975452f;			 // 400


void oneDimensionalFwdDCT_fast_up(int16_t *sInput) 
{
    
    int32_t sBuffer2[4];
    int j, j1;

    
    {
	j1 = 7 - j;
	
	
    
    
    
    
    
    
    
    
	(sBuffer2[2] * (int16_t) (m_c6 * 4096) +
	 sBuffer2[3] * (int16_t) (m_c2 * 4096)) >> 8;
    
	(sBuffer2[3] * (int16_t) (m_c6 * 4096) -
	 sBuffer2[2] * (int16_t) (m_c2 * 4096)) >> 8;
    
    
    
    
    
    
    
    
    
	(sBuffer[4] * (int16_t) (m_c7 * 4096) +
	 sBuffer[7] * (int16_t) (m_c1 * 4096)) >> 10;
    
	(sBuffer[5] * (int16_t) (m_c3 * 4096) +
	 sBuffer[6] * (int16_t) (m_c5 * 4096)) >> 10;
    
	(sBuffer[7] * (int16_t) (m_c7 * 4096) -
	 sBuffer[4] * (int16_t) (m_c1 * 4096)) >> 10;
    
	(sBuffer[6] * (int16_t) (m_c3 * 4096) -
	 sBuffer[5] * (int16_t) (m_c5 * 4096)) >> 10;

void oneDimensionalFwdDCT_fast_down(int16_t *sInput) 
{
    
    int32_t sBuffer2[4];
    int j, j1;

    
    {
	j1 = 7 - j;
	
	
    
    
    
    
    

    sInput[8 * 0] =
	((sBuffer2[0] + sBuffer2[1]) * (int16_t) (m_c4 * 4096)) >> 16;
    
	((sBuffer2[0] - sBuffer2[1]) * (int16_t) (m_c4 * 4096)) >> 16;
    
	(sBuffer2[2] * (int16_t) (m_c6 * 4096) +
	 sBuffer2[3] * (int16_t) (m_c2 * 4096)) >> 16;
    
	(sBuffer2[3] * (int16_t) (m_c6 * 4096) -
	 sBuffer2[2] * (int16_t) (m_c2 * 4096)) >> 16;
    
    
    
    
    
    
    
    
    
	(sBuffer[4] * (int16_t) (m_c7 * 4096) +
	 sBuffer[7] * (int16_t) (m_c1 * 4096)) >> 18;
    
	(sBuffer[5] * (int16_t) (m_c3 * 4096) +
	 sBuffer[6] * (int16_t) (m_c5 * 4096)) >> 18;
    
	(sBuffer[7] * (int16_t) (m_c7 * 4096) -
	 sBuffer[4] * (int16_t) (m_c1 * 4096)) >> 18;
    
	(sBuffer[6] * (int16_t) (m_c3 * 4096) -
	 sBuffer[5] * (int16_t) (m_c5 * 4096)) >> 18;

void fdct_enc_fast(int16_t *block) 
{
    int32_t i;

    for (i = 0; i < 8; i++)
	oneDimensionalFwdDCT_fast_up(block + 8 * i);
    
	oneDimensionalFwdDCT_fast_down(block + i);
