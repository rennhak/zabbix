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
	require_once "include/graphs.inc.php";
	require_once "include/classes/pie.inc.php";
	
	$page["file"]	= "chart7.php";
	$page["title"]	= "S_CHART";
	$page["type"]	= PAGE_TYPE_IMAGE;

include_once "include/page_header.php";

?>
<?php
//		VAR			TYPE	OPTIONAL FLAGS	VALIDATION	EXCEPTION
	$fields=array(
		"period"=>	array(T_ZBX_INT, O_OPT,	P_NZERO,	BETWEEN(ZBX_MIN_PERIOD,ZBX_MAX_PERIOD),	null),
		"from"=>	array(T_ZBX_INT, O_OPT,	P_NZERO,	null,			null),
		"stime"=>	array(T_ZBX_INT, O_OPT,	P_NZERO,	null,			null),
		"border"=>	array(T_ZBX_INT, O_OPT,	P_NZERO,	IN('0,1'),		null),
		"name"=>	array(T_ZBX_STR, O_OPT,	NULL,		null,			null),
		"width"=>	array(T_ZBX_INT, O_OPT,	NULL,		BETWEEN(0,65535),	null),
		"height"=>	array(T_ZBX_INT, O_OPT,	NULL,		BETWEEN(0,65535),	null),
		"graphtype"=>	array(T_ZBX_INT, O_OPT,	NULL,		IN("2,3"),		null),
		"graph3d"=>	array(T_ZBX_INT, O_OPT,	P_NZERO,	IN('0,1'),		null),
		"legend"=>	array(T_ZBX_INT, O_OPT,	P_NZERO,	IN('0,1'),		null),
		"items"=>	array(T_ZBX_STR, O_OPT,	NULL,		null,			null)
	);

	check_fields($fields);
?>
<?php
	$available_hosts = get_accessible_hosts_by_user($USER_DETAILS, PERM_READ_ONLY, PERM_RES_IDS_ARRAY, get_current_nodeid(true));

	$items = get_request('items', array());
	asort_by_key($items, 'sortorder');

	foreach($items as $gitem){
		if(!$host=DBfetch(DBselect('SELECT h.* FROM hosts h,items i WHERE h.hostid=i.hostid AND i.itemid='.$gitem['itemid']))){
			fatal_error(S_NO_ITEM_DEFINED);
		}
		if(!isset($available_hosts[$host['hostid']])){
			access_deny();
		}
	}
	
	
	$effectiveperiod = navigation_bar_calc();

	if(count($items) == 1){
		$_REQUEST['period'] = get_request('period',get_profile('web.item.graph.period', ZBX_PERIOD_DEFAULT, null, $items['itemid']));
		if($_REQUEST['period'] >= ZBX_MIN_PERIOD){
			update_profile('web.item.graph.period',$_REQUEST['period'], PROFILE_TYPE_INT, $items['itemid']);
		}
	}
	
	$graph = new Pie(get_request("graphtype"	,GRAPH_TYPE_NORMAL));
	$graph->SetHeader($host["host"].":".get_request("name",""));
	
	$graph3d = get_request('graph3d',0);
	$legend = get_request('legend',0);
	
	if($graph3d == 1) $graph->SwitchPie3D();
	$graph->SwitchLegend($legend);
	
	unset($host);

	if(isset($_REQUEST["period"]))		$graph->SetPeriod($_REQUEST["period"]);
	if(isset($_REQUEST["from"]))		$graph->SetFrom($_REQUEST["from"]);
	if(isset($_REQUEST["stime"]))		$graph->SetSTime($_REQUEST["stime"]);
	if(isset($_REQUEST["border"]))		$graph->SetBorder(0);

	$graph->SetWidth(get_request("width",		400));
	$graph->SetHeight(get_request("height",		300));
	
	foreach($items as $id => $gitem){
//		SDI($gitem);
		$graph->AddItem(
			$gitem["itemid"],
			$gitem["calc_fnc"],
			$gitem["color"],
			$gitem["type"],
			$gitem["periods_cnt"]
			);

//		unset($items[$id]);
	}
	$graph->Draw();
?>
<?php

include_once "include/page_footer.php";

?>
