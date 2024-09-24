#ifdef __cplusplus
extern "C" {
#endif
	void __cdecl __security_error_handler(
		int code,
		void *data)
	{
		_exit(3);
	}
#ifdef __cplusplus
}
#endif