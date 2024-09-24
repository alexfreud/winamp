//////////////////////////////////////////////////////////////////////
/*
 Smtp.cpp: implementation of the CSmtp and CSmtpMessage classes

 Written by Robert Simpson (robert@blackcastlesoft.com)
 Created 11/1/2000
 Version 1.7 -- Last Modified 06/18/2001

 1.7 - Modified the code that gets the GMT offset and the code that
       parses the date/time as per Noa Karsten's suggestions on 
       codeguru.
     - Added an FD_ZERO(&set) to the last part of SendCmd(), since
       I use the set twice and only zero it out once.  Submitted by
       Marc Allen.
     - Removed the requirement that a message have a body and/or an
       attachment.  This allows for sending messages with only a
       subject line.  Submitted by Marc Allen.
 1.6 - Apparently older versions of the STL do not have the clear()
       method for basic_string's.  I modified the code to use 
       erase() instead.
     - Added #include <atlbase.h> to the smtp.h file, which will
       allow any app to use these classes without problems.
 1.5 - Guess I should have checked EncodeQuotedPrintable() as well,
       since it did the same thing BreakMessage() did in adding an
       extranneous CRLF to the end of any text it processed.  Fixed.
 1.4 - BreakMesage() added an extranneous CRLF to the end of any
       text it processed, which is now fixed.  Certainly not a big
       deal, but it caused text attachments to not be 100% identical
       to the original.
 1.3 - Added a new class, CSmtpMimePart, to which the CSmtpAttachment
       and CSmtpMessageBody classes inherit.  This was done for
       future expansion.  CSmtpMimePart has a new ContentId string
       value for optionally assigning a unique content ID value to
       body parts and attachments.  This was done to support the
       multipart/related enhancement
     - Support for multipart/related messages, which can be used
       for sending html messages with embedded images.
     - Modifed CSmtpMessage, adding a new MimeType member variable
       so the user can specify a certain type of MIME format to use
       when coding the message.
     - Fixed a bug where multipart/alternative messages with multiple
       message bodies were not properly processed when attachments
       were also included in the message.
     - Some small optimizations during the CSmtpMessage::Parse routine

 1.2 - Vastly improved the time it takes to break a message,
       which was dog slow with large attachments.  My bad.
     - Added another overridable, SmtpProgress() which is
       called during the CSmtp::SendCmd() function when there
       is a large quantity of data being sent over the wire.
       Added CMD_BLOCK_SIZE to support the above new feature
     - Added support for UNICODE
     - Added the CSmtpAttachment class for better control and
       expandability for attachments.
     - Added alternative implementations for CSmtp::SendMessage
       which make it easier to send simple messages via a single
       function call.
     - Added a constructor to CSmtpAddress for assigning default
       values during initialization.
     - Added a #pragma comment(lib,"wsock32.lib") to the smtp.h
       file so existing projects don't have to have their linker
       options modified.

 1.1 - Rearranged the headers so they are written out as:
       From,To,Subject,Date,MimeVersion,
       followed by all remaining headers
     - Modified the class to support multipart/alternative with
       multiple message bodies.

 Note that CSimpleMap does not sort the key values, and CSmtp
 takes advantage of this by writing the headers out in the reverse
 order of how they will be parsed before being sent to the SMTP
 server.  If you modify the code to use std::map or any other map
 class, the headers may be alphabetized by key, which may cause
 some mail clients to show the headers in the body of the message
 or cause other undesirable results when viewing the message.
*/
//////////////////////////////////////////////////////////////////////

#include "Smtp.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction for CSmtpMessageBody
//////////////////////////////////////////////////////////////////////
CSmtpMessageBody::CSmtpMessageBody(LPCTSTR pszBody, LPCTSTR pszEncoding, LPCTSTR pszCharset, EncodingEnum encode)
{

  // Set the default message encoding method
  // To transfer html messages, make Encoding = _T("text/html")
  if (pszEncoding) Encoding = pszEncoding;
  if (pszCharset)  Charset  = pszCharset;
  if (pszBody)     Data     = pszBody;
  TransferEncoding          = encode;
}

const CSmtpMessageBody& CSmtpMessageBody::operator=(LPCTSTR pszBody)
{
  Data = pszBody;
  return *this;
}

const CSmtpMessageBody& CSmtpMessageBody::operator=(const String& strBody)
{
  Data = strBody;
  return *this;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction for CSmtpAttachment
//////////////////////////////////////////////////////////////////////
CSmtpAttachment::CSmtpAttachment(LPCTSTR pszFilename, LPCTSTR pszAltName, BOOL bIsInline, LPCTSTR pszEncoding, LPCTSTR pszCharset, EncodingEnum encode)
{
  if (pszFilename) FileName = pszFilename;
  if (pszAltName)  AltName  = pszAltName;
  if (pszEncoding) Encoding = pszEncoding;
  if (pszCharset)  Charset  = pszCharset;
  TransferEncoding = encode;
  Inline = bIsInline;
}

const CSmtpAttachment& CSmtpAttachment::operator=(LPCTSTR pszFilename)
{
  FileName = pszFilename;
  return *this;
}

const CSmtpAttachment& CSmtpAttachment::operator=(const String& strFilename)
{
  FileName = strFilename;
  return *this;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction for CSmtpAddress
//////////////////////////////////////////////////////////////////////
CSmtpAddress::CSmtpAddress(LPCTSTR pszAddress, LPCTSTR pszName)
{
  if (pszAddress) Address = pszAddress;
  if (pszName) Name = pszName;
}

const CSmtpAddress& CSmtpAddress::operator=(LPCTSTR pszAddress)
{
  Address = pszAddress;
  return *this;
}

const CSmtpAddress& CSmtpAddress::operator=(const String& strAddress)
{
  Address = strAddress;
  return *this;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction for CSmtpMessage
//////////////////////////////////////////////////////////////////////
CSmtpMessage::CSmtpMessage()
{
  TIME_ZONE_INFORMATION tzi;
  DWORD dwRet;
  long Offset; 

  // Get local time and timezone offset 
  GetLocalTime(&Timestamp); 
  GMTOffset = 0; 
  dwRet = GetTimeZoneInformation(&tzi); 
  Offset = tzi.Bias; 
  if (dwRet == TIME_ZONE_ID_STANDARD) Offset += tzi.StandardBias; 
  if (dwRet == TIME_ZONE_ID_DAYLIGHT) Offset += tzi.DaylightBias; 
  GMTOffset = -((Offset / 60) * 100 + (Offset % 60)); 

  MimeType = mimeGuess;
}

// Write all the headers to the e-mail message.
// This is done just before sending it, when we're sure the user wants it to go out.
void CSmtpMessage::CommitHeaders()
{
  TCHAR szTime[64] = {0};
  TCHAR szDate[64] = {0};
  TCHAR szOut[1024] = {0};
  String strHeader;
  String strValue;
  int n;

  // Assign a few standard headers to the message
  strHeader = _T("X-Priority");
  strValue  = _T("3 (Normal)");
  // Only add the key if it doesn't exist already in the headers map
  if (Headers.FindKey(strHeader) == -1) Headers.Add(strHeader,strValue);
  
  strHeader = _T("X-MSMail-Priority");
  strValue  = _T("Normal");
  if (Headers.FindKey(strHeader) == -1) Headers.Add(strHeader,strValue);
  
  strHeader = _T("X-Mailer");
  strValue  = _T("ATL CSmtp Class Mailer by Robert Simpson (robert@blackcastlesoft.com)");
  if (Headers.FindKey(strHeader) == -1) Headers.Add(strHeader,strValue);
  
  strHeader = _T("Importance");
  strValue  = _T("Normal");
  if (Headers.FindKey(strHeader) == -1) Headers.Add(strHeader,strValue);

  // Get the time/date stamp and GMT offset for the Date header.
  GetDateFormat(MAKELCID(LANG_ENGLISH, SORT_DEFAULT),0,&Timestamp,_T("ddd, d MMM yyyy"),szDate,64); 
  GetTimeFormat(MAKELCID(LANG_ENGLISH, SORT_DEFAULT),0,&Timestamp,_T("H:mm:ss"),szTime,64); 
  
  // Add the date/time stamp to the message headers 
  wsprintf(szOut,_T("%s %s %c%4.4d"),szDate,szTime,(GMTOffset>0)?'+':'-',GMTOffset); 
  
  strHeader = _T("Date");
  strValue = szOut;
  Headers.Remove(strHeader);
  Headers.Add(strHeader,strValue);

  // Write out the subject header
  strHeader = _T("Subject");
  strValue = Subject;
  Headers.Remove(strHeader);
  Headers.Add(strHeader,strValue);

  // Write out the TO header
  strValue.erase();
  strHeader = _T("To");
  if (Recipient.Name.length())
  {
    wsprintf(szOut,_T("\"%s\" "),Recipient.Name.c_str());
    strValue += szOut;
  }
  if (Recipient.Address.length())
  {
    wsprintf(szOut,_T("<%s>"),Recipient.Address.c_str());
    strValue += szOut;
  }
  // Write out all the CC'd names
  for (n = 0;n < CC.GetSize();n++)
  {
    if (strValue.length()) strValue += _T(",\r\n\t");
    if (CC[n].Name.length())
    {
      wsprintf(szOut,_T("\"%s\" "),CC[n].Name.c_str());
      strValue += szOut;
    }
    wsprintf(szOut,_T("<%s>"),CC[n].Address.c_str());
    strValue += szOut;
  }
  Headers.Remove(strHeader);
  Headers.Add(strHeader,strValue);

  // Write out the FROM header
  strValue.erase();
  strHeader = _T("From");
  if (Sender.Name.length())
  {
    wsprintf(szOut,_T("\"%s\" "),Sender.Name.c_str());
    strValue += szOut;
  }
  wsprintf(szOut,_T("<%s>"),Sender.Address.c_str());
  strValue += szOut;
  Headers.Remove(strHeader);
  Headers.Add(strHeader,strValue);
}

// Parse a message into a single string
void CSmtpMessage::Parse(String& strDest)
{
  String strHeader;
  String strValue;
  String strTemp;
  String strBoundary;  
  String strInnerBoundary;
  TCHAR szOut[1024];
  int n;

  strDest.erase();
  // Get a count of the sections to see if this will be a multipart message
  n  = Message.GetSize();
  n += Attachments.GetSize();

  // Remove this header in case the message is being reused
  strHeader = _T("Content-Type");
  Headers.Remove(strHeader);

  // If we have more than one section, then this is a multipart MIME message
  if (n > 1)
  {
    wsprintf(szOut,_T("CSmtpMsgPart123X456_000_%8.8X"),GetTickCount());
    strBoundary = szOut;

    lstrcpy(szOut,_T("multipart/"));

    if (MimeType == mimeGuess)
    {
      if (Attachments.GetSize() == 0) MimeType = mimeAlternative;
      else MimeType = mimeMixed;
    }
    switch(MimeType)
    {
    case mimeAlternative:
      lstrcat(szOut,_T("alternative"));
      break;
    case mimeMixed:
      lstrcat(szOut,_T("mixed"));
      break;
    case mimeRelated:
      lstrcat(szOut,_T("related"));
      break;
    }
    lstrcat(szOut,_T(";\r\n\tboundary=\""));
    lstrcat(szOut,strBoundary.c_str());
    lstrcat(szOut,_T("\""));

    strValue = szOut;
    Headers.Add(strHeader,strValue);
  }

  strHeader = _T("MIME-Version");
  strValue  = MIME_VERSION;
  Headers.Remove(strHeader);
  Headers.Add(strHeader,strValue);

  // Remove any message ID in the header and replace it with this message ID, if it exists
  strHeader = _T("Message-ID");
  Headers.Remove(strHeader);
  if (MessageId.length())
  {
    wsprintf(szOut,_T("<%s>"),MessageId.c_str());
    strValue = szOut;
    Headers.Add(strHeader,strValue);
  }

  // Finalize the message headers
  CommitHeaders();

  // Write out all the message headers -- done backwards on purpose!
  for (n = Headers.GetSize();n > 0;n--)
  {
    wsprintf(szOut,_T("%s: %s\r\n"),Headers.GetKeyAt(n-1).c_str(),Headers.GetValueAt(n-1).c_str());
    strDest += szOut;
  }
  if (strBoundary.length())
  {
    wsprintf(szOut,_T("\r\n%s\r\n"),MULTIPART_MESSAGE);
    strDest += szOut;
  }

  // If we have attachments and multiple message bodies, create a new multipart section
  // This is done so we can display our multipart/alternative section separate from the
  // main multipart/mixed environment, and support both attachments and alternative bodies.
  if (Attachments.GetSize() && Message.GetSize() > 1 && strBoundary.length())
  {
    wsprintf(szOut,_T("CSmtpMsgPart123X456_001_%8.8X"),GetTickCount());
    strInnerBoundary = szOut;
    
    wsprintf(szOut,_T("\r\n--%s\r\nContent-Type: multipart/alternative;\r\n\tboundary=\"%s\"\r\n"),strBoundary.c_str(),strInnerBoundary.c_str());
    strDest += szOut;
  }

  for (n = 0;n < Message.GetSize();n++)
  {
    // If we're multipart, then write the boundary line
    if (strBoundary.length() || strInnerBoundary.length())
    {
      strDest += _T("\r\n--");
      // If we have an inner boundary, write that one.  Otherwise write the outer one
      if (strInnerBoundary.length()) strDest += strInnerBoundary;
      else strDest += strBoundary;
      strDest += _T("\r\n");
    }
    strValue.erase();
    strDest += _T("Content-Type: ");
    strDest += Message[n].Encoding;
    // Include the character set if the message is text
    if (_tcsnicmp(Message[n].Encoding.c_str(),_T("text/"),5) == 0)
    {
      wsprintf(szOut,_T(";\r\n\tcharset=\"%s\""),Message[n].Charset.c_str());
      strDest += szOut;
    }
    strDest += _T("\r\n");

    // Encode the message
    strValue = Message[n].Data;
    EncodeMessage(Message[n].TransferEncoding,strValue,strTemp);

    // Write out the encoding method used and the encoded message
    strDest += _T("Content-Transfer-Encoding: ");    
    strDest += strTemp;
    
    // If the message body part has a content ID, write it out
    if (Message[n].ContentId.length())
    {
      wsprintf(szOut,_T("\r\nContent-ID: <%s>"),Message[n].ContentId.c_str());
      strDest += szOut;
    }
    strDest += _T("\r\n\r\n");
    strDest += strValue;
  }

  // If we have multiple message bodies, write out the trailing inner end sequence
  if (strInnerBoundary.length())
  {
    wsprintf(szOut,_T("\r\n--%s--\r\n"),strInnerBoundary.c_str());
    strDest += szOut;
  }

  // Process any attachments
  for (n = 0;n < Attachments.GetSize();n++)
  {
    DWORD dwBytes = 0;
    CRegKey cKey;
    TCHAR szFilename[MAX_PATH] = {0};

    // Get the filename of the attachment
    strValue = Attachments[n].FileName;

    // Open the file
    lstrcpy(szFilename,strValue.c_str());
    HANDLE hFile = CreateFile(szFilename,GENERIC_READ,0,NULL,OPEN_EXISTING,0,NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
      // Get the size of the file, allocate the memory and read the contents.
      DWORD dwSize = GetFileSize(hFile,NULL);
      LPBYTE pData = (LPBYTE)malloc(dwSize + 1);
      ZeroMemory(pData,dwSize+1);

      if (ReadFile(hFile,pData,dwSize,&dwBytes,NULL))
      {
        // Write out our boundary marker
        if (strBoundary.length())
        {
          wsprintf(szOut,_T("\r\n--%s\r\n"),strBoundary.c_str());
          strDest += szOut;
        }

        // If no alternate name is supplied, strip the path to get the base filename
        LPTSTR pszFile;
        if (!Attachments[n].AltName.length())
        {
          // Strip the path from the filename
          pszFile = _tcsrchr(szFilename,'\\');
          if (!pszFile) pszFile = szFilename;
          else pszFile ++;
        }
        else pszFile = (LPTSTR)Attachments[n].AltName.c_str();

        // Set the content type for the attachment.
        TCHAR szType[MAX_PATH] = {0};
        lstrcpy(szType,_T("application/octet-stream"));

        // Check the registry for a content type that overrides the above default
        LPTSTR pszExt = _tcschr(pszFile,'.');
        if (pszExt)
        {
          if (!cKey.Open(HKEY_CLASSES_ROOT,pszExt,KEY_READ))
          {
            DWORD dwSize = MAX_PATH;
			cKey.QueryValue(_T("Content Type"), NULL, szType, &dwSize);
            cKey.Close();
          }
        }

        // If the attachment has a specific encoding method, use it instead
        if (Attachments[n].Encoding.length())
          lstrcpy(szType,Attachments[n].Encoding.c_str());

        // Write out the content type and attachment types to the message
        wsprintf(szOut,_T("Content-Type: %s"),szType);
        strDest += szOut;
        // If the content type is text, write the charset
        if (_tcsnicmp(szType,_T("text/"),5) == 0)
        {
          wsprintf(szOut,_T(";\r\n\tcharset=\"%s\""),Attachments[n].Charset.c_str());
          strDest += szOut;
        }
        wsprintf(szOut,_T(";\r\n\tname=\"%s\"\r\n"),pszFile);
        strDest += szOut;

        // Encode the attachment
        EncodeMessage(Attachments[n].TransferEncoding,strValue,strTemp,pData,dwSize);

        // Write out the transfer encoding method
        wsprintf(szOut,_T("Content-Transfer-Encoding: %s\r\n"),strTemp.c_str());
        strDest += szOut;

        // Write out the attachment's disposition
        strDest += _T("Content-Disposition: ");
        
        if (Attachments[n].Inline) strDest += _T("inline");
        else strDest += _T("attachment");
        
        strDest += _T(";\r\n\tfilename=\"");
        strDest += pszFile;

        // If the attachment has a content ID, write it out
        if (Attachments[n].ContentId.length())
        {
          wsprintf(szOut,_T("\r\nContent-ID: <%s>"),Attachments[n].ContentId.c_str());
          strDest += szOut;
        }
        strDest += _T("\r\n\r\n");

        // Write out the encoded attachment
        strDest += strValue;
        strTemp.erase();
        strValue.erase();
      }
      // Close the file and clear the temp buffer
      CloseHandle(hFile);
      free(pData);
    }
  }

  // If we are multipart, write out the trailing end sequence
  if (strBoundary.length())
  {
    wsprintf(szOut,_T("\r\n--%s--\r\n"),strBoundary.c_str());
    strDest += szOut;
  }
}

// Parses text into quoted-printable lines.
// See RFC 1521 for full details on how this works.
void CSmtpMessage::EncodeQuotedPrintable(String& strDest, String& strSrc)
{
  String strTemp;
  String strTemp2;
  LPTSTR pszTok1;
  LPTSTR pszTok2;
  TCHAR szSub[16];
  TCHAR ch;
  int n;

  strDest.erase();
  if (!strSrc.length()) return;

  // Change = signs and non-printable characters to =XX
  pszTok1 = (LPTSTR)strSrc.c_str();
  pszTok2 = pszTok1;
  do
  {
    if (*pszTok2 == '=' || *pszTok2 > 126 || 
      (*pszTok2 < 32 && (*pszTok2 != '\r' && *pszTok2 != '\n' && *pszTok2 != '\t')))
    {
      ch = *pszTok2;
      *pszTok2 = 0;
      strTemp += pszTok1;
      *pszTok2 = ch;
      wsprintf(szSub,_T("=%2.2X"),(BYTE)*pszTok2);
      strTemp += szSub;
      pszTok1 = pszTok2 + 1;
    }
    pszTok2 ++;
  } while (pszTok2 && *pszTok2);

  // Append anything left after the search
  if (_tcslen(pszTok1)) strTemp += pszTok1;

  pszTok1 = (LPTSTR)strTemp.c_str();
  while (pszTok1)
  {
    pszTok2 = _tcschr(pszTok1,'\r');
    if (pszTok2) *pszTok2 = 0;
    while (1)
    {
      if (_tcslen(pszTok1) > 76)
      {
        n = 75; // Breaking at the 75th character
        if (pszTok1[n-1] == '=') n -= 1; // If the last character is an =, don't break the line there
        else if (pszTok1[n-2] == '=') n -= 2; // If we're breaking in the middle of a = sequence, back up!
        
        // Append the first section of the line to the total string
        ch = pszTok1[n];
        pszTok1[n] = 0;
        strDest += pszTok1;
        pszTok1[n] = ch;
        strDest += _T("=\r\n");
        pszTok1 += n;
      }
      else // Line is less than or equal to 76 characters
      {
        n = (int)_tcslen(pszTok1); // If we have some trailing data, process it.
        if (n)
        {
          if (pszTok1[n-1] == ' ' || pszTok1[n-1] == '\t') // Last character is a space or tab
          {
            wsprintf(szSub,_T("=%2.2X"),(BYTE)pszTok1[n-1]);
            // Replace the last character with an =XX sequence
            pszTok1[n-1] = 0;
            strTemp2 = pszTok1;
            strTemp2 += szSub;
            // Since the string may now be larger than 76 characters, we have to reprocess the line
            pszTok1 = (LPTSTR)strTemp2.c_str();
          }
          else // Last character is not a space or tab
          {
            strDest += pszTok1;
            if (pszTok2) strDest += _T("\r\n");
            break; // Exit the loop which processes this line, and move to the next line
          }
        }
        else
        {
          if (pszTok2) strDest += _T("\r\n");
          break; // Move to the next line
        }
      }
    }
    if (pszTok2)
    {
      *pszTok2 = '\r';
      pszTok2 ++;
      if (*pszTok2 == '\n') pszTok2 ++;
    }
    pszTok1 = pszTok2;
  }
}

// Breaks a message's lines into a maximum of 76 characters
// Does some semi-intelligent wordwrapping to ensure the text is broken properly.
// If a line contains no break characters, it is forcibly truncated at the 76th char
void CSmtpMessage::BreakMessage(String& strDest, String& strSrc, int nLength)
{
  String strTemp = strSrc;
  LPTSTR pszTok1;
  LPTSTR pszTok2;
  LPTSTR pszBreak;
  LPTSTR pszBreaks = _T(" -;.,?!");
  TCHAR ch;
  int nLen;

  strDest.erase();
  if (!strSrc.length()) return;

  nLen = (int)strTemp.length();
  nLen += (nLen / 60) * 2;

  strDest.reserve(nLen);

  // Process each line one at a time
  pszTok1 = (LPTSTR)strTemp.c_str();
  while (pszTok1)
  {
    pszTok2 = _tcschr(pszTok1,'\r');
    if (pszTok2) *pszTok2 = 0;

    BOOL bNoBreaks = (!_tcspbrk(pszTok1,pszBreaks));
    nLen = (int)_tcslen(pszTok1);
    while (nLen > nLength)
    {
      // Start at the 76th character, and move backwards until we hit a break character
      pszBreak = &pszTok1[nLength - 1];
      
      // If there are no break characters in the string, skip the backward search for them!
      if (!bNoBreaks)
      {
        while (!_tcschr(pszBreaks,*pszBreak) && pszBreak > pszTok1)
          pszBreak--;
      }
      pszBreak ++;
      ch = *pszBreak;
      *pszBreak = 0;
      strDest += pszTok1;
      
      strDest += _T("\r\n");
      *pszBreak = ch;
    
      nLen -= (int)(pszBreak - pszTok1);
      // Advance the search to the next segment of text after the break
      pszTok1 = pszBreak;
    }
    strDest += pszTok1;
    if (pszTok2)
    {
      strDest += _T("\r\n");
      *pszTok2 = '\r';
      pszTok2 ++;
      if (*pszTok2 == '\n') pszTok2 ++;
    }
    pszTok1 = pszTok2;
  }
}

// Makes the message into a 7bit stream
void CSmtpMessage::Make7Bit(String& strDest, String& strSrc)
{
  LPTSTR pszTok;

  strDest = strSrc;

  pszTok = (LPTSTR)strDest.c_str();
  do
  {
    // Replace any characters above 126 with a ? character
    if (*pszTok > 126 || *pszTok < 0)
      *pszTok = '?';
    pszTok ++;
  } while (pszTok && *pszTok);
}

// Encodes a message or binary stream into a properly-formatted message
// Takes care of breaking the message into 76-byte lines of text, encoding to
// Base64, quoted-printable and etc.
void CSmtpMessage::EncodeMessage(EncodingEnum code, String& strMsg, String& strMethod, LPBYTE pByte, DWORD dwSize)
{
  String strTemp;
  LPTSTR pszTok1;
  LPTSTR pszTok2;
  LPSTR pszBuffer = NULL;
  DWORD dwStart = GetTickCount();

  if (!pByte)
  {
    pszBuffer = (LPSTR)malloc(strMsg.length() + 1);
    _T2A(pszBuffer,strMsg.c_str());
    pByte = (LPBYTE)pszBuffer;
    dwSize = (DWORD)strMsg.length();
  }

  // Guess the encoding scheme if we have to
  if (code == encodeGuess) code = GuessEncoding(pByte, dwSize);

  switch(code)
  {
  case encodeQuotedPrintable:
    strMethod = _T("quoted-printable");

    pszTok1 = (LPTSTR)malloc((dwSize+1) * sizeof(TCHAR));
    _A2T(pszTok1,(LPSTR)pByte);
    strMsg = pszTok1;
    free(pszTok1);
    
    EncodeQuotedPrintable(strTemp, strMsg);
    break;
  case encodeBase64:
    strMethod = _T("base64");
    {
      CBase64 cvt;      
      cvt.Encode(pByte, dwSize);
      LPSTR pszTemp = (LPSTR)cvt.EncodedMessage();
      pszTok1 = (LPTSTR)malloc((lstrlenA(pszTemp)+1) * sizeof(TCHAR));
      _A2T(pszTok1,pszTemp);
    }
    strMsg = pszTok1;
    free(pszTok1);

    BreakMessage(strTemp, strMsg);
    break;
  case encode7Bit:
    strMethod = _T("7bit");

    pszTok1 = (LPTSTR)malloc((dwSize+1) * sizeof(TCHAR));
    _A2T(pszTok1,(LPSTR)pByte);
    strMsg = pszTok1;
    free(pszTok1);

    Make7Bit(strTemp, strMsg);
    strMsg = strTemp;
    BreakMessage(strTemp, strMsg);
    break;
  case encode8Bit:
    strMethod = _T("8bit");

    pszTok1 = (LPTSTR)malloc((dwSize+1) * sizeof(TCHAR));
    _A2T(pszTok1,(LPSTR)pByte);
    strMsg = pszTok1;
    free(pszTok1);

    BreakMessage(strTemp, strMsg);
    break;
  }

  if (pszBuffer) free(pszBuffer);

  strMsg.erase();

  // Parse the message text, replacing CRLF. sequences with CRLF.. sequences
  pszTok1 = (LPTSTR)strTemp.c_str();
  do
  {
    pszTok2 = _tcsstr(pszTok1,_T("\r\n."));
    if (pszTok2)
    {
      *pszTok2 = 0;
      strMsg += pszTok1;
      *pszTok2 = '\r';
      strMsg += _T("\r\n..");
      pszTok1 = pszTok2 + 3;
    }
  } while (pszTok2);  
  strMsg += pszTok1;

  TCHAR szOut[MAX_PATH] = {0};
  wsprintf(szOut,_T("Encoding took %dms\n"),GetTickCount() - dwStart);
  OutputDebugString(szOut);
}

// Makes a best-guess of the proper encoding to use for this stream of bytes
// It does this by counting the # of lines, the # of 8bit bytes and the number
// of 7bit bytes.  It also records the line and the count of lines over
// 76 characters.
// If the stream is 90% or higher 7bit, it uses a text encoding method.  If the stream
// is all at or under 76 characters, it uses 7bit or 8bit, depending on the content.
// If the lines are longer than 76 characters, use quoted printable.
// If the stream is under 90% 7bit characters, use base64 encoding.
EncodingEnum CSmtpMessage::GuessEncoding(LPBYTE pByte, DWORD dwLen)
{
  int n7Bit = 0;
  int n8Bit = 0;
  int nLineStart = 0;
  int nLinesOver76 = 0;
  int nLines = 0;
  DWORD n;

  // Count the content type, byte by byte
  for (n = 0;n < dwLen; n++)
  {
    if (pByte[n] > 126 || (pByte[n] < 32 && pByte[n] != '\t' && pByte[n] != '\r' && pByte[n] != '\n'))
      n8Bit ++;
    else n7Bit ++;

    // New line?  If so, record the line size
    if (pByte[n] == '\r')
    {
      nLines ++;
      nLineStart = (n - nLineStart) - 1;
      if (nLineStart > 76) nLinesOver76 ++;
      nLineStart = n + 1;      
    }
  }
  // Determine if it is mostly 7bit data
  if ((n7Bit * 100) / dwLen > 89)
  {
    // At least 90% text, so use a text-base encoding scheme
    if (!nLinesOver76)
    {
      if (!n8Bit) return encode7Bit;
      else return encode8Bit;
    }
    else return encodeQuotedPrintable;
  }
  return encodeBase64;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction for CSmtp
//////////////////////////////////////////////////////////////////////
CSmtp::CSmtp()
{
  LPSERVENT pEnt;
  
  m_bExtensions = TRUE;       // Use ESMTP if possible
  m_dwCmdTimeout = 30;        // Default to 30 second timeout
  m_hSocket = INVALID_SOCKET;
  m_bConnected = m_bUsingExtensions = FALSE;

  // Try and get the SMTP service entry by name
  pEnt = getservbyname("SMTP","tcp");
  if (pEnt) m_wSmtpPort = pEnt->s_port;
  else m_wSmtpPort = htons(25);

}

CSmtp::~CSmtp()
{
  // Make sure any open connections are shut down
  Close();
}

// Connects to a SMTP server.  Returns TRUE if successfully connected, or FALSE otherwise.
BOOL CSmtp::Connect(LPTSTR pszServer)
{
  SOCKADDR_IN addr;
  int nRet;
  CHAR szHost[MAX_PATH] = {0};

  _T2A(szHost,pszServer);
 // Shut down any active connection
  Close();
// test 
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD( 2, 1 );
	WSAStartup( wVersionRequested, &wsaData );
// end test
  // Resolve the hostname
  addr.sin_family = AF_INET;
  addr.sin_port = m_wSmtpPort;
  addr.sin_addr.s_addr = inet_addr(szHost);
   
  if (addr.sin_addr.s_addr == INADDR_NONE)
  {
	
    LPHOSTENT pHost = gethostbyname(szHost);
    if (!pHost) 
	{
		return FALSE;
	}

    addr.sin_addr.s_addr = *(LPDWORD)pHost->h_addr;
  }


  
  // Create a socket
  m_hSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  if (m_hSocket == INVALID_SOCKET) return FALSE;

  // Connect to the host
  if (connect(m_hSocket,(LPSOCKADDR)&addr,sizeof(addr)) == SOCKET_ERROR)
  {
	 Close();
    return FALSE;
  }

  // Get the initial response string
  nRet = SendCmd(NULL);
  if (nRet != 220)
  {
    RaiseError(nRet);
    Close();
    return FALSE;
  }

  // Send a HELLO message to the SMTP server
  if (SendHello())
  {
    Close();
    return FALSE;
  }

  return TRUE;
}

// Closes any active SMTP sessions and shuts down the socket.
void CSmtp::Close()
{
  if (m_hSocket != INVALID_SOCKET)
  {
    // If we're connected to a server, tell them we're quitting
    if (m_bConnected) SendQuitCmd();
    // Shutdown and close the socket
    shutdown(m_hSocket,2);
    closesocket(m_hSocket);
  }
  m_hSocket = INVALID_SOCKET;
}

// Send a command to the SMTP server and wait for a response
int CSmtp::SendCmd(LPTSTR pszCmd)
{
  USES_CONVERSION;
  FD_SET set;
  TIMEVAL tv;
  int nRet = 0;
  DWORD dwTick;
  CHAR szResult[CMD_RESPONSE_SIZE] = {0};
  LPSTR pszPos;
  LPSTR pszTok;
  BOOL bReportProgress = FALSE;
  LPSTR pszBuff;
  
  ZeroMemory(szResult,CMD_RESPONSE_SIZE);
  FD_ZERO(&set);

  // If we have a command to send, then send it.
  if (pszCmd)
  {
    pszBuff = (LPSTR)malloc(lstrlen(pszCmd)+1);
    _T2A(pszBuff,pszCmd);
    
    // Make sure the input buffer is clear before sending
    nRet = 1;
    while (nRet > 0)
    {
      FD_SET(m_hSocket,&set);
      tv.tv_sec = 0;
      tv.tv_usec = 0;
      nRet = select(1,&set,NULL,NULL,&tv);
	  if (nRet == 1) nRet = recv(m_hSocket,szResult,CMD_RESPONSE_SIZE,0);
    }
    DWORD dwPosition = 0;
    DWORD dwLen = lstrlen(pszCmd);
    if (dwLen > CMD_BLOCK_SIZE) bReportProgress = TRUE;

    while (dwLen != dwPosition)
    {
      DWORD dwMax = min(CMD_BLOCK_SIZE,dwLen - dwPosition);
      nRet = send(m_hSocket,&pszBuff[dwPosition],dwMax,0);
      if (nRet == SOCKET_ERROR)
      {
        free(pszBuff);
        return nRet;
      }
      dwPosition += dwMax;
      if (bReportProgress)
      {
        if (!SmtpProgress(pszBuff,dwPosition,dwLen))
        {
          free(pszBuff);
          return -1;
        }
      }
    }
    // Wait for the CMD to finish being sent
    FD_ZERO(&set);
    FD_SET(m_hSocket,&set);
    nRet = select(1,NULL,&set,NULL,NULL);
    free(pszBuff);
  }

  // Prepare to receive a response
  ZeroMemory(szResult,CMD_RESPONSE_SIZE);
  pszPos = szResult;
  // Wait for the specified timeout for a full response string
  dwTick = GetTickCount();
   while (GetTickCount() - dwTick < (m_dwCmdTimeout * 1000))
  {
    FD_SET(m_hSocket,&set);
    
    tv.tv_sec = m_dwCmdTimeout - ((GetTickCount() - dwTick) / 1000);
    tv.tv_usec = 0;

    // Check the socket for readability
    nRet = select(1,&set,NULL,NULL,&tv);
    if (nRet == SOCKET_ERROR) break;

    // If the socket has data, read it.
    if (nRet == 1)
    {
      nRet = recv(m_hSocket,pszPos,CMD_RESPONSE_SIZE - (int)(pszPos - szResult),0);
      // Treats a graceful shutdown as an error
      if (nRet == 0) nRet = SOCKET_ERROR;
      if (nRet == SOCKET_ERROR) break;
      
      // Add the data to the total response string & check for a LF
      pszPos += nRet;
      pszTok = strrchr(szResult,'\n');
      if (pszTok)
      {
        // Truncate CRLF combination and exit our wait loop
        pszTok --;
        pszTok[0] = 0;
        break;
      }
    }
  }
  // Assign the response string
  m_strResult = A2CT(szResult);

  // Evaluate the numeric response code
  if (nRet && nRet != SOCKET_ERROR)
  {
    szResult[3] = 0;
    nRet = atoi(szResult);
    SmtpCommandResponse(pszCmd, nRet, (LPTSTR)m_strResult.c_str());
  }
  else nRet = -1;

  return nRet;
}

// Placeholder function -- overridable
// This function is called when the SMTP server gives us an unexpected error
// The <nError> value is the SMTP server's numeric error response, and the <pszErr>
// is the descriptive error text
//
// <pszErr> may be NULL if the server failed to respond before the timeout!
// <nError> will be -1 if a catastrophic failure occurred.
//
// Return 0, or nError.  The return value is currently ignored.
int CSmtp::SmtpError(int /*nError*/, LPTSTR pszErr)
{
#ifdef _DEBUG
  if (pszErr)
  {
    OutputDebugString(_T("SmtpError: "));
    OutputDebugString(pszErr);
    OutputDebugString(_T("\n"));
  }
#endif
  return 0;
}

// Placeholder function -- overridable
// Currently the only warning condition that this class is designed for is
// an authentication failure.  In that case, <nWarning> will be 535,
// which is the RFC error for authentication failure.  If authentication
// fails, you can override this function to prompt the user for a new
// username and password.  Change the <m_strUser> and <m_strPass> member
// variables and return TRUE to retry authentication.
//
// <pszWarning> may be NULL if the server did not respond in time!
//
// Return FALSE to abort authentication, or TRUE to retry.
int CSmtp::SmtpWarning(int /*nWarning*/, LPTSTR pszWarning)
{
#ifdef _DEBUG
  if (pszWarning)
  {
    OutputDebugString(_T("SmtpWarning: "));
    OutputDebugString(pszWarning);
    OutputDebugString(_T("\n"));
  }
#endif
  return 0;
}

// Placeholder function -- overridable
// This is an informational callback only, and provides a means to inform
// the caller as the SMTP session progresses.
// ALWAYS check for NULL values on <pszCmd> and <pszResponse> before performing
// any actions!
// <nResponse> will be -1 if a catastrophic failure occurred, but that will
// be raised in the SmtpError() event later on during processing.
void CSmtp::SmtpCommandResponse(LPTSTR pszCmd, int /*nResponse*/, LPTSTR pszResponse)
{
#ifdef _DEBUG
  if (pszCmd)
  {
    TCHAR szOut[MAX_PATH+1] = {0};
    OutputDebugString(_T("SmtpCommand : "));
    while (lstrlen(pszCmd) > MAX_PATH)
    {
      lstrcpyn(szOut,pszCmd,MAX_PATH+1);
      OutputDebugString(szOut);
      Sleep(100);
      pszCmd += MAX_PATH;
    }
    OutputDebugString(pszCmd);
  }
  OutputDebugString(_T("SmtpResponse: "));
  OutputDebugString(pszResponse);
  OutputDebugString(_T("\n"));
#endif
}

// Placeholder function -- overridable
// This is a progress callback to indicate that data is being sent over the wire
// and that the operation may take some time.
// Return TRUE to continue sending, or FALSE to abort the transfer
BOOL CSmtp::SmtpProgress(LPSTR /*pszBuffer*/, DWORD /*dwBytesSent*/, DWORD /*dwBytesTotal*/)
{
  return TRUE; // Continue sending the data
}

// Raises a SmtpError() condition
int CSmtp::RaiseError(int nError)
{
  // If the error code is -1, something catastrophic happened
  // so we're effectively not connected to any SMTP server.
  if (nError == -1) m_bConnected = FALSE;
  return SmtpError(nError, (LPTSTR)m_strResult.c_str());
}

// Warnings are recoverable errors that we may be able to continue working with
int CSmtp::RaiseWarning(int nWarning)
{
  return SmtpWarning(nWarning, (LPTSTR)m_strResult.c_str());
}

// E-Mail's a message
// Returns 0 if successful, -1 if an internal error occurred, or a positive
// error value if the SMTP server gave an error or failure response.
int CSmtp::SendMessage(CSmtpMessage &msg)
{
  int nRet;
  int n;
//  int nRecipients = 0;
  int nRecipientCount = 0;

  // Check if we have a sender
  if (!msg.Sender.Address.length()) return -1;
  
  // Check if we have recipients
  if (!msg.Recipient.Address.length() && !msg.CC.GetSize()) return -1;

  // Check if we have a message body or attachments
  // *** Commented out to remove the requirement that a message have a body or attachments
  //  if (!msg.Message.GetSize() && !msg.Attachments.GetSize()) return -1;

  // Send the sender's address
  nRet = SendFrom((LPTSTR)msg.Sender.Address.c_str());
  if (nRet) return nRet;
  // If we have a recipient, send it
  nRecipientCount = 0; // Count of recipients
  if (msg.Recipient.Address.length())
  {
    nRet = SendTo((LPTSTR)msg.Recipient.Address.c_str());
    if (!nRet) nRecipientCount ++;
  }

  // If we have any CC's, send those.
  for (n = 0;n < msg.CC.GetSize();n++)
  {
    nRet = SendTo((LPTSTR)msg.CC[n].Address.c_str());
    if (!nRet) nRecipientCount ++;
  }

  // If we have any bcc's, send those.
  for (n = 0;n < msg.BCC.GetSize();n++)
  {
    nRet = SendTo((LPTSTR)msg.BCC[n].Address.c_str());
    if (!nRet) nRecipientCount ++;
  }
  // If we failed on all recipients, we must abort.
  if (!nRecipientCount)
    RaiseError(nRet);
  else
    nRet = SendData(msg);

  return nRet;
}

// Simplified way to send a message.
// <pvAttachments> can be either an LPTSTR containing NULL terminated strings, in which
// case <dwAttachmentCount> should be zero, or <pvAttachments> can be an LPTSTR * 
// containing an array of LPTSTR's, in which case <dwAttachmentCount> should equal the
// number of strings in the array.
int CSmtp::SendMessage(CSmtpAddress &addrFrom, CSmtpAddress &addrTo, LPCTSTR pszSubject, LPTSTR pszMessage, LPVOID pvAttachments, DWORD dwAttachmentCount)
{
  CSmtpMessage message;
  CSmtpMessageBody body;
  CSmtpAttachment attach;

  body = pszMessage;

  message.Sender = addrFrom;
  message.Recipient = addrTo;
  message.Message.Add(body);
  message.Subject = pszSubject;

  // If the attachment count is zero, but the pvAttachments variable is not NULL,
  // assume that the ppvAttachments variable is a string value containing NULL terminated
  // strings.  A double NULL ends the list.
  // Example: LPTSTR pszAttachments = "foo.exe\0bar.zip\0autoexec.bat\0\0";
  if (!dwAttachmentCount && pvAttachments)
  {
    LPTSTR pszAttachments = (LPTSTR)pvAttachments;
    while (lstrlen(pszAttachments))
    {
      attach.FileName = pszAttachments;
      message.Attachments.Add(attach);
      pszAttachments = &pszAttachments[lstrlen(pszAttachments)];
    }
  }

  // dwAttachmentCount is not zero, so assume pvAttachments is an array of LPTSTR's
  // Example: LPTSTR *ppszAttachments = {"foo.exe","bar.exe","autoexec.bat"};
  if (pvAttachments && dwAttachmentCount)
  {
    LPTSTR *ppszAttachments = (LPTSTR *)pvAttachments;    
    while (dwAttachmentCount-- && ppszAttachments)
    {
      attach.FileName = ppszAttachments[dwAttachmentCount];
      message.Attachments.Add(attach);
    }
  }
  return SendMessage(message);
}

// Yet an even simpler method for sending a message
// <pszAddrFrom> and <pszAddrTo> should be e-mail addresses with no decorations
// Example:  "foo@bar.com"
// <pvAttachments> and <dwAttachmentCount> are described above in the alternative
// version of this function
int CSmtp::SendMessage(LPTSTR pszAddrFrom, LPTSTR pszAddrTo, LPTSTR pszSubject, LPTSTR pszMessage, LPVOID pvAttachments, DWORD dwAttachmentCount)
{
  CSmtpAddress addrFrom(pszAddrFrom);
  CSmtpAddress addrTo(pszAddrTo);

  return SendMessage(addrFrom,addrTo,pszSubject,pszMessage,pvAttachments,dwAttachmentCount);
}

// Tell the SMTP server we're quitting
// Returns 0 if successful, or a positive
// error value if the SMTP server gave an error or failure response.
int CSmtp::SendQuitCmd()
{
  int nRet;

  if (!m_bConnected) return 0;

  nRet = SendCmd(_T("QUIT\r\n"));
  if (nRet != 221) RaiseError(nRet);

  m_bConnected = FALSE;

  return (nRet == 221) ? 0:nRet;
}

// Initiate a conversation with the SMTP server
// Returns 0 if successful, or a positive
// error value if the SMTP server gave an error or failure response.
int CSmtp::SendHello()
{
  int nRet = 0;
  TCHAR szName[64] = {0};
  TCHAR szMsg[MAX_PATH] = {0};
  DWORD dwSize = 64;

  GetComputerName(szName,&dwSize);

  // First try a EHLO if we're using ESMTP
  wsprintf(szMsg,_T("EHLO %s\r\n"),szName);
  if (m_bExtensions) nRet = SendCmd(szMsg);

  // If we got a 250 response, we're using ESMTP, otherwise revert to regular SMTP
  if (nRet != 250)
  {
    m_bUsingExtensions = FALSE;
    szMsg[0] = 'H';
    szMsg[1] = 'E';
    nRet = SendCmd(szMsg);
  }
  else m_bUsingExtensions = TRUE;

  // Raise any unexpected responses
  if (nRet != 250)
  {
    RaiseError(nRet);
    return nRet;
  }

  // We're connected!
  m_bConnected = TRUE;

  // Send authentication if we have any.
  // We don't fail just because authentication failed, however.
  if (m_bUsingExtensions) SendAuthentication();

  return 0;
}

// Requests authentication for the session if the server supports it,
// and attempts to submit the user's credentials.
// Returns 0 if successful, or a positive
// error value if the SMTP server gave an error or failure response.
int CSmtp::SendAuthentication()
{
  USES_CONVERSION;
  int nRet = 0;
  CBase64 cvt;
  LPCSTR pszTemp;
  TCHAR szMsg[MAX_PATH] = {0};
  CHAR szAuthType[MAX_PATH] = {0};

  // This is an authentication loop, we can authenticate multiple times in case of failure.
  while(1)
  {
    // If we don't have a username, skip authentication
    if (!m_strUser.length()) return 0;
    
    // Make the authentication request
    nRet = SendCmd(_T("AUTH LOGIN\r\n"));
    // If it was rejected, we have to abort.
    if (nRet != 334)
    {
      RaiseWarning(nRet);
      return nRet;
    }
    
    // Authentication has 2 stages for username and password.
    // It is possible if the authentication fails here that we can
    // resubmit proper credentials.
    while (1)
    {
      // Decode the authentication string being requested
      _T2A(szAuthType,&(m_strResult.c_str())[4]);

      cvt.Decode(szAuthType);
      pszTemp = cvt.DecodedMessage();
      
      if (!lstrcmpiA(pszTemp,"Username:"))
        cvt.Encode(T2CA(m_strUser.c_str()));
      else if (!lstrcmpiA(pszTemp,"Password:"))
        cvt.Encode(T2CA(m_strPass.c_str()));
      else break;
      
      wsprintf(szMsg,_T("%s\r\n"),A2CT(cvt.EncodedMessage()));
      nRet = SendCmd(szMsg);
      
      // If we got a failed authentication request, raise a warning.
      // this gives the owner a chance to change the username and password.
      if (nRet == 535)
      {
        // Return FALSE to fail, or TRUE to retry
        nRet = RaiseWarning(nRet);
        if (!nRet)
        {
          // Reset the error back to 535.  It's now an error rather than a warning
          nRet = 535;
          break;
        }
      }
      // Break on any response other than 334, which indicates a request for more information
      if (nRet != 334) break;
    }
    // Break if we're not retrying a failed authentication
    if (nRet != TRUE) break;
  }
  // Raise an error if we failed to authenticate
  if (nRet != 235) RaiseError(nRet);

  return (nRet == 235) ? 0:nRet;
}

// Send a MAIL FROM command to the server
// Returns 0 if successful, or a positive
// error value if the SMTP server gave an error or failure response.
int CSmtp::SendFrom(LPTSTR pszFrom)
{
  int nRet = 0;
  TCHAR szMsg[MAX_PATH] = {0};

  wsprintf(szMsg,_T("MAIL FROM: <%s>\r\n"),pszFrom);
  
  while (1)
  {
    nRet = SendCmd(szMsg);
    // Send authentication if required, and retry the command
    if (nRet == 530) nRet = SendAuthentication();
    else break;
  }
  // Raise an error if we failed
  if (nRet != 250) RaiseError(nRet);
  return (nRet == 250) ? 0:nRet;
}

// Send a RCPT TO command to the server
// Returns 0 if successful, or a positive
// error value if the SMTP server gave an error or failure response.
int CSmtp::SendTo(LPTSTR pszTo)
{
  int nRet;
  TCHAR szMsg[MAX_PATH] = {0};
	
  wsprintf(szMsg,_T("RCPT TO: <%s>\r\n"),pszTo);
   nRet = SendCmd(szMsg);
  if (nRet != 250 && nRet != 251) RaiseWarning(nRet);
  return (nRet == 250 || nRet == 251) ? 0:nRet;
}

// Send the body of an e-mail message to the server
// Returns 0 if successful, or a positive
// error value if the SMTP server gave an error or failure response.
int CSmtp::SendData(CSmtpMessage &msg)
{
  int nRet;
  String strMsg;

  // Send the DATA command.  We need a 354 to proceed
  nRet = SendCmd(_T("DATA\r\n"));
  if (nRet != 354)
  {
    RaiseError(nRet);
    return nRet;
  }

  // Parse the body of the email message
  msg.Parse(strMsg);
  strMsg += _T("\r\n.\r\n");

  // Send the body and expect a 250 OK reply.
  nRet = SendCmd((LPTSTR)strMsg.c_str());
  if (nRet != 250) RaiseError(nRet);
  
  return (nRet == 250) ? 0:nRet;
}
