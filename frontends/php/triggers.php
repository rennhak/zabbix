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
	require_once "include/triggers.inc.php";
	require_once "include/forms.inc.php";

	$page["title"] = "S_CONFIGURATION_OF_TRIGGERS";
	$page["file"] = "triggers.php";

	show_header($page["title"],0,0);
	insert_confirm_javascript();
?>
<?php

//		VAR			TYPE	OPTIONAL FLAGS	VALIDATION	EXCEPTION
	$fields=array(
		"groupid"=>	array(T_ZBX_INT, O_OPT,	 P_SYS,	DB_ID,NULL),
		"hostid"=>	array(T_ZBX_INT, O_OPT,  P_SYS,	DB_ID,'isset({save})'),

		"triggerid"=>	array(T_ZBX_INT, O_OPT,  P_SYS,	DB_ID,'{form}=="update"'),

		"copy_type"	=>array(T_ZBX_INT, O_OPT,	 P_SYS,	IN("0,1"),'isset({copy})'),
		"copy_mode"	=>array(T_ZBX_INT, O_OPT,	 P_SYS,	IN("0"),NULL),

		"description"=>	array(T_ZBX_STR, O_OPT,  NULL,	NOT_EMPTY,'isset({save})'),
		"expression"=>	array(T_ZBX_STR, O_OPT,  NULL,	NOT_EMPTY,'isset({save})'),
		"priority"=>	array(T_ZBX_INT, O_OPT,  NULL,  IN("0,1,2,3,4,5"),'isset({save})'),
		"comments"=>	array(T_ZBX_STR, O_OPT,  NULL,	NULL,'isset({save})'),
		"url"=>		array(T_ZBX_STR, O_OPT,  NULL,	NULL,'isset({save})'),
		"status"=>	array(T_ZBX_STR, O_OPT,  NULL,	NULL,NULL),

		"dependences"=>		array(T_ZBX_INT, O_OPT,  NULL,	DB_ID, NULL),
		"new_dependence"=>	array(T_ZBX_STR, O_OPT,  NULL,	NOT_EMPTY,'isset({add_dependence})'),
		"rem_dependence"=>	array(T_ZBX_INT, O_OPT,  NULL,	DB_ID, NULL),

		"g_triggerid"=>	array(T_ZBX_INT, O_OPT,  NULL,	DB_ID, NULL),
		"copy_targetid"=>	array(T_ZBX_INT, O_OPT,	NULL,	DB_ID, NULL),
		"filter_groupid"=>	array(T_ZBX_INT, O_OPT, P_SYS,	DB_ID, 'isset({copy})&&{copy_type}==0'),

/* actions */
		"add_dependence"=>	array(T_ZBX_STR, O_OPT, P_SYS|P_ACT,	NULL,	NULL),
		"del_dependence"=>	array(T_ZBX_STR, O_OPT, P_SYS|P_ACT,	NULL,	NULL),
		"group_enable"=>	array(T_ZBX_STR, O_OPT, P_SYS|P_ACT,	NULL,	NULL),
		"group_disable"=>	array(T_ZBX_STR, O_OPT, P_SYS|P_ACT,	NULL,	NULL),
		"group_delete"=>	array(T_ZBX_STR, O_OPT, P_SYS|P_ACT,	NULL,	NULL),
		"copy"=>		array(T_ZBX_STR, O_OPT, P_SYS|P_ACT,	NULL,	NULL),
		"save"=>		array(T_ZBX_STR, O_OPT, P_SYS|P_ACT,	NULL,	NULL),
		"delete"=>		array(T_ZBX_STR, O_OPT, P_SYS|P_ACT,	NULL,	NULL),
		"cancel"=>		array(T_ZBX_STR, O_OPT, P_SYS,	NULL,	NULL),
/* other */
		"form"=>		array(T_ZBX_STR, O_OPT, P_SYS,	NULL,	NULL),
		"form_copy_to"=>	array(T_ZBX_STR, O_OPT, P_SYS,	NULL,	NULL),
		"form_refresh"=>	array(T_ZBX_INT, O_OPT,	NULL,	NULL,	NULL)
	);

	check_fields($fields);

	validate_group_with_host(PERM_READ_WRITE,array("allow_all_hosts","with_items"));
?>
<?php

/* FORM ACTIONS */
	if(isset($_REQUEST["save"]))
	{
		$now=mktime();
		if(isset($_REQUEST["status"]))	{ $status=1; }
		else			{ $status=0; }

		$deps = get_request("dependences",array());

		if(isset($_REQUEST["triggerid"])){

			$result=update_trigger($_REQUEST["triggerid"],
				$_REQUEST["expression"],$_REQUEST["description"],
				$_REQUEST["priority"],$status,$_REQUEST["comments"],$_REQUEST["url"],
				$deps);

			$triggerid = $_REQUEST["triggerid"];
			show_messages($result, S_TRIGGER_UPDATED, S_CANNOT_UPDATE_TRIGGER);
		} else {
			$triggerid=add_trigger($_REQUEST["expression"],$_REQUEST["description"],
				$_REQUEST["priority"],$status,$_REQUEST["comments"],$_REQUEST["url"],
				$deps);

			$result = $triggerid;

			show_messages($triggerid, S_TRIGGER_ADDED, S_CANNOT_ADD_TRIGGER);
		}

		if($result)
		{
			unset($_REQUEST["form"]);
		}
	}
	elseif(isset($_REQUEST["delete"])&&isset($_REQUEST["triggerid"]))
	{
		$result=delete_trigger($_REQUEST["triggerid"]);
		show_messages($result, S_TRIGGER_DELETED, S_CANNOT_DELETE_TRIGGER);
		if($result){
			unset($_REQUEST["form"]);
			unset($_REQUEST["triggerid"]);
		}
	}
	elseif(isset($_REQUEST["copy"])&&isset($_REQUEST["g_triggerid"])&&isset($_REQUEST["form_copy_to"]))
	{
		if(isset($_REQUEST['copy_targetid']) && $_REQUEST['copy_targetid'] > 0 && isset($_REQUEST['copy_type']))
		{
			if(0 == $_REQUEST['copy_type'])
			{ /* hosts */
				$hosts_ids = $_REQUEST['copy_targetid'];
			}
			else
			{ /* groups */
				$hosts_ids = array();
				$group_ids = "";
				foreach($_REQUEST['copy_targetid'] as $group_id)
				{
					$group_ids .= $group_id.',';
				}
				$group_ids = trim($group_ids,',');

				$db_hosts = DBselect('select distinct h.hostid from hosts h, hosts_groups hg'.
					' where h.hostid=hg.hostid and hg.groupid in ('.$group_ids.')');
				while($db_host = DBfetch($db_hosts))
				{
					array_push($hosts_ids, $db_host['hostid']);
				}
			}

			foreach($_REQUEST["g_triggerid"] as $trigger_id)
				foreach($hosts_ids as $host_id)
				{
					copy_trigger_to_host($trigger_id, $host_id, true);
				}
			unset($_REQUEST["form_copy_to"]);
		}
		else
		{
			error('No target selection.');
		}
		show_messages();
	}
/* DEPENDENCE ACTIONS */
	elseif(isset($_REQUEST["add_dependence"])&&isset($_REQUEST["new_dependence"]))
	{
		if(!isset($_REQUEST["dependences"]))
			$_REQUEST["dependences"] = array();

		if(!in_array($_REQUEST["new_dependence"], $_REQUEST["dependences"]))
			array_push($_REQUEST["dependences"], $_REQUEST["new_dependence"]);
	}
	elseif(isset($_REQUEST["del_dependence"])&&isset($_REQUEST["rem_dependence"]))
	{
		if(isset($_REQUEST["dependences"])){
			foreach($_REQUEST["dependences"]as $key => $val)
			{
				if(!in_array($val, $_REQUEST["rem_dependence"]))	continue;
				unset($_REQUEST["dependences"][$key]);
			}
		}
	}
/* GROUP ACTIONS */
	elseif(isset($_REQUEST["group_enable"])&&isset($_REQUEST["g_triggerid"]))
	{
		foreach($_REQUEST["g_triggerid"] as $triggerid)
		{
			$result=DBselect("select triggerid from triggers t where t.triggerid=".zbx_dbstr($triggerid));
			if(!($row = DBfetch($result))) continue;
			$result2=update_trigger_status($row["triggerid"],0);
		}
		show_messages(true, S_STATUS_UPDATED, S_CANNOT_UPDATE_STATUS);
	}
	elseif(isset($_REQUEST["group_disable"])&&isset($_REQUEST["g_triggerid"]))
	{
		foreach($_REQUEST["g_triggerid"] as $triggerid)
		{
			$result=DBselect("select triggerid from triggers t where t.triggerid=".zbx_dbstr($triggerid));
			if(!($row = DBfetch($result))) continue;
			$result2=update_trigger_status($row["triggerid"],1);
		}
		show_messages(true, S_STATUS_UPDATED, S_CANNOT_UPDATE_STATUS);
	}
	elseif(isset($_REQUEST["group_delete"])&&isset($_REQUEST["g_triggerid"]))
	{
		foreach($_REQUEST["g_triggerid"] as $triggerid)
		{
			$result=DBselect("select triggerid,templateid from triggers t where t.triggerid=".zbx_dbstr($triggerid));
			if(!($row = DBfetch($result))) continue;
			if($row["templateid"] <> 0)	continue;
			$del_res = delete_trigger($row["triggerid"]);
		}
		if(isset($del_res))
			show_messages(TRUE, S_TRIGGERS_DELETED, S_CANNOT_DELETE_TRIGGERS);
	}
?>

<?php
?>

<?php

	$form = new CForm();

	$form->AddVar("hostid",$_REQUEST["hostid"]);
	$form->AddItem(new CButton("form",S_CREATE_TRIGGER));

	show_header2(S_CONFIGURATION_OF_TRIGGERS_BIG, $form);
	echo BR;

	if(isset($_REQUEST["form_copy_to"]) && isset($_REQUEST["g_triggerid"]))
	{
		insert_copy_elements_to_forms("g_triggerid");
	}
	else if(!isset($_REQUEST["form"]))
	{
/* filter panel */
		$form = new CForm();

		$_REQUEST["groupid"] = get_request("groupid",0);
		$cmbGroup = new CComboBox("groupid",$_REQUEST["groupid"],"submit();");
		$cmbGroup->AddItem(0,S_ALL_SMALL);
		$result=DBselect("select groupid,name from groups where mod(groupid,100)=$ZBX_CURNODEID order by name");
		while($row=DBfetch($result))
		{
	// Check if at least one host with read permission exists for this group
			$result2=DBselect("select distinct h.hostid,h.host from hosts h,hosts_groups hg,items i".
				" where hg.groupid=".$row["groupid"]." and hg.hostid=h.hostid and i.hostid=h.hostid".
				" and h.status<>".HOST_STATUS_DELETED." order by h.host");
			while($row2=DBfetch($result2))
			{
//				if(!check_right("Host","U",$row2["hostid"]))	continue; /* TODO */
				$cmbGroup->AddItem($row["groupid"],$row["name"]);
				break;
			}
		}
		$form->AddItem(S_GROUP.SPACE);
		$form->AddItem($cmbGroup);

		if(isset($_REQUEST["groupid"]) && $_REQUEST["groupid"]>0)
		{
			$sql="select distinct h.hostid,h.host from hosts h,hosts_groups hg,items i".
				" where hg.groupid=".$_REQUEST["groupid"]." and hg.hostid=h.hostid and i.hostid=h.hostid".
				" and h.status<>".HOST_STATUS_DELETED." order by h.host";
		}
		else
		{
			$sql="select h.hostid,h.host from hosts h,items i where i.hostid=h.hostid and h.status<>".HOST_STATUS_DELETED.
				" and mod(h.hostid,100)=$ZBX_CURNODEID".
				" group by h.hostid,h.host order by h.host";
		}

		$result=DBselect($sql);

		$_REQUEST["hostid"] = get_request("hostid",0);
		$cmbHosts = new CComboBox("hostid",$_REQUEST["hostid"],"submit();");
		if($_REQUEST["groupid"]==0) $cmbHosts->AddItem(0,S_ALL_SMALL);

		$correct_hostid='no';
		$first_hostid = -1;
		while($row=DBfetch($result))
		{
//			if(!check_right("Host","U",$row["hostid"]))	continue; /* TODO */
			$cmbHosts->AddItem($row["hostid"],$row["host"]);

			if($_REQUEST["hostid"]!=0){
				if($_REQUEST["hostid"]==$row["hostid"])
					$correct_hostid = 'ok';
			}
			if($first_hostid <= 0)
				$first_hostid = $row["hostid"];
		}
		if($correct_hostid!='ok')
			if($_REQUEST["groupid"]==0)
				$_REQUEST["hostid"] = 0;
			else
				$_REQUEST["hostid"] = $first_hostid;

		$form->AddItem(SPACE.S_HOST.SPACE);
		$form->AddItem($cmbHosts);

		show_header2(S_TRIGGERS_BIG, $form);

/* TABLE */
		$form = new CForm('triggers.php');
		$form->SetName('triggers');
		$form->AddVar('hostid',$_REQUEST["hostid"]);

		$table = new CTableInfo(S_NO_TRIGGERS_DEFINED);
		$table->setHeader(array(
			$_REQUEST["hostid"] > 0 ? NULL : S_HOST,
			array(	new CCheckBox("all_triggers",NULL,
					"CheckAll('".$form->GetName()."','all_triggers');")
				,S_NAME
			),
			S_EXPRESSION, S_SEVERITY, S_STATUS, S_ERROR));

		$sql = "select distinct h.hostid,h.host,t.*".
			" from triggers t,hosts h,items i,functions f".
			" where f.itemid=i.itemid and h.hostid=i.hostid and t.triggerid=f.triggerid".
			" and mod(h.hostid,100)=$ZBX_CURNODEID";

		if($_REQUEST["hostid"] > 0) 
			$sql .= " and h.hostid=".$_REQUEST["hostid"];

		$sql .= " order by h.host,t.description";

		$result=DBselect($sql);
		while($row=DBfetch($result))
		{
			if(check_right_on_trigger(PERM_READ_ONLY,$row["triggerid"]) == 0)
			{
				continue;
			}

			$chkBox =  new CCheckBox(
                                        "g_triggerid[]",        /* name */
                                        NULL,                   /* checked */
                                        NULL,                   /* action */
                                        $row["triggerid"]);     /* value */

			if($row["templateid"] > 0) $chkBox->SetEnabled(false);
			$description = array($chkBox,SPACE);

			if($row["templateid"] == 0)
			{
				array_push($description,
					new CLink(expand_trigger_description($row["triggerid"]),
					"triggers.php?form=update&triggerid=".$row["triggerid"].
						"&hostid=".$row["hostid"], 'action')
					);
			} else {
				$real_hosts = get_realhosts_by_triggerid($row["triggerid"]);
				$real_host = DBfetch($real_hosts);
				if($real_host)
				{
					array_push($description,
						new CLink($real_host["host"],
							"triggers.php?&hostid=".$real_host["hostid"], 'action'),
						":",
						expand_trigger_description($row["triggerid"])
						);
				}
				else
				{
					array_push($description,
						new CSpan("error","on"),
						":",
						expand_trigger_description($row["triggerid"])
						);
				}
			}

			//add dependences
			$result1=DBselect("select t.triggerid,t.description from triggers t,trigger_depends d".
				" where t.triggerid=d.triggerid_up and d.triggerid_down=".$row["triggerid"]);
			if($row1=DBfetch($result1))
			{
				array_push($description,BR.BR."<strong>".S_DEPENDS_ON."</strong>".SPACE.BR);
				do
				{
					array_push($description,expand_trigger_description($row1["triggerid"]).BR);
				} while($row1=DBfetch($result1));
				array_push($description,BR);
			}
	
			if($row["priority"]==0)		$priority=S_NOT_CLASSIFIED;
			elseif($row["priority"]==1)	$priority=new CCol(S_INFORMATION,"information");
			elseif($row["priority"]==2)	$priority=new CCol(S_WARNING,"warning");
			elseif($row["priority"]==3)	$priority=new CCol(S_AVERAGE,"average");
			elseif($row["priority"]==4)	$priority=new CCol(S_HIGH,"high");
			elseif($row["priority"]==5)	$priority=new CCol(S_DISASTER,"disaster");
			else				$priority=$row["priority"];

			if($row["status"] == TRIGGER_STATUS_DISABLED)
			{
				$status= new CLink(S_DISABLED,
					"triggers.php?group_enable=1&g_triggerid%5B%5D=".$row["triggerid"].
						"&hostid=".$row["hostid"],
					'disabled');
			}
			else if($row["status"] == TRIGGER_STATUS_UNKNOWN)
			{
				$status= new CLink(S_UNCNOWN,
					"triggers.php?group_disable=1&g_triggerid%5B%5D=".$row["triggerid"].
						"&hostid=".$row["hostid"],
					'uncnown');
			}
			else if($row["status"] == TRIGGER_STATUS_ENABLED)
			{
				$status= new CLink(S_ENABLED,
					"triggers.php?group_disable=1&g_triggerid%5B%5D=".$row["triggerid"].
						"&hostid=".$row["hostid"],
					'enabled');
			}

			if($row["status"] != TRIGGER_STATUS_UNKNOWN)	$row["error"]=SPACE;

			if($row["error"]=="")		$row["error"]=SPACE;

			$table->addRow(array(
				$_REQUEST["hostid"] > 0 ? NULL : $row["host"],
				$description,
				explode_exp($row["expression"],1),
				$priority,
				$status,
				$row["error"]
			));
		}
		
		$footerButtons = array();
		array_push($footerButtons, new CButton('group_enable','Enable selected',
			"return Confirm('".S_ENABLE_SELECTED_TRIGGERS_Q."');"));
		array_push($footerButtons, SPACE);
		array_push($footerButtons, new CButton('group_disable','Disable selected',
			"return Confirm('".S_DISABLE_SELECTED_TRIGGERS_Q."');"));
		array_push($footerButtons, SPACE);
		array_push($footerButtons, new CButton('group_delete','Delete selected',
			"return Confirm('".S_DELETE_SELECTED_TRIGGERS_Q."');"));
		array_push($footerButtons, SPACE);
		array_push($footerButtons, new CButton('form_copy_to','Copy selected to ...'));
		$table->SetFooter(new CCol($footerButtons));

		$form->AddItem($table);
		$form->Show();
	}
	else
	{
/* FORM */
		$result=DBselect("select count(*) as cnt from hosts where mod(hostid,100)=$ZBX_CURNODEID");
		$row=DBfetch($result);
		if($row["cnt"]>0)
		{
			insert_trigger_form();
		} 
	}
?>

<?php
	show_page_footer();
?>
