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
	require_once "include/acknow.inc.php";
	require_once "include/triggers.inc.php";
	require_once "include/events.inc.php";

	$page["file"] = "tr_status.php";
	$page["title"] = "S_STATUS_OF_TRIGGERS";

?>
<?php
	$tr_hash = calc_trigger_hash();

	$triggers_hash = get_cookie('zbx_triggers_hash', '0,0');

	$new=explode(",",$tr_hash);
	$old=explode(",",$triggers_hash);

	zbx_set_post_cookie("zbx_triggers_hash",$tr_hash,time()+1800);

	$triggers_hash = get_cookie('zbx_triggers_hash', '0,0');
	
	$new=explode(",",$tr_hash);
	$old=explode(",",$triggers_hash);

	if( $old[1] != $new[1] )
	{
		if( $new[0] < $old[0] )	// Number of trigger decreased
			$status = "off";
		else			// Number of trigger increased
			$status = "on";

		$files_apdx = array(
			5 => 'disaster',
			4 => 'high',
			3 => 'average',
			2 => 'warning',
			1 => 'information',
			0 => 'not_classified');

		$prior_dif = $new[0]-$old[0];

		krsort($files_apdx);
		foreach($files_apdx as $priority => $apdx)
		{
			if(round($prior_dif / pow(100, $priority)) != 0)
			{
				$audio = 'audio/trigger_'.$status.'_'.$apdx.'.wav';
				break;
			}
		}

		if(!isset($audio) || !file_exists($audio))
			$audio = 'audio/trigger_'.$status.'.wav';
	}

?>
<?php
	define('ZBX_PAGE_DO_REFRESH', 1);

	if(isset($_REQUEST["fullscreen"]))
		define('ZBX_PAGE_NO_MENU', 1);
	
include_once "include/page_header.php";
echo '<script type="text/javascript" src="js/blink.js"></script>';
	
?>
<?php
//		VAR			TYPE	OPTIONAL FLAGS	VALIDATION	EXCEPTION
	$fields=array(
		"groupid"=>	array(T_ZBX_INT, O_OPT, P_SYS,	DB_ID, null),
		"hostid"=>	array(T_ZBX_INT, O_OPT, P_SYS,	DB_ID, null),
		"sort"=>	array(T_ZBX_STR, O_OPT,  null,	IN('"host","priority","description","lastchange"'), null),
		"noactions"=>	array(T_ZBX_STR, O_OPT,  null,	IN('"true","false"'), null),
		"compact"=>	array(T_ZBX_STR, O_OPT,  null,	IN('"true","false"'), null),
		"onlytrue"=>	array(T_ZBX_STR, O_OPT,  null,	IN('"true","false"'), null),
		"show_unknown"=>	array(T_ZBX_INT, O_OPT,	P_SYS,	IN(array(0,1)),	null),
		"select"=>	array(T_ZBX_STR, O_OPT,  null,	IN('"true","false"'), null),
		"txt_select"=>	array(T_ZBX_STR, O_OPT,  null,	null, null),
		"fullscreen"=>	array(T_ZBX_STR, O_OPT,  null,	null, null),
		"btnSelect"=>	array(T_ZBX_STR, O_OPT,  null,  null, null)
	);

	check_fields($fields);

	$_REQUEST["onlytrue"]		=	get_request("onlytrue", get_profile("web.tr_status.onlytrue", 'true'));
	$_REQUEST["noactions"]		=	get_request("noactions", get_profile("web.tr_status.noactions", 'true'));
	$_REQUEST["compact"]		=	get_request("compact", get_profile("web.tr_status.compact", 'true'));
	$_REQUEST['show_unknown']	=	get_request('show_unknown',get_profile('web.tr_status.show_unknown',0));

	validate_group_with_host(PERM_READ_ONLY,array("allow_all_hosts","always_select_first_host","monitored_hosts","with_monitored_items"),
		"web.tr_status.groupid","web.tr_status.hostid");

	update_profile("web.tr_status.onlytrue",$_REQUEST["onlytrue"]);
	update_profile("web.tr_status.noactions",$_REQUEST["noactions"]);
	update_profile("web.tr_status.compact",$_REQUEST["compact"]);
	update_profile('web.tr_status.show_unknown',$_REQUEST['show_unknown']);
	
?>
<?php
	if(isset($audio))
	{
		play_sound($audio);
	}
?>                                                                                                             
<?php
	$sort		 = get_request('sort',		'priority');
	$noactions	 = get_request('noactions',	'true');
	$compact	 = get_request('compact',	'true');
	$onlytrue	 = get_request('onlytrue',	'true');
	$show_unknown= get_request('show_unknown',0);
	$select		 = get_request('select',		'false');
	$txt_select	 = get_request('txt_select',	"");
	if($select == 'false') $txt_select = '';
	
	$unknown=($show_unknown == 0)?TRIGGER_VALUE_UNKNOWN:'';
?>
<?php
	$r_form = new CForm();
	$r_form->SetMethod('get');
	
	$cmbGroup = new CComboBox("groupid",$_REQUEST["groupid"],"submit()");
	$cmbHosts = new CComboBox("hostid",$_REQUEST["hostid"],"submit()");

	$cmbGroup->AddItem(0,S_ALL_SMALL);
	
	$availiable_hosts = get_accessible_hosts_by_user($USER_DETAILS,PERM_READ_LIST, null, null, get_current_nodeid());

	$result=DBselect("select distinct g.groupid,g.name from groups g, hosts_groups hg, hosts h, items i, functions f, triggers t ".
		" where h.hostid in (".$availiable_hosts.") ".
		" and hg.groupid=g.groupid and h.status=".HOST_STATUS_MONITORED.
		" and h.hostid=i.hostid and hg.hostid=h.hostid and i.status=".ITEM_STATUS_ACTIVE.
		" and i.itemid=f.itemid and t.triggerid=f.triggerid and t.status=".TRIGGER_STATUS_ENABLED.
		" order by g.name");
	while($row=DBfetch($result))
	{
		$cmbGroup->AddItem(
				$row['groupid'],
				get_node_name_by_elid($row['groupid']).$row['name']
				);
		unset($row);
	}
	$r_form->AddItem(array(S_GROUP.SPACE,$cmbGroup));
	
	if($_REQUEST["groupid"] > 0)
	{
		$sql="select h.hostid,h.host from hosts h,items i,hosts_groups hg, functions f, triggers t where h.status=".HOST_STATUS_MONITORED.
			" and h.hostid=i.hostid and hg.groupid=".$_REQUEST["groupid"]." and hg.hostid=h.hostid".
			" and i.status=".ITEM_STATUS_ACTIVE.
			" and i.itemid=f.itemid and t.triggerid=f.triggerid and t.status=".TRIGGER_STATUS_ENABLED.
			" and h.hostid in (".$availiable_hosts.") ".
			" group by h.hostid,h.host order by h.host";
	}
	else
	{
		$cmbHosts->AddItem(0,S_ALL_SMALL);
		$sql="select h.hostid,h.host from hosts h,items i, functions f, triggers t where h.status=".HOST_STATUS_MONITORED.
			" and i.status=".ITEM_STATUS_ACTIVE." and h.hostid=i.hostid".
			" and i.itemid=f.itemid and t.triggerid=f.triggerid and t.status=".TRIGGER_STATUS_ENABLED.
			" and h.hostid in (".$availiable_hosts.") ".
			" group by h.hostid,h.host order by h.host";
	}
	$result=DBselect($sql);
	while($row=DBfetch($result))
	{
		$cmbHosts->AddItem(
				$row['hostid'],
				get_node_name_by_elid($row['hostid']).$row['host']
				);
	}

	$r_form->AddItem(array(SPACE.S_HOST.SPACE,$cmbHosts));
	$r_form->AddVar("compact",$compact);
	$r_form->AddVar("onlytrue",$onlytrue);
	$r_form->AddVar('show_unknown',$show_unknown);
	$r_form->AddVar("noactions",$noactions);
	$r_form->AddVar("select",$select);
	$r_form->AddVar("txt_select",$txt_select);
	$r_form->AddVar("sort",$sort);
	if(isset($_REQUEST['fullscreen'])) $r_form->AddVar("fullscreen",1);

	show_table_header(
		new CLink(SPACE.S_STATUS_OF_TRIGGERS_BIG.SPACE.date("[H:i:s]",time()),"tr_status.php?onlytrue=$onlytrue&noactions=$noactions".
			"&compact=$compact&sort=$sort".(!isset($_REQUEST["fullscreen"]) ? '&fullscreen=1' : '')),
		$r_form);
	
	if(!isset($_REQUEST["fullscreen"]))
	{
		$left_col = array();
		array_push($left_col, '[', new CLink($onlytrue != 'true' ? S_SHOW_ONLY_TRUE : S_SHOW_ALL_TRIGGERS,
			"tr_status.php?onlytrue=".($onlytrue != 'true' ? 'true' : 'false').
			"&noactions=$noactions&compact=$compact&select=$select&txt_select=$txt_select&sort=$sort"
			), ']'.SPACE);
			
		array_push($left_col, '[', new CLink($show_unknown!=1?S_SHOW_UNKNOWN:S_HIDE_UNKNOWN,
			"tr_status.php?show_unknown=".($show_unknown!=1?'1':'0').
			"&onlytrue=$onlytrue&noactions=$noactions&compact=$compact&select=$select&txt_select=$txt_select&sort=$sort"
			), ']'.SPACE);
		
		array_push($left_col, '[', new CLink($noactions != 'true' ? S_HIDE_ACTIONS : S_SHOW_ACTIONS,
			"tr_status.php?noactions=".($noactions != 'true' ? 'true' : 'false').
			"&onlytrue=$onlytrue&show_unknown=$show_unknown&compact=$compact&select=$select&txt_select=$txt_select&sort=$sort"
			), ']'.SPACE);

		array_push($left_col, '[', new CLink($compact != 'true' ? S_HIDE_DETAILS: S_SHOW_DETAILS,
			"tr_status.php?compact=".($compact != 'true' ? 'true' : 'false').
			"&onlytrue=$onlytrue&show_unknown=$show_unknown&noactions=$noactions&select=$select&txt_select=$txt_select&sort=$sort"
			), ']'.SPACE);
		
		array_push($left_col, '[', new CLink($select != 'true' ? S_SELECT : S_HIDE_SELECT,
			"tr_status.php?select=".($select != 'true' ? 'true' : 'false').
			"&onlytrue=$onlytrue&show_unknown=$show_unknown&noactions=$noactions&compact=$compact&txt_select=$txt_select&sort=$sort"
			), ']');
			
		if($select=='true')
		{
			$form = new CForm();
			$form->SetMethod('get');
			
			$form->AddItem(new CTextBox("txt_select",$txt_select,15));
			$form->AddItem(new CButton("btnSelect", "Select"));
			$form->AddItem(new CButton("btnSelect", "Inverse select"));
			$form->AddVar("compact",$compact);
			$form->AddVar("onlytrue",$onlytrue);
			$form->AddVar("noactions",$noactions);
			$form->AddVar("select",$select);
			array_push($left_col,BR,$form);
		}
		show_table_header($left_col);
	}

  	if(isset($_REQUEST["fullscreen"]))
	{
		$triggerInfo = new CTriggersInfo();
		$triggerInfo->HideHeader();
		$triggerInfo->Show();
	}

	if(isset($_REQUEST["fullscreen"]))
	{
		$fullscreen="&fullscreen=1";
	}
	else
	{
		$fullscreen="";
	}
	
	$table  = new CTableInfo();
	$header=array();

	$headers_array = array(
		is_show_subnodes() ? array('simple_label'=>S_NODE) : null,
		$_REQUEST['hostid'] > 0 ? null : 
		array('select_label'=>S_HOST_BIG	, 'simple_label'=>S_HOST,		'sort'=>'host'),
		array('select_label'=>S_NAME_BIG	, 'simple_label'=>S_NAME,		'sort'=>'description'),
		array('simple_label'=>S_STATUS),
		array('select_label'=>S_SEVERITY_BIG	, 'simple_label'=>S_SEVERITY,		'sort'=>'priority'),
		array('select_label'=>S_LAST_CHANGE_BIG	, 'simple_label'=>S_LAST_CHANGE,	'sort'=>'lastchange'),
		array('simple_label'=>($noactions!='true') ? S_ACTIONS : NULL),
		array('simple_label'=>S_ACKNOWLEDGED),
		array('simple_label'=>S_COMMENTS)
		);

	$select_vars = (isset($sort) && $sort=="description") ? "&select=$select&txt_select=$txt_select" : "";
	foreach($headers_array as $el)
	{
		if(isset($el['sort']) && $sort == $el['sort'])
		{
			$descr = $el['select_label'];
		}
		else if(isset($el['sort']))
		{
			$descr = new CLink($el['simple_label'],"tr_status.php?sort=".$el['sort'].
				"&onlytrue=$onlytrue&noactions=$noactions&compact=$compact$select_vars$fullscreen");
		}
		else
		{
			$descr = $el['simple_label'];
		}
		array_push($header,$descr);
		unset($el);
	}
  
	$table->SetHeader($header);
	unset($header);

	switch ($sort)
	{
		case "host":		$sort="order by h.host";	if($_REQUEST["hostid"] <= 0)	break; /* else "description" */
		case "description":	$sort="order by t.description";				break;
		case "priority":	$sort="order by t.priority desc, t.description";	break;
		case "lastchange":	$sort="order by t.lastchange desc, t.priority";		break;
		default:			$sort="order by t.priority desc, t.description";
	}

	$cond=($_REQUEST['hostid'] > 0)?' AND h.hostid='.$_REQUEST['hostid'].' ':'';

	$sql = 'SELECT DISTINCT t.triggerid,t.status,t.description, '.
							' t.expression,t.priority,t.lastchange,t.comments,t.url,t.value,h.host '.
					' FROM triggers t,hosts h,items i,functions f '.
					' WHERE f.itemid=i.itemid AND h.hostid=i.hostid '.
						' AND t.triggerid=f.triggerid AND t.status='.TRIGGER_STATUS_ENABLED.
						' AND i.status='.ITEM_STATUS_ACTIVE.' AND '.DBin_node('t.triggerid').
						' AND h.hostid not in ('.get_accessible_hosts_by_user($USER_DETAILS,PERM_READ_ONLY, PERM_MODE_LT).') '. 
						' AND h.status='.HOST_STATUS_MONITORED.' '.$cond.' '.$sort;

	$result = DBselect($sql);

	while($row=DBfetch($result))
	{
// Check for dependencies

		if(!$eventid = first_initial_eventid($row,$show_unknown)) continue;

		$res_n = DBSelect('SELECT DISTINCT e.eventid, e.clock as lastchange, e.acknowledged, e.value '.
							' FROM events e'.
							' WHERE e.eventid='.$eventid.
							' LIMIT 1');
		$row_n = DBfetch($res_n);
		$row = array_merge($row,$row_n);
		
		$deps = DBfetch(DBselect("select count(*) as cnt from trigger_depends d, triggers t ".
			" where d.triggerid_down=".$row["triggerid"]." and d.triggerid_up=t.triggerid and t.value=1"));

		if($deps["cnt"]>0)
		{
			continue;
		}

		$elements=array();

		$description = expand_trigger_description($row["triggerid"]);

		if(isset($_REQUEST["btnSelect"]) && '' != $txt_select && ((stristr($description, $txt_select)) == ($_REQUEST["btnSelect"]=="Inverse select"))) continue;

		if($row["url"] != "")
		{
			$description = new CLink($description, $row["url"]);
		}

		if($compact != 'true')
		{
			$font = new CTag('font','yes');
			$font->AddOption('color','#000');
			$font->AddOption('size','-2');
			$font->AddItem(explode_exp($row["expression"],1));
			$description = array($description,BR, $font);
		}

		if((time(NULL)-$row["lastchange"])<TRIGGER_BLINK_PERIOD)
			$blink = array(1=>'<a name="blink">',	2=>'</a>');
		else
			$blink = array(1=>'', 		2=>'');
		
		$value = new CSpan($blink[1].trigger_value2str($row["value"]).$blink[2], get_trigger_value_style($row["value"]));

		if($noactions=='true')
		{
			$actions=NULL;
		}
		else
		{
			$actions=array(
				new CLink(S_CHANGE,'triggers.php?form=update&triggerid='.$row["triggerid"].url_param('hostid'),"action")
				);
		}

		$ack = '-';
		$header_event = 0;
				
		if(TRIGGER_SHOW_UNDEFINED_ACK || ($row['value'] != TRIGGER_VALUE_UNKNOWN)){
				$ack=($row['acknowledged'] != 1)?(new CLink(S_NOT_ACKNOWLEDGED,'acknow.php?eventid='.$row['eventid'],'on')):('-');
		}
//		if(!not_ack_event($eventid)) continue;
		if(($onlytrue=='true') && (($row['value'] != TRIGGER_VALUE_TRUE) && ((time() - $row['lastchange']) > TRIGGER_BLINK_PERIOD))){
		}
		else if(not_ack_event($eventid)){
			$table->AddRow(array(
					get_node_name_by_elid($row['triggerid']),
					$_REQUEST['hostid'] > 0 ? null : $row['host'],
					$description,
					$value,
					new CCol(
						get_severity_description($row["priority"]),
						get_severity_style($row["priority"])),
					new CLink(zbx_date2str(S_DATE_FORMAT_YMDHMS,$row["lastchange"]),"tr_events.php?triggerid=".$row["triggerid"],"action"),
					$actions,
					new CCol($ack,"center"),
					new CLink(($row["comments"] == "") ? S_ADD : S_SHOW,"tr_comments.php?triggerid=".$row["triggerid"],"action")
					));
			$description = expand_trigger_description($row["triggerid"]);
			$header_event = 1;
		}
		
		if($compact != 'true')
		{
			$font = new CTag('font','yes');
			$font->AddOption('color','#000');
			$font->AddOption('size','-2');
			$font->AddItem(explode_exp($row["expression"],1));
			$description = array($description, $font);
		}

		if(TRIGGER_FALSE_TIME_ACK > 0){

			$res_notack = get_notacknowledged($row['triggerid'],$show_unknown,$onlytrue);
			while($row_notack=DBfetch($res_notack)){

				if($row['eventid'] == $row_notack['eventid']) continue;
				if(($show_unknown == 0) && (!event_initial_time($row_notack,$show_unknown))) continue;
				
				if($header_event < 1){
					$value = new CSpan($blink[1].trigger_value2str($row_notack["value"]).$blink[2], get_trigger_value_style($row_notack["value"]));
				}
				else{
					$value = new CSpan(trigger_value2str($row_notack['value']), get_trigger_value_style($row_notack['value']));	
				}
				
				if($header_event == 1){
					$font = new CTag('font','yes');
					$font->AddOption('color','#808080');
					$font->AddItem(array('&nbsp;-&nbsp;',$description));
					$description = $font->ToString();					
				}
				$header_event++;
				
				$ack= new CLink(S_NOT_ACKNOWLEDGED,'acknow.php?eventid='.$row_notack['eventid'],'on');
				$table->AddRow(array(
						get_node_name_by_elid($row['triggerid']),
						$_REQUEST['hostid'] > 0 ? null : $row['host'],
						$description,
						$value,
						new CCol(
							get_severity_description($row["priority"]),
							get_severity_style($row["priority"])),
						new CLink(zbx_date2str(S_DATE_FORMAT_YMDHMS,$row_notack['clock']),"tr_events.php?triggerid=".$row["triggerid"],"action"),
						$actions,
						new CCol($ack,"center"),
						new CLink(($row["comments"] == "") ? S_ADD : S_SHOW,"tr_comments.php?triggerid=".$row["triggerid"],"action")
						));
			}
		}

		unset($row,$description, $actions);
	}
	zbx_add_post_js('blink.init();');
	$table->Show(false);

	show_table_header(S_TOTAL.": ".$table->GetNumRows());
?>
<?php

include_once "include/page_footer.php";

?>
