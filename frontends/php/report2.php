<?php
/* 
** ZABBIX
** Copyright (C) 2000-2005 SIA Zabbix
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**/
?>
<?php
	require_once "include/config.inc.php";
	require_once "include/hosts.inc.php";

	$page["title"]	= "S_AVAILABILITY_REPORT";
	$page["file"]	= "report2.php";

include_once "include/page_header.php";

?>
<?php
//		VAR			TYPE	OPTIONAL FLAGS	VALIDATION	EXCEPTION
	$fields=array(
		"groupid"=>		array(T_ZBX_INT, O_OPT,	P_SYS|P_NZERO,	DB_ID,			NULL),
		"hostid"=>		array(T_ZBX_INT, O_OPT,	P_SYS|P_NZERO,	DB_ID,			NULL),
		"triggerid"=>		array(T_ZBX_INT, O_OPT,	P_SYS|P_NZERO,	DB_ID,			NULL)
	);

	check_fields($fields);

	validate_group_with_host(PERM_READ_LIST,array("always_select_first_host","monitored_hosts","with_items"));
?>
<?php
	$r_form = new CForm();

	$cmbGroup = new CComboBox("groupid",$_REQUEST["groupid"],"submit()");
	$cmbHosts = new CComboBox("hostid",$_REQUEST["hostid"],"submit()");

	$cmbGroup->AddItem(0,S_ALL_SMALL);
	
	$availiable_hosts = get_accessible_hosts_by_userid($USER_DETAILS['userid'],PERM_READ_ONLY, null, null, $ZBX_CURNODEID);

	$result=DBselect("select distinct g.groupid,g.name from groups g, hosts_groups hg, hosts h, items i ".
		" where h.hostid in (".$availiable_hosts.") ".
		" and hg.groupid=g.groupid and h.status=".HOST_STATUS_MONITORED.
		" and h.hostid=i.hostid and hg.hostid=h.hostid and i.status=".ITEM_STATUS_ACTIVE.
		" order by g.name");
	while($row=DBfetch($result))
	{
		$cmbGroup->AddItem($row["groupid"],$row["name"]);
	}
	$r_form->AddItem(array(S_GROUP.SPACE,$cmbGroup));
	
	if($_REQUEST["groupid"] > 0)
	{
		$sql="select h.hostid,h.host from hosts h,items i,hosts_groups hg where h.status=".HOST_STATUS_MONITORED.
			" and h.hostid=i.hostid and hg.groupid=".$_REQUEST["groupid"]." and hg.hostid=h.hostid".
			" and h.hostid in (".$availiable_hosts.") ".
			" group by h.hostid,h.host order by h.host";
	}
	else
	{
		$sql="select h.hostid,h.host from hosts h,items i where h.status=".HOST_STATUS_MONITORED.
			" and h.hostid=i.hostid and h.hostid in (".$availiable_hosts.") ".
			" group by h.hostid,h.host order by h.host";
	}
	$result=DBselect($sql);
	while($row=DBfetch($result))
	{
		$cmbHosts->AddItem($row["hostid"],$row["host"]);
	}

	$r_form->AddItem(array(SPACE.S_HOST.SPACE,$cmbHosts));
	show_table_header(S_AVAILABILITY_REPORT_BIG, $r_form);

?>
<?php
	$denyed_hosts = get_accessible_hosts_by_userid($USER_DETAILS['userid'],PERM_READ_ONLY, PERM_MODE_LT);
	
	if(isset($_REQUEST["triggerid"]))
	{
		if (!$row = DBfetch(DBselect("select distinct h.hostid,h.host,t.description from hosts h,items i,functions f,triggers t ".
			" where t.triggerid=".$_REQUEST["triggerid"]." and t.triggerid=f.triggerid ".
			" and f.itemid=i.itemid and i.hostid=h.hostid ".
			" and h.hostid not in (".$denyed_hosts.") and ".DBid2nodeid("t.triggerid")."=".$ZBX_CURNODEID.
			" order by h.host,t.description ")))
			access_deny();
		
		show_table_header(array(new CLink($row["host"],"?hostid=".$row["hostid"])," : \"",expand_trigger_description_by_data($row),"\""));

		$table = new CTableInfo(null,"graph");
		$table->AddRow(new CImg("chart4.php?triggerid=".$_REQUEST["triggerid"]));
		$table->Show();
	}
	else if(isset($_REQUEST["hostid"]))
	{
		$row	= DBfetch(DBselect("select host from hosts where hostid=".$_REQUEST["hostid"]));
		show_table_header($row["host"]);

		$result = DBselect("select distinct h.hostid,h.host,t.triggerid,t.expression,t.description,t.value ".
			" from triggers t,hosts h,items i,functions f ".
			" where f.itemid=i.itemid and h.hostid=i.hostid and t.status=".TRIGGER_STATUS_ENABLED.
			" and t.triggerid=f.triggerid and h.hostid=".$_REQUEST["hostid"]." and h.status=".HOST_STATUS_MONITORED.
			" and h.hostid not in (".$denyed_hosts.") and ".DBid2nodeid("t.triggerid")."=".$ZBX_CURNODEID.
			" and i.status=".ITEM_STATUS_ACTIVE.
			" order by h.host, t.description");

		$table = new CTableInfo();
		$table->setHeader(array(S_NAME,S_TRUE,S_FALSE,S_UNKNOWN,S_GRAPH));
		while($row=DBfetch($result))
		{
			$availability = calculate_availability($row["triggerid"],0,0);

			$true	= new CSpan(sprintf("%.4f%%",$availability["true"]), "on");
			$false	= new CSpan(sprintf("%.4f%%",$availability["false"]), "off");
			$unknown= new CSpan(sprintf("%.4f%%",$availability["unknown"]), "unknown");
			$actions= new CLink(S_SHOW,"report2.php?hostid=".$_REQUEST["hostid"]."&triggerid=".$row["triggerid"],"action");

			$table->addRow(array(
				new CLink(
					expand_trigger_description_by_data($row),
					"events.php?triggerid=".$row["triggerid"],"action"),
				$true,
				$false,
				$unknown,
				$actions
				));
		}
		$table->show();
	}
?>
<?php
	
	include_once "include/page_footer.php";

?>
