//==========================================================================
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1999 - 2001  On2 Technologies Inc. All Rights Reserved.
//
//--------------------------------------------------------------------------
#if !defined(ON2DECRYPT_H)
#define ON2DECRYPT_H

//______________________________________________________________________________
//
//  On2Decrypt.h
//  test api for testing the encryption code in on2crypt.h
//--------------------------------------------------------------------------

#ifdef _USRDLL
#define DLLExport __declspec(dllexport)
#else
#define DLLExport
#endif

#define DLLCC __stdcall

#ifdef __cplusplus
extern "C"
{
#endif

typedef void* HOn2Decryptor;

DLLExport int DLLCC MakeDecryptor(unsigned char* pDRMX, int iDRMXLength, HOn2Decryptor* phOn2Decryptor);
//***************************************************************************************
// Name       : MakeDecryptor
// Description: set up an decryption session
// Inputs     : pDRMX -> information to be used by decryptor to set up decryption session
//              iDRMXLength -> number of bytes used in pDRMX
// Outputs    : phOn2Decryptor -> pointer to an decryption session
// Returns    : 0 = success 
//***************************************************************************************

DLLExport int DLLCC DecryptBytes(HOn2Decryptor hOn2Decryptor, unsigned char* pBufferIn, int iSizeIn, unsigned char* pBufferOut, int iSizeOutMax, int* piSizeOut);
//***************************************************************************************
// Name       : DecryptBytes
// Description: decrypts bytes in input buffer and stores them to the output buffer
// Inputs     : hOn2Decryptor -> handle of decryption sesion to use ( from make decryptor)
//            : pBufferIn -> buffer holding bytes to decrypt
//              iSizeIn -> number of bytes to decrypt of that buffer
// Outputs    : pBufferOut -> buffer for holding decrypted bytes
//              iSizeOutMax -> maximum number of bytes to write to pBufferOut
//              piSizeOut -> number of bytes actually written to pbufferout
// Returns    : 0 = success 
//***************************************************************************************

DLLExport int DLLCC DeleteDecryptor(HOn2Decryptor hOn2Decryptor);
//***************************************************************************************
// Name       : DeleteDecryptor
// Description: ends the decryption session and cleans up 
// Inputs     : hOn2Decryptor -> handle of decryption sesion to use ( from make decryptor)
// Outputs    : 
// Returns    : 0 = success 
//***************************************************************************************

typedef int (DLLCC *PFNMakeDecryptor)(unsigned char* pDRMX, int iDRMXLength, HOn2Decryptor* phOn2Decryptor);
typedef int (DLLCC *PFNDecryptBytes)(HOn2Decryptor hOn2Decryptor, unsigned char* pBufferIn, int iSizeIn, unsigned char* pBufferOut, int iSizeOutMax, int* piSizeOut);
typedef int (DLLCC *PFNDeleteDecryptor)(HOn2Decryptor hOn2Decryptor);

#ifdef __cplusplus
}  //  extern "C"
#endif

#endif  //  ON2DECRYPT_H
