function geturlparams( key )
{
	key = key.replace(/[\[]/,"\\\[").replace(/[\]]/,"\\\]");
	var regexS = "[\\#&]"+key+"=([^&#]*)";
	var regex = new RegExp( regexS );
	var results = regex.exec( window.location.href );
	if( results == null )
		return "";
	else
		return results[1];
}
function geterrorhex()
{
	var signedInt=geturlparams('errorcode');
	return "0x" + CvtI32(signedInt).toUpperCase();
}
function bitStr(N, bits) 
{ 
	var S = "", Q
	while (bits--) { S = (Q=N%2) + S ; N = (N-Q)/2 }
	return S;
}
function hex(N, bits)  
{ 
	return (0x10000 + N).toString(16).substring(5-bits) 
}
function Four(d, c, b, a, bits) 
{
	return hex(d, bits) + hex(c, bits) + hex(b, bits) + hex(a, bits) 
}
function CvtI32(F) 
{ 
	var X = F |0, a, b, c, d
	var ba = bitStr(a = X       & 0xFF, 8)
	var bb = bitStr(b = X >>  8 & 0xFF, 8)
	var bc = bitStr(c = X >> 16 & 0xFF, 8)
	var bd = bitStr(d = X >> 24 & 0xFF, 8)
	var hex = Four(d, c, b, a, 2) 
	return hex;
}
function tryagain()
{
	window.location.replace(unescape(geturlparams('url')));
}
function togglemore()
{
	var display=document.getElementById("errorMoreInfo").style.display;
	if (display == "block") {
		document.getElementById("errorMoreInfo").style.display="none";
	}
	else {
		document.getElementById("errorMoreInfo").style.display="block";
	}
}
function populatepage()
{
	var errorcode = parseInt(geturlparams('errorcode'));
	switch (errorcode)
	{
		case 404:
			var errorTitle = errorTitle404;
			var errorCode = errorCode404;
			var errorDescription = errorDescription404;
			break;
		case 403:
			var errorTitle = errorTitle403;
			var errorCode = errorCode403;
			var errorDescription = errorDescription403;
			break;
		case 500:
			var errorTitle = errorTitle500;
			var errorCode = errorCode500;
			var errorDescription = errorDescription500;
			break;
		case 503:
			var errorTitle = errorTitle503;
			var errorCode = errorCode503;
			var errorDescription = errorDescription503;
			break;
		case 502:
			var errorTitle = errorTitle502;
			var errorCode = errorCode502;
			var errorDescription = errorDescription502;
			break;
		case 501:
			var errorTitle = errorTitle501;
			var errorCode = errorCode501;
			var errorDescription = errorDescription501;
			break;
		case 504:
			var errorTitle = errorTitle504;
			var errorCode = errorCode504;
			var errorDescription = errorDescription504;
			break;
		case 505:
			var errorTitle = errorTitle505;
			var errorCode = errorCode505;
			var errorDescription = errorDescription505;
			break;
		case 400:
			var errorTitle = errorTitle400;
			var errorCode = errorCode400;
			var errorDescription = errorDescription400;
			break;
		case 401:
			var errorTitle = errorTitle401;
			var errorCode = errorCode401;
			var errorDescription = errorDescription401;
			break;
		case 402:
			var errorTitle = errorTitle402;
			var errorCode = errorCode402;
			var errorDescription = errorDescription402;
			break;
		case 405:
			var errorTitle = errorTitle405;
			var errorCode = errorCode405;
			var errorDescription = errorDescription405;
			break;
		case 406:
			var errorTitle = errorTitle406;
			var errorCode = errorCode406;
			var errorDescription = errorDescription406;
			break;
		case 407:
			var errorTitle = errorTitle407;
			var errorCode = errorCode407;
			var errorDescription = errorDescription407;
			break;
		case 408:
			var errorTitle = errorTitle408;
			var errorCode = errorCode408;
			var errorDescription = errorDescription408;
			break;
		case 409:
			var errorTitle = errorTitle409;
			var errorCode = errorCode409;
			var errorDescription = errorDescription409;
			break;
		case 410:
			var errorTitle = errorTitle410;
			var errorCode = errorCode410;
			var errorDescription = errorDescription410;
			break;
		case 411:
			var errorTitle = errorTitle411;
			var errorCode = errorCode411;
			var errorDescription = errorDescription411;
			break;
		case 413:
			var errorTitle = errorTitle413;
			var errorCode = errorCode413;
			var errorDescription = errorDescription413;
			break;
		case 414:
			var errorTitle = errorTitle414;
			var errorCode = errorCode414;
			var errorDescription = errorDescription414;
			break;
		case 415:
			var errorTitle = errorTitle415;
			var errorCode = errorCode415;
			var errorDescription = errorDescription415;    
			break;
		case -2146697214:
			var errorTitle = errorTitle800c0002;
			var errorCode = errorCode800c0002;
			var errorDescription = errorDescription800c0002;
			break;
		case -2146697213:
			var errorTitle = errorTitle800c0003;
			var errorCode = errorCode800c0003;
			var errorDescription = errorDescription800c0003;
			break;
		case -2146697212:
			var errorTitle = errorTitle800c0004;
			var errorCode = errorCode800c0004;
			var errorDescription = errorDescription800c0004;
			break;
		case -2146697211:
			var errorTitle = errorTitle800c0005;
			var errorCode = errorCode800c0005;
			var errorDescription = errorDescription800c0005;
			break;
		case -2146697210:
			var errorTitle = errorTitle800c0006;
			var errorCode = errorCode800c0006;
			var errorDescription = errorDescription800c0006;
			break;
		case -2146697209:
			var errorTitle = errorTitle800c0007;
			var errorCode = errorCode800c0007;
			var errorDescription = errorDescription800c0007;
			break;
		case -2146697208:
			var errorTitle = errorTitle800c0008;
			var errorCode = errorCode800c0008;
			var errorDescription = errorDescription800c0008;
			break;
		case -2146697207:
			var errorTitle = errorTitle800c0009;
			var errorCode = errorCode800c0009;
			var errorDescription = errorDescription800c0009;
			break;
		case -2146697206:
			var errorTitle = errorTitle800c000a;
			var errorCode = errorCode800c000a;
			var errorDescription = errorDescription800c000a;
			break;
		case -2146697205:
			var errorTitle = errorTitle800c000b;
			var errorCode = errorCode800c000b;
			var errorDescription = errorDescription800c000b;
			break;
		case -2146697204:
			var errorTitle = errorTitle800c000c;
			var errorCode = errorCode800c000c;
			var errorDescription = errorDescription800c000c;
			break;
		case -2146697203:
			var errorTitle = errorTitle800c000d;
			var errorCode = errorCode800c000d;
			var errorDescription = errorDescription800c000d;
			break;
		case -2146697202:
			var errorTitle = errorTitle800c000e;
			var errorCode = errorCode800c000e;
			var errorDescription = errorDescription800c000e;
			break;
		case -2146697201:
			var errorTitle = errorTitle800c000f;
			var errorCode = errorCode800c000f;
			var errorDescription = errorDescription800c000f;
			break;
		case -2146697200:
			var errorTitle = errorTitle800c0010;
			var errorCode = errorCode800c0010;
			var errorDescription = errorDescription800c0010;
			break;
		case -2146697196:
			var errorTitle = errorTitle800c0014;
			var errorCode = errorCode800c0014;
			var errorDescription = errorDescription800c0014;
			break;
		case -2146697195:
			var errorTitle = errorTitle800c0015;
			var errorCode = errorCode800c0015;
			var errorDescription = errorDescription800c0015;
			break;
		case -2146697194:
			var errorTitle = errorTitle800c0016;
			var errorCode = errorCode800c0016;
			var errorDescription = errorDescription800c0016;
			break;
		case -2146697193:
			var errorTitle = errorTitle800c0017;
			var errorCode = errorCode800c0017;
			var errorDescription = errorDescription800c0017;
			break;
		case -2146697192:
			var errorTitle = errorTitle800c0018;
			var errorCode = errorCode800c0018;
			var errorDescription = errorDescription800c0018;
			break;
		case -2146697960:
			var errorTitle = errorTitle800c0100;
			var errorCode = errorCode800c0100;
			var errorDescription = errorDescription800c0100;
			break;
		case -2146696704:
			var errorTitle = errorTitle800c0200;
			var errorCode = errorCode800c0200;
			var errorDescription = errorDescription800c0200;
			break;
		case -2146696448:
			var errorTitle = errorTitle800c0300;
			var errorCode = errorCode800c0300;
			var errorDescription = errorDescription800c0300;
			break;
		default:
			var errorTitle = errorTitleUnknown;
			var errorCode = errorCodeUnknown;
			var errorDescription = errorDescriptionUnknown;
	}
	document.getElementById("errorTitleText").innerHTML = errorTitle;
	document.getElementById("errorCode").innerHTML = errorCode;
	document.getElementById("errorDescText").innerHTML = errorDescription;
}