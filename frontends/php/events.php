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
	require_once('include/config.inc.php');
	require_once('include/hosts.inc.php');
	require_once('include/events.inc.php');
	require_once('include/actions.inc.php');
	require_once('include/discovery.inc.php');
	require_once('include/html.inc.php');
	
	$page["title"] = "S_LATEST_EVENTS";
	$page['file'] = 'events.php';
	$page['hist_arg'] = array('groupid','hostid');
	$page['scripts'] = array('calendar.js');
	
	$page['type'] = detect_page_type(PAGE_TYPE_HTML);
	
	if(PAGE_TYPE_HTML == $page['type']){
		define('ZBX_PAGE_DO_REFRESH', 1);
	}
	
	include_once('include/page_header.php');

?>
<?php
	$allow_discovery = check_right_on_discovery(PERM_READ_ONLY);

	$allowed_sources[] = EVENT_SOURCE_TRIGGERS;
	if($allow_discovery) $allowed_sources[] = EVENT_SOURCE_DISCOVERY;
		
//		VAR			TYPE	OPTIONAL FLAGS	VALIDATION	EXCEPTION
	$fields=array(
		'source'=>			array(T_ZBX_INT, O_OPT,	P_SYS,	IN($allowed_sources),	NULL),
		'groupid'=>			array(T_ZBX_INT, O_OPT,	P_SYS,	DB_ID,	NULL),
		'hostid'=>			array(T_ZBX_INT, O_OPT,	P_SYS,	DB_ID,	NULL),
		'triggerid'=>		array(T_ZBX_INT, O_OPT,	P_SYS,	DB_ID,	NULL),

		'next'=>			array(T_ZBX_STR, O_OPT,	P_SYS,	NULL,			NULL),
		'back'=>			array(T_ZBX_STR, O_OPT,	P_SYS,	NULL,			NULL),
		'nav_time'=>		array(T_ZBX_INT, O_OPT,	P_UNSET_EMPTY,	null,	NULL),
		
		'load'=>			array(T_ZBX_STR, O_OPT,	P_SYS,	NULL,			NULL),
		'fullscreen'=>		array(T_ZBX_INT, O_OPT,	P_SYS,	IN('0,1'),		NULL),

// filter
		'filter_rst'=>		array(T_ZBX_INT, O_OPT,	P_SYS,	IN(array(0,1)),	NULL),
		'filter_set'=>		array(T_ZBX_STR, O_OPT,	P_SYS,	null,	NULL),
		
		'show_unknown'=>	array(T_ZBX_INT, O_OPT,	P_SYS,	IN(array(0,1)),	NULL),
				
//ajax
		'favobj'=>		array(T_ZBX_STR, O_OPT, P_ACT,	NULL,			'isset({favid})'),
		'favid'=>		array(T_ZBX_STR, O_OPT, P_ACT,  NOT_EMPTY,		NULL),
		'state'=>		array(T_ZBX_INT, O_OPT, P_ACT,  NOT_EMPTY,		'isset({favobj})'),
	);

	$_REQUEST['source'] = get_request('source', get_profile('web.events.source', 0));

	check_fields($fields);
//SDI($_REQUEST);
/* AJAX */	
	if(isset($_REQUEST['favobj'])){
		if('hat' == $_REQUEST['favobj']){
			update_profile('web.events.hats.'.$_REQUEST['favid'].'.state',$_REQUEST['state'], PROFILE_TYPE_INT);
		}
		else if('filter' == $_REQUEST['favobj']){
			update_profile('web.events.filter.state',$_REQUEST['state'], PROFILE_TYPE_INT);
		}
	}	

	if((PAGE_TYPE_JS == $page['type']) || (PAGE_TYPE_HTML_BLOCK == $page['type'])){
		exit();
	}
//--------

/* FILTER */
	if(isset($_REQUEST['filter_rst'])){
		$_REQUEST['triggerid'] = 0;
		$_REQUEST['show_unknown'] = 0;		
	}
	
	$_REQUEST['triggerid'] = get_request('triggerid',get_profile('web.events.filter.triggerid',0));
	$show_unknown = get_request('show_unknown',get_profile('web.events.filter.show_unknown',0));
	
	if(isset($_REQUEST['filter_set']) || isset($_REQUEST['filter_rst'])){
		update_profile('web.events.filter.triggerid',$_REQUEST['triggerid']);
		update_profile('web.events.filter.show_unknown',$show_unknown, PROFILE_TYPE_INT);		
	}
// --------------

	validate_sort_and_sortorder('e.clock',ZBX_SORT_DOWN);
	
	$source = get_request('source', EVENT_SOURCE_TRIGGERS);
	update_profile('web.events.source',$source, PROFILE_TYPE_INT);
	
?>
<?php

	$_REQUEST['nav_time'] = get_request('nav_time', time());
	$_REQUEST['nav_time']+=(isset($_REQUEST['next']))?86400:0;
	$_REQUEST['nav_time']-=(isset($_REQUEST['back']))?86400:0;
	
?>
<?php
	$source = get_request('source', EVENT_SOURCE_TRIGGERS);

	$r_form = new CForm();
	$r_form->SetMethod('get');	
	$r_form->AddOption('name','events_menu');
	
	$r_form->AddVar('fullscreen',$_REQUEST['fullscreen']);
	$r_form->AddVar('nav_time',$_REQUEST['nav_time']);

	if(EVENT_SOURCE_TRIGGERS == $source){
	
	    $available_groups= get_accessible_groups_by_user($USER_DETAILS,PERM_READ_ONLY);
		$available_hosts = get_accessible_hosts_by_user($USER_DETAILS, PERM_READ_ONLY);
		
		$available_triggers = get_accessible_triggers(PERM_READ_ONLY, PERM_RES_DATA_ARRAY, get_current_nodeid());

		if(isset($_REQUEST['triggerid']) && ($_REQUEST['triggerid']>0)){
			$sql = 'SELECT DISTINCT h.hostid '.
					' FROM hosts h, functions f, items i'.
					' WHERE i.itemid=f.itemid '.
						' AND h.hostid=i.hostid '.
						' AND '.DBcondition('f.triggerid', $available_triggers).
						' AND f.triggerid='.$_REQUEST['triggerid'];
						
			if($host = DBfetch(DBselect($sql,1))){
				$_REQUEST['hostid'] = $host['hostid'];
				
				$sql = 'SELECT DISTINCT hg.groupid '.
						' FROM hosts_groups hg '.
						' WHERE hg.hostid='.$_REQUEST['hostid'].
							' AND '.DBcondition('hg.hostid',$available_hosts);
							
				if($group = DBfetch(DBselect($sql))){
					$_REQUEST['groupid'] = $group['groupid'];
				}
			}
			else{
				unset($_REQUEST['triggerid']);
			}
		}
		
//SDI($_REQUEST['groupid'].' : '.$_REQUEST['hostid']);
		validate_group_with_host(PERM_READ_ONLY, array('allow_all_hosts','monitored_hosts','with_items','always_select_first_host'));

		$cmbGroup = new CComboBox('groupid',$_REQUEST['groupid'],'submit()');
		$cmbHosts = new CComboBox('hostid',$_REQUEST['hostid'],'submit()');

		$cmbGroup->AddItem(0,S_ALL_SMALL);
		
        $result=DBselect('SELECT DISTINCT g.groupid,g.name '.
						' FROM groups g, hosts_groups hg, hosts h, items i '.
	                	' WHERE '.DBcondition('g.groupid',$available_groups).
			                ' AND hg.groupid=g.groupid '.
							' AND h.status='.HOST_STATUS_MONITORED.
			                ' AND h.hostid=i.hostid '.
							' AND hg.hostid=h.hostid '.
							' AND i.status='.ITEM_STATUS_ACTIVE.
		                ' ORDER BY g.name');

		while($row=DBfetch($result)){
			$cmbGroup->AddItem(
					$row['groupid'],
					get_node_name_by_elid($row['groupid']).$row['name']
					);
		}
		$r_form->AddItem(array(S_GROUP.SPACE,$cmbGroup));
		
		$cmbHosts->AddItem(0,S_ALL_SMALL);

		$sql_from = '';
		$sql_where = '';
		if($_REQUEST['groupid'] > 0){
			$sql_from .= ',hosts_groups hg ';
			$sql_where.= ' AND hg.hostid=h.hostid AND hg.groupid='.$_REQUEST['groupid'];
		}
		$sql='SELECT DISTINCT h.hostid,h.host '.
			' FROM hosts h,items i'.$sql_from.
			' WHERE h.status='.HOST_STATUS_MONITORED.
				' AND h.hostid=i.hostid '.
				' AND '.DBcondition('h.hostid',$available_hosts).
				$sql_where.
			' ORDER BY h.host';
			
		$result=DBselect($sql);
		while($row=DBfetch($result)){
			$cmbHosts->AddItem(
					$row['hostid'],
					get_node_name_by_elid($row['hostid']).$row['host']
					);
		}

		$r_form->AddItem(array(SPACE.S_HOST.SPACE,$cmbHosts));
	}
	
	if($allow_discovery){
		$cmbSource = new CComboBox('source', $source, 'submit()');
		$cmbSource->AddItem(EVENT_SOURCE_TRIGGERS, S_TRIGGER);
		$cmbSource->AddItem(EVENT_SOURCE_DISCOVERY, S_DISCOVERY);
		$r_form->AddItem(array(SPACE.S_SOURCE.SPACE, $cmbSource));
	}

// Header	
	$p_elements = array();
	
	$text = array(S_HISTORY_OF_EVENTS_BIG,SPACE,date('[H:i:s]',time()));
	
	$url = '?fullscreen='.($_REQUEST['fullscreen']?'0':'1');

	$fs_icon = new CDiv(SPACE,'fullscreen');
	$fs_icon->AddOption('title',$_REQUEST['fullscreen']?S_NORMAL.' '.S_VIEW:S_FULLSCREEN);
	$fs_icon->AddAction('onclick',new CScript("javascript: document.location = '".$url."';"));
	
	$p_elements[] = get_table_header(S_EVENTS,$r_form);
	
//-------------

// Day View Calc

	$start = $_REQUEST['nav_time'];
	$start  = mktime(0, 0, 0, date('m',$start)  , date('d',$start), date('Y',$start));
	$end = $start+86400;
//SDI('Start: '.date('d m Y',$start));
//SDI('End: '.date('d m Y',$end));
// -------------
	
	if($source == EVENT_SOURCE_DISCOVERY){
		$table = get_history_of_discovery_events($start, $end);
	}
	else{
		$config = select_config();

		$sql_from = $sql_cond = '';

		if($_REQUEST['hostid'] > 0){
			$sql_cond = ' and h.hostid='.$_REQUEST['hostid'];
		}
		else if($_REQUEST['groupid'] > 0){
			$sql_from = ', hosts_groups hg ';
			$sql_cond = ' AND h.hostid=hg.hostid and hg.groupid='.$_REQUEST['groupid'];
		}
		else{
			$sql_from = '';
			$sql_cond = ' AND '.DBcondition('h.hostid',$available_hosts);
		}
		
		$sql_cond.=(isset($_REQUEST['triggerid']) && ($_REQUEST['triggerid']>0))?(' AND t.triggerid='.$_REQUEST['triggerid'].' '):'';

//---
		$triggers = array();
		$trigger_list = array();

		$sql = 'SELECT DISTINCT t.triggerid,t.priority,t.description,t.expression,h.host,t.type '.
				' FROM triggers t, functions f, items i, hosts h '.$sql_from.
				' WHERE '.DBcondition('t.triggerid', $available_triggers).
					' AND t.triggerid=f.triggerid '.
					' AND f.itemid=i.itemid '.
					' AND i.hostid=h.hostid '.
					' AND h.status='.HOST_STATUS_MONITORED.
					$sql_cond;
							
		$rez = DBselect($sql);
		while($rowz = DBfetch($rez)){
			$triggers[$rowz['triggerid']] = $rowz;
			$trigger_list[$rowz['triggerid']] = $rowz['triggerid'];
		}
				
		$sql_cond=($show_unknown == 0)?(' AND e.value<>'.TRIGGER_VALUE_UNKNOWN.' '):('');
		$sql_cond.=' AND e.clock>'.$start;
		$sql_cond.=' AND e.clock<'.$end;

		$table = new CTableInfo(S_NO_EVENTS_FOUND); 
		$table->SetHeader(array(
				make_sorting_link(S_TIME,'e.clock'),
				is_show_subnodes() ? S_NODE : null,
				$_REQUEST['hostid'] == 0 ? S_HOST : null,
				S_DESCRIPTION,
				S_STATUS,
				S_SEVERITY,
				S_DURATION,
				($config['event_ack_enable'])?S_ACK:NULL,
				S_ACTIONS
			));

		if(!empty($triggers)){
			$col=0;
			
			$sql = 'SELECT e.eventid, e.objectid as triggerid, e.clock, e.value, e.acknowledged '.
					' FROM events e '.
					' WHERE '.DBcondition('e.objectid', $trigger_list).
						' AND (e.object+0)='.EVENT_OBJECT_TRIGGER.
						$sql_cond.
					order_by('e.clock');
//SDI($sql);
			$result = DBselect($sql);
			while($row=DBfetch($result)){
				
				$value = new CCol(trigger_value2str($row['value']), get_trigger_value_style($row["value"]));
				
				$row = array_merge($triggers[$row['triggerid']],$row);
				if((0 == $show_unknown) && (!event_initial_time($row,$show_unknown))) continue;
				
				$duration = zbx_date2age($row['clock']);
				if($next_event = get_next_event($row,$show_unknown)){
					$duration = zbx_date2age($row['clock'],$next_event['clock']);
				}
// Actions								
				$actions= get_event_actions_status($row['eventid']);
//--------		
	
				if($config['event_ack_enable']){
					if($row['acknowledged'] == 1){
						$ack=new CLink(S_YES,'acknow.php?eventid='.$row['eventid'],'action');
					}
					else{
						$ack= new CLink(S_NO,'acknow.php?eventid='.$row['eventid'],'on');
					}
				}
	
				$table->AddRow(array(
					date("Y.M.d H:i:s",$row["clock"]),
					is_show_subnodes() ? get_node_name_by_elid($row['triggerid']) : null,
					$_REQUEST["hostid"] == 0 ? $row['host'] : null,
					new CLink(
						expand_trigger_description_by_data($row, ZBX_FLAG_EVENT),
						"tr_events.php?triggerid=".$row["triggerid"].'&eventid='.$row['eventid'],
						"action"
						),
					$value,
					new CCol(get_severity_description($row["priority"]), get_severity_style($row["priority"],$row['value'])),
					$duration,
					($config['event_ack_enable'])?$ack:NULL,
					$actions
				));
					
				$col++;
			}
		}
	}


/************************* FILTER **************************/
/***********************************************************/

// Navigation
	$nav_time = get_request('nav_time',time());
	
	$navForm = new CForm('events.php');
	$navForm->AddVar('groupid',$_REQUEST['groupid']);
	$navForm->AddVar('hostid',$_REQUEST['hostid']);
	$navForm->AddVar('fullscreen',$_REQUEST['fullscreen']);
//	$navForm->AddVar('nav_time',$_REQUEST['nav_time']);
	
	$back = new CButton('back','« '.S_DAY);
	$next = new CButton('next',S_DAY.' »');
	if(($nav_time+86400) > time()) $next->AddOption('disabled','disabled');
	
	$script = new CScript("javascript: if(CLNDR['nav_time'].clndr.setSDateFromOuterObj()){".
							"this.action += '?nav_time='+parseInt(CLNDR['nav_time'].clndr.sdt.getTime()/1000);}"
							);
	$navForm->AddAction('onsubmit',$script);
	
	$clndr_icon = new CImg('images/general/bar/cal.gif','calendar', 16, 12, 'pointer');
	$clndr_icon->AddAction('onclick',"javascript: var pos = getPosition(this); pos.top+=14; pos.left-=174; CLNDR['nav_time'].clndr.clndrshow(pos.top,pos.left);");
	$clndr_icon->AddOption('style','vertical-align: bottom;');
	
	$nav_clndr =  array(
					new CNumericBox('nav_day',(($_REQUEST['nav_time']>0)?date('d',$_REQUEST['nav_time']):''),2),
					new CNumericBox('nav_month',(($_REQUEST['nav_time']>0)?date('m',$_REQUEST['nav_time']):''),2),
					new CNumericBox('nav_year',(($_REQUEST['nav_time']>0)?date('Y',$_REQUEST['nav_time']):''),4),
					$clndr_icon
				);
	zbx_add_post_js('create_calendar(null,["nav_day","nav_month","nav_year"],"nav_time");');

	$navForm->AddItem(array($back,SPACE,$next,new CSpan(' | ','divider'), $nav_clndr, SPACE, new CButton('load',S_SHOW.' »')));

	$navigation = $navForm;
//------------

	$filterForm = new CFormTable(S_FILTER);//,'events.php?filter_set=1','POST',null,'sform');
	$filterForm->AddOption('name','zbx_filter');
	$filterForm->AddOption('id','zbx_filter');
	$filterForm->SetMethod('get');
	
	$filterForm->AddVar('nav_time',$_REQUEST['nav_time']);

	if(EVENT_SOURCE_TRIGGERS == $source){
	
		$script = new CScript("javascript: if(CLNDR['events_since'].clndr.setSDateFromOuterObj()){". 
								"$('filter_timesince').value = parseInt(CLNDR['events_since'].clndr.sdt.getTime()/1000);}".
							"if(CLNDR['events_till'].clndr.setSDateFromOuterObj()){". 
								"$('filter_timetill').value = parseInt(CLNDR['events_till'].clndr.sdt.getTime()/1000);}"
							);
		$filterForm->AddAction('onsubmit',$script);
		
		$filterForm->AddVar('triggerid',$_REQUEST['triggerid']);
		
		if(isset($_REQUEST['triggerid']) && ($_REQUEST['triggerid']>0)){
			$trigger = expand_trigger_description($_REQUEST['triggerid']);
		} 
		else{
			$trigger = "";
		}
		$row = new CRow(array(
						new CCol(S_TRIGGER,'form_row_l'),
						new CCol(array(
									new CTextBox("trigger",$trigger,96,'yes'),
									new CButton("btn1",S_SELECT,"return PopUp('popup.php?"."dstfrm=".$filterForm->GetName()."&dstfld1=triggerid&dstfld2=trigger"."&srctbl=triggers&srcfld1=triggerid&srcfld2=description&real_hosts=1');",'T')
								),'form_row_r')
							));
							
		$filterForm->AddRow($row);

		$filterForm->AddVar('show_unknown',$show_unknown);
		
		$unkcbx = new CCheckBox('show_unk',$show_unknown,null,'1');
		$unkcbx->SetAction('javascript: create_var("'.$filterForm->GetName().'", "show_unknown", (this.checked?1:0), 0); ');
		
		$filterForm->AddRow(S_SHOW_UNKNOWN,$unkcbx);

		$reset = new CButton("filter_rst",S_RESET);
		$reset->SetType('button');
		$reset->SetAction('javascript: var uri = new url(location.href); uri.setArgument("filter_rst",1); location.href = uri.getUrl();');

		$filterForm->AddItemToBottomRow(new CButton("filter_set",S_FILTER));
		$filterForm->AddItemToBottomRow($reset);

		$p_elements[] = create_filter(S_FILTER,$navigation,$filterForm,'tr_filter',get_profile('web.events.filter.state',0));
	}
	else{
		$p_elements[] = get_thin_table_header(SPACE,$navigation);
	}
//-------

	$p_elements[] = $table;
	$p_elements[] = get_thin_table_header(SPACE,$navigation);
	
	$events_hat = create_hat(
			$text,
			$p_elements,
			$fs_icon,
			'hat_events',
			get_profile('web.events.hats.hat_events.state',1)
	);

	$events_hat->Show();
?>
<?php

include_once "include/page_footer.php";

?>
