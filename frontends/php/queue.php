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
	$page['hist_arg'] = array('show');
	
	define('ZBX_PAGE_DO_REFRESH', 1);

include_once "include/page_header.php";

?>
<?php
//		VAR			TYPE	OPTIONAL FLAGS	VALIDATION	EXCEPTION
	$fields=array(
		"show"=>		array(T_ZBX_INT, O_OPT,	P_SYS,	IN("0,1,2"),	NULL)
	);

	check_fields($fields);
	
	$available_hosts = get_accessible_hosts_by_user($USER_DETAILS,PERM_READ_ONLY,PERM_RES_IDS_ARRAY);
?>

<?php
	$_REQUEST["show"] = get_request("show", 0);

	$form = new CForm();
	$form->SetMethod('get');
	
	$cmbMode = new CComboBox("show", $_REQUEST["show"], "submit();");
	$cmbMode->AddItem(0, S_OVERVIEW);
	$cmbMode->AddItem(1, S_OVERVIEW_BY_PROXY);
	$cmbMode->AddItem(2, S_DETAILS);
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

	$result = DBselect('SELECT i.itemid,i.nextcheck,i.description,i.key_,i.type,h.host,h.hostid,h.proxy_hostid '.
		' FROM items i,hosts h '.
		' WHERE i.status='.ITEM_STATUS_ACTIVE.
			' AND i.type in ('.implode(',',$item_types).') '.
			' AND ((h.status='.HOST_STATUS_MONITORED.' AND h.available != '.HOST_AVAILABLE_FALSE.') '.
				' OR (h.status='.HOST_STATUS_MONITORED.' AND h.available='.HOST_AVAILABLE_FALSE.' AND h.disable_until<='.$now.')) '.
			' AND i.hostid=h.hostid '.
			' AND i.nextcheck + 5 <'.$now.
			' AND i.key_ NOT IN ('.zbx_dbstr('status').','.zbx_dbstr('icmpping').','.zbx_dbstr('icmppingsec').','.zbx_dbstr('zabbix[log]').') '.
			' AND i.value_type not in ('.ITEM_VALUE_TYPE_LOG.') '.
			' AND '.DBcondition('h.hostid',$available_hosts).
			' AND '.DBin_node('h.hostid', get_current_nodeid()).
		' ORDER BY i.nextcheck,h.host,i.description,i.key_');

	$table = new CTableInfo(S_THE_QUEUE_IS_EMPTY);

	if($_REQUEST["show"]==0){

		foreach($item_types as $type){
			$sec_10[$type]=0;
			$sec_30[$type]=0;
			$sec_60[$type]=0;
			$sec_300[$type]=0;
			$sec_600[$type]=0;
			$sec_rest[$type]=0;
		}

		while($row=DBfetch($result)){
			if($now-$row["nextcheck"]<=10)			$sec_10[$row["type"]]++;
			else if($now-$row["nextcheck"]<=30)		$sec_30[$row["type"]]++;
			else if($now-$row["nextcheck"]<=60)		$sec_60[$row["type"]]++;
			else if($now-$row["nextcheck"]<=300)		$sec_300[$row["type"]]++;
			else if($now-$row["nextcheck"]<=600)		$sec_600[$row["type"]]++;
			else	$sec_rest[$row["type"]]++;

		}
		
		$table->setHeader(array(S_ITEMS,S_5_SECONDS,S_10_SECONDS,S_30_SECONDS,S_1_MINUTE,S_5_MINUTES,S_MORE_THAN_10_MINUTES));
		foreach($item_types as $type){
			$elements=array(
				item_type2str($type),
				new CCol($sec_10[$type],($sec_10[$type])?"unknown_trigger":"normal"),
				new CCol($sec_30[$type],($sec_30[$type])?"information":"normal"),
				new CCol($sec_60[$type],($sec_60[$type])?"warning":"normal"),
				new CCol($sec_300[$type],($sec_300[$type])?"average":"normal"),
				new CCol($sec_600[$type],($sec_600[$type])?"high":"normal"),
				new CCol($sec_rest[$type],($sec_rest[$type])?"disaster":"normal")
			);
			
			$table->addRow($elements);
		}
	}
	else if ($_REQUEST["show"] == 1)
	{
		$db_proxies = DBselect('select hostid from hosts where status='.HOST_STATUS_PROXY);

		while (null != ($db_proxy = DBfetch($db_proxies))){
			$sec_10[$db_proxy['hostid']]	= 0;
			$sec_30[$db_proxy['hostid']]	= 0;
			$sec_60[$db_proxy['hostid']]	= 0;
			$sec_300[$db_proxy['hostid']]	= 0;
			$sec_600[$db_proxy['hostid']]	= 0;
			$sec_rest[$db_proxy['hostid']]	= 0;
		}

		$sec_10[0]	= 0;
		$sec_30[0]	= 0;
		$sec_60[0]	= 0;
		$sec_300[0]	= 0;
		$sec_600[0]	= 0;
		$sec_rest[0]	= 0;

		while ($row = DBfetch($result))
		{
			$diff = $now - $row['nextcheck'];

			if ($diff <= 10)	$sec_10[$row['proxy_hostid']]++;
			else if ($diff <= 30)	$sec_30[$row['proxy_hostid']]++;
			else if ($diff <= 60)	$sec_60[$row['proxy_hostid']]++;
			else if ($diff <= 300)	$sec_300[$row['proxy_hostid']]++;
			else if ($diff <= 600)	$sec_600[$row['proxy_hostid']]++;
			else	$sec_rest[$row['proxy_hostid']]++;

		}

		$table->setHeader(array(S_PROXY,S_5_SECONDS,S_10_SECONDS,S_30_SECONDS,S_1_MINUTE,S_5_MINUTES,S_MORE_THAN_10_MINUTES));

		$db_proxies = DBselect('select hostid,host from hosts where status='.HOST_STATUS_PROXY.' order by host');

		while (null != ($db_proxy = DBfetch($db_proxies))){
			$elements = array(
				$db_proxy['host'],
				new CCol($sec_10[$db_proxy['hostid']], $sec_10[$db_proxy['hostid']] ? "unknown_trigger" : "normal"),
				new CCol($sec_30[$db_proxy['hostid']], $sec_30[$db_proxy['hostid']] ? "information" : "normal"),
				new CCol($sec_60[$db_proxy['hostid']], $sec_60[$db_proxy['hostid']] ? "warning" : "normal"),
				new CCol($sec_300[$db_proxy['hostid']], $sec_300[$db_proxy['hostid']] ? "average" : "normal"),
				new CCol($sec_600[$db_proxy['hostid']], $sec_600[$db_proxy['hostid']] ? "high" : "normal"),
				new CCol($sec_rest[$db_proxy['hostid']], $sec_rest[$db_proxy['hostid']] ? "disaster" : "normal")
			);
			$table->addRow($elements);
		}
		$elements = array(
			new CCol(S_SERVER, 'bold'),
			new CCol($sec_10[0], $sec_10[0] ? 'unknown_trigger' : 'normal'),
			new CCol($sec_30[0], $sec_30[0] ? 'information' : 'normal'),
			new CCol($sec_60[0], $sec_60[0] ? 'warning' : 'normal'),
			new CCol($sec_300[0], $sec_300[0] ? 'average' : 'normal'),
			new CCol($sec_600[0], $sec_600[0] ? 'high' : 'normal'),
			new CCol($sec_rest[0], $sec_rest[0] ? 'disaster' : 'normal')
		);
		$table->addRow($elements);
	}
	else if ($_REQUEST["show"] == 2)
	{
		$table->SetHeader(array(
				S_NEXT_CHECK,
				is_show_subnodes() ? S_NODE : null,
				S_HOST,
				S_DESCRIPTION
				));
		while($row=DBfetch($result)){
			$table->AddRow(array(
				date("m.d.Y H:i:s",
					$row["nextcheck"]),
				get_node_name_by_elid($row['hostid']),
				$row['host'],
				item_description($row)
				));
		}
	}

	$table->Show();

	if($_REQUEST["show"]!=0){
		show_table_header(S_TOTAL.": ".$table->GetNumRows());
	}
?>
<?php

include_once "include/page_footer.php";

?>
