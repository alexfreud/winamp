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
#if !defined(ON2CRYPT_H)
#define ON2CRYPT_H

//______________________________________________________________________________
//
//  On2Crypt.h
//  API to on2comp's encryption dll
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

typedef void* HOn2Encryptor;

DLLExport int DLLCC MakeEncryptor(unsigned char* pString, int iLength, HOn2Encryptor* phOn2Encryptor);
//***************************************************************************************
// Name       : MakeEncryptor
// Description: set up an encryption session
// Inputs     : pString -> information to be used by encryptor to set up encryption session
//              iLength -> number of bytes used in pString
// Outputs    : phOn2Encryptor -> pointer to an encryption session
// Returns    : 0 = success 
//***************************************************************************************

DLLExport int DLLCC GetEncryptionCode(char* pfccCode);  //  size of szCode must be >= 4
//***************************************************************************************
// Name       : GetEncryptionCode
// Description: get the 4 character code to use as a reference for this encryption dll
// Inputs     : 
// Outputs    : pfccCode 4 character code
// Returns    : 0 = success 
//***************************************************************************************


DLLExport int DLLCC GetDRMXLength(HOn2Encryptor hOn2Encryptor, int* piLength);
//***************************************************************************************
// Name       : GetDRMXLength
// Description: calculates the length of decryption chunk to be produced
// Inputs     : hOn2Encryptor -> handle of encryption sesion to use ( from make encryptor)
// Outputs    : piLength -> filled with length of extra information 
// Returns    : 0 = success 
//***************************************************************************************

DLLExport int DLLCC GenerateDRMX(HOn2Encryptor hOn2Encryptor, unsigned char* pBuffer);
//***************************************************************************************
// Name       : GenerateDRMX
// Description: generates a decryption chunk
// Inputs     : hOn2Encryptor -> handle of encryption sesion to use ( from make encryptor)
// Outputs    : pBuffer is filled with information necessary to decrypt the file we are 
//              encrypting
// Returns    : 0 = success 
//***************************************************************************************

DLLExport int DLLCC EncryptedSize(HOn2Encryptor hOn2Encryptor, int iSizeIn, int* piSizeOut);
//***************************************************************************************
// Name       : EncryptedSize
// Description: returns size that an encrypted chunk will be given a size in
// Inputs     : hOn2Encryptor -> handle of encryption sesion to use ( from make encryptor)
//            : iSizeIn -> size of input data 
// Outputs    : piSizeOut -> size of output data
// Returns    : 0 = success 
//***************************************************************************************

DLLExport int DLLCC EncryptBytes(HOn2Encryptor hOn2Encryptor, unsigned char* pBufferIn, int iSizeIn, unsigned char* pBufferOut, int iSizeOutMax, int* piSizeOut);
//***************************************************************************************
// Name       : EncryptBytes
// Description: encrypts bytes in input buffer and stores them to the output buffer
// Inputs     : hOn2Encryptor -> handle of encryption sesion to use ( from make encryptor)
//            : pBufferIn -> buffer holding bytes to encrypt
//              iSizeIn -> number of bytes to encrypt of that buffer
// Outputs    : pBufferOut -> buffer for holding encrypted bytes
//              iSizeOutMax -> maximum number of bytes to write to pBufferOut
//              piSizeOut -> number of bytes actually written to pbufferout
// Returns    : 0 = success 
//***************************************************************************************


DLLExport int DLLCC EncryptorError(char* szError, int nErrorMax);
//***************************************************************************************
// Name       : EncryptorError
// Description: gets a string description of the last error
// Inputs     : szError -> pointer to string
//              nErrorMax -> the largest number of bytes to fill in in szerror
// Outputs    : 
// Returns    : 0 = success 
//***************************************************************************************


DLLExport int DLLCC DeleteEncryptor(HOn2Encryptor hOn2Encryptor);
//***************************************************************************************
// Name       : DeleteEncryptor
// Description: ends the encryption session and cleans up 
// Inputs     : hOn2Encryptor -> handle of encryption sesion to use ( from make encryptor)
// Outputs    : 
// Returns    : 0 = success 
//***************************************************************************************

typedef int (DLLCC *PFNMakeEncryptor)(unsigned char* pString, int iLength, HOn2Encryptor* phOn2Encryptor);
typedef int (DLLCC *PFNGetEncryptionCode)(char* pfccCode);  //  size of szCode must be >= 4
typedef int (DLLCC *PFNGetDRMXLength)(HOn2Encryptor hOn2Encryptor, int* piLength);
typedef int (DLLCC *PFNGenerateDRMX)(HOn2Encryptor hOn2Encryptor, unsigned char* pBuffer);
typedef int (DLLCC *PFNEncryptedSize)(HOn2Encryptor hOn2Encryptor, int iSizeIn, int* piSizeOut);
typedef int (DLLCC *PFNEncryptBytes)(HOn2Encryptor hOn2Encryptor, unsigned char* pBufferIn, int iSizeIn, unsigned char* pBufferOut, int iSizeOutMax, int* piSizeOut);
typedef int (DLLCC *PFNEncryptorError)(char* szError, int nErrorMax);
typedef int (DLLCC *PFNDeleteEncryptor)(HOn2Encryptor hOn2Encryptor);

#ifdef __cplusplus
}  //  extern "C"
#endif

#endif  //  ON2CRYPT_H
