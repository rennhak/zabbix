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
	require_once "include/items.inc.php";

	$page["title"] = "S_QUEUE_BIG";
	$page["file"] = "queue.php";
	
	define('ZBX_PAGE_DO_REFRESH', 1);

include_once "include/page_header.php";

?>
<?php
//		VAR			TYPE	OPTIONAL FLAGS	VALIDATION	EXCEPTION
	$fields=array(
		"show"=>		array(T_ZBX_INT, O_OPT,	P_SYS,	IN("0,1"),	NULL)
	);

	check_fields($fields);
?>

<?php
	$_REQUEST["show"] = get_request("show", 0);

	$form = new CForm();
	$form->SetMethod('get');
	
	$cmbMode = new CComboBox("show", $_REQUEST["show"], "submit();");
	$cmbMode->AddItem(0, S_OVERVIEW);
	$cmbMode->AddItem(1, S_DETAILS);
	$form->AddItem($cmbMode);

	show_table_header(S_QUEUE_OF_ITEMS_TO_BE_UPDATED_BIG, $form);
?>

<?php
	$now = time();

	$item_types = array(
			ITEM_TYPE_ZABBIX,
			ITEM_TYPE_ZABBIX_ACTIVE,
			ITEM_TYPE_SNMPV1,
			//ITEM_TYPE_TRAPPER,
			ITEM_TYPE_SNMPV2C,
			ITEM_TYPE_SNMPV3,
			ITEM_TYPE_SIMPLE,
			ITEM_TYPE_INTERNAL,
			ITEM_TYPE_AGGREGATE,
			//ITEM_TYPE_HTTPTEST,
			ITEM_TYPE_EXTERNAL);

	$result = DBselect("select i.itemid,i.nextcheck,i.description,i.key_,i.type,h.host,h.hostid ".
		" from items i,hosts h ".
		" where i.status=".ITEM_STATUS_ACTIVE." and i.type in (".implode(',',$item_types).") ".
		" and ((h.status=".HOST_STATUS_MONITORED." and h.available != ".HOST_AVAILABLE_FALSE.") ".
		" or (h.status=".HOST_STATUS_MONITORED." and h.available=".HOST_AVAILABLE_FALSE." and h.disable_until<=$now)) ".
		" and i.hostid=h.hostid and i.nextcheck<$now and i.key_ not in ('status','icmpping','icmppingsec','zabbix[log]') ".
		" and i.value_type not in (".ITEM_VALUE_TYPE_LOG.") ".
		" and h.hostid in (".get_accessible_hosts_by_user($USER_DETAILS,PERM_READ_ONLY,null,null,get_current_nodeid()).")".
//		" and h.hostid in (".get_accessible_hosts_by_user($USER_DETAILS,PERM_READ_ONLY,null,null,$ZBX_LOCALNODEID).")".
//		" and ".DBin_node('h.hostid', $ZBX_LOCALNODEID).
		" and ".DBin_node('h.hostid', get_current_nodeid()).
		" order by i.nextcheck,h.host,i.description,i.key_");

	$table = new CTableInfo(S_THE_QUEUE_IS_EMPTY);

	if($_REQUEST["show"]==0)
	{

		foreach($item_types as $type)
		{
			$sec_5[$type]=0;
			$sec_10[$type]=0;
			$sec_30[$type]=0;
			$sec_60[$type]=0;
			$sec_300[$type]=0;
			$sec_rest[$type]=0;
		}

		while($row=DBfetch($result))
		{
			if($now-$row["nextcheck"]<=5)		$sec_5[$row["type"]]++;
			elseif($now-$row["nextcheck"]<=10)	$sec_10[$row["type"]]++;
			elseif($now-$row["nextcheck"]<=30)	$sec_30[$row["type"]]++;
			elseif($now-$row["nextcheck"]<=60)	$sec_60[$row["type"]]++;
			elseif($now-$row["nextcheck"]<=300)	$sec_300[$row["type"]]++;
			else					$sec_rest[$row["type"]]++;

		}
		$table->setHeader(array(S_ITEMS,S_5_SECONDS,S_10_SECONDS,S_30_SECONDS,S_1_MINUTE,S_5_MINUTES,S_MORE_THAN_5_MINUTES));
		foreach($item_types as $type)
		{
			$elements=array(item_type2str($type),$sec_5[$type],$sec_10[$type],
				new CCol($sec_30[$type],"warning"),
				new CCol($sec_60[$type],"average"),
				new CCol($sec_300[$type],"high"),
				new CCol($sec_rest[$type],"disaster"));
			$table->addRow($elements);
		}
	}
	else
	{
		$table->SetHeader(array(
				S_NEXT_CHECK,
				is_show_subnodes() ? S_NODE : null,
				S_HOST,
				S_DESCRIPTION
				));
		while($row=DBfetch($result))
		{
			$table->AddRow(array(
				date("m.d.Y H:i:s",
					$row["nextcheck"]),
				get_node_name_by_elid($row['hostid']),
				$row['host'],
				item_description($row["description"],$row["key_"])
				));
		}
	}

	$table->Show();
?>
<?php
	if($_REQUEST["show"]!=0)
	{
		show_table_header(S_TOTAL.": ".$table->GetNumRows());
	}
?>

<?php

include_once "include/page_footer.php";

?>
