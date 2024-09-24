#ifndef NULLSOFT_UTILITY_RENTRYH
#define NULLSOFT_UTILITY_RENTRYH

#include <string>


namespace Nullsoft
{
	namespace Utility
	{


		class ReEntryGuard
		{
		public:
			ReEntryGuard() : entered(false)
			{}


			bool FunctionCall(std::string funcName = "Unknown")
			{

				if (entered)
				{
					char errorMsg[256];
					sprintf(errorMsg, "%s branched to %s", firstFunc.c_str(), funcName.c_str());
					::MessageBox(NULL, errorMsg, "Class ReEntry error", MB_OK);
					return false;
				}
				else
				{
					firstFunc = funcName;
					entered = true;
					return true;
				}
	

			}
			void LeaveFunction()
			{
				entered = false;
				firstFunc = "";
			}
		private:
			bool entered;
			std::string firstFunc;

		};
		class ReEntry
		{
		public:
			ReEntry(ReEntryGuard &_entry, std::string funcName = "Unknown") : entry(&_entry)
			{
				entry->FunctionCall(funcName);
			}

			~ReEntry()
			{
				entry->LeaveFunction();
			}

		private:
			ReEntryGuard *entry;

		};
	}
}
#endif
