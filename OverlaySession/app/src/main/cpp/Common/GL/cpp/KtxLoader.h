#pragma once
/*
-----------------------------------------------------------------------------
Copyright (c) 2020 - 2022 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
Copyright (c) 2000-2014 Torus Knot Software Ltd
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#include <cstdlib>
#include <GLES3/gl3.h>

#define KTX_IDENTIFIER_REF  { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A }
#define KTX_ENDIAN_REF      (0x04030201)
#define KTX_ENDIAN_REF_REV  (0x01020304)
#define KTX_HEADER_SIZE		(64)

#ifndef GL_TEXTURE_1D
    #define GL_TEXTURE_1D                    0x0DE0
#endif
#ifndef GL_TEXTURE_3D
    #define GL_TEXTURE_3D                    0x806F
#endif
#ifndef GL_TEXTURE_1D_ARRAY_EXT
    #define GL_TEXTURE_1D_ARRAY_EXT          0x8C18
    #define GL_TEXTURE_2D_ARRAY_EXT          0x8C1A
#endif

// Based on values from <stdint.h>
typedef char        int8;       // This should really be "int8_t"
typedef int16_t     int16;
typedef int32_t     int32;
typedef int64_t     int64;
typedef uint8_t     uint8;
typedef uint16_t    uint16;
typedef uint32_t    uint32;
typedef uint64_t    uint64;

typedef int32_t     bool32;

namespace QtiGL
{
    typedef enum 
    {
	    KTX_SUCCESS = 0,
        KTX_HEADER_ERROR,
        KTX_UNEXPECTED_END_OF_STREAM,
	    KTX_INVALID_VALUE,
	    KTX_UNSUPPORTED_TEXTURE_TYPE,
	    KTX_GL_ERROR,
    } TKTXErrorCode;

    //KTX header as defined by Khronos
    typedef struct
    {
	    uint8   identifier[12];
	    uint32  endianness;
	    uint32  glType;
	    uint32  glTypeSize;
	    uint32  glFormat;
	    uint32  glInternalFormat;
	    uint32  glBaseInternalFormat;
	    uint32  pixelWidth;
	    uint32  pixelHeight;
	    uint32  pixelDepth;
	    uint32  numberOfArrayElements;
	    uint32  numberOfFaces;
	    uint32  numberOfMipmapLevels;
	    uint32  bytesOfKeyValueData;
    } TKTXHeader;

    // This will cause compilation to fail if the struct size doesn't match 
    typedef int KTX_HEADER_SIZE_ASSERT [sizeof(TKTXHeader) == KTX_HEADER_SIZE];

    typedef struct 
    {
	    uint32  nTextureDimensions;
	    uint32  glTarget;
	    bool32  bCompressed;
    } TKTXTextureInfo;

    class KtxTexture
    {
        //constructors
    public:
        KtxTexture();
        ~KtxTexture();

        //methods
    public:
        TKTXErrorCode   LoadKtxFromBuffer(void* pBuffer, uint32 nBufferSize, GLuint* texture, GLenum* target, TKTXHeader* pOutHeader = 0, bool isProtected = false);

        uint32          GetDataSize() { return m_nDataSize; }
        uint8 *         GetData() { return m_pData; }

        uint32			GetWidht() { return m_nWidth; }
		uint32			GetHeight() { return m_nHeight; }

    private:
        TKTXErrorCode   ParseBuffer(void* pBuffer, unsigned int nBufferSize, GLuint* pTexture, GLenum* pTarget, TKTXHeader* pOutHeader = 0, bool isProtected = false);
        TKTXErrorCode   ParseHeader(TKTXHeader* pHeader, TKTXTextureInfo* pTextureInfo);
        uint32          StreamRead(void* pData, unsigned int nSize);
        uint32          StreamSkip(unsigned int nSize);

        //attributes
    private:
        uint8*  m_pData;
        uint32  m_nDataSize;
        uint8*  m_pStreamBuffer;
        uint32  m_nStreamBufferSize;
        uint32  m_nStreamBufferIndex;
        uint32  m_nWidth, m_nHeight;
    };

}
