<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">
<meta http-equiv="Content-Type" content="text/html; charset=<% getCharset(); %>">
<link rel="stylesheet" href="../images/style.css" type="text/css">
<script language="javascript" src="../js/language_<% getCfgZero(1, "LanguageType"); %>.js"></script>
<script language="javascript" src="../js/common.js"></script>
<script language="javascript">
var lan2 = '<% getCfgZero(1, "Lan2Enabled"); %>';
var secs;
var timerID = null;
var timerRunning = false;

function StartTheTimer()
{
	if (secs==0) {
		TimeoutReload(5);
		//window.location.reload();
		window.location.href=window.location.href;	//reload page
    }
	else{
        self.status = secs;
        secs = secs - 1;
        timerRunning = true;
        timerID = self.setTimeout("StartTheTimer()", 1000);
    }
}

function TimeoutReload(timeout)
{
	secs = timeout;
	if(timerRunning)
		clearTimeout(timerID);
	
	timerRunning = false;
	StartTheTimer();	
}

function dhcpTypeSwitch()
{
	document.getElementById("dhcpclient").style.display = "none";
	document.getElementById("dhcpClientList").style.display = "none";
	document.getElementById("mask").style.display = "none";
	document.getElementById("gateway").style.display = "none";
	document.getElementById("lease").style.display = "none";
	document.getElementById("pridns").style.display = "none";
	document.getElementById("secdns").style.display = "none";
	
	if (document.lanCfg.lanDhcpType.options.selectedIndex == 1)	{
		document.getElementById("dhcpclient").style.display = "";
		document.getElementById("dhcpClientList").style.display = "";
		document.getElementById("mask").style.display = "";
		document.getElementById("gateway").style.display = "";
		document.getElementById("lease").style.display = "";
		document.getElementById("pridns").style.display = "";
		document.getElementById("secdns").style.display = "";
	}
}

function Load_Setting()
{
	var opmode = "<% getCfgZero(1, "OperationMode"); %>";
	var dhcp = <% getCfgZero(1, "dhcpEnabled"); %>;
	var stp = <% getCfgZero(1, "stpEnabled"); %>;
	var lltd = <% getCfgZero(1, "lltdEnabled"); %>;
	var igmp = <% getCfgZero(1, "igmpEnabled"); %>;
	var radvd = <% getCfgZero(1, "radvdEnabled"); %>;
	var pppoe = <% getCfgZero(1, "pppoeREnabled"); %>;
	var dns = <% getCfgZero(1, "dnsPEnabled"); %>;
	var wan = "<% getCfgZero(1, "wanConnectionMode"); %>";
	var lltdb = "<% getLltdBuilt(); %>";
	var igmpb = "<% getIgmpProxyBuilt(); %>";
	var radvdb = "<% getRadvdBuilt(); %>";
	var pppoeb = "<% getPppoeRelayBuilt(); %>";
	var dnsp = "<% getDnsmasqBuilt(); %>";
	var hashost = "<% getHostSupp(); %>";

	if (lan2 == "1"){
		var lan2_ip = '<% getCfgGeneral(1, "lan2_ipaddr"); %>';
		var lan2_nm = '<% getCfgGeneral(1, "lan2_netmask"); %>';

		document.lanCfg.lan2enabled[0].checked = true;
		document.lanCfg.lan2Ip.disabled = false;
		document.lanCfg.lan2Ip.value = lan2_ip;
		document.lanCfg.lan2Netmask.disabled = false;
		document.lanCfg.lan2Netmask.value = lan2_nm;
	}
	else{
		document.lanCfg.lan2enabled[1].checked = true;
		document.lanCfg.lan2Ip.disabled = true;
		document.lanCfg.lan2Netmask.disabled = true;
	}

	document.lanCfg.lanDhcpType.options.selectedIndex = 1*dhcp;
	dhcpTypeSwitch();
	document.lanCfg.stpEnbl.options.selectedIndex = 1*stp;
	document.lanCfg.lltdEnbl.options.selectedIndex = 1*lltd;
	document.lanCfg.igmpEnbl.options.selectedIndex = 1*igmp;
	document.lanCfg.radvdEnbl.options.selectedIndex = 1*radvd;
	document.lanCfg.pppoeREnbl.options.selectedIndex = 1*pppoe;
	document.lanCfg.dnspEnbl.options.selectedIndex = 1*dns;

	if (lltdb == "0") {
		document.getElementById("lltd").style.display = "none";
		document.lanCfg.lltdEnbl.options.selectedIndex = 0;
	}
	if (igmpb == "0") {
		document.getElementById("igmpProxy").style.display = "none";
		document.lanCfg.igmpEnbl.options.selectedIndex = 0;
	}
	if (radvdb == "0") {
		document.getElementById("radvd").style.display = "none";
		document.lanCfg.radvdEnbl.options.selectedIndex = 0;
	}
	if (pppoeb == "0") {
		document.getElementById("pppoerelay").style.display = "none";
		document.lanCfg.pppoeREnbl.options.selectedIndex = 0;
	}
	if (dnsp == "0") {
		document.getElementById("dnsproxy").style.display = "none";
		document.lanCfg.dnspEnbl.options.selectedIndex = 0;
	}
}

function formCheck()
{
	if (!isIpAddrMsg(document.lanCfg.lanIp.value, MM_ipaddr)) 
		return false;
	
	if (!isMaskAddrMsg(document.lanCfg.lanNetmask.value, MM_submask))
		return false;
	
	if (document.lanCfg.lanDhcpType.options.selectedIndex == 1) {
		if (!isIpAddrMsg(document.lanCfg.dhcpStart.value, "DHCP "+MM_sipaddr)) 
			return false;
		
		if (!isIpSubnet(document.lanCfg.dhcpStart.value, document.lanCfg.lanNetmask.value, document.lanCfg.lanIp.value)) {
			alert(JS_msg28);
			return false;
		}
		
		if (!isIpAddrMsg(document.lanCfg.dhcpEnd.value, "DHCP "+MM_eipaddr)) 
			return false;
		
		if (!isIpSubnet(document.lanCfg.dhcpEnd.value, document.lanCfg.lanNetmask.value, document.lanCfg.lanIp.value)) {
			alert(JS_msg29);
			return false;
		}
		
		if (!isIpRange(document.lanCfg.dhcpStart.value, document.lanCfg.dhcpEnd.value)) {
			alert(JS_msg30);
			return false;
		}
		
		if ((document.lanCfg.dhcpStart.value == document.lanCfg.lanIp.value) || (document.lanCfg.dhcpEnd.value == document.lanCfg.lanIp.value)) {
			alert(JS_msg31);
			document.lanCfg.dhcpStart.focus();
			return false;		
		}
		
		if (!isMaskAddrMsg(document.lanCfg.dhcpMask.value, MM_submask)) 
			return false;		
		
		if (document.lanCfg.dhcpPriDns.value != "") 
			if (!isIpAddrMsg(document.lanCfg.dhcpPriDns.value, MM_pridns)) 
				return false; 
		
		if (document.lanCfg.dhcpSecDns.value != "")
			if (!isIpAddrMsg(document.lanCfg.dhcpSecDns.value, MM_secdns)) 
				return false; 
		
		if (!isIpAddrMsg(document.lanCfg.dhcpGateway.value, MM_default_gateway)) 
			return false;
		
		if (!isIpSubnet(document.lanCfg.dhcpGateway.value, document.lanCfg.lanNetmask.value, document.lanCfg.lanIp.value)) {
			alert(JS_msg32);
			return false;
		}
		
		if (!isNumberRange(document.lanCfg.dhcpLease.value, 60, 86400)) { 
			alert(MM_lease_time + JS_msg100);
			return false;
		}
	}
	
	return true;
}

function lan2_enable_switch()
{
	if (document.lanCfg.lan2enabled[1].checked == true){
		document.lanCfg.lan2Ip.disabled = true;
		document.lanCfg.lan2Netmask.disabled = true;
	}
	else{
		document.lanCfg.lan2Ip.disabled = false;
		document.lanCfg.lan2Netmask.disabled = false;
	}
}

var oldIp;
function recIpCfg()
{
	oldIp = document.lanCfg.lanIp.value;
}

/*
 * Try to modify dhcp server configurations:
 *   dhcp start/end ip address to the same as new lan ip address
 */
function modDhcpCfg()
{
	var i, j;
	var mask = document.lanCfg.lanNetmask.value;
	var newNet = document.lanCfg.lanIp.value;

	//support simple subnet mask only
	if (mask == "255.255.255.0")
		mask = 3;
	else if (mask == "255.255.0.0")
		mask = 2;
	else if (mask == "255.0.0.0")
		mask = 1;
	else
		return;

	//get the old subnet
	for (i=0, j=0; i<oldIp.length; i++) {
		if (oldIp.charAt(i) == '.') {
			j++;
			if (j != mask)
				continue;
			oldIp = oldIp.substring(0, i);
			break;
		}
	}

	//get the new subnet
	for (i=0, j=0; i<newNet.length; i++) {
		if (newNet.charAt(i) == '.') {
			j++;
			if (j != mask)
				continue;
			newNet = newNet.substring(0, i);
			break;
		}
	}

	document.lanCfg.dhcpStart.value = document.lanCfg.dhcpStart.value.replace(oldIp, newNet);
	document.lanCfg.dhcpEnd.value = document.lanCfg.dhcpEnd.value.replace(oldIp, newNet);
	document.lanCfg.dhcpGateway.value = document.lanCfg.lanIp.value;
	document.lanCfg.dhcpPriDns.value = document.lanCfg.lanIp.value; //by luot
}

function dhcpClientClick(url)
{
	if (document.lanCfg.lanDhcpType.options.selectedIndex == 1)
		openWindow(url, 'DHCPTbl', 700, 400);
}

function resetForm()
{
	location=location; 
}
</script>
</head>

<body onLoad="Load_Setting()">
<form method=post name="lanCfg" action="/goform/setLan" onSubmit="return formCheck()">
<input type="hidden" name="submit-url" value="/internet/lanbak.asp">
<table width="800" border="0" cellpadding="0" cellspacing="0" bgcolor="#cae9fa">
  	<tr>
		<td colspan="3" valign="top" height="11"></td>
  	</tr>
	<tr>
		<td width="20"></td>
		<td valign="top">
		<table width="760" border="0" cellpadding="0" cellspacing="0" height="68">
			<tr>
				<td class="pgTitle" height="34"><script>dw(MM_lan_settings)</script></td>
				<td class="pgButton" align="right"></td>
			</tr>
			<tr>
				<td colspan="2" class="pgHelp"><script>dw(JS_msg_lan)</script></td>
			</tr>
		</table>

		</td>
		<td width="20"></td>
	</tr>
	
	<tr>
		<td width="20"></td>
		<td>
		<table width="760" border="0" cellpadding="0" cellspacing="0">
			<tr>
				<td class="pgTitle" height="34"><script>dw(MM_basic_settings)</script></td>
				<td class="pgButton" align="right"></td>
			</tr>
			<tr>
  <tr>
  <td class="pgleft"><script>dw(MM_macaddr)</script>:</td>
  <td class="pgRight"><% getLanMac(); %></td>
</tr>
<tr id="hostname" style="display:none">
  <td class="pgleft"><script>dw(MM_hostname)</script>:</td>
  <td class="pgRight"> <input name="hostname" maxlength=16 value="<% getCfgGeneral(1, "HostName"); %>"></td>
</tr>
<tr>
  <td class="pgleft"><script>dw(MM_ipaddr)</script>:</td>
  <td class="pgRight"><input name="lanIp" maxlength=15 value="<% getLanIp(); %>" onFocus="recIpCfg()" onBlur="modDhcpCfg()"></td>
</tr>
<tr>
  <td class="pgleft"><script>dw(MM_submask)</script>:</td>
  <td class="pgRight"><input name="lanNetmask" maxlength=15 value="<% getLanNetmask(); %>"></td>
</tr>
<tr id="brGateway" style="display:none">
  <td class="pgleft"><script>dw(MM_default_gateway)</script></td>
  <td class="pgRight"><input name="lanGateway" maxlength=15 value="<% getWanGateway(); %>"></td>
</tr>
<tr id="brPriDns" style="display:none">
  <td class="pgleft"><script>dw(MM_pridns)</script>:</td>
  <td class="pgRight"><input name="lanPriDns" maxlength=15 value="<% getDns(1); %>"></td>
</tr>
<tr id="brSecDns" style="display:none">
  <td class="pgleft"><script>dw(MM_secdns)</script>:</td>
  <td class="pgRight"><input name="lanSecDns" maxlength=15 value="<% getDns(2); %>"></td>
</tr>
		</table>
		</td>
		<td width="20"></td>
	</tr>
	
	<tr>
	<td></td>
	<td class="pgTitle" height="34"><script>dw(MM_dhcp_server_settings)</script></td>
	<td></td>
	</tr>
	<tr>
		<td width="20"></td>
		<td>
		<table width="760" border="0" cellpadding="0" cellspacing="0">
			<tr>
  <td class="pgleft"><script>dw(MM_dhcp_server)</script>:</td>
  <td class="pgRight"><select name="lanDhcpType" onChange="dhcpTypeSwitch();">
      <option value="DISABLE"><script>dw(MM_disable)</script></option>
      <option value="SERVER"><script>dw(MM_enable)</script></option>
    </select></td>
</tr>
<tr id="dhcpclient">
  <td class="pgleft"><script>dw(MM_dhcppool)</script>:</td>
  <td class="pgRight"><input name="dhcpStart" maxlength=15 value="<% getCfgGeneral(1, "dhcpStart"); %>"> - <input name="dhcpEnd" maxlength=15 value="<% getCfgGeneral(1, "dhcpEnd"); %>"> <script>dw('<input type="button" class="button" id="dhcpClientList" value="'+MM_client_list+'" onClick=dhcpClientClick(\"dhcpcliinfo.asp\")>')</script></td>
</tr>
<tr id="gateway">
  <td class="pgleft"><script>dw(MM_default_gateway)</script>:</td>
  <td class="pgRight"><input name="dhcpGateway" maxlength=15 value="<% getCfgGeneral(1, "dhcpGateway"); %>"></td>
</tr>
<tr id="mask">
  <td class="pgleft"><script>dw(MM_submask)</script>:</td>
  <td class="pgRight"><input name="dhcpMask" maxlength=15 value="<% getCfgGeneral(1, "dhcpMask"); %>"></td>
</tr>
<tr id="pridns" style="display:none">
  <td class="pgleft"><script>dw(MM_pridns)</script>:</td>
  <td class="pgRight"><input name="dhcpPriDns" maxlength=15 value="<% getCfgGeneral(1, "dhcpPriDns"); %>"></td>
</tr>
<tr id="secdns" style="display:none">
  <td class="pgleft"><script>dw(MM_secdns)</script>:</td>
  <td class="pgRight"><input name="dhcpSecDns" maxlength=15 value="<% getCfgGeneral(1, "dhcpSecDns"); %>"></td>
</tr>
<tr id="lease">
  <td class="pgleft"><script>dw(MM_lease_time)</script>:</td>
  <td class="pgRight"><input name="dhcpLease" size="8" maxlength=8 value="<% getCfgGeneral(1, "dhcpLease"); %>"> <script>dw(MM_seconds)</script> (60-86400)</td>
</tr>
<tr style="display:none">
  <td colspan="2">&nbsp;</td>
</tr>
<tr style="display:none">
  <td class="title2" colspan="2"><script>dw(MM_other_settings)</script><hr></td>
</tr>
<tr style="display:none">
  <td class="pgleft">LAN2</td>
  <td class="pgRight"><input type="radio" name="lan2enabled" value="1" onClick="lan2_enable_switch()"><script>dw(MM_enable)</script><input type="radio" name="lan2enabled" value="0" onClick="lan2_enable_switch()" checked><script>dw(MM_disable)</script></td>
</tr>
<tr style="display:none">
  <td class="pgleft">LAN2 <script>dw(MM_ipaddr)</script>:</td>
  <td class="pgRight"><input name="lan2Ip" maxlength=15 value=""></td>
</tr>
<tr style="display:none">
  <td class="pgleft">LAN2 <script>dw(MM_submask)</script>:</td>
  <td class="pgRight"><input name="lan2Netmask" maxlength=15 value=""></td>
</tr>
<tr style="display:none">
  <td class="pgleft"><script>dw(MM_8021d_tree)</script>:</td>
  <td class="pgRight"><select name="stpEnbl">
      <option value="0"><script>dw(MM_disable)</script></option>
      <option value="1"><script>dw(MM_enable)</script></option>
    </select></td>
</tr>
<tr id="lltd" style="display:none">
  <td class="pgleft">LLTD:</td>
  <td class="pgRight"><select name="lltdEnbl">
      <option value="0"><script>dw(MM_disable)</script></option>
      <option value="1"><script>dw(MM_enable)</script></option>
    </select></td>
</tr>
<tr id="igmpProxy" style="display:none">
  <td class="pgleft"><script>dw(MM_igmp_proxy)</script>:</td>
  <td class="pgRight"><select name="igmpEnbl">
      <option value="0"><script>dw(MM_disable)</script></option>
      <option value="1"><script>dw(MM_enable)</script></option>
    </select></td>
</tr>
<tr id="radvd" style="display:none">
  <td class="pgleft"><script>dw(MM_router_advertisement)</script>:</td>
  <td class="pgRight"><select name="radvdEnbl">
      <option value="0"><script>dw(MM_disable)</script></option>
      <option value="1"><script>dw(MM_enable)</script></option>
    </select></td>
</tr>
<tr id="pppoerelay" style="display:none">
  <td class="pgleft"><script>dw(MM_pppoe_relay)</script>:</td>
  <td class="pgRight"><select name="pppoeREnbl">
      <option value="0"><script>dw(MM_disable)</script></option>
      <option value="1"><script>dw(MM_enable)</script></option>
    </select></td>
</tr>
<tr id="dnsproxy" style="display:none">
  <td class="pgleft"><script>dw(MM_dns_proxy)</script>:</td>
  <td class="pgRight"><select name="dnspEnbl">
      <option value="0"><script>dw(MM_disable)</script></option>
      <option value="1"><script>dw(MM_enable)</script></option>
    </select></td>
</tr>
 <tr>
 	<td class="pgleft"></td>
    <td class="pgRight">
      <script>dw('<input type=submit class=button value="'+BT_apply+'" onClick="TimeoutReload(20)"> &nbsp; &nbsp;\
      <input type=button class=button value="'+BT_reset+'" onClick="resetForm();">')</script>
    </td>
  </tr>
		</table>
		</td>
		<td width="20"></td>
	</tr>
	<tr>
 	<td width="20"></td>
    <td>
      <span id="wirelessbac">
	<iframe width="760" height="500" id="wirelessbasic_Page" name="wirelessbasic_Page" src="/wireless/basic.asp" frameborder="0" marginheight="0" scrolling="no"> </iframe>
</span>
		</td>
		<td width="20"></td>
	</tr>
</table>	
</form>
<table width="800" height="100%" border="0" cellpadding="0" cellspacing="0" bgcolor="#cae9fa">
</body>
</html>