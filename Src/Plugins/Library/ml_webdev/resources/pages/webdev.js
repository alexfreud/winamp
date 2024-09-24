function GetUrlParam(name)
{
	name = name.replace(/[\[]/,"\\\[").replace(/[\]]/,"\\\]");
	var regexS = "[\\?&]"+name+"=([^&#]*)";
	var regex = new RegExp( regexS );
	var results = regex.exec( window.location.href );
	if( results == null )
		return "";
	else
		return results[1];
}

function WebDev_OpenService(serviceId, forceUrl)
{
	if (typeof(window.external.WebDev) == "undefined")
		alert("Cannot access Webdev Api");
	else if (false == window.external.WebDev.serviceOpen(serviceId, forceUrl))
		alert("Unable to open service");
} 
function WebDev_OpenDocumentation() 
{
	WebDev_OpenService(701, null);
}

function WebDev_OpenJSAPI2Test() 
{
	WebDev_OpenService(702, null);
}

function WebDev_CreateService() 
{
	if (typeof(window.external.WebDev) == "undefined")
		alert("Cannot access Webdev Api");
	else if (false == window.external.WebDev.serviceCreate())
		alert("Unable to create service");
}

function WebDevEditor_Init(url)
{
	var serviceId = parseInt(GetUrlParam("serviceId"), 10);

	if (typeof(window.external.WebDev) == "undefined")
		alert("Cannot access Webdev Api");
	else
	{
		var info = window.external.WebDev.serviceGetInfo(serviceId);
		if (null == info)
		{
			alert("Unable to get service information");
		}
		else
		{
			document.getElementById("scvedt_edt_id").value = info.id;
			document.getElementById("scvedt_edt_name").value = info.name;
			document.getElementById("scvedt_edt_url").value = info.url;
			document.getElementById("scvedt_edt_icon").value = info.icon;
			document.getElementById("svcedt_chk_bypass").checked = (true == info.preauthorized) ? "checked" : null;
		}
	}
}

function WebDevEditor_Save()
{
	if (typeof(window.external.WebDev) == "undefined")
		alert("Cannot access Webdev Api");
	else
	{
		var serviceId = parseInt(document.getElementById("scvedt_edt_id").value, 10);
		if (0 != serviceId)
		{
			var serviceName = document.getElementById("scvedt_edt_name").value;
			var serviceUrl = document.getElementById("scvedt_edt_url").value;
			var serviceIcon = document.getElementById("scvedt_edt_icon").value;
			var serviceAuth = document.getElementById("svcedt_chk_bypass").checked;
			if (false == window.external.WebDev.serviceSetInfo(serviceId, serviceName, serviceIcon, serviceUrl, serviceAuth))
			{
				alert("Unable to set service info");
			}
			if (false == window.external.WebDev.serviceOpen(serviceId, 1)) 
			{
				alert("Unable to navigate");
			}
		}
	}
}

function WebDevEditor_Close()
{
	if (typeof(window.external.WebDev) == "undefined")
		alert("Cannot access Webdev Api");
	else
	{
		var serviceId = parseInt(document.getElementById("scvedt_edt_id").value, 10);
		if (0 == serviceId || false == window.external.WebDev.serviceOpen(serviceId, 1)) 
			alert("Unable to navigate");
	}
}