<html>
<head>
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">
<meta http-equiv="Content-Type" content="text/html; charset=<% getCharset(); %>">
<link rel="stylesheet" href="../style/normal_ws.css" type="text/css">
<script language="javascript" src="../js/language_<% getCfgZero(1, "LanguageType"); %>.js"></script>
<script language="javascript" src="../js/common.js"></script>
<script language="javascript">
function Load_Setting()
{
	<% setRefreshSta11nConfiguration(); %>
	document.getElementById("div_mac_addr").style.display = "none";
	document.sta_org_del.mac.disabled = true;

	document.sta_org_del.mpdu_apply.disabled = true;
	var str = document.sta_org_del.selectedbssid.value;
	if (str.length > 0){
		document.getElementById("div_mac_addr").style.display = "";
		document.sta_org_del.mac.disabled = false;
	}
}

function submit_apply()
{
	document.sta_org_del.submit();
	opener.location.reload();
	window.close();
}

function showSelectBssid()
{
	if (document.sta_org_del.selectbssid.value)
		opener.showCwinSelectedBssid();
}

function selectedBSSID()
{
	document.sta_org_del.mpdu_apply.disabled = true;
	if (document.sta_org_del.mac.checked)
		document.sta_org_del.mpdu_apply.disabled = false;
}

function resetForm()
{
	location=location; 
}
</script>
</head>
<body onLoad="Load_Setting()" class="mainbody">
<blockquote>
<table width=700><tr><td>
<h2>Delete AMPDU Originator</h2>
<form method=post name=sta_org_del action="/goform/setStaOrgDel">
<input type=hidden name=selectedbssid>
<table width=100% border=0 cellpadding=3 cellspacing=1> 
  <tr>
    <td class="title" colspan="2">MPDU Aggregation</td>
  </tr>
  <tr>
    <td class="thead">TID</td>
    <td><select name="tid">
	<option value=0 selected>0</option>
	<option value=1 >1</option>
	<option value=2 >2</option>
	<option value=3 >3</option>
	<option value=4 >4</option>
	<option value=5 >5</option>
	<option value=6 >6</option>
	<option value=7 >7</option>
      </select></td>
  </tr>
</table>
<br />

<table width=100% border=0 cellpadding=3 cellspacing=1> 
  <tr>
    <td class="title" colspan="2">Connected BSSIDs</td>
  </tr>
  <tr>
    <td class="thead">BSSID</td>
    <td id="div_mac_addr"><input type=checkbox name="mac" onClick=selectedBSSID()><script>opener.getBssid();document.write(document.sta_org_del.selectedbssid.value);</script></td>
  </tr>
</table>
<table width=100% border=0 cellpadding=3 cellspacing=1> 
  <tr>
    <td>
      <input type=submit class=button value="Apply" name="mpdu_apply" onClick="submit_apply();"> &nbsp; &nbsp;
      <input type=button class=button value="Reset" onClick="resetForm();">
    </td>
  </tr>
</table>
</form>

</td></tr></table>
</body></html>
