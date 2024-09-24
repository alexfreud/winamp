#include "./main.h"
#include "./resource.h"
#include <strsafe.h>

wchar_t*  GetUnitStatusText(wchar_t *buffer, unsigned int cchBuffer, DWORD sense, DWORD asc, DWORD ascq)
{
	DWORD strcode = IDS_UNKNOWN;
	switch(sense)
	{
		case 0x00:
			switch(asc)
			{
				case 0x00:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_000000; break;   
						case 0x01: strcode = IDS_DRIVEERRORCODE_000001; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_000002; break;
						case 0x03: strcode = IDS_DRIVEERRORCODE_000003; break;
						case 0x04: strcode = IDS_DRIVEERRORCODE_000004; break;
						case 0x05: strcode = IDS_DRIVEERRORCODE_000005; break;
						case 0x11: strcode = IDS_DRIVEERRORCODE_000011; break;
						case 0x12: strcode = IDS_DRIVEERRORCODE_000012; break;
						case 0x13: strcode = IDS_DRIVEERRORCODE_000013; break;
						case 0x14: strcode = IDS_DRIVEERRORCODE_000014; break;
						case 0x15: strcode = IDS_DRIVEERRORCODE_000015; break;
						case 0x16: strcode = IDS_DRIVEERRORCODE_000016; break;
					}
                    break;
			}
			break;
		case 0x01:
			switch(asc)
			{
				case 0x0B:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_010B00; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_010B01; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_010B02; break;
					}
                    break;
				case 0x0C:
					switch(ascq)
					{
						case 0x0A: strcode = IDS_DRIVEERRORCODE_010C0A; break;
					}
                    break;
				case 0x17:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_011700; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_011701; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_011702; break;
						case 0x03: strcode = IDS_DRIVEERRORCODE_011703; break;
						case 0x04: strcode = IDS_DRIVEERRORCODE_011704; break;
						case 0x05: strcode = IDS_DRIVEERRORCODE_011705; break;
						case 0x06: strcode = IDS_DRIVEERRORCODE_011706; break;
						case 0x07: strcode = IDS_DRIVEERRORCODE_011707; break;
						case 0x08: strcode = IDS_DRIVEERRORCODE_011708; break;
						case 0x09: strcode = IDS_DRIVEERRORCODE_011709; break;
					}
                    break;
				case 0x18:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_011800; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_011801; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_011802; break;
						case 0x03: strcode = IDS_DRIVEERRORCODE_011803; break;
						case 0x04: strcode = IDS_DRIVEERRORCODE_011804; break;
						case 0x05: strcode = IDS_DRIVEERRORCODE_011805; break;
						case 0x06: strcode = IDS_DRIVEERRORCODE_011806; break;
						case 0x07: strcode = IDS_DRIVEERRORCODE_011807; break;
						case 0x08: strcode = IDS_DRIVEERRORCODE_011808; break;
					}
					break;
				case 0x1E:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_011E00; break;
					}
                    break;
				case 0x37:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_013700; break;
					}
                    break;
				case 0x5D:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_015D00; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_015D01; break;
						case 0xFF: strcode = IDS_DRIVEERRORCODE_015DFF; break;
					}
                    break;
				case 0x6A:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_016A00; break;
					}
                    break;
				case 0x73:
					switch(ascq)
					{
						case 0x01: strcode = IDS_DRIVEERRORCODE_017301; break;
						case 0x06: strcode = IDS_DRIVEERRORCODE_017306; break;
					}
                    break;
			}
			break;
		case 0x02:
			switch(asc)
			{
				case 0x04:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_020400; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_020401; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_020402; break;
						case 0x03: strcode = IDS_DRIVEERRORCODE_020403; break;
						case 0x04: strcode = IDS_DRIVEERRORCODE_020404; break;
						case 0x05: strcode = IDS_DRIVEERRORCODE_020405; break;
						case 0x06: strcode = IDS_DRIVEERRORCODE_020406; break;
						case 0x07: strcode = IDS_DRIVEERRORCODE_020407; break;
						case 0x08: strcode = IDS_DRIVEERRORCODE_020408; break;
					}
                    break;
				case 0x05:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_020500; break;
					}
                    break;
				case 0x06:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_020600; break;
					}
                    break;
				case 0x30:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_023000; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_023001; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_023002; break;
						case 0x03: strcode = IDS_DRIVEERRORCODE_023003; break;
						case 0x04: strcode = IDS_DRIVEERRORCODE_023004; break;
						case 0x05: strcode = IDS_DRIVEERRORCODE_023005; break;
						case 0x07: strcode = IDS_DRIVEERRORCODE_023007; break;
					}
					break;
				case 0x35:
					switch(ascq)
					{
						case 0x02: strcode = IDS_DRIVEERRORCODE_023502; break;
					}
                    break;
				case 0x3A:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_023A00; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_023A01; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_023A02; break;
					}
                    break;
				case 0x3E:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_023E00; break;
					}
                    break;
				case 0x53:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_025300; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_025302; break;
					}
                    break;
				case 0x57:
					switch(ascq)
					{
						case 0x00:  strcode = IDS_DRIVEERRORCODE_025700; break;
					}
                    break;
				case 0x68:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_026800; break;
					}
                    break;
			}
			break;
		case 0x03:
			switch(asc)
			{
				case 0x00:
					switch(ascq)
					{
						case 0x14: strcode = IDS_DRIVEERRORCODE_030014; break;
					}
                    break;
				case 0x02:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_030200; break;
						case 0x80: strcode = IDS_DRIVEERRORCODE_030280; break;
						case 0x81: strcode = IDS_DRIVEERRORCODE_030281; break;
						case 0x82: strcode = IDS_DRIVEERRORCODE_030282; break;
						case 0x83: strcode = IDS_DRIVEERRORCODE_030283; break;
					}
                    break;
				case 0x03:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_030300; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_030301; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_030302; break;
					}
                    break;
				case 0x06:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_030600; break;
					}
					break;
				case 0x0C:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_030C00; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_030C01; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_030C02; break;
						case 0x03: strcode = IDS_DRIVEERRORCODE_030C03; break;
						case 0x04: strcode = IDS_DRIVEERRORCODE_030C04; break;
						case 0x05: strcode = IDS_DRIVEERRORCODE_030C05; break;
						case 0x06: strcode = IDS_DRIVEERRORCODE_030C06; break;
						case 0x07: strcode = IDS_DRIVEERRORCODE_030C07; break;
						case 0x08: strcode = IDS_DRIVEERRORCODE_030C08; break;
						case 0x09: strcode = IDS_DRIVEERRORCODE_030C09; break;
						case 0x0A: strcode = IDS_DRIVEERRORCODE_030C0A; break;
					}
                    break;
				case 0x10:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_031000; break;
					}
                    break;
				case 0x11:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_031100; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_031101; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_031102; break;
						case 0x03: strcode = IDS_DRIVEERRORCODE_031103; break;
						case 0x04: strcode = IDS_DRIVEERRORCODE_031104; break;
						case 0x05: strcode = IDS_DRIVEERRORCODE_031105; break;
						case 0x06: strcode = IDS_DRIVEERRORCODE_031106; break;
						case 0x07: strcode = IDS_DRIVEERRORCODE_031107; break;
						case 0x08: strcode = IDS_DRIVEERRORCODE_031108; break;
						case 0x09: strcode = IDS_DRIVEERRORCODE_031109; break;
						case 0x0A: strcode = IDS_DRIVEERRORCODE_03110A; break;
						case 0x0B: strcode = IDS_DRIVEERRORCODE_03110B; break;
						case 0x0C: strcode = IDS_DRIVEERRORCODE_03110C; break;
						case 0x0D: strcode = IDS_DRIVEERRORCODE_03110D; break;
						case 0x0E: strcode = IDS_DRIVEERRORCODE_03110E; break;
						case 0x0F: strcode = IDS_DRIVEERRORCODE_03110F; break;
						case 0x10: strcode = IDS_DRIVEERRORCODE_031110; break;
					}
                    break;
				case 0x12:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_031200; break;
					}
                    break;
				case 0x13:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_031300; break;
					}
                    break;
				case 0x14:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_031400; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_031401; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_031402; break;
						case 0x03: strcode = IDS_DRIVEERRORCODE_031403; break;
						case 0x04: strcode = IDS_DRIVEERRORCODE_031404; break;
						case 0x05: strcode = IDS_DRIVEERRORCODE_031405; break;
						case 0x06: strcode = IDS_DRIVEERRORCODE_031406; break;
					}
                    break;
				case 0x15:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_031500; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_031501; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_031502; break;
					}
                    break;
				case 0x16:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_031600; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_031601; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_031602; break;
						case 0x03: strcode = IDS_DRIVEERRORCODE_031603; break;
						case 0x04: strcode = IDS_DRIVEERRORCODE_031604; break;
					}
                    break;
				case 0x19:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_031900; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_031901; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_031902; break;
						case 0x03: strcode = IDS_DRIVEERRORCODE_031903; break;
					}
                    break;
				case 0x1F:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_031F00; break;
					}
                    break;
				case 0x2D:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_032D00; break;
					}
                    break;
				case 0x30:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_033000; break;
					}
                    break;
				case 0x31:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_033100; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_033101; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_033102; break;
					}
                    break;
				case 0x32:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_033200; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_033201; break;
					}
                    break;
				case 0x33:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_033300; break;
					}
                    break;
				case 0x36:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_033600; break;
					}
                    break;
				case 0x3B:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_033B00; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_033B01; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_033B02; break;
						case 0x03: strcode = IDS_DRIVEERRORCODE_033B03; break;
						case 0x06: strcode = IDS_DRIVEERRORCODE_033B06; break;
						case 0x07: strcode = IDS_DRIVEERRORCODE_033B07; break;
						case 0x08: strcode = IDS_DRIVEERRORCODE_033B08; break;
						case 0x09: strcode = IDS_DRIVEERRORCODE_033B09; break;
						case 0x0A: strcode = IDS_DRIVEERRORCODE_033B0A; break;
						case 0x0B: strcode = IDS_DRIVEERRORCODE_033B0B; break;
						case 0x0C: strcode = IDS_DRIVEERRORCODE_033B0C; break;
					}
                    break;
				case 0x51:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_035100; break;
					}
                    break;
				case 0x52:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_035200; break;
					}
                    break;
				case 0x57:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_035700; break;
					}
                    break;
				case 0x5C:
					switch(ascq)
					{
						case 0x02: strcode = IDS_DRIVEERRORCODE_035C02; break;
					}
                    break;
				case 0x61:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_036100; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_036101; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_036102; break;
					}
                    break;
				case 0x6C:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_036C00; break;
						
					}
                    break;
				case 0x6D:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_036D00; break;
					}
                    break;
				case 0x70: strcode = IDS_DRIVEERRORCODE_0370NN; break;
				case 0x71:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_037100; break;
					}
                    break;
				case 0x72:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_037200; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_037201; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_037202; break;
					}
                    break;
				case 0x73:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_037300; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_037302; break;
						case 0x03: strcode = IDS_DRIVEERRORCODE_037303; break;
						case 0x04: strcode = IDS_DRIVEERRORCODE_037304; break;
						case 0x05: strcode = IDS_DRIVEERRORCODE_037305; break;
					}
                    break;
			}
			break;
		case 0x04:
			switch(asc)
			{
				case 0x00:
					switch(ascq)
					{
						case 0x17: strcode = IDS_DRIVEERRORCODE_040017; break;
					}
                    break;
				case 0x01:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_040100; break;
					}
                    break;
				case 0x05:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_040500; break;
					}
                    break;
				case 0x08:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_040800; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_040801; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_040802; break;
						case 0x03: strcode = IDS_DRIVEERRORCODE_040803; break;
					}
					break;
				case 0x09:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_040900; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_040901; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_040902; break;
						case 0x03: strcode = IDS_DRIVEERRORCODE_040903; break;
					}
					break;
				case 0x1B:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_041B00; break;
					}
                    break;
				case 0x1C:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_041C00; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_041C01; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_041C02; break;
					}
                    break;
				case 0x34:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_043400; break;
					}
                    break;
				case 0x35:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_043500; break;
						case 0x03: strcode = IDS_DRIVEERRORCODE_043503; break;
					}
                    break;
				case 0x3B:
					switch(ascq)
					{
						case 0x04: strcode = IDS_DRIVEERRORCODE_043B04; break;
						case 0x05: strcode = IDS_DRIVEERRORCODE_043B05; break;
						case 0x16: strcode = IDS_DRIVEERRORCODE_043B16; break;
					}
                    break;
				case 0x3E:
					switch(ascq)
					{
						case 0x01: strcode = IDS_DRIVEERRORCODE_043E01; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_043E02; break;
					}
					break;
				case 0x44:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_044400; break;
					}
					break;
				case 0x46:
					switch(ascq)
					{ 	
						case 0x00: strcode = IDS_DRIVEERRORCODE_044600; break;
					}
					break;
				case 0x47:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_044700; break;
					}
					break;
				case 0x4A:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_044A00; break;
					}
					break;
				case 0x4B:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_044B00; break;
					}
					break;
				case 0x4C:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_044C00; break;
					}
					break;
				case 0x53:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_045300; break;
					}
					break;
				case 0x54:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_045400; break;
					}
					break;
				case 0x60:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_046000; break;
					}
					break;
				case 0x62:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_046200; break;
					}
					break;
				case 0x65:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_046500; break;
					}
					break;
				case 0x66:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_046600; break;
					 	case 0x01: strcode = IDS_DRIVEERRORCODE_046601; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_046602; break;
						case 0x03: strcode = IDS_DRIVEERRORCODE_046603; break;
					}
					break;
				case 0x67:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_046700; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_046701; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_046702; break;
						case 0x03: strcode = IDS_DRIVEERRORCODE_046703; break;
						case 0x04: strcode = IDS_DRIVEERRORCODE_046704; break;
						case 0x05: strcode = IDS_DRIVEERRORCODE_046705; break;
						case 0x06: strcode = IDS_DRIVEERRORCODE_046706; break;
						case 0x07: strcode = IDS_DRIVEERRORCODE_046707; break;
					}
					break;
				case 0x69:
					switch(ascq)
					{	
						case 0x01: strcode = IDS_DRIVEERRORCODE_046901; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_046902; break;
					}
					break;
				case 0x6E:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_046E00; break;
					}
					break;
				case 0xB6:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_04B600; break;
					}
					break;
			}
			break;
		case 0x05:
			switch(asc)
			{
				case 0x00:
					switch(ascq)
					{	
						case 0x11: strcode = IDS_DRIVEERRORCODE_050011; break;
					}
					break;
				case 0x07:
					switch(ascq)
					{	
						case 0x00: strcode = IDS_DRIVEERRORCODE_050700; break;
					}
					break;
				case 0x1A:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_051A00; break;
					}
					break;
				case 0x20:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_052000; break;
					}
					break;
				case 0x21:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_052100; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_052101; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_052102; break;
					}
					break;
				case 0x24:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_052400; break;
					}
					break;
				case 0x25:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_052500; break;
					}
					break;
				case 0x26:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_052600; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_052601; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_052602; break;
						case 0x03: strcode = IDS_DRIVEERRORCODE_052603; break;
						case 0x04: strcode = IDS_DRIVEERRORCODE_052604; break;
					}
					break;
				case 0x27:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_052700; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_052701; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_052702; break;
						case 0x03: strcode = IDS_DRIVEERRORCODE_052703; break;
						case 0x04: strcode = IDS_DRIVEERRORCODE_052704; break;
						case 0x05: strcode = IDS_DRIVEERRORCODE_052705; break;
					}
					break;
				case 0x2B:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_052B00; break;
					}
					break;
				case 0x2C:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_052C00; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_052C01; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_052C02; break;
						case 0x03: strcode = IDS_DRIVEERRORCODE_052C03; break;
						case 0x04: strcode = IDS_DRIVEERRORCODE_052C04; break;
					}
					break;
				case 0x30:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_053000; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_053002; break;
						case 0x04: strcode = IDS_DRIVEERRORCODE_053004; break;
						case 0x05: strcode = IDS_DRIVEERRORCODE_053005; break;
						case 0x06: strcode = IDS_DRIVEERRORCODE_053006; break;
						case 0x08: strcode = IDS_DRIVEERRORCODE_053008; break;
						case 0x09: strcode = IDS_DRIVEERRORCODE_053009; break;
					}
					break;
				case 0x35:
					switch(ascq)
					{	
			 			case 0x01: strcode = IDS_DRIVEERRORCODE_053501; break;
						case 0x04: strcode = IDS_DRIVEERRORCODE_053504; break;
					}
					break;
				case 0x39:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_053900; break;
					}
					break;
				case 0x3D:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_053D00; break;
					}
					break;
				case 0x43:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_054300; break;
					}
					break;
				case 0x53:
					switch(ascq)
					{	
			 			case 0x02: strcode = IDS_DRIVEERRORCODE_055302; break;
					}
					break;
				case 0x55:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_055500; break;
					}
					break;
				case 0x63:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_056300; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_056301; break;
					}
					break;
				case 0x64:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_056400; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_056401; break;
					}
					break;
				case 0x6F:
					switch(ascq)
					{	
						case 0x00: strcode = IDS_DRIVEERRORCODE_056F00; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_056F01; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_056F02; break;
						case 0x03: strcode = IDS_DRIVEERRORCODE_056F03; break;
						case 0x04: strcode = IDS_DRIVEERRORCODE_056F04; break;
						case 0x05: strcode = IDS_DRIVEERRORCODE_056F05; break;
					}
					break;
				case 0x72:
					switch(ascq)
					{	
			 			case 0x03: strcode = IDS_DRIVEERRORCODE_057203; break;
						case 0x05: strcode = IDS_DRIVEERRORCODE_057205; break;
					}
					break;
				case 0x81:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_058100; break;
					}
					break;
				case 0x85:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_058500; break;
					}
					break;
			}
			break;
		case 0x06:
			switch(asc)
			{
				case 0x0A:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_060A00; break;
					}
					break;
				case 0x28:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_062800; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_062801; break;
					}
					break;
				case 0x29:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_062900; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_062901; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_062902; break;
						case 0x03: strcode = IDS_DRIVEERRORCODE_062903; break;
						case 0x04: strcode = IDS_DRIVEERRORCODE_062904; break;
					}
					break;
				case 0x2A:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_062A00; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_062A01; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_062A02; break;
						case 0x03: strcode = IDS_DRIVEERRORCODE_062A03; break;
					}
					break;
				case 0x2E:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_062E00; break;
					}
					break;
				case 0x2F:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_062F00; break;
					}
					break;
				case 0x3B:
					switch(ascq)
					{	
			 			case 0x0D: strcode = IDS_DRIVEERRORCODE_063B0D; break;
						case 0x0E: strcode = IDS_DRIVEERRORCODE_063B0E; break;
						case 0x0F: strcode = IDS_DRIVEERRORCODE_063B0F; break;
			 			case 0x11: strcode = IDS_DRIVEERRORCODE_063B11; break;
			 			case 0x12: strcode = IDS_DRIVEERRORCODE_063B12; break;
						case 0x13: strcode = IDS_DRIVEERRORCODE_063B13; break;
						case 0x14: strcode = IDS_DRIVEERRORCODE_063B14; break;
						case 0x15: strcode = IDS_DRIVEERRORCODE_063B15; break;
					}
					break;
				case 0x3F:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_063F00; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_063F01; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_063F02; break;
						case 0x03: strcode = IDS_DRIVEERRORCODE_063F03; break;
					}
					break;
				case 0x55:
					switch(ascq)
					{	
			 			case 0x01: strcode = IDS_DRIVEERRORCODE_065501; break;
					}
					break;
				case 0x5A:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_065A00; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_065A01; break;
			 			case 0x02: strcode = IDS_DRIVEERRORCODE_065A02; break;
						case 0x03: strcode = IDS_DRIVEERRORCODE_065A03; break;
					}
					break;
				case 0x5B:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_065B00; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_065B01; break;
		 				case 0x02: strcode = IDS_DRIVEERRORCODE_065B02; break;
						case 0x03: strcode = IDS_DRIVEERRORCODE_065B03; break;
					}
					break;
				case 0x5C:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_065C00; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_065C01; break;
					}
					break;
				case 0x5E:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_065E00; break;
			 			case 0x01: strcode = IDS_DRIVEERRORCODE_065E01; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_065E02; break;
						case 0x03: strcode = IDS_DRIVEERRORCODE_065E03; break;
			 			case 0x04: strcode = IDS_DRIVEERRORCODE_065E04; break;
					}
					break;
				case 0x6B:
					switch(ascq)
					{	
			 			case 0x00: strcode = IDS_DRIVEERRORCODE_066B00; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_066B01; break;
			 			case 0x02: strcode = IDS_DRIVEERRORCODE_066B02; break;
					}
					break;
			}
			break;
		case 0x07:
			switch(asc)
			{
				case 0x27:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_072700; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_072701; break;
						case 0x02: strcode = IDS_DRIVEERRORCODE_072702; break;
						case 0x03: strcode = IDS_DRIVEERRORCODE_072703; break;
						case 0x04: strcode = IDS_DRIVEERRORCODE_072704; break;
						case 0x05: strcode = IDS_DRIVEERRORCODE_072705; break;
					}
					break;
			}
			break;
		case 0x08: 
			switch(asc)
			{
				case 0x21:
					switch(ascq)
					{
 						case 0x02: strcode = IDS_DRIVEERRORCODE_082102; break;
					}
					break;
			}
			break;
		case 0x09:
			switch(asc)
			{
				case 0x80:
					switch(ascq)
					{
						case 0x00: strcode = IDS_DRIVEERRORCODE_098000; break;
						case 0x01: strcode = IDS_DRIVEERRORCODE_098001; break;
						case 0x05: strcode = IDS_DRIVEERRORCODE_098005; break;
						case 0x06: strcode = IDS_DRIVEERRORCODE_098006; break;
						case 0x07: strcode = IDS_DRIVEERRORCODE_098007; break;
						case 0x0A: strcode = IDS_DRIVEERRORCODE_09800A; break;
						case 0x0B: strcode = IDS_DRIVEERRORCODE_09800B; break;
						case 0x0C: strcode = IDS_DRIVEERRORCODE_09800C; break;
					}
					break;
			}
			break;
	}
	LoadStringW(hResource, strcode, buffer, cchBuffer);
	if (IDS_UNKNOWN == strcode)	StringCchCatW(buffer, cchBuffer, L".");
	return buffer;
}


