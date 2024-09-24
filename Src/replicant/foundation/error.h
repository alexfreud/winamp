#pragma once
#ifdef __cplusplus
extern "C" {
#endif
enum
{
	NErr_Success = 0,
	NErr_True = 0,
	NErr_Error = 1,			// generic error
	NErr_OutOfMemory = 2,
	NErr_FileNotFound = 3,
	NErr_NullPointer = 4,
	NErr_NotImplemented = 5,// I'm a lazy programmer
	NErr_EndOfFile = 6,		// also used for "end of enumeration"
	NErr_NeedMoreData = 7,	// input buffer was too small to provide useful output.  Use this instead of NErr_ReadTruncated when it is expected that the caller can call the function again with more data
	NErr_False = 8,			// returned from a bool-like function to indicate "false" as opposed to "i had an error while figuring it out"
	NErr_FailedCreate = 9,	// Object could not be created
	NErr_Closed = 10,
	NErr_TryAgain = 11,		// often used in round-robin "isMine()" loops to indicate that you'll take it if no one else wants it first. can also be used for device I/O when the device is busy
	NErr_NoDevice = 12,
	NErr_UnsupportedFormat = 13,
	NErr_Unknown = 14,		// NOT meant to be "some unknown error".  Usually returned when some passed in enumeration or keyword was an unknown, unexpected or unsupported value
	NErr_Insufficient = 15,	// output buffer was too small
	NErr_Empty = 16,
	NErr_LostSynchronization = 17,
	NErr_TimedOut = 19,
	NErr_BadParameter = 20,
	NErr_NoAction = 21,		// Returned when no action performed, for example when initializing but something has already been initialized
	
	// Test case related values
	NErr_TestFailed = 18,		// Result on a test failure, typically used by unit tests and other test cases.
	NErr_TestPassed = 0,		// Result on a test success, typically used by unit tests and other test cases.
	NErr_TestError = 1,			// Result on a test error, typically used by unit tests and other test cases.
	NErr_TestNotComplete = 22,	// Result on a premature stop, typically used by unit tests and other test cases.  This is to protect against a scenerio where a test case is in a 'PASSED' state up to a certain point but cannot finish execution due to data missing, environmental issues, etc.
	
	NErr_Malformed = 23,		// some peice of data was malformed or had unexpected value (typically returned by parsers)
	NErr_WrongFormat = 24,		// data was understood but is indicating a different format than expected.  e.g. an layer 2 header being encountered by a layer 3 parser
	NErr_Reserved = 25,			// typically returned when a parser encounters data with a reserved flag set to true
	NErr_Changed = 26,			// something changed.  e.g. samplerate changed mid-stream
	NErr_Interrupted = 27,
	NErr_ConnectionFailed = 28, // generic "can't connect" error
	NErr_DNS = 29,				// no DNS entry for the host

	/* the follow map NError codes to HTTP error codes. but they can be used for other purposes, too */
	NErr_BadRequest = 30,			// aka HTTP 400
	NErr_Unauthorized = 31,			// aka HTTP 401
	NErr_Forbidden = 32,			// aka HTTP 403
	NErr_NotFound = 33,				// aka HTTP 404, differentiated from NErr_FileNotFound
	NErr_BadMethod = 34,			// aka HTTP 405
	NErr_NotAcceptable = 35,		// aka HTTP 406
	NErr_ProxyAuthenticationRequired = 36, // aka HTTP 407
	NErr_RequestTimeout = 37,		// aka HTTP 408
	NErr_Conflict = 38,				// aka HTTP 409
	NErr_Gone = 39,					// aka HTTP 410
	NErr_InternalServerError = 40,	// aka HTTP 500
	NErr_ServiceUnavailable = 41,	// aka HTTP 503

	NErr_Exception = 42,				// Underlying library returns an error or exception that wasn't understood
	NErr_Underrun = 43,					// Asynchronous thread not supplying data fast enough, buffer has insufficient data
	NErr_NoMatchingImplementation = 44,	// Returned when a function that delegates functionality to a matching component is unable to find one e.g. api_playlistmanager::Load
	NErr_IntegerOverflow = 45,
	NErr_IncompatibleVersion = 46,		// returned e.g. when a "size" field in a passed struct was larger than expected, or when a flag was set that's not understood
	NErr_Disabled = 47,
	NErr_ParameterOutOfRange = 48,		// Used to signify that a paramater was passed in that is out of bounds for valid values.
	NErr_OSNotSupported = 49, // something is not supported on this OS (e.g. WASAPI audio on Windows XP)
	NErr_UnsupportedInterface = 50, // used for some APIs (notably svc_decode).  It means that you can provide the requested functionality for the provided data (e.g. filename) but don't support the requested interface 
	NErr_DirectPointer = 51,
	NErr_ReadOnly = 52,
	NErr_EndOfEnumeration = NErr_EndOfFile, // we'll eventually make this its own number
	NErr_ReadTruncated = 54, // somewhat similar to NErr_NeedMoreData.  Meant to be used e.g. when a file or input buffer is shorter than expected.  Use this instead of NErr_NeedMoreData when the caller cannot provide more data.
	NErr_Aborted = 55,
	NErr_BadReturnValue = 56, // e.g. a callback function returns an unexpected value
	NErr_MaximumDepth = 57,
	NErr_Stopped = 58,
	NErr_LengthRequired = 59, // aka HTTP 411
	NErr_PreconditionFailed = 60, // aka HTTP 411
	NErr_TooLarge = 61, // aka HTTP 413
};

typedef int NError;
typedef int ns_error_t; // TODO: eventually make this the name of the enum

#ifdef __cplusplus
}
#endif

// be careful. only use this if your stack variables self-destruct
#define NSERROR_RETURN_ON_FAILURE(x) { int local_ret = x; if (local_ret != NErr_Success) return local_ret; }
